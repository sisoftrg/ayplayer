//(c)2003 sisoft\trg - AYplayer.
/* $Id: unlzh.c,v 1.2 2004/03/11 17:27:58 root Exp $ */
#include "ayplay.h"

static _US left[1019],right[1019],pt_table[256],bitbuf=0;
static _UC subbitbuf=0,bitcount=0,*text,pt_len[128];
static _UC *ibuf,*obuf;
static _UL origsize,compsize;

static void fillbuf(_UC n)
{
	while(n>bitcount) {
		n-=bitcount;
		bitbuf=(bitbuf<<bitcount)+(subbitbuf>>(8-bitcount));
		if(compsize) {
    			compsize--;
			subbitbuf=(_UC)*(ibuf++);
		} else subbitbuf=0;
		bitcount=8;
	}
	bitcount-=n;
	bitbuf=(bitbuf<<n)+(subbitbuf>>(8-n));
	subbitbuf<<=n;
}

static _US getbits(_UC n)
{
	_US x=bitbuf>>(16-n);
	fillbuf(n);
	return x;
}

static void make_table(short nchar,_UC bitlen[],short tablebits,_US table[])
{
	_US count[17],weight[17],start[17],total=0,*p;
	unsigned i;
	int j,k=1<<tablebits,l,m=16-tablebits,n,avail=nchar;
	for(i=1;i<=16;i++) {
		count[i]=0;
		weight[i]=1<<(16-i);
	}
	for(i=0;i<nchar;i++)count[bitlen[i]]++;
	for(i=1;i<=16;i++) {
		start[i]=total;
		total+=weight[i]*count[i];
	}
	if(total&0xffff)erro(_("bad file structure"));
	for(i=1;i<=tablebits;i++) {
		start[i]>>=m;
		weight[i]>>=m;
	}
	j=start[tablebits+1]>>m;
	if(j)for(i=j;i<k;i++)table[i]=0;
	for(j=0;j<nchar;j++) {
		k=bitlen[j];
		if(!k)continue;
		l=start[k]+weight[k];
		if(k<=tablebits)for(i=start[k];i<l;i++)table[i]=j;
		    else {
			p=&table[(i=start[k])>>m];
			i<<=tablebits;
			n=k-tablebits;
			while(--n>=0) {
				if(!*p) {
					right[avail]=left[avail]=0;
					*p=avail++;
				}
				if(i&0x8000)p=&right[*p];
				    else p=&left[*p];
				i<<=1;
			}
			*p=j;
		}
		start[k]=l;
	}
}

static void read_pt_len(short nn,short nbit,short i_special)
{
	short i=0,c,n=getbits(nbit);
	if(!n) {
		c=getbits(nbit);
		for(i=0;i<nn;i++)pt_len[i]=0;
		for(i=0;i<256;i++)pt_table[i]=c;
	    } else {
		while(i<n) {
			c=bitbuf>>13;
			if(c==7) {
				_US mask=1<<12;
				while(mask&bitbuf){mask>>=1;c++;}
			}
			fillbuf((c<7)?3:c-3);
			pt_len[i++]=c;
			if(i==i_special) {
				c=getbits(2);
				while(--c>=0)pt_len[i++]=0;
			}
		}
		while(i<nn)pt_len[i++]=0;
		make_table(nn,pt_len,8,pt_table);
	}
}

void unlh5(_UC *_ibuf,_UC *_obuf,_UL _origsize,_UL _compsize)
{
	_US blocksize=0,dicsiz=1<<13,c_table[8192],mask=1,loc=0;
	int i,j,k,c,n,q,dicsiz1=dicsiz-1;
	_UC c_len[1024];
	_UL count=0;
	text=(_UC*)malloc(dicsiz);
	if(text==NULL)erro(_("out of memory"));
	memset(text,' ',dicsiz);
	ibuf=_ibuf;obuf=_obuf;
	origsize=_origsize;
	compsize=_compsize;
	fillbuf(16);
	while(count<origsize) {
		if (!blocksize) {
			blocksize=getbits(16);
			read_pt_len(19,5,3);
			n=getbits(9);i=0;
			if(!n) {
				c=getbits(9);
				for(i=0;i<510;i++)c_len[i]=0;
				for(i=0;i<4096;i++)c_table[i]=c;
			    } else {
				while(i<n) {
					c=pt_table[bitbuf>>8];
					if(c>=19) {
						_US mask=1<<7;
						do {
							if(bitbuf&mask)c=right[c];
							    else c=left[c];
							mask>>=1;
						} while(c>=19);
					}
					fillbuf(pt_len[c]);
					if(c<=2) {
						if(!c)c=1;
						  else if(c==1)c=getbits(4)+3;
						    else c=getbits(9)+20;
						while(--c>=0)c_len[i++]=0;
					} else c_len[i++]=c-2;
				}
				while(i<510)c_len[i++]=0;
				make_table(510,c_len,12,c_table);
			}
			read_pt_len(14,4,-1);
		}
		blocksize--;
		c=c_table[bitbuf>>4];
		if(c<510)fillbuf(c_len[c]);
		    else {
			fillbuf(12);
			mask=1<<15;
			do {
				if(bitbuf&mask)c=right[c];
				    else c=left[c];
				mask>>=1;
			} while(c>=510);
			fillbuf(c_len[c]-12);
		}
		if(c<=255) {
			text[loc++]=c;
			if(loc==dicsiz) {
				memcpy(obuf,text,dicsiz);
				obuf+=dicsiz;
				loc=0;
			}
			count++;
		    } else {
			j=c-253;
			q=pt_table[bitbuf>>8];
			mask=1<<15;
			if(q<14)fillbuf(pt_len[q]);
			    else {
				fillbuf(8);
				do {
					if(bitbuf&mask)q=right[q];
					    else q=left[q];
					mask>>=1;
				} while(q>=14);
				fillbuf(pt_len[q]-8);
			}
			if(q)q=(1<<(q-1))+getbits(q-1);
			i=(loc-q-1)&dicsiz1;
			count+=j;
			for(k=0;k<j;k++) {
				c=text[(i+k)&dicsiz1];
				text[loc++]=c;
				if(loc==dicsiz) {
					memcpy(obuf,text,dicsiz);
					obuf+=dicsiz;
					loc=0;
				}
			}
		}
	}
	if(loc) {
		memcpy(obuf,text,loc);
		obuf+=loc;
	}
	free(text);
}
