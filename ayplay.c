//(c)2002 sisoft\trg - AYplayer.
/* $Id: ayplay.c,v 1.9 2003/06/25 00:39:25 root Exp $ */
#include "ayplay.h"
#include "z80.h"

_UC *ibuf,*obuf;
_UL origsize,compsize,count,q,tick,t,lp;
enum {UNK=0,VTX,PSG,HOB,PT2,PT3,STP,STC,PSC,ASC} formats;
int quitflag=0,ca,cb,cc,ft=UNK;
#define PLADR 18432

void erro(char *ermess)
{
	if(ermess)printf("\n” Error: %s!\n",ermess);
	    else puts("* Usage: ayplayer filename");
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

void playemu()
{
	_UC i;
	int r=-1,v=-1;
	DANM(haltstate)=0;
	PC=PLADR+4;SP=PLADR+1024;
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
	char hdr[17];
	FILE *infile;
	char *nam=NULL;
	_UC *tt1=NULL,*tt2=NULL;
	struct stat sb;
	_US sadr=0,iadr=0,padr=0,sngadr=0;
	puts("\n\tAY Player'2003, for real AY chip on LPT port");
	puts("(c)Stepan Pologov (siSoft\\TRG), 2:5050/125, sisoft@udm.net");
	if(argc!=2)erro(NULL);
	if(!strcasecmp(strrchr(argv[1],'.'),".gz")) {
		char cmd[256];
		nam=tmpnam(NULL);
		strncat(nam,argv[1]+(strchr(argv[1],'.')-argv[1]),4);
		snprintf(cmd,255,"gzip -cd %s >%s",argv[1],nam);
		if(system(cmd))erro("can't gzip sound file");
	}
	if(stat(nam?nam:argv[1],&sb))erro("can't stat sound file");
	if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".vtx"))ft=VTX;
	    else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".psg"))ft=PSG;
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
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".psc"))ft=PSC;
		else if(!strcasecmp(strrchr(nam?nam:argv[1],'.'),".asc"))ft=ASC;
		else if(!strncasecmp(strrchr(nam?nam:argv[1],'.'),".$",2))ft=HOB;
		if(ft) {
			PRNM(init)();PRNM(reset)();
			DANM(mem)[PLADR]=0xcd;
			DANM(mem)[PLADR+3]=0x76;
			DANM(mem)[PLADR+4]=0xcd;
			DANM(mem)[PLADR+7]=0x76;
			DANM(mem)[0x52]=0xc9;
			DANM(mem)[0]=0x76;
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
				printf("hob: s: %u, l: %lu, i: %u, p: %u, sng: %u\n",sadr,sb.st_size,iadr,padr,sngadr);
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
			}
			*(_US*)(DANM(mem)+PLADR+1)=(_US)iadr;
			*(_US*)(DANM(mem)+PLADR+5)=(_US)padr;
			PC=PLADR;SP=PLADR+1024;
			while(!DANM(haltstate)){/*printf("pc=%u,sp=%u\n",PC,SP);*/PRNM(step)(1);}
			fclose(infile);
		}
	}
	if(nam)unlink(nam);
	signal(SIGHUP,sighup);signal(SIGINT,sighup);
	printf("\nFile:    %s\n",argv[1]);
	printf("Type:    %s",nam?"packed ":"");
	switch(ft) {
	    case VTX:
		puts("Vortex Tracker");
		ibuf=vtxinfo(ibuf);
		compsize=sb.st_size-(ibuf-tt1);
		if((tt2=obuf=(_UC*)calloc(14,tick))==NULL)erro("out of memory");
		unlh5(ibuf,obuf,origsize,compsize);
		obuf=tt2;free(tt1);tt1=NULL;
		break;
	    case PSG:
		puts("PSG file");
		for(t=5,tick=0;t<sb.st_size;t++)if(ibuf[t]==0xff)tick++;
		printf("Length:  %lu min, %lu sec\n",tick/50L/60L,tick/50L%60L);
		lp=sb.st_size-4;q=0;
		break;
	    case PT2:
		printf("Protracker 2.x\nName:    ");
		fwrite(DANM(mem)+sngadr+101,30,1,stdout);
		printf("\n");
		lp=0;q=0;
		break;
	    case PT3:
		fwrite(DANM(mem)+sngadr,15,1,stdout);
		printf("\nName:    ");
		fwrite(DANM(mem)+sngadr+30,32,1,stdout);
		printf("\nAuthor:  ");
		fwrite(DANM(mem)+sngadr+66,32,1,stdout);
		printf("\n");
		lp=0;q=0;
		break;
	    case STP:
		puts("Sound Tracker Pro");
		if(*(_UC*)(DANM(mem)+sadr+45)>=32) {
			printf("Name:    ");
			fwrite(DANM(mem)+sadr+45,25,1,stdout);
			printf("\n");
		}
		lp=0;q=0;
		break;
	    case STC:
		puts("Sound Tracker");
		if(*(_UC*)(DANM(mem)+sadr+49)>=32) {
			printf("Name:    ");
			fwrite(DANM(mem)+sadr+49,10,1,stdout);
			printf("\nAuthor:  ");
			fwrite(DANM(mem)+sadr+63,12,1,stdout);
			printf("\n");
		}
		lp=0;q=0;
		break;
	    case PSC:
		fwrite(DANM(mem)+sngadr,9,1,stdout);
		printf("\nName:    ");
		fwrite(DANM(mem)+sngadr+25,19,1,stdout);
		printf("\nAuthor:  ");
		fwrite(DANM(mem)+sngadr+49,19,1,stdout);
		printf("\n");
		lp=0;q=0;
		break;
	    case ASC:
		puts("ASC Sound Master");
		if(*(_UC*)(DANM(mem)+sadr+39)>=32) {
			printf("Name:    ");
			fwrite(DANM(mem)+sadr+39,6,1,stdout);
			printf("\nAuthor:  ");
			fwrite(DANM(mem)+sadr+49,32,1,stdout);
			printf("\n");
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
			playemu();break;
		}
		indik();
		usleep(2000);
	}
	if(tt1)free(tt1);
	if(tt2)free(tt2);
	sreg(8,0);sreg(9,0);sreg(10,0);
	printf("\nExiting..\n");
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
	return 0;
}
