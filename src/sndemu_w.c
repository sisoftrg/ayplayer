/* (c)2003 sisoft\trg - AYplayer.
\* $Id: sndemu_w.c,v 1.3 2004/08/02 09:44:26 root Exp $
 \ base version of this file was taken from x128 emulator by James McKay. */

#include "ayplay.h"
#ifdef ADLIB
#ifndef UNIX
#ifndef WIN32
#include <i86.h>
#endif
#endif
#include "z80.h"

static _UC PSG[16];
static _UC white_noise=1;
static _UC Volumes[16]= {
        0xFF,0x3F,0x3F,0x3F,0x2C,0x26,0x22,0x1D,
        0x16,0x12,0x0C,0x0A,0x07,0x05,0x02,0x00
};

static void PutAdlib(_UC R,_UC V)
{
    _UC J;
    outp(0x388,R);for(J=0;J<6;J++) inp(0x388);
    outp(0x389,V);for(J=0;J<35;J++) inp(0x388);
}

static void MuteSound(void)
{
    _UC OldPSG7=255;
    OldPSG7=PSG[7];
    PSG[7]=255;
    sound_ay_write(7,255);
    PSG[7]=OldPSG7;
}

int sound_init()
{
    _UC I,A1,A2;
    /********* Detect if Adlib is present. *************/
    PutAdlib(0x01,0x00);    /* Delete test    */
    PutAdlib(0x04,0x60);    /* Mask and disable timers */
    PutAdlib(0x04,0x80);    /* Reset timers            */
    A1=inp(0x388)&0xE0;     /* Read status             */
    PutAdlib(0x02,0xFF);    /* Set Timer1 to 0xFF      */
    PutAdlib(0x04,0x21);    /* Unmask and start Timer1 */
    for(I=0;I<0xC8;I++) inp(0x388);  /* Wait 80us      */
    A2=inp(0x388)&0xE0;     /* Read status             */
    PutAdlib(0x04,0x60);    /* Mask and disable timers */
    PutAdlib(0x04,0x80);    /* Reset timers            */
    if((A2!=0xC0)||A1)return(0);
    /* Allow the FM chips to control WaveForm */
    for(I=0;I<255;I++) PutAdlib(I,0x00);
    PutAdlib(0x01,0x20);
    PutAdlib(0xBD,0x20|8);
    /* Turn sound off */
    MuteSound();
    for(I=0;I<3;I++) {
        PutAdlib(0x20+I,0x01);
        PutAdlib(0x23+I,0x01);
        PutAdlib(0x40+I,0x18);
        PutAdlib(0x43+I,0x3F);
        PutAdlib(0x60+I,0xF0);
        PutAdlib(0x63+I,0xF0);
        PutAdlib(0x80+I,0x14);
        PutAdlib(0x83+I,0x13);
        PutAdlib(0xE0+I,0x02);
        PutAdlib(0xE3+I,0x00);
        PutAdlib(0xC0+I,0x0A);
        PutAdlib(0xB0+I,0x00);
    }
    PutAdlib(0x34,0x21);
    PutAdlib(0x54,0x3F);
    PutAdlib(0x74,0x99);
    PutAdlib(0x94,0x00);
    PutAdlib(0xF4,0x00);
    PutAdlib(0xBD,0x28);
    return(1);
}

void sound_end()
{
    MuteSound();         /* Turn off the sound      */
    PutAdlib(0x04,0x60); /* Mask and disable timers */
    PutAdlib(0x04,0x80); /* Reset timers            */
}

void sound_ay_reset() {}

static void Set_Tone(_UC R)
{
    _UC J;
    _US Lath;
    long Tune;
    Lath=((_US)PSG[(R<<1)+1]<<8)|PSG[R<<1];
    Lath&=4095; /* Only 12 bits required */
    Tune=Lath? 2345678L/Lath:1;
    for(J=0;Tune>0x0400;Tune>>=1) J++;
    switch(R) {
    case 0:
        if(!(PSG[7]&1)) {
            PutAdlib(0xA0+R,Tune&0xFF);
            PutAdlib(0xB0+R,((Tune>>8)&0x03)|(J<<2)|0x20);
        }
        if(!(PSG[7]&8)) {
            PutAdlib(0xA7,Tune&0xFF);
            PutAdlib(0xB7,((Tune>>8)&0x03)|(J<<2)|0x20);
        }
        break;
    case 1:
        if(!(PSG[7]&2)) {
            PutAdlib(0xA0+R,Tune&0xFF);
            PutAdlib(0xB0+R,((Tune>>8)&0x03)|(J<<2)|0x20);
        }
        if(!(PSG[7]&16)) {
            PutAdlib(0xA7,Tune&0xFF);
            PutAdlib(0xB7,((Tune>>8)&0x03)|(J<<2)|0x20);
        }
        break;
    case 2:
        if(!(PSG[7]&4)) {
            PutAdlib(0xA0+R,Tune&0xFF);
            PutAdlib(0xB0+R,((Tune>>8)&0x03)|(J<<2)|0x20);
        }
        if(!(PSG[7]&32)) {
            PutAdlib(0xA7,Tune&0xFF);
            PutAdlib(0xB7,((Tune>>8)&0x03)|(J<<2)|0x20);
        }
        break;
    }
//#ifdef IGNORE_BANANA
    switch(((PSG[7]>>R)&9)) {
    case 9: /* Neither */
        break;
    case 8: /* Just Tone */
        PutAdlib(0xA0+R,Tune&0xFF);
        PutAdlib(0xB0+R,((Tune>>8)&0x03)|(J<<2)|0x20);
        break;
    case 1: /* Just Noise */
        PutAdlib(0xA7,Tune&0xFF);
        PutAdlib(0xB7,((Tune>>8)&0x03)|(J<<2)|0x20);
        break;
    case 0: /* Both */
        PutAdlib(0xA0+R,Tune&0xFF);
        PutAdlib(0xB0+R,((Tune>>8)&0x03)|(J<<2)|0x20);
        PutAdlib(0xA7,Tune&0xFF);
        PutAdlib(0xB7,((Tune>>8)&0x03)|(J<<2)|0x20);
        break;
    }
//#endif
}

static void Set_Vol(_UC R)
{
    _UC V;
    V=PSG[R+8];
    if(V&0x10) V=12;
    if((PSG[7]&(1<<R)))PutAdlib(0x43+R,Volumes[0]);
    else PutAdlib(0x43+R,Volumes[V]);
    if((PSG[7]&0x38)==0x38)PutAdlib(0x54,Volumes[0]);
    else if(white_noise) PutAdlib(0x54,Volumes[V]);
}

void sound_ay_write(int R,int V)
{
    PSG[R]=(_UC)V;
    _disable();
    switch(R) {
        case 0:
        case 1: Set_Tone(0);break;
        case 2:
        case 3: Set_Tone(1);break;
        case 4:
        case 5: Set_Tone(2);break;
        case 6: break;
        case 7: Set_Vol(0);Set_Vol(1);Set_Vol(2);break;
        case 8: Set_Vol(0);break;
        case 9: Set_Vol(1);break;
        case 10:Set_Vol(2);break;
    }
    _enable();
}

#endif
