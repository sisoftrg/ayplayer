//(c)2004 sisoft\trg - AYplayer.
/* $Id: ayplay.c,v 1.1 2004/03/11 14:24:10 root Exp $ */
#include "ayplay.h"
#include "z80.h"

enum {
    UNK=0,VTX,PSG,AY,YM,HOB,PT1,PT2,PT3,STP,STC,PSC,ASC,GTR,FTC,SQT,FLS,FXM
} formats;
static _US sp;
static _UC *ibuf,*obuf;
static _UL origsize,compsize,q,tick,t=0,lp;
static char name[]="Name:    ",author[]="Author:  ";
static int quitflag=0;
static int ca,cb,cc,ft=UNK;

#define PLADR 18432
#define WRD(x) ((*(_UC*)(x))*256+(*(_UC*)((x)+1)))
#define PTR(x) ((*(char*)(x))*256+(*(char*)((x)+1)))

void erro(char *ermess)
{
	if(ermess)printf("\n* Error: %s!\n",ermess);
	else puts("* Support: VTX,PSG,AY,YM,PT[123],STP,STC,ZXS,PSC,ASC,GTR,FTC,SQT,FLS,FXM,Hobeta.\n"
		  "* Usage: ayplayer filename");
	exit(-1);
}

#ifdef UNIX
static void sighup(int sig)
{
	signal(sig,sighup);
	quitflag=1;
}
#endif

void sreg(char reg,_UC dat)
{
#ifdef LPT_PORT
#ifdef UNIX
	ioperm(LPT_PORT,3,1);
#endif
	outb(reg,LPT_PORT);
	outb(6,LPT_PORT+2);
	outb(15,LPT_PORT+2);
	outb(dat,LPT_PORT);
	outb(14,LPT_PORT+2);
	outb(15,LPT_PORT+2);
	outb(0,LPT_PORT);
	outb(6,LPT_PORT+2);
	outb(15,LPT_PORT+2);
#ifdef UNIX
	ioperm(LPT_PORT,3,0);
#endif
#else
	sound_ay_write(reg,dat);
#endif
}

static void playvtx()
{
	int i;
	_UC a;
	for(i=0;i<14;i++) {
		a=obuf[i*tick+t];
		if(i==7)a|=192;
		if(i==8)ca=a;
		if(i==9)cb=a;
		if(i==10)cc=a;
		if(!(i==13&&a==255))sreg(i,a);
	}
	t++;
	if(t>=tick)t=lp;
}

static void playpsg()
{
	_UC a,i;
	while((i=ibuf[t+++4])<0xfd&&t<lp) {
		a=ibuf[t+++4];
		if(i==8)ca=a;
		if(i==9)cb=a;
		if(i==10)cc=a;
		if(i<14)sreg(i,a);
	}
	q++;
	if(t>=lp||q>=tick||i==0xfd)t=q=0;
}

static void playemu()
{
	_UC i;
	int r=-1,v=-1;
	DANM(haltstate)=0;
	PC=PLADR+4;SP=sp;
	while(!DANM(haltstate)) {
		DANM(r)=128;DANM(v)=128;
//		printf("pc=%u,sp=%u\n",PC,SP);
		PRNM(step)(0);
		if((i=DANM(r))!=128)r=i;
		if((i=DANM(v))!=128)v=i;
		if(r>=0&&v>=0) {
			if(r==7)v|=192;
			if(r==8)ca=v;
			if(r==9)cb=v;
			if(r==10)cc=v;
			if(r<14)sreg(r,v);
			v=-1;r=-1;
		}
	}
	q++;
	if(tick&&q>tick)q=lp;
}

static void indik()
{
	int i;
	static unsigned sn=0;
	char a[]={"                "};
	char b[]={"                "};
	char c[]={"                "},s;
	static unsigned char ma=0,mb=0,mc=0;
	static unsigned short mca=0,mcb=0,mcc=0;
	static char slashes[]=":::/-\\|/-\\:::\\-/|\\-/";
	for(i=0;i<(ca&15);i++)a[i+1]='=';if(i)a[ca&15]='-';if(ca>15)*a=ca>16?'>':'<';
	if((ca&15)>=ma){ma=ca&15;mca=0;}
	else{mca++;if(mca>6&&mca<104&&!(mca%(13-mca/8))&&ma)ma--;}
	if(ma&&a[ma]==' ')a[ma]=POINT;
	for(i=0;i<(cb&15);i++)b[i+1]='=';if(i)b[cb&15]='-';if(cb>15)*b=cb>16?'>':'<';
	if((cb&15)>=mb){mb=cb&15;mcb=0;}
	else {mcb++;if(mcb>6&&mcb<104&&!(mcb%(13-mcb/8))&&mb)mb--;}
	if(mb&&b[mb]==' ')b[mb]=POINT;
	for(i=0;i<(cc&15);i++)c[i+1]='=';if(i)c[cc&15]='-';if(cc>15)*c=cc>16?'>':'<';
	if((cc&15)>=mc){mc=cc&15;mcc=0;}
	else {mcc++;if(mcc>6&&mcc<104&&!(mcc%(13-mcc/8))&&mc)mc--;}
	if(mc&&c[mc]==' ')c[mc]=POINT;
	s=slashes[(++sn)%100/5];
	switch(ft) {
	    case VTX: 
		printf("%02lu%c%02lu   A:%s   B:%s   C:%s\r",t/q/60L,s,t/q%60L,a,b,c);
		break;
	    default:
		printf("%02lu%c%02lu   A:%s   B:%s   C:%s\r",q/50L/60L,s,q/50L%60L,a,b,c);
		break;
	}
	fflush(stdout);
}

static _UC *vtxinfo(char *buf)
{
	_US yea;
	printf("Chip:    %s\n",*buf++=='a'?"AY-3-8910(12)":"YM2149F");
	printf("Regime:  ");
	switch(*++buf)
	{
		case 0: puts("Mono"); break;
		case 1: puts("ABC stereo"); break;
		case 2: puts("ACB stereo"); break;
		case 3: puts("BAC stereo"); break;
		case 4: puts("BCA stereo"); break;
		case 5: puts("CAB stereo"); break;
		case 6: puts("CBA stereo"); break;
		default:erro("uncknown mode");
	}
	lp=*(_US*)(++buf);
	buf+=6;q=*buf++;
	yea=*(_US*)buf++;
	if(yea)printf("Year:    %u\n",yea);
	origsize=*(_UL*)++buf;buf+=4;
	if(!origsize)erro("playable data is empty");
	if(strlen(buf))printf("%s%s\n",name,buf);buf+=strlen(buf);
	if(strlen(++buf))printf("%s%s\n",author,buf);buf+=strlen(buf);
	if(strlen(++buf))printf("Origin:  %s\n",buf);buf+=strlen(buf);
	if(strlen(++buf))printf("Editor:  %s\n",buf);buf+=strlen(buf);
	if(strlen(++buf))printf("Comment: %s\n",buf);buf+=strlen(buf);
	tick=origsize/14L;printf("Length:  %lu min, %lu sec\n",tick/q/60L,tick/q%60L);
	if(q!=50)puts("Warning: music may not play correctly!");
	return((_UC*)(++buf));
}

static _US xfind(_US sa,char *s,int l)
{
	_US f=0;
	while(memcmp(DANM(mem)+sa+f,s,l)&&f<256)f++;
	if(f>=255)f=0;
	return f;
}

static _US xstr(char *n,_US sa,char *e,_US x)
{
	_US l=x,i=0;
	if(e)l=xfind(sa,e,x);
	if(l) {
		while(i<l&&DANM(mem)[sa+i]==' ')i++;
		if(i<l) {
			if(n)printf(n);
			fwrite(DANM(mem)+sa+i,l-i,1,stdout);
			printf("\n");
		}
	}
	if(e)l+=x;
	return l;
}

int main(int argc,char *argv[])
{
	int co=0;
	FILE *infile;
	struct stat sb;
	char *nam=NULL;
	_UC *tt1=NULL,*tt2=NULL;
	_US sadr=0,iadr=0,padr=0,sngadr=0,i;
	puts("\n AY Player'2004, for real AY chip on LPT port");
	puts("(c) Stepan Pologov (sisoft\\\\trg), sisoft@bk.ru");
	if(argc!=2||strchr(argv[1],'.')==NULL)erro(NULL);
#ifndef LPT_PORT
	if(!sound_init())erro("can't init soundcard");
#endif
	if(!strcmp(argv[1],".")) {
		sb.st_size=DEMO_S;ft=DEMO_T;
		if((ibuf=tt1=(_UC*)malloc(sb.st_size))==NULL)erro("out of memory");
		memcpy(ibuf,DEMO_D,DEMO_S);
		puts("\nFile:    AYPlayer demo song");
		goto playz;
	}
	if(!strcasecmp(strrchr(argv[1],'.'),".gz")) {
		char cmd[256];
		nam=tmpnam(NULL);
		strncat(nam,argv[1]+(strchr(argv[1],'.')-argv[1]),4);
		sprintf(cmd,"gzip -cd %s >%s 2>/dev/null",argv[1],nam);
		if(system(cmd)){unlink(nam);erro("can't gzip sound file");}
	}
	if(stat(nam?nam:argv[1],&sb))erro("can't stat sound file");
	if(sb.st_size<128)erro("file is empty");
	if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".vtx"))ft=VTX;
	    else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".psg"))ft=PSG;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".ym"))ft=YM;
	if(ft) {
		if((ibuf=tt1=(_UC*)malloc(sb.st_size))==NULL)erro("out of memory");
		if((infile=fopen(nam?nam:argv[1],"rb"))==NULL)erro("can't open sound file");
		fread(ibuf,sb.st_size,1,infile);
		fclose(infile);
	} else {
		if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".pt2"))ft=PT2;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".pt3"))ft=PT3;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".pt1"))ft=PT1;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".stp"))ft=STP;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".stc"))ft=STC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".zxs"))ft=STC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".psc"))ft=PSC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".asc"))ft=ASC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".gtr"))ft=GTR;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".ftc"))ft=FTC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".sqt"))ft=SQT;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".fls"))ft=FLS;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".fxm"))ft=FXM;
		else if(!strncasecmp(strrchr(nam?nam:argv[1],'.'),".$",2))ft=HOB;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".ay"))ft=AY;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".m"))ft=PT3;
		if(ft) {
			PRNM(init)();PRNM(reset)();
			DANM(mem)[PLADR]=0xcd;
			DANM(mem)[PLADR+3]=0x76;
			DANM(mem)[PLADR+4]=0xcd;
			DANM(mem)[PLADR+7]=0x76;
			for(i=0;i<256;i++)DANM(mem)[i]=0xc9;
			for(i=256;i<16384;i++)DANM(mem)[i]=0xff;
			DANM(mem)[0]=0x76;DANM(mem)[0x38]=0xfb;sp=PLADR+1024;
			if((infile=fopen(nam?nam:argv[1],"rb"))==NULL)erro("can't open sound file");
again:			switch(ft) {
			    case HOB: {
				unsigned short crc=0;
				char hdr[17]={0};
				fread(hdr,17,1,infile);
				for(i=0;i<15;i++)crc+=hdr[i]*257+i;
				if(crc!=*(_US*)(hdr+15))puts("\nWarn: corrupted hobeta file!");
				sb.st_size=*(_US*)(hdr+11);
				if(sb.st_size<128)sb.st_size=*(_US*)(hdr+13);
				switch(hdr[8]) {
				    case 'M': 
					    if(hdr[9]=='P'&&hdr[10]=='S')ft=PSC;
						else ft=PT2;
					    break;
				    case 'm': ft=PT3;break;
				    case 'G': ft=GTR;break;
				    case 'Y': ft=FTC;break;
				    case 'S': erro("Uncompiled SoundTracker modules is not supported");
				    case 'C': if(sb.st_size!=6912)break;
				    default : erro("Extension of this file is not supported");
				}
				if(ft!=HOB)goto again;
				iadr=sadr=*(_US*)(hdr+9);
				if(iadr<16384)iadr=49152;
				padr=iadr+5;ft=PT3;co=1;
				fread(DANM(mem)+sadr,sb.st_size,1,infile);
				sngadr=*(_US*)(DANM(mem)+iadr+1);
				if(DANM(mem)[padr]!=0xc3){padr++;ft=PT2;}
				if(*(_US*)(DANM(mem)+sadr)==0x83e){padr=iadr+48;ft=SQT;}
				if(!memcmp(DANM(mem)+sadr+17,"KSA SOFT",8)) {
					if((*(_US*)(DANM(mem)+sadr+4)-sadr)>256) {
						iadr=*(_US*)(DANM(mem)+sadr+4)-0x4e;
						memcpy(DANM(mem)+iadr,DANM(mem)+sadr,sb.st_size);
						sadr=iadr;padr=iadr+6;
					}
					ft=STP;
				}
				if(!memcmp(DANM(mem)+sadr+20,"SOUND TR",8)){ft=STC;iadr=sadr+11;padr=iadr+3;}
				if(!memcmp(DANM(mem)+sadr+1091,"SONG BY ",8)){ft=STC;iadr=sadr;padr=iadr+6;}
				if(!memcmp(DANM(mem)+sadr+7,"SONG BY ST",10)){ft=STC;co=0;fseek(infile,17,SEEK_SET);goto again;}
				if(!memcmp(DANM(mem)+sadr+20,"ASM COMP",8)){ft=ASC;iadr=sadr+11;padr=iadr+3;}
				if(xfind(sadr,"ASM COMP",8)){ft=ASC;co=0;fseek(infile,17,SEEK_SET);goto again;}
				if(!memcmp(DANM(mem)+sadr,"ProTrack",8)){ft=PT3;co=0;fseek(infile,17,SEEK_SET);goto again;}
				if(!memcmp(DANM(mem)+sadr+77,"PT 3 P",6)){ft=PT3;iadr=sadr;padr=iadr+5;co=2;}
				if(!memcmp(DANM(mem)+sadr+9,"PSC ",4)){ft=PSC;iadr=sadr;padr=iadr+6;}
				if(ft==PT2&&*(DANM(mem)+sadr+131+*(DANM(mem)+sadr+1))==0xff){co=0;fseek(infile,17,SEEK_SET);goto again;}
				printf("hob: s: %u, l: %lu, i: %u, p: %u, sng: %u\n",sadr,sb.st_size,iadr,padr,sngadr);
				} break;
			    case PT1:
				memcpy(DANM(mem)+PT1_init,pt1_player,PT1_song-PT1_init);
				fread(DANM(mem)+PT1_song,sb.st_size,1,infile);
				iadr=PT1_init;
				padr=PT1_play;
				sngadr=PT1_song;
				break;
			    case PT2:
				memcpy(DANM(mem)+PT2_init,pt2_player,PT2_song-PT2_init);
				fread(DANM(mem)+PT2_song,sb.st_size,1,infile);
				iadr=PT2_init;
				padr=PT2_play;
				sngadr=PT2_song;
				break;
			    case PT3:
				memcpy(DANM(mem)+PT3_init,pt3_player,PT3_song-PT3_init);
				fread(DANM(mem)+PT3_song,sb.st_size,1,infile);i=6;
				if(*(_UC*)(DANM(mem)+PT3_song+13)>='0'&&*(_UC*)(DANM(mem)+PT3_song+13)<='9')
					i=*(_UC*)(DANM(mem)+PT3_song+13)-'0';
				switch(*(_UC*)(DANM(mem)+PT3_song+99)) {
				    case 0: if(i<=3)i=0; else i=1; break;
				    case 1: i=2; break;
				    case 2: if(i<=3)i=3; else i=4; break;
				    default: if(i<=3)i=5; else i=6; break;
				}
				memcpy(DANM(mem)+PT3_table,pt3_tables+96*i,192);
				iadr=PT3_init;
				padr=PT3_play;
				sngadr=PT3_song;
				break;
			    case STP:
				memcpy(DANM(mem)+STP_init,stp_player,STP_song-STP_init);
				fread(DANM(mem)+STP_song,sb.st_size,1,infile);
				iadr=STP_init;
				padr=STP_play;
				sngadr=STP_song;
				break;
			    case STC:
				memcpy(DANM(mem)+STC_start,stc_player,STC_song-STC_start);
				fread(DANM(mem)+STC_song,sb.st_size,1,infile);
				sadr=STC_start;
				iadr=STC_init;
				padr=STC_play;
				sngadr=STC_song;
				break;
			    case PSC:
				memcpy(DANM(mem)+PSC_init,psc_player,PSC_song-PSC_init);
				fread(DANM(mem)+PSC_song,sb.st_size,1,infile);
				iadr=PSC_init;
				padr=PSC_play;
				sngadr=PSC_song;
				break;
			    case ASC:
				memcpy(DANM(mem)+ASC_start,asc_player,ASC_song-ASC_start);
				fread(DANM(mem)+ASC_song,sb.st_size,1,infile);
				sadr=ASC_start;
				iadr=ASC_init;
				padr=ASC_play;
				sngadr=ASC_song;
				break;
			    case GTR:
				memcpy(DANM(mem)+GTR_init,gtr_player,GTR_song-GTR_init);
				fread(DANM(mem)+GTR_song,sb.st_size,1,infile);
				iadr=GTR_init;
				padr=GTR_play;
				sngadr=GTR_song;
				break;
			    case FTC:
				memcpy(DANM(mem)+FTC_init,ftc_player,FTC_song-FTC_init);
				fread(DANM(mem)+FTC_song,sb.st_size,1,infile);
				iadr=FTC_init;
				padr=FTC_play;
				sngadr=FTC_song;
				break;
			    case SQT: {
				_US j=0,k,*p,fl;
				memcpy(DANM(mem)+SQT_init,sqt_player,SQT_song-SQT_init);
				fread(DANM(mem)+SQT_song,sb.st_size,1,infile);
				i=*(_US*)(DANM(mem)+SQT_song+2)-10;
				k=*(_US*)(DANM(mem)+SQT_song+8)-i;
				while(*(DANM(mem)+SQT_song+k)) {
					if(j<(*(DANM(mem)+SQT_song+k)&0x7f))j=*(DANM(mem)+SQT_song+k)&0x7f;k+=2;
					if(j<(*(DANM(mem)+SQT_song+k)&0x7f))j=*(DANM(mem)+SQT_song+k)&0x7f;k+=2;
					if(j<(*(DANM(mem)+SQT_song+k)&0x7f))j=*(DANM(mem)+SQT_song+k)&0x7f;k+=3;
				}
				p=(_US*)(DANM(mem)+SQT_song+2);
				fl=(*(_US*)(DANM(mem)+SQT_song+6)-i+(j<<1))/2;
				for(k=0;k<fl;k++) {
					*p-=i;
					*p+=SQT_song;
					p++;
				}
				iadr=SQT_init;
				padr=SQT_play;
				sngadr=SQT_song;
				} break;
			    case FLS: {
				int i3,i1,i2;
				_US *p,song_len=sb.st_size;
				memcpy(DANM(mem)+FLS_init,fls_player,FLS_song-FLS_init);
				fread(DANM(mem)+FLS_song,sb.st_size,1,infile);
				i3=*(_US*)(DANM(mem)+FLS_song+2)-16;
				if(i3>=0) do {
					i2=*(_US*)(DANM(mem)+FLS_song+4)-i3+2;
					if(i2>=8&&i2<song_len) {
						p=(_US*)(DANM(mem)+FLS_song+i2);
						i1=*p-i3;
						if(i1>=8&&i1<song_len) {
							p=(_US*)(DANM(mem)+FLS_song+i2-4);
							i2=*p-i3;
							if(i2>=6&&i2<song_len)
							    if(i1-i2==0x20) {
								i2=*(_US*)(DANM(mem)+FLS_song+8)-i3;
								if(i2>21&&i2<song_len) {
									i1=*(_US*)(DANM(mem)+FLS_song+6)-i3;
									if(i1>20&&i1<song_len)
									    if(*(_UC*)(DANM(mem)+FLS_song+i1-1)==0) {
										while(i1<song_len&&*(_UC*)(DANM(mem)+FLS_song+i1)!=255)
										    do {
											if(*(_UC*)(DANM(mem)+FLS_song+i1)<=0x5f||*(_UC*)(DANM(mem)+FLS_song+i1)==0x80||*(_UC*)(DANM(mem)+FLS_song+i1)==0x81) {
												i1++;
												break;
											}
											if(*(_UC*)(DANM(mem)+FLS_song+i1)>=0x82&&*(_UC*)(DANM(mem)+FLS_song+i1)<=0x8e)i1++;
											i1++;
										} while(i1<song_len);
										if(i1+1==i2)break;
									}
								}
							}
						}
					}
					i3--;
				} while(i3>=0);
				if(i3<0)erro("bad fls file");
				p=(_US*)(DANM(mem)+FLS_song);
				i1=*(_US*)(DANM(mem)+FLS_song+4)-i3+(long)p;
				i2=*(_US*)(DANM(mem)+FLS_song)-i3+(long)p+2;
				do {
					*p-=i3;
					*p+=FLS_song;
					p++;
				} while((long)p!=i1);
				p++;
				do {
					*p-=i3;
					*p+=FLS_song;
					p+=2;
				} while((long)p!=i2);
				iadr=FLS_init;
				padr=FLS_play;
				sngadr=FLS_song;
				} break;
			    case AY:
				tt1=(_UC*)malloc(sb.st_size);
				if(tt1==NULL)erro("out of memory");
				fread(tt1,sb.st_size,1,infile);
				if(memcmp(tt1,"ZXAYEMUL",8)){puts("unknown format");exit(-1);}
				ibuf=tt1+18;
				ibuf+=PTR(ibuf)+2;
				ibuf+=PTR(ibuf)+4;
				tick=WRD(ibuf);ibuf+=6;
				obuf=ibuf+PTR(ibuf);
				ibuf+=PTR(ibuf+2)+2;
				sp=WRD(obuf);
				if(!sp)sp=PLADR+1024;
				iadr=WRD(obuf+2);
				padr=WRD(obuf+4);
//				printf("iadr=%u,padr=%u,sp=%u\n",iadr,padr,sp);
				do {
//					printf("to=%u,from=%u,len=%u\n",WRD(ibuf),4+PTR(ibuf+4),WRD(ibuf+2));
					memcpy(DANM(mem)+WRD(ibuf),ibuf+4+PTR(ibuf+4),WRD(ibuf+2));
					if(!iadr)iadr=WRD(ibuf);ibuf+=6;
				} while(WRD(ibuf));
				break;
			    case FXM: {
				char bu[6];
				memcpy(DANM(mem)+FXM_init,ftc_player,FXM_song-FXM_init);
				fread(bu,6,1,infile);
				if(strncasecmp(bu,"FXSM",4))erro("bad file format");
				sngadr=*(unsigned short*)(bu+4);
				fread(DANM(mem)+sngadr,sb.st_size-6,1,infile);
				memcpy(DANM(mem)+FXM_song,DANM(mem)+sngadr,sb.st_size-6);
				iadr=FXM_init;
				padr=FXM_play;
				} break;
			}
			*(_US*)(DANM(mem)+PLADR+1)=(_US)iadr;
			PC=PLADR;SP=sp;
			while(iadr&&!DANM(haltstate)){/*printf("pc=%u,sp=%u\n",PC,SP);*/PRNM(step)(1);}
			if(!padr)padr=DANM(mem)[(dbyte)(((int)RI<<8)+0xFF)]+256*DANM(mem)[(dbyte)(((int)RI<<8)+0xFF+1)];
			*(_US*)(DANM(mem)+PLADR+5)=(_US)padr;
			fclose(infile);
		}
	}
	if(nam)unlink(nam);
	printf("\nFile:    %s\n",argv[1]);
playz:
#ifdef UNIX
	signal(SIGHUP,sighup);signal(SIGINT,sighup);
#endif
	printf("Type:    %s%s",nam?"packed ":"",co?"compiled ":"");
#ifndef LPT_PORT
	sound_ay_reset();
#endif
//	{FILE *xx=fopen("debug","wb");fwrite(DANM(mem),1,65536,xx);fclose(xx);}
	switch(ft) {
	    case VTX:
		puts("Vortex Tracker");
		ibuf=vtxinfo((char*)ibuf);
		compsize=sb.st_size-(ibuf-tt1);
		if((tt2=obuf=(_UC*)calloc(14,tick))==NULL)erro("out of memory");
		unlh5(ibuf,obuf,origsize,compsize);
		obuf=tt2;free(tt1);tt1=NULL;
		break;
/*
  Id:dword;
  Leo:array[0..7]of char;+4
  Num_of_tiks:dword;+12
  Song_Attr:dword;+16
  Num_of_Dig:word;+20
  ChipFrq:dword;+22
  InterFrq:word;+26
  Loop:dword;+28
  Add_Size:word;+32
*/
	    case YM: {
		_UL j;
		_US i1=0,i2;
		struct {int l;_UC *b;} ds[1024];
		puts("YM file");
		if(memcmp(ibuf+2,"-lh5-",5))erro("unknown archive type");
		compsize=*(long*)(ibuf+7);
		origsize=*(long*)(ibuf+11);
		ibuf+=*(_UC*)ibuf+2;
		if((tt2=obuf=(_UC*)calloc(1,origsize))==NULL)erro("out of memory");
		unlh5(ibuf,obuf,origsize,compsize);
		obuf=tt2;free(tt1);tt1=NULL;
		if(*obuf!='Y'||obuf[1]!='M')erro("unknown format");
		tick=*(_UL*)(obuf+12);
		i=*(_US*)(obuf+20);
		if(i>0) {
			i2=*(_US*)(obuf+32)+34;
			while(i>0) {
				j=*(_UL*)(obuf+i2);
				ds[i1].l=j;
				ds[i1].b=obuf+i2+4;
				i2+=j+4;
				i1++;
				i--;
			}
//			if((*(_UL*)(obuf+16)&0x6000000)==0x2000000) {
		}

		} break;
	    case PSG:
		puts("PSG file");
		if(memcmp(ibuf,"PSG\x1a",4))erro("bad psg file");
		for(t=5,tick=0;t<sb.st_size;t++)if(ibuf[t]==0xff)tick++;
		printf("Length:  %lu min, %lu sec\n",tick/50L/60L,tick/50L%60L);
		lp=sb.st_size-4;q=0;
		break;
	    case AY:
		puts("AY file");
		ibuf=tt1+12;co=1;
		printf("Misc:    %s\n",ibuf+PTR(ibuf));
		printf("%s%s\n",author,ibuf+2+PTR(ibuf+2));
		ibuf+=6;ibuf+=PTR(ibuf);
		printf("%s%s\n",name,ibuf+PTR(ibuf));
		if(tick)printf("Length:  %lu min, %lu sec\n",tick/50L/60L,tick/50L%60L);
		lp=0;q=0;
		break;
	    case PT1:
		puts("Protracker 1.x");
		xstr(name,sngadr+69,NULL,30);
		lp=0;q=0;
		break;
	    case PT2:
		puts("Protracker 2.x");
		xstr(name,sngadr+101,NULL,30);
		lp=0;q=0;
		break;
	    case PT3:
		q=15;if(co==2)sngadr=*(_US*)(DANM(mem)+iadr+1);
		if(!strncasecmp((char*)(DANM(mem)+sngadr),"Vortex",6))q=23;
		xstr(NULL,sngadr,NULL,q);
		xstr(name,sngadr+30,NULL,32);
		xstr(author,sngadr+66,NULL,32);
		lp=0;q=0;
		break;
	    case STP:
		puts("Sound Tracker Pro");
		i=xfind(sngadr,"KSA ",4);
		if(i)xstr(name,sngadr+i+28,NULL,25);
		    else {
			i=xfind(sadr,"KSA ",4);
			if(i)xstr(name,sadr+i+28,NULL,25);
		}
		lp=0;q=0;
		break;
	    case STC:
		puts("Sound Tracker");
//???		if(*(_UC*)(DANM(mem)+sadr+49)>=32) {
//			xstr(name,sadr+49,NULL,10);
//			xstr(author,sadr+63,NULL,12);
//		}
		if((*(_UC*)(DANM(mem)+sngadr+7)>=32)&&strncasecmp((char*)(DANM(mem)+sngadr+7),"SONG BY ST C",12)&&strncasecmp((char*)(DANM(mem)+sngadr+7),"SOUND TR",8))
		    xstr(name,sngadr+7,NULL,18);
		lp=0;q=0;
		break;
	    case PSC:
		xstr(NULL,sngadr,NULL,9);
		xstr(name,sngadr+25,NULL,19);
		xstr(author,sngadr+49,NULL,19);
		lp=0;q=0;
		break;
	    case ASC:
		puts("ASC Sound Master");
		i=xfind(sngadr,"ASM ",4);
		if(i) {
			xstr(name,sngadr+i+19+1*(*(DANM(mem)+sngadr+i+19)==0xfd),NULL,20);
			xstr(author,sngadr+i+43,NULL,20);
		} else if(*(_UC*)(DANM(mem)+sadr+39)>=32) {
			i=xstr(name,sadr+39," BY ",4);
			xstr(author,sadr+39+i,NULL,32);
		}
		lp=0;q=0;
		break;
	    case GTR:
		puts("Global Tracker");
		xstr(name,sngadr+7,NULL,32);
		lp=0;q=0;
		break;
	    case FTC:
		puts("Fast Tracker");
		xstr(name,sngadr+8,NULL,42);
		lp=0;q=0;
		break;
	    case SQT:
		puts("SQ-Tracker");
		lp=0;q=0;
		break;
	    case FLS:
		puts("Flash Tracker");
		lp=0;q=0;
		break;
	    case FXM:
		puts("Fuxoft AY language");
		lp=0;q=0;
		break;
	    default:
		puts("unknown format");
		exit(-1);
		break;
	}
	t=0;printf("Playing..\n\n");
	while(!quitflag
#ifdef WIN
	    &&!_bios_keybrd(_KEYBRD_READY)
#endif
	    ) {
		switch(ft) {
		    case VTX:
			playvtx();
			break;
		    case PSG:
			playpsg();
			break;
		    case PT1: case PT2: case PT3:
		    case STP: case STC: case PSC:
		    case ASC: case GTR: case FTC:
		    case SQT: case FLS: case AY: case FXM:
			playemu();
			break;
		}
		indik();
#ifndef LPT_PORT
#ifndef ADLIB
		sound_frame(1);
#endif
#endif
		XSLEEP;
	}
	if(tt1)free(tt1);
	if(tt2)free(tt2);
	sreg(8,0);sreg(9,0);sreg(10,0);
#ifndef LPT_PORT
	sound_end();
#endif
	printf("\nExiting..\n");
#ifdef UNIX
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
#else
	getch();
#endif
	return 0;
}
