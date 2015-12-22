/*--------------------------------------------------------------
	Common Library for libufp
		Written by H.Goto , Jan. 1995
		Revised by H.Goto , Feb. 1996
		Revised by H.Goto , July 1996
		Revised by H.Goto , Feb. 1997
		Revised by H.Goto , Apr. 2000
		Revised by H.Goto , Nov. 2008
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1995-2008  Hideaki Goto

        All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
  its documentation for any purpose is hereby granted without fee,
  provided that (i) the above copyright notice and this permission
  notice appear in all copies and in supporting documentation, (ii)
  the name of the author, Hideaki Goto, may not be used in any
  advertising or otherwise to promote the sale, use or other
  dealings in this software without prior written authorization
  from the author, (iii) this software may not be used for
  commercial products without prior written permission from the
  author, and (iv) the notice of modification is specified in cases
  where modified copies of this software are distributed.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
  THE AUTHOR WILL NOT BE RESPONSIBLE FOR ANY DAMAGE CAUSED BY THIS
  SOFTWARE.
--------------------------------------------------------------------*/


#include	<stdio.h>
#include	<stdlib.h>

#include	"comlib.h"


void _uf_wordswap(ushort *d){
	unsigned char	H,L;
	L = (uchar)*d;
	H = (uchar)(*d >> 8);
	*d = ((ushort)L << 8) | (ushort)H;
}


void _uf_dwordswap(uint32 *d){
	unsigned char	B0,B1,B2,B3;
	B0 = (uchar)*d;
	B1 = (uchar)(*d >> 8);
	B2 = (uchar)(*d >> 16);
	B3 = (uchar)(*d >> 24);
	*d = ((ushort)B0 << 24) | ((ushort)B1 << 16) | ((ushort)B2 << 8) | (ushort)B3;
}


void _uf_convert_1to8(uchar *src,uchar *dst,int size,uchar fg,uchar bg){
	int	size2;
	int	cc;
	size2 = size % 8;
	size /= 8;
	for( ; size > 0 ; --size ){
		cc = (int)*(src++);
		if((cc&0x80)==0) *(dst++)=bg; else *(dst++)=fg;
		if((cc&0x40)==0) *(dst++)=bg; else *(dst++)=fg;
		if((cc&0x20)==0) *(dst++)=bg; else *(dst++)=fg;
		if((cc&0x10)==0) *(dst++)=bg; else *(dst++)=fg;
		if((cc&0x08)==0) *(dst++)=bg; else *(dst++)=fg;
		if((cc&0x04)==0) *(dst++)=bg; else *(dst++)=fg;
		if((cc&0x02)==0) *(dst++)=bg; else *(dst++)=fg;
		if((cc&0x01)==0) *(dst++)=bg; else *(dst++)=fg;
	}
	if ( size2 != 0 ){
		cc = (int)*src;
		for ( ; size2 > 0 ; --size2 ){
			if((cc&0x80)==0) *(dst++)=bg; else *(dst++)=fg;
			cc <<= 1;
		}
	}
}


void _uf_convert_8to1(uchar *src,uchar *dst,int size,uchar threshold){
	int	i,i2,th;
	uchar	d,tm;
	th = (int)threshold;
	for ( i=0 ; i<(size/8) ; i++ ){
		d = 0;
		tm = 0x80;
		for ( i2=0 ; i2<8 ; i2++ ){
			if ( (int)*(src++) < th )  d |= tm;
			tm >>= 1;
		}
		*(dst++) = d;
	}
	i = size % 8;
	if ( i != 0 ){
		d = 0;
		tm = 0x80;
		for ( i2=0 ; i2<i ; i2++ ){
			if ( (int)*(src++) < th )  d |= tm;
			tm >>= 1;
		}
		*(dst++) = d;
	}
}


int uf_GetLittleWORD(FILE *fp){
	ushort	w;
	if ( 2 != fread((void *)&w,1,2,fp) )  return(EOF);
	ifMSB _uf_wordswap(&w);
	return( (int)((uint)w) );
}


int uf_GetLittleDWORD(FILE *fp){
	uint32	dw;
				/* EOF conflicts with a possible data. */
	if ( 4 != fread((void *)&dw,1,4,fp) )  return(EOF);
	ifMSB _uf_dwordswap(&dw);
	return( (int)dw );
}


int uf_GetBigWORD(FILE *fp){
	ushort	w;
	if ( 2 != fread((void *)&w,1,2,fp) )  return(EOF);
	ifLSB  _uf_wordswap(&w);
	return( (int)((uint)w) );
}


int uf_GetBigDWORD(FILE *fp){
	uint32	dw;
				/* EOF conflicts with a possible data. */
	if ( 4 != fread((void *)&dw,1,4,fp) )  return(EOF);
	ifLSB _uf_dwordswap(&dw);
	return( (int)dw );
}


