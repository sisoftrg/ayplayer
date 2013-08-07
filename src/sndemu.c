/* AYplayer (c)2001-2006 sisoft//trg.
\* $Id: sndemu.c,v 1.4 2006/08/10 03:13:55 root Exp $
 \ base version of this file was taken from aylet-0.3 by Russell Marks. */

#define ALSA

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifndef LPT_PORT
#ifdef UNIX
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef ALSA
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#else/*alsa*/
#include <sys/soundcard.h>
#endif
#else/*unix*/
#ifdef WIN32
#include <windows.h>
#include <malloc.h>
#include <mmsystem.h>
#ifdef DSOUND
#include <dsound.h>
#endif
#endif
#endif
#include <stdio.h>
#include "ayplay.h"
#include "z80.h"

#define BASE_SOUND_FRAG_PWR	6
#define AY_CLOCK		1773400
#define AMPL_AY_TONE		26
#define AY_CHANGE_MAX		16384
#define STEREO_BUF_SIZE		1024

#ifdef ALSA
static snd_pcm_t *asnd=NULL;
#else
static int soundfd=-1;
#endif
int sixteenbit=1;
#ifdef UNIX
int sound_freq=48000;
#else
int sound_freq=44100;
#endif
int sound_stereo=1;

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

struct ay_change_tag {
    unsigned char reg,val;
};

static struct ay_change_tag ay_change[AY_CHANGE_MAX];
static int ay_change_count;
static int rstereobuf_l[STEREO_BUF_SIZE],rstereobuf_r[STEREO_BUF_SIZE];
static int rstereopos,rchan1pos,rchan2pos,rchan3pos;

#ifdef WIN32
#ifdef DSOUND

#define MAX_AUDIO_BUFFER 8192*5
LPDIRECTSOUND lpDS; /* DirectSound object */
LPDIRECTSOUNDBUFFER lpDSBuffer; /* sound buffer */
DWORD nextpos; /* next position is circular buffer */

static int driver_init(int *freqptr,int *stereoptr)
{
    WAVEFORMATEX pcmwf; /* waveformat struct */
    DSBUFFERDESC dsbd; /* buffer description */
    CoInitialize(NULL);
    if(CoCreateInstance(&CLSID_DirectSound,NULL,CLSCTX_INPROC_SERVER,&IID_IDirectSound,(void**)&lpDS) != DS_OK) {
        return 0;
    }
    if(IDirectSound_Initialize(lpDS,NULL) != DS_OK) {
        return 0;
    }
    if(IDirectSound_SetCooperativeLevel(lpDS,GetDesktopWindow(),DSSCL_NORMAL) != DS_OK) {
        return 0;
    }
    memset(&pcmwf,0,sizeof(WAVEFORMATEX));
    pcmwf.cbSize=0;
    pcmwf.nChannels=*stereoptr ? 2 : 1;
    pcmwf.nBlockAlign=pcmwf.nChannels; /* number of channels * number of bytes per channel */
    pcmwf.nSamplesPerSec=*freqptr;
    pcmwf.nAvgBytesPerSec=pcmwf.nSamplesPerSec * pcmwf. nBlockAlign;
    pcmwf.wBitsPerSample=8;
    pcmwf.wFormatTag=WAVE_FORMAT_PCM;
    memset(&dsbd,0,sizeof(DSBUFFERDESC));
    dsbd.dwBufferBytes=MAX_AUDIO_BUFFER;
    dsbd.dwFlags=DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE;
    dsbd.dwSize=sizeof(DSBUFFERDESC);
    dsbd.lpwfxFormat=&pcmwf;
    if(IDirectSound_CreateSoundBuffer(lpDS,&dsbd,&lpDSBuffer,NULL) != DS_OK) {
        return 0;
    }
    if(IDirectSoundBuffer_Play(lpDSBuffer,0,0,DSBPLAY_LOOPING) != DS_OK) {
        return 0;
    }
    nextpos=0;
    return 1;
}

static void driver_end()
{
    IDirectSoundBuffer_Stop(lpDSBuffer);
    IDirectSoundBuffer_Release(lpDSBuffer);
    IDirectSound_Release(lpDS);
    CoUninitialize();
}

static void driver_frame(unsigned char *data,int len)
{
    HRESULT hres;
    int i;
    UCHAR * ucbuffer1,*ucbuffer2;
    DWORD length1,length2;
    hres=IDirectSoundBuffer_Lock(lpDSBuffer,nextpos,(DWORD)len,(void **)&ucbuffer1,&length1,(void **)&ucbuffer2,&length2,DSBLOCK_ENTIREBUFFER);
    if (hres != DS_OK) return; /* couldn't get a lock on the buffer */
    for(i=0; i<length1 && i<len; i++) {
        ucbuffer1[ i ]=*data++;
        nextpos++;
    }
    for(i=0; i<length2 && i+length1<len; i++) {
        ucbuffer2[ i ]=*data++;
        nextpos++;
    }
    if(nextpos >= MAX_AUDIO_BUFFER) nextpos -= MAX_AUDIO_BUFFER;
    IDirectSoundBuffer_Unlock(lpDSBuffer,ucbuffer1,length1,ucbuffer2,length2);
}
#else

HWAVEOUT WaveOutHandle;

static int driver_init(int *freqptr,int *stereoptr)
{
    WAVEFORMATEX pcmwf;
    UINT hr;
    pcmwf.wFormatTag        = WAVE_FORMAT_PCM;
    pcmwf.nChannels         = *stereoptr ? 2 : 1;
    pcmwf.wBitsPerSample    = 8;
    pcmwf.nBlockAlign       = pcmwf.nChannels * pcmwf.wBitsPerSample / 8;
    pcmwf.nSamplesPerSec    = *freqptr;
    pcmwf.nAvgBytesPerSec   = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
    pcmwf.cbSize            = 0;
    hr = waveOutOpen(&WaveOutHandle, WAVE_MAPPER, &pcmwf, 0, 0, 0);
    if(hr)return 0;
}

static void driver_end()
{
    waveOutReset(WaveOutHandle);
    waveOutClose(WaveOutHandle);
}

static void driver_frame(unsigned char *data,int len)
{
    WAVEHDR wavehdr;
    wavehdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
    wavehdr.lpData = (LPSTR)data;
    wavehdr.dwBufferLength = len;
    wavehdr.dwBytesRecorded = 0;
    wavehdr.dwUser = 0;
    wavehdr.dwLoops = -1;
    waveOutPrepareHeader(WaveOutHandle, &wavehdr, sizeof(WAVEHDR));
    waveOutWrite(WaveOutHandle, &wavehdr, sizeof(WAVEHDR));
    if(wavehdr.lpData) {
        waveOutUnprepareHeader(WaveOutHandle, &wavehdr, sizeof(WAVEHDR));
	wavehdr.dwFlags &= ~WHDR_PREPARED;
    }
}
#endif
#endif

#ifdef UNIX
#ifdef ALSA

static int driver_init(int *freqptr,int *stereoptr)
{
	int e;
	snd_pcm_hw_params_t *hwp;
	snd_pcm_sw_params_t *swp;
	snd_pcm_uframes_t bsize,thresh;
	snd_pcm_uframes_t periods,bframes;
	periods=512;bframes=periods*16;
	if((e=snd_pcm_open(&asnd,"default",SND_PCM_STREAM_PLAYBACK,0))<0){asnd=NULL;return 0;}
	snd_pcm_nonblock(asnd,0);
	if((e=snd_pcm_hw_params_malloc(&hwp))<0)return 0;
	if((e=snd_pcm_hw_params_any(asnd,hwp))<0)return 0;
	if((e=snd_pcm_hw_params_set_access(asnd,hwp,SND_PCM_ACCESS_RW_INTERLEAVED))<0)return 0;
	if((e=snd_pcm_hw_params_set_format(asnd,hwp,sixteenbit?SND_PCM_FORMAT_S16_LE:SND_PCM_FORMAT_S8))<0)return 0;
	if((e=snd_pcm_hw_params_set_channels(asnd,hwp,*stereoptr?2:1))<0)return 0;
	if((e=snd_pcm_hw_params_set_rate_near(asnd,hwp,(unsigned*)freqptr,0))<0)return 0;
	if((e=snd_pcm_hw_params_set_buffer_size_near(asnd,hwp,&bframes))<0)return 0;
	if((e=snd_pcm_hw_params_set_period_size_near(asnd,hwp,&periods,0))<0)return 0;
	if((e=snd_pcm_hw_params(asnd,hwp))<0)return 0;
	snd_pcm_hw_params_get_period_size(hwp,&periods,0);
	snd_pcm_hw_params_get_buffer_size(hwp,&bsize);
	if(periods==bsize)return 0;
	snd_pcm_hw_params_free(hwp);
	if((e=snd_pcm_sw_params_malloc(&swp))!=0)return 0;
	if((e=snd_pcm_sw_params_current(asnd,swp))!=0)return 0;
	e=bsize/periods*periods;thresh=e;
	if(thresh<1)thresh=(snd_pcm_uframes_t)1;
	if(thresh>(snd_pcm_uframes_t)e)thresh=(snd_pcm_uframes_t)e;
	if((e=snd_pcm_sw_params_set_start_threshold(asnd,swp,thresh))<0)return 0;
	if((e=snd_pcm_sw_params_set_stop_threshold(asnd,swp,bsize))<0)return 0;
	if((e=snd_pcm_sw_params(asnd,swp))!=0)return 0;
	snd_pcm_sw_params_free(swp);
	snd_pcm_reset(asnd);
    return(1);
}

static void driver_end()
{
	if(!asnd)return;
	snd_pcm_drain(asnd);
	snd_pcm_close(asnd);
	asnd=NULL;
}

static void driver_frame(unsigned char *data,int len)
{
	int e,n;
	static unsigned char buf16[8192];

    if(sixteenbit) {
        unsigned char *src,*dst;
        int f;
        src=data; dst=buf16;
        for(f=0;f<len;f++) {
            *dst++=128;
            *dst++=*src++-128;
        }
        data=buf16;
    }
    len >>= 1;

	for(n=0;n<len;) {
		e=snd_pcm_writei(asnd,data+n*(sound_stereo?2:1),len-n);
		if(e==-EPIPE)snd_pcm_prepare(asnd);
		else if(e==-ESTRPIPE)snd_pcm_resume(asnd);
		else snd_pcm_wait(asnd,1000);
		if(e>0)n+=e;
	}
}

#else

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
    if(ioctl(soundfd,SNDCTL_DSP_SPEED,freqptr)<0) {
        close(soundfd);
        return(0);
    }
    frag=(0x80000|BASE_SOUND_FRAG_PWR);
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
#endif

#define CLOCK_RESET(clock) ay_tick_incr=(int)(65536.*clock/sound_freq)

static void sound_ay_init()
{
    int f;
    static const int levels[16]= {
            0x0000, 0x0344, 0x04BC, 0x06ED,
            0x0A3D, 0x0F23, 0x1515, 0x2277,
            0x2898, 0x4142, 0x5B2B, 0x726C,
            0x9069, 0xB555, 0xD79B, 0xFFFF
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
        int pos=6*sound_freq/8000;
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
  (var)=0;								\
  if(level) {								\
    if(ay_tone_high[chan]) (var)=(level);				\
    else (var)=-(level),is_low=1;					\
  }									\
  ay_tone_tick[chan]+=tone_count;					\
  count=0;								\
  while(ay_tone_tick[chan]>=ay_tone_period[chan]) {			\
    count++;								\
    ay_tone_tick[chan]-=ay_tone_period[chan];				\
    ay_tone_high[chan]=!ay_tone_high[chan];				\
    if(count==1 && level && ay_tone_tick[chan]<tone_count) {		\
      if(is_low) (var)+=AY_GET_SUBVAL(chan);				\
      else (var)-=AY_GET_SUBVAL(chan);					\
    }									\
  }									\
  if(count>1) (var)=-(level)

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
    int is_low;
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
	if((mixer&1)==0) {
            level=chan1;
            AY_DO_TONE(chan1,0);
        }
        if((mixer&0x08)==0 && noise_toggle)chan1=0;
	if((mixer&2)==0) {
            level=chan2;
            AY_DO_TONE(chan2,1);
        }
        if((mixer&0x10)==0 && noise_toggle)chan2=0;
	if((mixer&4)==0) {
            level=chan3;
            AY_DO_TONE(chan3,2);
        }
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
        ay_change[ay_change_count].reg=reg&15;
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
    if((int)sizeof(buf)<fulllen) {
        usleep(20000);
        return;
    }
    driver_frame(buf,fulllen);
}

#endif
