//(c)2003 sisoft\trg - AYplayer.
/* $Id: sndemu_w.c,v 1.1 2003/11/05 12:41:23 root Exp $ */

//base version of this file was taken from x128 emulator by James McKay.

#include "ayplay.h"
#ifdef ADLIB
#ifdef WIN
#include <i86.h>
#endif
#include "z80.h"

static _UC PSG[16];
static _UC VolA=0,VolB=0,VolC=0;
static _UC white_noise=1;
static _UC Volumes[16]= {
        0xFF,0x3F,0x3F,0x3F,0x2C,0x26,0x22,0x1D,
        0x16,0x12,0x0C,0x0A,0x07,0x05,0x02,0x00
};

/*** PutAdlib ****************************************/
/*** Write value V into Adlib R.          ***/
/*****************************************************/
static void PutAdlib(_UC R,_UC V)
{
    _UC J;
    outp(0x388,R);for(J=0;J<6;J++) inp(0x388);
    outp(0x389,V);for(J=0;J<35;J++) inp(0x388);
}

/*** MuteSound ***************************************/
/*** Turn all sound off.                           ***/
/*****************************************************/
static void MuteSound(void)
{
    _UC OldPSG7=255;
    OldPSG7=PSG[7];
    PSG[7]=255;
    PSGOut(7,255);
    PSG[7]=OldPSG7;
}

/*** InitAdlib ***************************************/
/*** Return 0 if Adlib is not found, otherwise     ***/
/*** initialize it and return 1.                   ***/
/*****************************************************/
int sound_init()
{
    _UC I,A1,A2;
    /* If Adlib is not to be used, don't initialize */
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
    /***************************************************/
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

/*** TrashAdlib **************************************/
/*** If Adlib was used then shut it down.          ***/
/*****************************************************/
void sound_end()
{
    MuteSound();         /* Turn off the sound      */
    PutAdlib(0x04,0x60); /* Mask and disable timers */
    PutAdlib(0x04,0x80); /* Reset timers            */
}

void sound_ay_reset() {}

/*** PlayA *******************************************/
/*** Play sound via PSG channel A. Called when the ***/
/*** registers related to this channel are changed ***/
/*****************************************************/
static void PlayA()
{
    _UC J;
    _US Lath;
    long Tune;
    if(!(PSG[7]&0x01)) {
        PutAdlib(0x43,Volumes[VolA]);
        Lath=((_US)PSG[1]<<8)|PSG[0];
        Lath&=4095; /* Only 12 bits required */
        Tune=Lath? 2345678L/Lath:1;
        for(J=0;Tune>0x0400;Tune>>=1) J++;
        PutAdlib(0xA0,Tune&0xFF);
        PutAdlib(0xB0,((Tune>>8)&0x03)|(J<<2)|0x20);
    } else PutAdlib(0x43,Volumes[0]);
}

/*** PlayB *******************************************/
/*** Play sound via PSG channel B. Called when the ***/
/*** registers related to this channel are changed ***/
/*****************************************************/
static void PlayB()
{
    _UC J;
    _US Lath;
    long Tune;
    if(!(PSG[7]&0x02)) {
        PutAdlib(0x44,Volumes[VolB]);
        Lath=((_US)PSG[3]<<8)|PSG[2];
        Lath&=4095;
        Tune=Lath? 2345678L/Lath:1;
        for(J=0;Tune>0x0400;Tune>>=1) J++;
        PutAdlib(0xA1,Tune&0xFF);
        PutAdlib(0xB1,((Tune>>8)&0x03)|(J<<2)|0x20);
    } else PutAdlib(0x44,Volumes[0]);
}

/*** PlayC *******************************************/
/*** Play sound via PSG channel C. Called when the ***/
/*** registers related to this channel are changed ***/
/*****************************************************/
static void PlayC()
{
    _UC J;
    _US Lath;
    long Tune;
    if(!(PSG[7]&0x04)) {
        PutAdlib(0x45,Volumes[VolC]);
        Lath=((_US)PSG[5]<<8)|PSG[4];
        Lath&=4095;
        Tune=Lath? 2345678L/Lath:1;
        for(J=0;Tune>0x0400;Tune>>=1) J++;
        PutAdlib(0xA2,Tune&0xFF);
        PutAdlib(0xB2,((Tune>>8)&0x03)|(J<<2)|0x20);
    } else PutAdlib(0x45,Volumes[0]);
}

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
#ifdef IGNORE_BANANA
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
#endif
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

/*** PSGOut ******************************************/
/*** Write value V into AY8910 R.         ***/
/*** Appropriate sound routines are called as it   ***/
/*** is done.                                      ***/
/*****************************************************/

#ifdef USING_NEW_PSGOUT
void PSGOut(_UC R,_UC V)
{
    _UC R2, V2;
    PSG[R]=V;
    _disable();
    switch(R) {
        case 7: PlayA();PlayB();PlayC();PlayNoise();break;
        case 6: PlayNoise();break;
	case 11:case 12: break; /* No envelope support yet */
	case 0: case 1: case 8: PlayA();break;
        case 8: if(PSG[8]&0x10) VolA=15; else VolA=PSG[8]&0x0F;
        	PlayA();break;
	case 2: case 3: case 9: PlayB();break;
	case 4: case 5: case 10: PlayC();break;
    }
    _enable();
}
#endif

void sound_ay_write(int R,int V)
{
    _UC R2, V2;
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
