//(c)2003 sisoft\trg - AYplayer.
/* $Id: sndemu.c,v 1.3 2003/11/05 12:41:23 root Exp $ */

//base version of this file was taken from aylet-0.3 by Russell Marks.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "z80.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifndef LPT_PORT
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#define BASE_SOUND_FRAG_PWR	4
#define AY_CLOCK		1773400
#define AMPL_AY_TONE		28
#define AY_CHANGE_MAX		8000
#define STEREO_BUF_SIZE 1024

int soundfd=-1;
int sixteenbit=1;
int sound_freq=44100;
int sound_stereo=1;
int sound_stereo_narrow=0;

static int sound_framesiz;
static unsigned char ay_tone_levels[16];
static unsigned char *sound_buf;
static unsigned char *sound_ptr;
static int sound_oldpos,sound_fillpos,sound_oldval,sound_oldval_orig;
static unsigned int ay_tone_tick[3],ay_tone_high[3],ay_noise_tick;
static unsigned int ay_tone_subcycles,ay_env_subcycles;
static unsigned int ay_env_internal_tick,ay_env_tick;
static unsigned int ay_tick_incr;
static unsigned int ay_tone_period[3],ay_noise_period,ay_env_period;
static unsigned char sound_ay_registers[16];

static struct ay_change_tag {
    unsigned char reg,val;
};

static struct ay_change_tag ay_change[AY_CHANGE_MAX];
static int ay_change_count;
static int rstereobuf_l[STEREO_BUF_SIZE],rstereobuf_r[STEREO_BUF_SIZE];
static int rstereopos,rchan1pos,rchan2pos,rchan3pos;


#ifdef UNIX
static int driver_init(int *freqptr,int *stereoptr)
{
    int frag,tmp;
    if((soundfd=open("/dev/dsp",O_WRONLY))<0)return(0);
    tmp=sixteenbit?AFMT_S16_LE:AFMT_U8;
    if(ioctl(soundfd,SNDCTL_DSP_SETFMT,&tmp)==-1) {
        close(soundfd);
        return(0);
    }
    tmp=(*stereoptr)?1:0;
    if(ioctl(soundfd,SNDCTL_DSP_STEREO,&tmp)<0) {
        close(soundfd);
        return(0);
    }
    frag=(0x40000|BASE_SOUND_FRAG_PWR);
    if(ioctl(soundfd,SNDCTL_DSP_SPEED,freqptr)<0) {
        close(soundfd);
        return(0);
    }
    if(*freqptr>8250) frag++;
    if(*freqptr>16500) frag++;
    if(*freqptr>33000) frag++;
    if(*stereoptr) frag++;
    if(sixteenbit) frag++;
    if(ioctl(soundfd,SNDCTL_DSP_SETFRAGMENT,&frag)<0) {
        close(soundfd);
        return(0);
    }
    return(1);
}

static void driver_end()
{
    if(soundfd>1)close(soundfd);
}

static void driver_frame(unsigned char *data,int len)
{
    static unsigned char buf16[8192];
    int ret=0,ofs=0;
    if(sixteenbit) {
        unsigned char *src,*dst;
        int f;
        src=data; dst=buf16;
        for(f=0;f<len;f++) {
            *dst++=128;
            *dst++=*src++-128;
        }
        data=buf16;
        len<<=1;
    }
    while(len) {
        ret=write(soundfd,data+ofs,len);
        if(ret>0)ofs+=ret,len-=ret;
    }
}

#endif

#define CLOCK_RESET(clock) ay_tick_incr=(int)(65536.*clock/sound_freq)

static void sound_ay_init()
{
    int f;
    static int levels[16]= {
            0x0000, 0x0385, 0x053D, 0x0770,
            0x0AD7, 0x0FD5, 0x15B0, 0x230C,
            0x2B4C, 0x43C1, 0x5A4B, 0x732F,
            0x9204, 0xAFF1, 0xD921, 0xFFFF
    };
    for(f=0;f<16;f++)ay_tone_levels[f]=(levels[f]*AMPL_AY_TONE+0x8000)/0xffff;
    ay_noise_tick=ay_noise_period=0;
    ay_env_internal_tick=ay_env_tick=ay_env_period=0;
    ay_tone_subcycles=ay_env_subcycles=0;
    for(f=0;f<3;f++)ay_tone_tick[f]=ay_tone_high[f]=0,ay_tone_period[f]=1;
    CLOCK_RESET(AY_CLOCK);
    ay_change_count=0;
}

int sound_init()
{
    int f;
    if(!driver_init(&sound_freq,&sound_stereo))return(0);
    sound_framesiz=sound_freq/50;
    if((sound_buf=malloc(sound_framesiz*(sound_stereo+1)))==NULL) {
        sound_end();
        return(0);
    }
    sound_oldval=sound_oldval_orig=128;
    sound_oldpos=-1;
    sound_fillpos=0;
    sound_ptr=sound_buf;
    sound_ay_init();
    if(sound_stereo) {
        int pos=(sound_stereo_narrow?3:6)*sound_freq/8000;
        for(f=0;f<STEREO_BUF_SIZE;f++)rstereobuf_l[f]=rstereobuf_r[f]=0;
        rstereopos=0;
        rchan1pos=-pos;
        rchan2pos=pos,rchan3pos=0;
    }
    return(1);
}

void sound_end()
{
    if(sound_buf)free(sound_buf);
    driver_end();
}

#define AY_GET_SUBVAL(chan) (level*2*ay_tone_tick[chan]/tone_count)

#define AY_DO_TONE(var,chan) \
  is_low=0;								\
  if(is_on) {								\
    (var)=0;								\
    if(level) {								\
      if(ay_tone_high[chan]) (var)= (level);				\
      else (var)=-(level),is_low=1;					\
    }									\
  }									\
  ay_tone_tick[chan]+=tone_count;					\
  count=0;								\
  while(ay_tone_tick[chan]>=ay_tone_period[chan]) {			\
    count++;								\
    ay_tone_tick[chan]-=ay_tone_period[chan];				\
    ay_tone_high[chan]=!ay_tone_high[chan];				\
    if(is_on && count==1 && level && ay_tone_tick[chan]<tone_count) {	\
      if(is_low) (var)+=AY_GET_SUBVAL(chan);				\
      else (var)-=AY_GET_SUBVAL(chan);					\
    }									\
  }									\
  if(is_on && count>1) (var)=-(level)

#define GEN_STEREO(pos,val) \
  if((pos)<0) {							\
    rstereobuf_l[rstereopos]+=(val);				\
    rstereobuf_r[(rstereopos-pos)%STEREO_BUF_SIZE]+=(val);	\
  } else {							\
    rstereobuf_l[(rstereopos+pos)%STEREO_BUF_SIZE]+=(val);	\
    rstereobuf_r[rstereopos]+=(val);				\
  }

#define AY_ENV_CONT	8
#define AY_ENV_ATTACK	4
#define AY_ENV_ALT	2
#define AY_ENV_HOLD	1

static void sound_ay_overlay()
{
    static int rng=1;
    static int noise_toggle=0;
    static int env_first=1,env_rev=0,env_counter=15;
    int tone_level[3];
    int mixer,envshape;
    int f,g,level,count;
    unsigned char *ptr;
    struct ay_change_tag *change_ptr=ay_change;
    int changes_left=ay_change_count;
    int reg,r;
    int is_low,is_on;
    int chan1,chan2,chan3;
    unsigned int tone_count,noise_count;
    for(f=0,ptr=sound_buf;f<sound_framesiz;f++) {
        while(changes_left /*&& f>=change_ptr->ofs*/) {
            sound_ay_registers[reg=change_ptr->reg]=change_ptr->val;
            change_ptr++; changes_left--;
            switch(reg)
            {
	    case 0: case 1: case 2: case 3: case 4: case 5:
                r=reg>>1;
                ay_tone_period[r]=(sound_ay_registers[reg&~1]|(sound_ay_registers[reg|1]&15)<<8);
                if(!ay_tone_period[r])ay_tone_period[r]++;
                if(ay_tone_tick[r]>=ay_tone_period[r]*2)ay_tone_tick[r]%=ay_tone_period[r]*2;
                break;
            case 6:
                ay_noise_tick=0;
                ay_noise_period=(sound_ay_registers[reg]&31);
                break;
	    case 11: case 12:
                ay_env_period=sound_ay_registers[11]|(sound_ay_registers[12]<<8);
                break;
            case 13:
                ay_env_internal_tick=ay_env_tick=ay_env_subcycles=0;
                env_first=1;
                env_rev=0;
                env_counter=(sound_ay_registers[13]&AY_ENV_ATTACK)?0:15;
                break;
            }
        }
        for(g=0;g<3;g++)tone_level[g]=ay_tone_levels[sound_ay_registers[8+g]&15];
        envshape=sound_ay_registers[13];
        level=ay_tone_levels[env_counter];
        for(g=0;g<3;g++)if(sound_ay_registers[8+g]&16)tone_level[g]=level;
        ay_env_subcycles+=ay_tick_incr;
        noise_count=0;
        while(ay_env_subcycles>=(16<<16)) {
            ay_env_subcycles-=(16<<16);
            noise_count++;
            ay_env_tick++;
            while(ay_env_tick>=ay_env_period) {
                ay_env_tick-=ay_env_period;
                if(env_first||((envshape&AY_ENV_CONT) && !(envshape&AY_ENV_HOLD))) {
                    if(env_rev)env_counter-=(envshape&AY_ENV_ATTACK)?1:-1;
                	else env_counter+=(envshape&AY_ENV_ATTACK)?1:-1;
                    if(env_counter<0) env_counter=0;
                    if(env_counter>15) env_counter=15;
                }
                ay_env_internal_tick++;
                while(ay_env_internal_tick>=16) {
                    ay_env_internal_tick-=16;
                    if(!(envshape&AY_ENV_CONT))env_counter=0;
                    else {
                        if(envshape&AY_ENV_HOLD) {
                            if(env_first && (envshape&AY_ENV_ALT))
                                env_counter=(env_counter?0:15);
                        } else {
                            if(envshape&AY_ENV_ALT)env_rev=!env_rev;
                            else env_counter=(envshape&AY_ENV_ATTACK)?0:15;
                        }
                    }
                    env_first=0;
                }
                if(!ay_env_period) break;
            }
        }
        chan1=tone_level[0];
        chan2=tone_level[1];
        chan3=tone_level[2];
        mixer=sound_ay_registers[7];
        ay_tone_subcycles+=ay_tick_incr;
        tone_count=ay_tone_subcycles>>(3+16);
        ay_tone_subcycles&=(8<<16)-1;
        level=chan1; is_on=!(mixer&1);
        AY_DO_TONE(chan1,0);
        if((mixer&0x08)==0 && noise_toggle)chan1=0;
        level=chan2; is_on=!(mixer&2);
        AY_DO_TONE(chan2,1);
        if((mixer&0x10)==0 && noise_toggle)chan2=0;
        level=chan3; is_on=!(mixer&4);
        AY_DO_TONE(chan3,2);
        if((mixer&0x20)==0 && noise_toggle)chan3=0;
        if(!sound_stereo) {
            (*ptr++)+=chan1+chan2+chan3;
        } else {
            GEN_STEREO(rchan1pos,chan1);
            GEN_STEREO(rchan2pos,chan2);
            GEN_STEREO(rchan3pos,chan3);
            (*ptr++)+=rstereobuf_l[rstereopos];
            (*ptr++)+=rstereobuf_r[rstereopos];
            rstereobuf_l[rstereopos]=rstereobuf_r[rstereopos]=0;
            rstereopos++;
            if(rstereopos>=STEREO_BUF_SIZE)rstereopos=0;
        }
        ay_noise_tick+=noise_count;
        while(ay_noise_tick>=ay_noise_period) {
            ay_noise_tick-=ay_noise_period;
            if((rng&1)^((rng&2)?1:0))noise_toggle=!noise_toggle;
            rng|=((rng&1)^((rng&4)?1:0))?0x20000:0;
            rng>>=1;
            if(!ay_noise_period) break;
        }
    }
}

void sound_ay_write(int reg,int val)
{
    if(reg>=15) return;
    if(ay_change_count<AY_CHANGE_MAX) {
        ay_change[ay_change_count].reg=reg;
        ay_change[ay_change_count].val=val;
        ay_change_count++;
    }
}

void sound_ay_reset()
{
    int f;
    ay_change_count=0;
    for(f=0;f<16;f++)sound_ay_write(f,0);
    for(f=0;f<3;f++)ay_tone_high[f]=0;
    ay_tone_subcycles=ay_env_subcycles=0;
    sound_oldval=sound_oldval_orig=128;
    CLOCK_RESET(AY_CLOCK);
}

int sound_frame(int really_play)
{
    unsigned char *ptr;
    int fulllen=sound_framesiz*(sound_stereo+1),f;
    ptr=sound_buf+(sound_stereo?sound_fillpos*2:sound_fillpos);
    for(f=sound_fillpos;f<sound_framesiz;f++) {
        *(ptr)++=(sound_oldval);
        if(sound_stereo)*(ptr)++=(sound_oldval);
    }
    sound_ay_overlay();
    if(really_play)driver_frame(sound_buf,fulllen);
    sound_oldpos=-1;
    sound_fillpos=0;
    sound_ptr=sound_buf;
    ay_change_count=0;
    return(0);
}

void sound_frame_blank()
{
    static int first=1;
    static unsigned char buf[2048];
    int fulllen=sound_framesiz*(sound_stereo+1);
    if(first) {
        first=0;
        memset(buf,128,sizeof(buf));
    }
    if(sizeof(buf)<fulllen) {
        usleep(20000);
        return;
    }
    driver_frame(buf,fulllen);
}

#endif
