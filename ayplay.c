//(c)2002 sisoft\trg - AYplayer.
/* $Id: ayplay.c,v 1.4 2003/05/28 09:39:52 root Exp $ */
#include "ayplay.h"

#define VTX 1
#define PSG 2

_UL origsize,compsize,count,q,tick,t,lp;
_UC *ibuf,*obuf;
int quitflag=0,ca,cb,cc,ft;

void erro(char *ermess)
{
	puts("\n\tAY Player'2002, for real AY chip on LPT port");
	puts("(c)Stepan Pologov (siSoft\\TRG), 2:5050/125, sisoft@udm.net");
	puts("* Usage: ayplayer filename");
	if(*ermess)printf("\n” Error: %s!\n",ermess);
	exit(-1);
}

void sighup(int sig)
{
	signal(sig,sighup);
	quitflag=1;
}

void sreg(char reg,_UC dat)
{
	ioperm(dPort,3,1);
	outb(reg,dPort);
	outb(6,Port);
	outb(15,Port);
	outb(dat,dPort);
	outb(14,Port);
	outb(15,Port);
	outb(0,dPort);
	outb(6,Port);
	outb(15,Port);
	ioperm(dPort,3,0);
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
	    case PSG:
		printf("%02lu:%02lu   A: %s   B: %s   C: %s\r",q/50L/60L,q/50L%60L,a,b,c);
		break;
	}
	fflush(stdout);
}

char *vtxinfo(char *buf)
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
	if(strlen(buf))printf("Name:    %s\n",buf);buf+=strlen(buf);
	if(strlen(++buf))printf("Author:  %s\n",buf);buf+=strlen(buf);
	if(strlen(++buf))printf("Origin:  %s\n",buf);buf+=strlen(buf);
	if(strlen(++buf))printf("Editor:  %s\n",buf);buf+=strlen(buf);
	if(strlen(++buf))printf("Comment: %s\n",buf);buf+=strlen(buf);
	tick=origsize/14L;printf("Length:  %lu min, %lu sec\n",tick/q/60L,tick/q%60L);
	if(q!=50)printf("Warning: music may not play correctly!\n");
	return(++buf);
}

int main(int argc,char *argv[])
{
	FILE *infile;
	char *nam=NULL;
	_UC *tt1,*tt2=NULL;
	struct stat sb;
	struct timespec ts;
	if(argc!=2)erro(NULL);
	if(!strcasecmp(strrchr(argv[1],'.'),".gz")) {
		char cmd[256];
		nam=tmpnam(NULL);
		snprintf(cmd,255,"gzip -cd %s >%s",argv[1],nam);
		if(system(cmd))erro("can't gzip sound file");
	}
	if(stat(nam?nam:argv[1],&sb))erro("can't stat sound file");
	if((ibuf=tt1=(_UC*)malloc(sb.st_size))==NULL)erro("out of memory");
	if((infile=fopen(nam?nam:argv[1],"rb"))==NULL)erro("can't open sound file");
	fread(ibuf,sb.st_size,1,infile);
	fclose(infile);if(nam)unlink(nam);
	signal(SIGHUP,sighup);signal(SIGINT,sighup);
	printf("\nFile:    %s%s\n",argv[1],nam?" (packed)":"");
	printf("Type:    ");
	if(*ibuf=='a'||*ibuf=='y') {
		puts("Vortex Tracker");ft=VTX;
		ibuf=vtxinfo(ibuf);
		compsize=sb.st_size-(ibuf-tt1);
		if((tt2=obuf=(_UC*)calloc(14,tick))==NULL)erro("out of memory");
		unlh5(ibuf,obuf,origsize,compsize);
		obuf=tt2;free(tt1);tt1=NULL;
	} else if(!memcmp(ibuf,"PSG\x1a",4)) {
		puts("PSG file");ft=PSG;
		for(t=5,tick=0;t<sb.st_size;t++)if(ibuf[t]==0xff)tick++;
		printf("Length:  %lu min, %lu sec\n",tick/50L/60L,tick/50L%60L);
		lp=sb.st_size-4;q=0;
	} else erro("unknown format");
	t=0;printf("Playing..\n\n");
	while(!quitflag) {
		switch(ft) {
		    case VTX: playvtx();break;
		    case PSG: playpsg();break;
		}
		indik();
		ts.tv_sec=0;
		ts.tv_nsec=10;
		nanosleep(&ts,&ts);
	}
	if(tt1)free(tt1);
	if(tt2)free(tt2);
	sreg(8,0);sreg(9,0);sreg(10,0);
	printf("\nExiting..\n");
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
	return 0;
}
