//(c)2003 sisoft\trg - AYplayer.
/* $Id: ayplay.c,v 1.20 2003/10/30 08:54:14 root Exp $ */
#include "ayplay.h"
#include "z80.h"

_US sp;
_UC *ibuf,*obuf;
_UL origsize,compsize,count,q,tick,t=0,lp;
enum {UNK=0,VTX,PSG,AY,YM,HOB,PT2,PT3,STP,STC,PSC,ASC} formats;
static char name[]="Name:    ",author[]="Author:  ";
static int quitflag=0;
int ca,cb,cc,ft=UNK;
#define PLADR 18432

#define WRD(x) ((*(_UC*)(x))*256+(*(_UC*)((x)+1)))
#define PTR(x) ((*(char*)(x))*256+(*(char*)((x)+1)))

void erro(char *ermess)
{
	if(ermess)printf("\n* Error: %s!\n",ermess);
	    else puts("* Support: VTX, PSG, AY, YM, PT2, PT3, STP, STC, PSC, ASC, Hobeta.\n* Usage: ayplayer filename");
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
#ifdef UNIX
	ioperm(dPort,3,1);
#endif
	outb(reg,dPort);
	outb(6,Port);
	outb(15,Port);
	outb(dat,dPort);
	outb(14,Port);
	outb(15,Port);
	outb(0,dPort);
	outb(6,Port);
	outb(15,Port);
#ifdef UNIX
	ioperm(dPort,3,0);
#endif
}

void playvtx()
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

void playpsg()
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

void playemu()
{
	_UC i;
	int r=-1,v=-1;
	DANM(haltstate)=0;
	PC=PLADR+4;SP=sp;
	while(!DANM(haltstate)) {
		DANM(r)=128;DANM(v)=128;
//		printf("pc=%u,sp=%u\n",PC,SP);
		PRNM(step)(1);
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

void indik()
{
	int i;
	char a[16]={"               "};
	char b[16]={"               "};
	char c[16]={"               "};
	for(i=0;i<(ca&15);i++)a[i]='=';if(i)a[(ca&15)-1]='-';
	for(i=0;i<(cb&15);i++)b[i]='=';if(i)b[(cb&15)-1]='-';
	for(i=0;i<(cc&15);i++)c[i]='=';if(i)c[(cc&15)-1]='-';
	switch(ft) {
	    case VTX: 
		printf("%02lu:%02lu   A: %s   B: %s   C: %s\r",t/q/60L,t/q%60L,a,b,c);
		break;
	    default:
		printf("%02lu:%02lu   A: %s   B: %s   C: %s\r",q/50L/60L,q/50L%60L,a,b,c);
		break;
	}
	fflush(stdout);
}

_UC *vtxinfo(char *buf)
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
		if(n)printf(n);
		while(i<l&&DANM(mem)[sa+i]==' ')i++;
		fwrite(DANM(mem)+sa+i,l-i,1,stdout);
		printf("\n");
	}
	if(e)l+=x;
	return l;
}

int main(int argc,char *argv[])
{
	FILE *infile;
	struct stat sb;
	char *nam=NULL;
	char hdr[17]={0};
	_UC *tt1=NULL,*tt2=NULL;
	_US sadr=0,iadr=0,padr=0,sngadr=0,i;
	puts("\n\tAY Player'2003, for real AY chip on LPT port");
	puts("(c)Stepan Pologov (sisoft\\TRG), 2:5050/125, sisoft@udm.net");
	if(argc!=2||strchr(argv[1],'.')==NULL)erro(NULL);
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
		sprintf(cmd,"gzip -cd %s >%s",argv[1],nam);
		if(system(cmd))erro("can't gzip sound file");
	}
	if(stat(nam?nam:argv[1],&sb))erro("can't stat sound file");
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
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".stp"))ft=STP;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".stc"))ft=STC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".zxs"))ft=STC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".psc"))ft=PSC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".asc"))ft=ASC;
		else if(!strncasecmp(strrchr(nam?nam:argv[1],'.'),".$",2))ft=HOB;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".ay"))ft=AY;
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
			    case HOB:
				fread(hdr,17,1,infile);
				sb.st_size=*(_US*)(hdr+11);
				switch(hdr[8]) {
				    case 'M': ft=PT2;break;
				    case 'm': ft=PT3;break;
				}
				if(ft!=HOB)goto again;
				iadr=sadr=*(_US*)(hdr+9);
				padr=iadr+5;ft=PT3;
				fread(DANM(mem)+sadr,sb.st_size,1,infile);
				if(DANM(mem)[padr]!=0xc3){padr++;ft=PT2;}
				if(!memcmp(DANM(mem)+sadr+17,"KSA SOFT",8))ft=STP;
				if(!memcmp(DANM(mem)+sadr+20,"SOUND TR",8)){ft=STC;iadr=sadr+11;padr=iadr+3;}
				if(!memcmp(DANM(mem)+sadr+20,"ASM COMP",8)){ft=ASC;iadr=sadr+11;padr=iadr+3;}
				if(!memcmp(DANM(mem)+sadr+9,"PSC ",4))ft=PSC;
				sngadr=*(_US*)(DANM(mem)+iadr+1);
//				printf("hob: s: %u, l: %lu, i: %u, p: %u, sng: %u\n",sadr,sb.st_size,iadr,padr,sngadr);
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
				fread(DANM(mem)+PT3_song,sb.st_size,1,infile);
//				memcpy(DANM(mem)+PT3_table,pt3_tables+192*(*(_UC*)(DANM(mem)+PT3_song+99)),192);
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
	printf("Type:    %s",nam?"packed ":"");
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
		struct {int l;_UC *b;} ds[256];
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
		for(t=5,tick=0;t<sb.st_size;t++)if(ibuf[t]==0xff)tick++;
		printf("Length:  %lu min, %lu sec\n",tick/50L/60L,tick/50L%60L);
		lp=sb.st_size-4;q=0;
		break;
	    case AY:
		puts("AY file");
		ibuf=tt1+12;
		printf("Misc:    %s\n",ibuf+PTR(ibuf));
		printf("%s%s\n",author,ibuf+2+PTR(ibuf+2));
		ibuf+=6;ibuf+=PTR(ibuf);
		printf("%s%s\n",name,ibuf+PTR(ibuf));
		if(tick)printf("Length:  %lu min, %lu sec\n",tick/50L/60L,tick/50L%60L);
		lp=0;q=0;
		break;
	    case PT2:
		puts("Protracker 2.x");
		xstr(name,sngadr+101,NULL,30);
		lp=0;q=0;
		break;
	    case PT3:
		xstr(NULL,sngadr,NULL,15);
		xstr(name,sngadr+30,NULL,32);
		xstr(author,sngadr+66,NULL,32);
		lp=0;q=0;
		break;
	    case STP:
		puts("Sound Tracker Pro");
		if(!memcmp(DANM(mem)+sngadr+10,"KSA ",4))
		    xstr(name,sngadr+38,NULL,24);
			else if(*(_UC*)(DANM(mem)+sadr+45)>=32)
			    xstr(name,sadr+45,NULL,25);
		lp=0;q=0;
		break;
	    case STC:
		puts("Sound Tracker");
		if(*(_UC*)(DANM(mem)+sadr+49)>=32) {
			xstr(name,sadr+49,NULL,10);
			xstr(author,sadr+63,NULL,12);
		}
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
			xstr(name,sngadr+i+19,NULL,20);
			xstr(author,sngadr+i+43,NULL,20);
		} else if(*(_UC*)(DANM(mem)+sadr+39)>=32) {
			i=xstr(name,sadr+39," BY ",4);
			xstr(author,sadr+39+i,NULL,32);
		}
		lp=0;q=0;
		break;
	    default:
		puts("unknown format");
		exit(-1);
		break;
	}
	t=0;printf("Playing..\n\n");
	while(!quitflag) {
		switch(ft) {
		    case VTX: playvtx();break;
		    case PSG: playpsg();break;
		    case PT2: case PT3:
		    case STP: case STC:
		    case PSC: case ASC:
		    case AY:
			playemu();break;
		}
		indik();
		usleep(Sleep);
	}
	if(tt1)free(tt1);
	if(tt2)free(tt2);
	sreg(8,0);sreg(9,0);sreg(10,0);
	printf("\nExiting..\n");
#ifdef UNIX
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
#endif
	return 0;
}
