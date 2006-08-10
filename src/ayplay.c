/* AYplayer (c)2001-2006 sisoft//trg.
\* $Id: ayplay.c,v 1.9 2006/08/10 03:13:52 root Exp $ */
#include "ayplay.h"
#include "z80.h"

enum {
    UNK=0,VTX,PSG,AY,AYM,YM,HOB,PT1,PT2,PT3,STP,STC,PSC,ASC,GTR,FTC,SQT,FLS,FXM
} formats;
static _US sp;
static _UC *ibuf,*obuf,*mem;
static _UL origsize,compsize,q,tick=0,t=0,lp;
static char *name,*author,*comm;
static int quitflag=0,ca,cb,cc,ft=UNK;

#define PLADR 10240
#define WRD(x) (((*(_UC*)(x))<<8)+(*(_UC*)((x)+1)))
#define DWRD(x) (((*(_UC*)(x))<<24)+((*(_UC*)((x)+1))<<16)+((*(_UC*)((x)+2))<<8)+(*(_UC*)((x)+3)))
#define PTR(x) (((*(char*)(x))<<8)+(*(char*)((x)+1)))

void erro(char *ermess)
{
	if(ermess)printf(_("\n* Error: %s!\n"),ermess);
	else printf(_("* Support: %s.\n* Usage: ayplayer filename [part]\n"),
	"VTX,PSG,AY,AYM,YM,PTx,STP,STC,ZXS,PSC,ASC,GTR,FTC,SQT,FLS,FXM,Hobeta");
	exit(-1);
}

#ifdef UNIX
static void sighup(int sig)
{
	signal(sig,sighup);
	quitflag=1;
}
#ifdef USE_ITIMER
static void sigalrm(int sig)
{
	signal(sig,sigalrm);
}
#endif
#endif

void sreg(char reg,_UC dat)
{
#ifdef LPT_PORT
	outb(reg,LPT_PORT);
	outb(6,LPT_PORT+2);
	outb(15,LPT_PORT+2);
	outb(dat,LPT_PORT);
	outb(14,LPT_PORT+2);
	outb(15,LPT_PORT+2);
	outb(0,LPT_PORT);
	outb(6,LPT_PORT+2);
	outb(15,LPT_PORT+2);
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
		else if(i==8)ca=a;
		else if(i==9)cb=a;
		else if(i==10)cc=a;
		if(i!=13||a!=255)sreg(i,a);
	}
	if(++t>=tick)t=lp;
}

static void playpsg()
{
	_UC a,i=0;
	static _US nosn=0;
	if(nosn>0)nosn--;
	else while((i=ibuf[t+++4])<0xfd&&t<lp) {
		a=ibuf[t+++4];
		if(i==7)a|=192;
		else if(i==8)ca=a;
		else if(i==9)cb=a;
		else if(i==10)cc=a;
		if(i<14)sreg(i,a);
	}
	if(i==0xfe)nosn=ibuf[t+++4]<<2;
	if(++q>=tick||t>=lp||i==0xfd)t=q=0;
}

static void playemu()
{
	_UC i;
	int r=-1,v=-1;
	DANM(haltstate)=0;
	PC=PLADR+4;SP=sp;
	while(!DANM(haltstate)) {
		DANM(r)=128;DANM(v)=128;
		/*printf("pc=%u,sp=%u\n",PC,SP);*/
		PRNM(step)(0);
		if((i=DANM(r))!=128)r=i;
		if((i=DANM(v))!=128)v=i;
		if(r>=0&&v>=0) {
			if(r==7)v|=192;
			else if(r==8)ca=v;
			else if(r==9)cb=v;
			else if(r==10)cc=v;
			if(r<14)sreg(r,v);
			v=-1;r=-1;
		}
	}
	if(++q>tick&&tick)q=lp;
}

static void indik()
{
	int i;
	static unsigned sn=0;
	char a[18],b[18],c[18],s;
	static _UC ma=0,mb=0,mc=0;
	static _US mca=0,mcb=0,mcc=0;
	static char slashes[]=":::/-\\|/-\\:::\\-/|\\-/";
	*a=ca>15?ca>16?'>':'<':' ';
	for(i=0;i<(ca&15);i++)a[i+1]='=';
	if(i>=ma)ma=i,mca=0;
	else if(ma&&mca++&&!(mca%(ma/2+1)))ma--;
	if(i)a[i++]='-';
	for(;i<17;i++)a[i]=' ';a[i]=0;
	if(ma&&a[ma]==' ')a[ma]=POINT;
	*b=cb>15?cb>16?'>':'<':' ';
	for(i=0;i<(cb&15);i++)b[i+1]='=';
	if(i>=mb)mb=i,mcb=0;
	else if(mb&&mcb++&&!(mcb%(mb/2+1)))mb--;
	if(i)b[i++]='-';
	for(;i<17;i++)b[i]=' ';b[i]=0;
	if(mb&&b[mb]==' ')b[mb]=POINT;
	*c=cc>15?cc>16?'>':'<':' ';
	for(i=0;i<(cc&15);i++)c[i+1]='=';
	if(i>=mc)mc=i,mcc=0;
	else if(mc&&mcc++&&!(mcc%(mc/2+1)))mc--;
	if(i)c[i++]='-';
	for(;i<17;i++)c[i]=' ';c[i]=0;
	if(mc&&c[mc]==' ')c[mc]=POINT;
	s=slashes[(++sn)%100/5];
	switch(ft) {
	    case YM: 
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
	printf(_("Chip:    %s\n"),*buf++=='a'?"AY-3-8910(12)":"YM2149F");
	printf(_("Regime:  "));
	switch(*++buf)
	{
		case 0: puts(_("Mono")); break;
		case 1: puts(_("ABC stereo")); break;
		case 2: puts(_("ACB stereo")); break;
		case 3: puts(_("BAC stereo")); break;
		case 4: puts(_("BCA stereo")); break;
		case 5: puts(_("CAB stereo")); break;
		case 6: puts(_("CBA stereo")); break;
		default:erro(_("unknown mode"));
	}
	lp=*(_US*)(++buf);
	buf+=6;q=*buf++;
	yea=*(_US*)buf++;
	if(yea)printf(_("Year:    %u\n"),yea);
	origsize=*(_UL*)++buf;buf+=4;
	if(!origsize)erro(_("playable data is empty"));
	if(strlen(buf))printf("%s%s\n",name,buf);buf+=strlen(buf);
	if(strlen(++buf))printf("%s%s\n",author,buf);buf+=strlen(buf);
	if(strlen(++buf))printf(_("Origin:  %s\n"),buf);buf+=strlen(buf);
	if(strlen(++buf))printf(_("Editor:  %s\n"),buf);buf+=strlen(buf);
	if(strlen(++buf))printf("%s%s\n",comm,buf);buf+=strlen(buf);
	tick=origsize/14;
	return((_UC*)(++buf));
}

static _US xfind(_US sa,char *s,int l)
{
	_US f=0;
	while(memcmp(mem+sa+f,s,l)&&f<256)f++;
	if(f>255)f=0;
	return f;
}

static _US xstr(char *n,_US sa,char *e,_US x)
{
	_US l=x,i=0;
	if(e)l=xfind(sa,e,x);
	if(l) {
		while(i<l&&mem[sa+i]==' ')i++;
		if(i<l) {
			if(n)printf(n);
			fwrite(mem+sa+i,1,l-i,stdout);
			printf("\n");
		}
	}
	if(e)l+=x;
	return l;
}

static void play_frame()
{
	switch(ft) {
	    case YM:
	    case VTX:
		playvtx();
		break;
	    case PSG:
		playpsg();
		break;
	    default:
		playemu();
		break;
	}
	indik();
#ifndef LPT_PORT
#ifndef ADLIB
	sound_frame(1);
#endif
#endif
}

int main(int argc,char *argv[])
{
	int co=0,ps=0;
	FILE *infile;
	struct stat sb;
	char *nam=NULL,*s;
	_UC *tt1=NULL,*tt2=NULL;
	_US sadr=0,iadr=0,padr=0,sngadr=0,i;
#ifdef USE_ITIMER
	struct itimerval it;
#endif
	puts("\n AY sound player for real AY chip on LPT port");
	puts("(c) Stepan Pologov (sisoft//trg), sisoft@bk.ru");
	GT_INIT;
	if((argc!=2&&(argc!=3||(*argv[2]&0xf0)!=0x30))||!strchr(argv[1],'.'))erro(NULL);
#ifndef LPT_PORT
	if(!sound_init())erro(_("can't init soundcard"));
#else
#ifdef HAVE_IOPERM
	ioperm(LPT_PORT,3,1);
#endif
#endif
	comm=_("Comment: ");
	name=_("Name:    ");
	author=_("Author:  ");
	if(!strcmp(argv[1],".")) {
		sb.st_size=DEMO_S;ft=DEMO_T;
		if(!(ibuf=tt1=malloc(sb.st_size)))erro(_("out of memory"));
		memcpy(ibuf,DEMO_D,DEMO_S);
		puts(_("\nFile:    AYPlayer demo song"));
		goto playz;
	}
#ifdef UNIX
	if(!strcasecmp(strrchr(argv[1],'.'),".gz")) {
		char cmd[256];
		nam=tmpnam(NULL);
		strncat(nam,strchr(argv[1],'.'),4);
		snprintf(cmd,256,"gzip -cd %s >%s 2>/dev/null",argv[1],nam);
		if(system(cmd)){unlink(nam);erro(_("can't gzip sound file"));}
	}
#endif
	if(argc==3)ps=atoi(argv[2]);
	if(stat(nam?nam:argv[1],&sb))erro(_("can't stat sound file"));
	if(sb.st_size<128)erro(_("file is empty"));
	if(!(s=strrchr(nam?nam:argv[1],'.')))erro(_("no file extension"));
	if(!strcasecmp(++s,"vtx"))ft=VTX;
	else if(!strcasecmp(s,"psg"))ft=PSG;
	else if(!strcasecmp(s,"ym"))ft=YM;
	if(ft) {
		if(!(ibuf=tt1=malloc(sb.st_size)))erro(_("out of memory"));
		if(!(infile=fopen(nam?nam:argv[1],"rb")))erro(_("can't open sound file"));
		fread(ibuf,1,sb.st_size,infile);
		fclose(infile);
	} else {
		if(!strcasecmp(s,"pt2"))ft=PT2;
		else if(!strcasecmp(s,"pt3"))ft=PT3;
		else if(!strcasecmp(s,"pt1"))ft=PT1;
		else if(!strcasecmp(s,"stp"))ft=STP;
		else if(!strcasecmp(s,"stc"))ft=STC;
		else if(!strcasecmp(s,"zxs"))ft=STC;
		else if(!strcasecmp(s,"psc"))ft=PSC;
		else if(!strcasecmp(s,"asc"))ft=ASC;
		else if(!strcasecmp(s,"gtr"))ft=GTR;
		else if(!strcasecmp(s,"ftc"))ft=FTC;
		else if(!strcasecmp(s,"sqt"))ft=SQT;
		else if(!strcasecmp(s,"fls"))ft=FLS;
		else if(!strcasecmp(s,"fxm"))ft=FXM;
		else if(!strcasecmp(s,"aym"))ft=AYM;
		else if(!strcasecmp(s,"ay"))ft=AY;
		else if(!strcmp(s,"m"))ft=PT3;
		else if(!strcmp(s,"M"))ft=PT2;
		else if(*s=='$'&&s[1])ft=HOB;
		if(ft) {
			PRNM(init)();
			PRNM(reset)();
			mem=DANM(mem);
			for(i=0;i<256;i++)mem[i]=0xc9;
			for(;i<16384;i++)mem[i]=0xff;
			mem[0]=0x76;mem[0x38]=0xfb;
			mem[PLADR]=0xcd;
			mem[PLADR+3]=0x76;
			mem[PLADR+4]=0xcd;
			mem[PLADR+7]=0x76;
			sp=PLADR+2048;
			if(!(infile=fopen(nam?nam:argv[1],"rb")))erro(_("can't open sound file"));
again:			switch(ft) {
			    case HOB: {
				char hdr[17]={0};
				unsigned short crc;
				fread(hdr,1,17,infile);
				if(!strncasecmp(hdr,"ProTracker ",10)){ft=PT3;co=0;fseek(infile,0,SEEK_SET);goto again;}
				for(crc=0,i=0;i<15;++i)crc+=(_UC)hdr[i]*257+i;
				if(crc!=*(_US*)(hdr+15))puts(_("\nWarn: corrupted hobeta file!"));
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
				    case 'S': erro(_("Uncompiled SoundTracker modules is not supported"));
				    case 'C': if(sb.st_size!=6912)break;
				    default : erro(_("Extension of this file is not supported"));
				}
				if(ft!=HOB)goto again;
				iadr=sadr=*(_US*)(hdr+9);
				if(iadr<16384)iadr=49152;
				padr=iadr+5;ft=PT3;co=1;
				fread(mem+sadr,1,sb.st_size,infile);
				sngadr=*(_US*)(mem+iadr+1);
				if(mem[padr]!=0xc3){padr++;ft=PT2;}
				if(*(_US*)(mem+sadr)==0x83e){padr=iadr+48;ft=SQT;}
				if(!memcmp(mem+sadr+17,"KSA SOFT",8)) {
					if((*(_US*)(mem+sadr+4)-sadr)>256) {
						iadr=*(_US*)(mem+sadr+4)-0x4e;
						memcpy(mem+iadr,mem+sadr,sb.st_size);
						sadr=iadr;padr=iadr+6;
					}
					ft=STP;
				}
				if(!memcmp(mem+sadr+20,"SOUND TR",8)){ft=STC;iadr=sadr+11;padr=iadr+3;}
				if(!memcmp(mem+sadr+1091,"SONG BY ",8)){ft=STC;iadr=sadr;padr=iadr+6;}
				if(!memcmp(mem+sadr+7,"SONG BY ST",10)){ft=STC;co=0;fseek(infile,17,SEEK_SET);goto again;}
				if(!memcmp(mem+sadr+20,"ASM COMP",8)){ft=ASC;iadr=sadr+11;padr=iadr+3;}
				if(xfind(sadr,"ASM COMP",8)){ft=ASC;co=0;fseek(infile,17,SEEK_SET);goto again;}
				if(!memcmp(mem+sadr,"ProTrack",8)){ft=PT3;co=0;fseek(infile,17,SEEK_SET);goto again;}
				if(!memcmp(mem+sadr+77,"PT 3 P",6)){ft=PT3;iadr=sadr;padr=iadr+5;co=2;}
				if(!memcmp(mem+sadr+9,"PSC ",4)){ft=PSC;iadr=sadr;padr=iadr+6;}
				if(ft==PT2&&mem[sadr+131+mem[sadr+1]]==0xff){co=0;fseek(infile,17,SEEK_SET);goto again;}
				/*printf("hob: s: %u, l: %lu, i: %u, p: %u, sng: %u\n",sadr,sb.st_size,iadr,padr,sngadr);*/
				} break;
			    case PT1:
				memcpy(mem+PT1_init,pt1_player,PT1_song-PT1_init);
				fread(mem+PT1_song,1,sb.st_size,infile);
				iadr=PT1_init;
				padr=PT1_play;
				sngadr=PT1_song;
				break;
			    case PT2:
				memcpy(mem+PT2_init,pt2_player,PT2_song-PT2_init);
				fread(mem+PT2_song,1,sb.st_size,infile);
				iadr=PT2_init;
				padr=PT2_play;
				sngadr=PT2_song;
				break;
			    case PT3:
				memcpy(mem+PT3_init,pt3_player,PT3_song-PT3_init);
				fread(mem+PT3_song,1,sb.st_size,infile);i=6;
				if(mem[PT3_song+13]>='0'&&mem[PT3_song+13]<='9')
				    i=mem[PT3_song+13]-'0';
				switch(mem[PT3_song+99]) {
				    case 0: if(i<=3)i=0; else i=1; break;
				    case 1: i=2; break;
				    case 2: if(i<=3)i=3; else i=4; break;
				    default: if(i<=3)i=5; else i=6; break;
				}
				memcpy(mem+PT3_table,pt3_tables+96*i,192);
				iadr=PT3_init;
				padr=PT3_play;
				sngadr=PT3_song;
				break;
			    case STP:
				memcpy(mem+STP_init,stp_player,STP_song-STP_init);
				fread(mem+STP_song,1,sb.st_size,infile);
				iadr=STP_init;
				padr=STP_play;
				sngadr=STP_song;
				break;
			    case STC:
				memcpy(mem+STC_start,stc_player,STC_song-STC_start);
				fread(mem+STC_song,1,sb.st_size,infile);
				sadr=STC_start;
				iadr=STC_init;
				padr=STC_play;
				sngadr=STC_song;
				break;
			    case PSC:
				memcpy(mem+PSC_init,psc_player,PSC_song-PSC_init);
				fread(mem+PSC_song,1,sb.st_size,infile);
				iadr=PSC_init;
				padr=PSC_play;
				sngadr=PSC_song;
				break;
			    case ASC:
				memcpy(mem+ASC_start,asc_player,ASC_song-ASC_start);
				fread(mem+ASC_song,1,sb.st_size,infile);
				sadr=ASC_start;
				iadr=ASC_init;
				padr=ASC_play;
				sngadr=ASC_song;
				break;
			    case GTR:
				memcpy(mem+GTR_init,gtr_player,GTR_song-GTR_init);
				fread(mem+GTR_song,1,sb.st_size,infile);
				iadr=GTR_init;
				padr=GTR_play;
				sngadr=GTR_song;
				break;
			    case FTC:
				memcpy(mem+FTC_init,ftc_player,FTC_song-FTC_init);
				fread(mem+FTC_song,1,sb.st_size,infile);
				iadr=FTC_init;
				padr=FTC_play;
				sngadr=FTC_song;
				break;
			    case SQT: {
				_US j=0,k,*p,fl;
				memcpy(mem+SQT_init,sqt_player,SQT_song-SQT_init);
				fread(mem+SQT_song,1,sb.st_size,infile);
				i=*(_US*)(mem+SQT_song+2)-10;
				k=*(_US*)(mem+SQT_song+8)-i;
				while(mem[SQT_song+k]) {
					if(j<(mem[SQT_song+k]&0x7f))j=mem[SQT_song+k]&0x7f;k+=2;
					if(j<(mem[SQT_song+k]&0x7f))j=mem[SQT_song+k]&0x7f;k+=2;
					if(j<(mem[SQT_song+k]&0x7f))j=mem[SQT_song+k]&0x7f;k+=3;
				}
				p=(_US*)(mem+SQT_song+2);
				fl=(*(_US*)(mem+SQT_song+6)-i+(j<<1))/2;
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
				memcpy(mem+FLS_init,fls_player,FLS_song-FLS_init);
				fread(mem+FLS_song,1,sb.st_size,infile);
				i3=*(_US*)(mem+FLS_song+2)-16;
				if(i3>=0) do {
					i2=*(_US*)(mem+FLS_song+4)-i3+2;
					if(i2>=8&&i2<song_len) {
						p=(_US*)(mem+FLS_song+i2);
						i1=*p-i3;
						if(i1>=8&&i1<song_len) {
							p=(_US*)(mem+FLS_song+i2-4);
							i2=*p-i3;
							if(i2>=6&&i2<song_len)
							    if(i1-i2==0x20) {
								i2=*(_US*)(mem+FLS_song+8)-i3;
								if(i2>21&&i2<song_len) {
									i1=*(_US*)(mem+FLS_song+6)-i3;
									if(i1>20&&i1<song_len)
									    if(!mem[FLS_song+i1-1]) {
										while(i1<song_len&&mem[FLS_song+i1]!=255)
										    do {
											if(mem[FLS_song+i1]<=0x5f||mem[FLS_song+i1]==0x80||mem[FLS_song+i1]==0x81) {
												i1++;
												break;
											}
											if(mem[FLS_song+i1]>=0x82&&mem[FLS_song+i1]<=0x8e)i1++;
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
				if(i3<0)erro(_("bad fls file"));
				p=(_US*)(mem+FLS_song);
				i1=*(_US*)(mem+FLS_song+4)-i3+(long)p;
				i2=*(_US*)(mem+FLS_song)-i3+(long)p+2;
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
				if(!(tt1=malloc(sb.st_size)))erro(_("out of memory"));
				fread(tt1,1,sb.st_size,infile);
				if(strncmp((char*)tt1,"ZXAY",4))erro(_("unknown format"));
				if(strncmp((char*)(tt1+4),"EMUL",4))erro(_("bad file format"));
				if(ps<1)ps=tt1[17]+1;
				if(ps-1>tt1[16])ps=tt1[16]+1;
				ibuf=tt1+18;
				ibuf+=PTR(ibuf)+4*ps-2;
				ibuf+=PTR(ibuf)+4;
				tick=WRD(ibuf);
				RA=ABK=RH=HBK=RD=DBK=RB=BBK=XH=YH=ibuf[4];
				RF=FBK=RL=LBK=RE=EBK=RC=CBK=XL=YL=ibuf[5];
				ibuf+=6;obuf=ibuf+PTR(ibuf);
				ibuf+=PTR(ibuf+2)+2;
				if(WRD(obuf))sp=WRD(obuf);
				iadr=WRD(obuf+2);
				padr=WRD(obuf+4);
				do {
					i=WRD(ibuf+2);
					if(i+WRD(ibuf)>65536)i=65536-WRD(ibuf);
					if(i+4+PTR(ibuf+4)+ibuf-tt1>sb.st_size)
					    i=sb.st_size-(4+PTR(ibuf+4)+ibuf-tt1);
					/*printf("to=%u,from=%u,len=%u\n",WRD(ibuf),4+PTR(ibuf+4)+ibuf-tt1,i);*/
					if(i)memcpy(mem+WRD(ibuf),ibuf+4+PTR(ibuf+4),i);
					if(!iadr)iadr=WRD(ibuf);ibuf+=6;
				} while(WRD(ibuf));
				/*printf("iadr=%u,padr=%u,sp=%u\n",iadr,padr,sp);*/
				RI=3;
				break;
			    case AYM:
				if(!(tt1=malloc(sb.st_size)))erro(_("out of memory"));
				fread(tt1,1,sb.st_size,infile);
				if(strncmp((char*)tt1,"AYM0",4))erro(_("unknown format"));
				ibuf=tt1+0x45;
				iadr=*(_US*)(tt1+0x30);
				padr=*(_US*)(tt1+0x32);
				if(ps<1)ps=tt1[0x36]+1;
				if(ps-1<tt1[0x34])ps=tt1[0x34]+1;
				if(ps-1>tt1[0x35])ps=tt1[0x35]+1;
				tt1[0x38+tt1[0x37]]=ps-1;
				AF=*(_US*)(tt1+0x38);BC=*(_US*)(tt1+0x3a);
				DE=*(_US*)(tt1+0x3c);HL=*(_US*)(tt1+0x3e);
				IX=*(_US*)(tt1+0x40);IY=*(_US*)(tt1+0x42);
				for(i=0;i<tt1[0x44];i++) {
					/*printf("to=%u,from=%u,len=%u\n",*(_US*)ibuf,ibuf+4-tt1,*(_US*)(ibuf+2));*/
					memcpy(mem+*(_US*)ibuf,ibuf+4,*(_US*)(ibuf+2));
					ibuf+=*(_US*)(ibuf+2)+4;
				}
				break;
			    case FXM: {
				char bu[6];
				memcpy(mem+FXM_init,fxm_player,FXM_song-FXM_init);
				fread(bu,1,6,infile);
				if(strncmp(bu,"FXSM",4))erro(_("bad file format"));
				sngadr=*(unsigned short*)(bu+4);
				fread(mem+sngadr,1,sb.st_size-6,infile);
				memcpy(mem+FXM_song,mem+sngadr,sb.st_size-6);
				iadr=FXM_init;
				padr=FXM_play;
				} break;
			}
			*(_US*)(mem+PLADR+1)=(_US)iadr;
			PC=PLADR;SP=sp;
			while(iadr&&!DANM(haltstate)){/*printf("pc=%u,sp=%u\n",PC,SP);*/PRNM(step)(1);}
			if(!padr)padr=mem[(dbyte)(((int)RI<<8)+0xFF)]+256*mem[(dbyte)(((int)RI<<8)+0xFF+1)]/*,printf("new padr=%d\n",padr)*/;
			*(_US*)(mem+PLADR+5)=(_US)padr;
			fclose(infile);
		}
	}
#ifdef UNIX
	if(nam)unlink(nam);
#endif
	printf(_("\nFile:    %s\n"),argv[1]);
playz:
#ifdef UNIX
	signal(SIGHUP,sighup);signal(SIGINT,sighup);
#ifdef USE_ITIMER
	signal(SIGALRM,sigalrm);
#endif
#endif
	printf(_("Type:    %s%s"),nam?_("packed "):"",co?_("compiled "):"");
#ifndef LPT_PORT
	sound_ay_reset();
#endif
	/*{FILE *f=fopen("dbg","wb");fwrite(mem,1,65536,f);fclose(f);}*/
	lp=0;q=50;
	switch(ft) {
	    case VTX:
		puts("Vortex Tracker");
		ibuf=vtxinfo((char*)ibuf);
		compsize=sb.st_size-(ibuf-tt1);
		if(!(tt2=obuf=calloc(14,tick)))erro(_("out of memory"));
		unlh5(ibuf,obuf,origsize,compsize);
		free(tt1);tt1=NULL;
		break;
	    case YM: {
		_UC *raw;
		_US i2,yv;
		if(memcmp(ibuf+2,"-lh5-",5))erro(_("unknown archive type"));
		compsize=*(long*)(ibuf+7);
		origsize=*(long*)(ibuf+11);
		ibuf+=*(_UC*)ibuf+2;
		if(!(tt2=obuf=calloc(1,origsize)))erro(_("out of memory"));
		unlh5(ibuf,obuf,origsize,compsize);
		free(tt1);tt1=NULL;
		/*{FILE *f=fopen("ym_dbg","wb");fwrite(tt2,1,origsize,f);fclose(f);}*/
		if(*obuf!='Y'||obuf[1]!='M'||strncmp((char*)(obuf+4),"LeOnArD!",8))erro(_("unknown format"));
		yv=obuf[2]-'0';if(yv==3&&obuf[3]=='b')yv=4;
		printf("YM%d file\n",yv);
		if(yv>=5) {
			tick=DWRD(obuf+12);
			i2=WRD(obuf+20);
			q=WRD(obuf+26);
			lp=DWRD(obuf+28);
			if(lp>=tick)lp=0;
			i=34+WRD(obuf+32);
			for(;i2>0;i2--)i+=DWRD(obuf+i)+4;
			if((i2=strlen((char*)obuf+i)))printf("%s%s\n",name,obuf+i);i+=i2+1;
			if((i2=strlen((char*)obuf+i)))printf("%s%s\n",author,obuf+i);i+=i2+1;
			if((i2=strlen((char*)obuf+i)))printf("%s%s\n",comm,obuf+i);i+=i2+1;
			if(DWRD(obuf+16)&1)yv=5;
			else yv=6;
		} else {
			tick=(origsize-(yv==4?8:4))/14;
			if(yv==4)lp=DWRD(obuf+4+tick*14);
			i=4;
		}
		if(!(raw=calloc(14,tick)))erro(_("out of memory"));
		if(yv<6)memcpy(raw,obuf+i,tick*14);
		else for(i2=0;i2<tick;i2++)
			for(yv=0;yv<14;yv++)
			    raw[yv*tick+i2]=obuf[i+i2*16+yv];
		free(tt2);tt2=obuf=raw;
		} break;
	    case PSG:
		puts("PSG file");
		if(memcmp(ibuf,"PSG\x1a",4))erro(_("bad psg file"));
		for(t=5,tick=0;t<(_UL)sb.st_size;t++) {
			if(ibuf[t]==0xff)tick++;
			else if(ibuf[t]==0xfe)t++,tick+=ibuf[t]<<2;
			else if(ibuf[t]==0xfd)break;
		}
		if(ibuf[4]>=10&&ibuf[4]<128&&ibuf[5]<128)q=ibuf[5];
		lp=sb.st_size-4;
		break;
	    case AY:
		puts("AY file");
		ibuf=tt1+12;co=1;
		if(ibuf[PTR(ibuf)])printf("%s%s\n",author,ibuf+PTR(ibuf));ibuf+=2;
		if(ibuf[PTR(ibuf)])printf(_("Misc:    %s\n"),ibuf+PTR(ibuf));
		printf(_("Songs #: %d (first=%d)\n"),ibuf[2]+1,ibuf[3]+1);
		ibuf+=PTR(ibuf+4)+4*ps;
		if(ibuf[PTR(ibuf)])printf("%s%s\n",name,ibuf+PTR(ibuf));
		break;
	    case AYM:
		puts("AYM file");co=1;
		memcpy(mem+PLADR+1024,tt1+4,44);
		xstr(name,PLADR+1024,NULL,28);
		xstr(author,PLADR+1052,NULL,16);
		printf(_("Songs #: %d (first=%d)\n"),tt1[0x35]-tt1[0x34]+1,tt1[0x36]+1);
		break;
	    case PT1:
		puts("Protracker 1.x");
		xstr(name,sngadr+69,NULL,30);
		break;
	    case PT2:
		puts("Protracker 2.x");
		xstr(name,sngadr+101,NULL,30);
		break;
	    case PT3:
		i=15;if(co==2)sngadr=*(_US*)(mem+iadr+1);
		if(!strncasecmp((char*)(mem+sngadr),"Vortex",6))i=23;
		xstr(NULL,sngadr,NULL,i);
		xstr(name,sngadr+30,NULL,32);
		xstr(author,sngadr+66,NULL,32);
		break;
	    case STP:
		puts("Sound Tracker Pro");
		i=xfind(sngadr,"KSA ",4);
		if(i)xstr(name,sngadr+i+28,NULL,25);
		    else {
			i=xfind(sadr,"KSA ",4);
			if(i)xstr(name,sadr+i+28,NULL,25);
		}
		break;
	    case STC:
		puts("Sound Tracker");
		if(mem[sngadr+7]>=32&&strncasecmp((char*)mem+sngadr+7,"SONG BY ST",10)&&strncasecmp((char*)mem+sngadr+7,"SOUND TRA",9))
		    xstr(name,sngadr+7,NULL,18);
		break;
	    case PSC:
		xstr(NULL,sngadr,NULL,9);
		xstr(name,sngadr+25,NULL,19);
		xstr(author,sngadr+49,NULL,19);
		break;
	    case ASC:
		puts("ASC Sound Master");
		i=xfind(sngadr,"ASM ",4);
		if(i) {
			xstr(name,sngadr+i+19+(mem[sngadr+i+19]==0xfd),NULL,20);
			xstr(author,sngadr+i+43,NULL,20);
		} else if(mem[sadr+39]>=32) {
			i=xstr(name,sadr+39," BY ",4);
			xstr(author,sadr+39+i,NULL,32);
		}
		break;
	    case GTR:
		puts("Global Tracker");
		xstr(name,sngadr+7,NULL,32);
		break;
	    case FTC:
		puts("Fast Tracker");
		xstr(name,sngadr+8,NULL,42);
		break;
	    case SQT:
		puts("SQ-Tracker");
		break;
	    case FLS:
		puts("Flash Tracker");
		break;
	    case FXM:
		puts("Fuxoft AY language");
		break;
	    default:
		erro(_("unknown format"));
		break;
	}
	if(tick)printf(_("Length:  %lu min, %lu sec\n"),tick/q/60,(tick/q)%60);
	if(ps>0&&ft!=AY&&ft!=AYM)ps=0;
	t=0;if(ps<2)puts(_("Playing..\n"));
	else printf(_("Playing part %d..\n\n"),ps);
#ifdef USE_ITIMER
	it.it_interval.tv_sec=0;
	it.it_interval.tv_usec=1000000/q;
	it.it_value=it.it_interval;
	setitimer(ITIMER_REAL,&it,NULL);
#else
	if(q!=50)puts(_("Warning: music may not play correctly!"));
#endif
	if(ft!=YM&&ft!=VTX)q=0;
	while(!quitflag
#ifndef UNIX
#ifndef WIN32
	    &&!_bios_keybrd(_KEYBRD_READY)
#endif
#endif
	    ) {
		play_frame();
		XSLEEP;
	}
	puts(_("\nExiting.."));
	if(tt1)free(tt1);
	if(tt2)free(tt2);
	sreg(8,0);sreg(9,0);sreg(10,0);
#ifndef LPT_PORT
	sound_end();
#else
#ifdef HAVE_IOPERM
	ioperm(LPT_PORT,3,0);
#endif
#endif
#ifdef UNIX
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
#else
#ifndef WIN32
	getch();
#endif
#endif
	return 0;
}
