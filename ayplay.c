//(c)2002 sisoft\trg - AYplayer.
/* $Id: ayplay.c,v 1.2 2003/05/26 14:06:24 root Exp $ */
#include "ayplay.h"

_UL origsize,compsize,count,q,tick,t;
_UC *ibuf,*obuf;
_US lp;
int quitflag=0,ca,cb,cc;

void erro(char *ermess)
{
	puts("\n\tAY Player'2002, for real AY chip on LPT port");
	puts("(c)Stepan Pologov (siSoft\\TRG), 2:5050/125, sisoft@udm.net");
	puts("* Usage: ayplayer filename.vtx");
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
/*
_UC ireg(char reg)
{
	_UC dat;
	ioperm(dPort,3,1);
	outb(reg,dPort);
	outb(6,Port);
	outb(39,Port);
	dat=inb(dPort);
	outb(15,Port);
	ioperm(dPort,3,0);
	return dat;
}
*/
void playvtx()
{
	long i;
	_UC a;
	for(i=0;i<14;i++) {
		a=*(obuf+i*tick+t);
		if(i==7)a|=192;
		if(i==8)ca=a;
		if(i==9)cb=a;
		if(i==10)cc=a;
		if(!(i==13&&a==255))sreg(i,a);
	}
	t++;
	if(t>=tick)t=lp;
}

void indik()
{
	int i;
	char a[16]={"               "};
	char b[16]={"               "};
	char c[16]={"               "};
//	ca=ireg(8);cb=ireg(9);cc=ireg(10);
	for(i=0;i<(ca&15);i++)a[i]='=';if(i)a[(ca&15)-1]='-';
	for(i=0;i<(cb&15);i++)b[i]='=';if(i)b[(cb&15)-1]='-';
	for(i=0;i<(cc&15);i++)c[i]='=';if(i)c[(cc&15)-1]='-';
	printf("%02lu:%02lu   A: %s   B: %s   C: %s\r",t/q/60L,t/q%60L,a,b,c);
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
	return(++buf);
}

int main(int argc,char *argv[])
{
	_UC *ttt,*tt1,*tt2;
	_UL fsize,osize;
	FILE *infile;
	struct timespec ts;
	if(argc!=2)erro("");
	if((ibuf=tt1=(_UC*)malloc(65535))==NULL)erro("out of memory");
	if((infile=fopen(argv[1],"rb"))==NULL)erro("can't open music file");
	fread(ibuf,65535,1,infile);
	fsize=ftell(infile);
	fclose(infile);
	signal(SIGHUP,sighup);
	signal(SIGINT,sighup);
	printf("File:    %s\n",argv[1]);
	printf("Type:    ");
	if(*ibuf=='a'||*ibuf=='y')puts("Vortex Tracker");
	    else erro("uncknown format");
	ttt=vtxinfo(ibuf);
	compsize=fsize-(ttt-ibuf);
	ibuf=ttt;osize=origsize;
	if((obuf=tt2=(_UC*)calloc(14,tick))==NULL)erro("out of memory");
	unlh5(ibuf,obuf,origsize,compsize);
	obuf=tt2;
	t=0;printf("Playing..\n\n");

	while(!quitflag) {
		playvtx();
		indik();
		ts.tv_sec=0;
		ts.tv_nsec=10;
		nanosleep(&ts,&ts);
		    
	}
	sreg(8,0);
	sreg(9,0);
	sreg(10,0);
	printf("\nExiting..\n");
	free(tt1);free(tt2);
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
	return 0;
}
