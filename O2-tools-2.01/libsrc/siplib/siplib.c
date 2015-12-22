/*--------------------------------------------------------------
	Image Processing functions
		Written by H.Goto , Jul 1994
		Revised by H.Goto , Nov 1995
		Revised by H.Goto , May 1996
		Revised by H.Goto , Sep 1997
		Revised by H.Goto , Oct 1998
		Revised by H.Goto , Apr 2000
		Revised by H.Goto , Nov 2001
		Revised by H.Goto , Nov 2001
		Revised by H.Goto , Feb 2002
		Revised by H.Goto , May 2002
		Revised by H.Goto , Jul 2003
		Revised by H.Goto , Dec 2005
		Revised by H.Goto , Apr 2007
		Revised by H.Goto , Dec 2008
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1994-2008  Hideaki Goto

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
#include	<memory.h>

#define		siplib_c
#include	"siplib.h"


/*------------------------------------------------------
	Miscellaneous
------------------------------------------------------*/

				/* Get pointer to the specific scanline */

char *sip_getimgptr(SIPImage *image,int y){
	return((char *)image->pdata[y]);
}


				/* Get line size in bytes */

int sip_getlinebytes(int len,int depth,int pad){
	int	l;
	pad /= 8;
	if ( depth == 1 ){
		switch (pad){
		  case 2:	return( ((len +15) /16) *2 );
		  case 4:	return( ((len +31) /32) *4 );
		  default:	return( (len +7) /8 );
		}
	}
	l = len * (depth >> 3);
	switch (pad){
	  case 2:	return( ((l +1) /2) *2 );
	  case 4:	return( ((l +3) /4) *4 );
	  default:	return(l);
	}
}


				/* Copy ScanLine */

void sip_CopyLine(char *src,char *dst,int width,int depth){
	int	i;
	short	*sps,*dps;
	int32	*spl,*dpl;
	i = (int)width;
	if ( width < 8 ){
		switch (depth){
		  case 1:	i = (i + 7) /8;
		  case 8:
				for ( ; i>0 ; i-- )  *dst++ = *src++;
				break;
		  case 16:	sps = (short *)src;
				dps = (short *)dst;
				for ( ; i>0 ; i-- )  *dps++ = *sps++;
				break;
		  case 32:	spl = (int32 *)src;
				dpl = (int32 *)dst;
				for ( ; i>0 ; i-- )  *dpl++ = *spl++;
				break;
		}
		return;
	}
	switch (depth){
	  case 1:	i = (i + 7) /8;  break;
	  case 8:	break;
	  case 16:	i *= 2;  break;
	  case 32:	i *= 4;  break;
	}
	memcpy(dst,src,i);
	return;
}





/*------------------------------------------------------
	Image object manipulation
------------------------------------------------------*/


SIPImage	SIPImage0 = { 0,0,8,16,0,NULL };


				/* Create image */

SIPImage *sip_CreateImage(int width,int height,int depth){
	SIPImage	*imgtmp;
	long		linesize;
	long		bufsize;
	void		**pdata;
	int		i;
	linesize = (long)((ulong)sip_getlinebytes(width,depth,16));
	bufsize = linesize * (long)((ulong)height);
	if ( bufsize  <= 0 )  return(NULL);
	if ( NULL == (imgtmp = (SIPImage *)malloc(sizeof(SIPImage))) ){
		return(NULL);
	}
	if ( NULL == (pdata = (void **)malloc(sizeof(void *) * height)) ){
		free((void *)imgtmp);
		return(NULL);
	}
	imgtmp->pdata  = pdata;
	imgtmp->width  = width;
	imgtmp->height = height;
	imgtmp->depth  = depth;
	imgtmp->bitmap_pad  = 16;
	imgtmp->bytes_per_line  = linesize;
	if ( NULL == (imgtmp->data = (char *)malloc(bufsize)) ){
		free((void *)pdata);
		free((void *)imgtmp);
		return(NULL);
	}
	for ( i=0 ; i<height ; i++ ){
		pdata[i] = (void *)(imgtmp->data + i*linesize);
	}
	return(imgtmp);
}


				/* Destroy image */

int sip_DestroyImage(SIPImage *image){
	if ( NULL != image ){
		if ( NULL != image->data ){
			free((void *)image->pdata);
			free((void *)image->data);
			image->data = NULL;
		}
		free((void *)image);
	}
	return(0);
}


				/* Duplicate image */

SIPImage *sip_DuplicateImage(SIPImage *image){
	SIPImage	*imgtmp;
	int	y;
	if ( 0 == (imgtmp = sip_CreateImage(image->width,image->height,image->depth)) )  return(0);
	for ( y=0 ; y<image->height ; y++ ){
		memcpy((void*)imgtmp->pdata[y], \
		  (void*)image->pdata[y], image->bytes_per_line);
	}
	return(imgtmp);
}


				/* Fill a line by specified data (raw mode) */

int sip_rawClearLine(char *buf,int len,int depth,int32 data){
	int	c;
	char	db;
	char	*pc;
	short	*ps;
	int32	*pl;
	if ( data )  db = (char)0xff;  else db = 0;
	pl = (int32 *)( ps = (short *)( pc = buf ) );
	c = len;
	switch ( depth ){
	  case 1:	c = (c +7) /8;
			for ( ; c>0 ; c-- )  *pc++ = db;
			break;
	  case 8:	for ( ; c>0 ; c-- )  *pc++ = (char)data;
			break;
	  case 16:	for ( ; c>0 ; c-- )  *ps++ = (short)data;
			break;
	  case 32:	for ( ; c>0 ; c-- )  *pl++ = (int32)data;
			break;
	}
	return(0);
}


				/* Fill a line by specified data */

int sip_ClearLine(SIPImage *image,int y,int32 data){
	char	*p;
	if ( NULL == image->data )  return(-1);
	if ( NULL == (p = sip_getimgptr(image,y)) )  return(-1);
	return( sip_rawClearLine(p,image->width,image->depth,data) );
}


				/* Fill image by specified data */

int sip_ClearImage(SIPImage *image,int32 data){
	int	c,y;
	char	db;
	char	*p,*pc;
	short	*ps;
	int32	*pl;
	if ( data )  db = (char)0xff;  else db = 0;
	if ( NULL == image->data )  return(-1);
	if ( NULL == (p = sip_getimgptr(image,0)) )  return(-1);
	for ( y=0 ; y < image->height ; y++ ){
		pl = (int32 *)( ps = (short *)(pc = p) );
		c = image->width;
		switch ( image->depth ){
		  case 1:	c = (c +7) /8;
				for ( ; c>0 ; c-- )  *pc++ = db;
				break;
		  case 8:	for ( ; c>0 ; c-- )  *pc++ = (char)data;
				break;
		  case 16:	for ( ; c>0 ; c-- )  *ps++ = (short)data;
				break;
		  case 32:	for ( ; c>0 ; c-- )  *pl++ = (int32)data;
				break;
		}
		p += image->bytes_per_line;
	}
	return(0);
}


				/* Copy area from src_image to dst_image */
				/*   1. depth,width,height must match    */
				/*   2. x,y,width,height must be         */
				/*         multiples of 8 for 1bit-image */

int sip_CopyArea(SIPImage *src,SIPRectangle *srect,
		 SIPImage *dst,SIPRectangle *drect){
	ushort	x,y;
	char	*sp,*dp;
	char	*spb,*dpb;
	short	*sps,*dps;
	int32	*spl,*dpl;
	int	depth;
	if ( src->depth    != dst->depth    )  return(-2);
	depth = src->depth;
	if ( srect->width  != drect->width  )  return(-2);
	if ( srect->height != drect->height )  return(-2);
	if ( (srect->x < 0) || (srect->y < 0) )  return(-1);
	if ( (drect->x < 0) || (drect->y < 0) )  return(-1);
	if ( (srect->x + srect->width ) > src->width  )  return(0);
	if ( (srect->y + srect->height) > src->height )  return(0);
	if ( (drect->x + drect->width ) > dst->width  )  return(0);
	if ( (drect->y + drect->height) > dst->height )  return(0);
	if ( NULL == (sp = sip_getimgptr(src,srect->y)) )  return(-1);
	if ( NULL == (dp = sip_getimgptr(dst,drect->y)) )  return(-1);
	if ( depth == 1 ){
		if ( srect->width & 0x07 )  return(-1);
		if ( srect->x & 0x07 )  return(-1);
		if ( drect->x & 0x07 )  return(-1);
		sp += srect->x >> 3;
		dp += drect->x >> 3;
	}
	else{	sp += (depth >> 3) * srect->x;
		dp += (depth >> 3) * drect->x;
	}
	for ( y=0 ; y < srect->height ; y++ ){
		spl = (int32 *)( sps = (short *)( spb = sp ) );
		dpl = (int32 *)( dps = (short *)( dpb = dp ) );
		switch (depth){
		  case 1:	for ( x=0 ; x < (srect->width >> 3) ; x++ )
								*dpb++ = *spb++;
				break;
		  case 8:	for ( x=0 ; x < srect->width ; x++ )  *dpb++ = *spb++;
				break;
		  case 16:	for ( x=0 ; x < srect->width ; x++ )  *dps++ = *sps++;
				break;
		  case 32:	for ( x=0 ; x < srect->width ; x++ )  *dpl++ = *spl++;
				break;
		}
		sp += src->bytes_per_line;
		dp += dst->bytes_per_line;
	}
	return(0);
}


				/* Get a scan line data */
				/*     len = -1  means "to end of line" */

int sip_GetScanLine(SIPImage *image, \
		void *buf,int x,int y,int len,int depth){
	int	i;
	char	*p;
	int	direct = 0;
	char	*buf8;
	short	*buf16, *p16;
	int32	*buf32, *p32;
	if ( (x < 0) || (x >= image->width ) )  return(-1);
	if ( (y < 0) || (y >= image->height) )  return(-1);
	if ( (len < 0) || (x + len) > image->width )  len = image->width - x;
	if ( NULL == (p = sip_getimgptr(image,y)) )  return(-1);
	if ( depth == image->depth )  direct = 1;
	if ( ! direct ){
		return(-2);
	}
	else{	switch (image->depth){
		  case 1:	if ( x   & 0x07 )  return(-1);
				p += x >> 3;
				if ( len & 0x07 )  return(-1);
				len >>= 3;
				buf8  = (char *)buf;
				for ( i=0 ; i<len ; i++ )
					*buf8++ = *p++;
				break;
		  case 8:	p += x;
				buf8  = (char *)buf;
				for ( i=0 ; i<len ; i++ )
					*buf8++ = *p++;
				break;
		  case 16:	p += 2 * x;
				buf16  = (short *)buf;
				p16 = (short *)p;
				for ( i=0 ; i<len ; i++ )
					*buf16++ = *p16++;
				break;
		  case 32:	p += 4 * x;
				buf32  = (int32 *)buf;
				p32 = (int32 *)p;
				for ( i=0 ; i<len ; i++ )
					*buf32++ = *p32++;
				break;
		}
	}
	return(0);
}


				/* Put a scan line data */
				/*     len = -1  means "to end of line" */

int sip_PutScanLine(SIPImage *image, \
		void *buf,int x,int y,int len,int depth){
	int	i;
	char	*p;
	int	direct = 0;
	char	*buf8;
	short	*buf16, *p16;
	int32	*buf32, *p32;
	if ( (x < 0) || (x >= image->width ) )  return(-1);
	if ( (y < 0) || (y >= image->height) )  return(-1);
	if ( (len < 0) || (x + len) > image->width )  len = image->width - x;
	if ( NULL == (p = sip_getimgptr(image,y)) )  return(-1);
	if ( depth == image->depth )  direct = 1;
	if ( ! direct ){
		return(-2);
	}
	else{	switch (image->depth){
		  case 1:	if ( x   & 0x07 )  return(-1);
				p += x >> 3;
				if ( len & 0x07 )  return(-1);
				len >>= 3;
				buf8  = (char *)buf;
				for ( i=0 ; i<len ; i++ )
					*p++ = *buf8++;
				break;
		  case 8:	p += x;
				memcpy(p,buf,len);
				break;
		  case 16:	p += 2 * x;
				buf16  = (short *)buf;
				p16 = (short *)p;
				for ( i=0 ; i<len ; i++ )
					*p16++ = *buf16++;
				break;
		  case 32:	p += 4 * x;
				buf32  = (int32 *)buf;
				p32 = (int32 *)p;
				for ( i=0 ; i<len ; i++ )
					*p32++ = *buf32++;
				break;
		}
	}
	return(0);
}


				/* Get a vertical scan line data */
				/*     len = -1  means "to end of line" */

int sip_GetVertLine(SIPImage *image, \
		void *buf,int x,int y,int len,int depth){
	int	i;
	char	*p;
	int	direct = 0;
	uchar	mask0,mask,d;
	char	*buf8;
	short	*buf16;
	int32	*buf32;
	if ( (x < 0) || (x >= image->width ) )  return(-1);
	if ( (y < 0) || (y >= image->height) )  return(-1);
	if ( (len < 0) || (y + len) > image->height )  len = image->height - y;
	if ( NULL == (p = sip_getimgptr(image,y)) )  return(-1);
	if ( depth == image->depth )  direct = 1;
	if ( ! direct ){
		return(-2);
	}
	else{	switch (image->depth){
		  case 1:	mask0 = (mask = (uchar)0x80) >> (x & 0x07);
				d = 0;
				if ( y   & 0x07 )  return(-1);
				p += (x >> 3);
				buf8 = (char *)buf;
				for ( i=0 ; i<len ; i++ ){
					if ( mask0 & *p )  d |= mask;
					p += image->bytes_per_line;
					if ( 0 == (mask >>= 1) ){
						*(++buf8) = d;
						mask = (uchar)0x80;
					}
				}
				if ( mask != (uchar)0x80 )  *buf8++ = d;
				break;
		  case 8:	p += x;
				buf8 = (char *)buf;
				for ( i=0 ; i<len ; i++ ){
					*buf8++ = *p;
					p += image->bytes_per_line;
				}
				break;
		  case 16:	p += 2 * x;
				buf16 = (short *)buf;
				for ( i=0 ; i<len ; i++ ){
					*buf16++ = *((short *)p);
					p += image->bytes_per_line;
				}
				break;
		  case 32:	p += 4 * x;
				buf32 = (int32 *)buf;
				for ( i=0 ; i<len ; i++ ){
					*buf32++ = *((int32 *)p);
					p += image->bytes_per_line;
				}
				break;
		}
	}
	return(0);
}


				/* Put a vertical scan line data */
				/*     len = -1  means "to end of line" */

int sip_PutVertLine(SIPImage *image, \
		void *buf,int x,int y,int len,int depth){
	int	i;
	char	*p;
	int	direct = 0;
	uchar	mask0,mask,d0,d;
	char	*buf8;
	short	*buf16;
	int32	*buf32;
	if ( (x < 0) || (x >= image->width ) )  return(-1);
	if ( (y < 0) || (y >= image->height) )  return(-1);
	if ( (len < 0) || (y + len) > image->height )  len = image->height - y;
	if ( NULL == (p = sip_getimgptr(image,y)) )  return(-1);
	if ( depth == image->depth )  direct = 1;
	if ( ! direct ){
		return(-2);
	}
	else{	switch (image->depth){
		  case 1:	mask0 = (mask = (uchar)0x80) >> (x & 0x07);
				d = *((char *)buf);
				if ( y   & 0x07 )  return(-1);
				p += (x >> 3);
				buf8 = (char *)buf;
				for ( i=0 ; i<len ; i++ ){
					d0 = *p & ~mask0;
					if ( mask & d )  d0 |= mask0;
					*p = d0;
					p += image->bytes_per_line;
					if ( 0 == (mask >>= 1) ){
						d = *(++buf8);
						mask = (uchar)0x80;
					}
				}
				break;
		  case 8:	p += x;
				buf8 = (char *)buf;
				for ( i=0 ; i<len ; i++ ){
					*p = *buf8++;
					p += image->bytes_per_line;
				}
				break;
		  case 16:	p += 2 * x;
				buf16 = (short *)buf;
				for ( i=0 ; i<len ; i++ ){
					*((short *)p) = *buf16++;
					p += image->bytes_per_line;
				}
				break;
		  case 32:	p += 4 * x;
				buf32 = (int32 *)buf;
				for ( i=0 ; i<len ; i++ ){
					*((int32 *)p) = *buf32++;
					p += image->bytes_per_line;
				}
				break;
		}
	}
	return(0);
}


				/* Get pixel value on bitmap image */
uchar sip_getbit(SIPImage *image,int x,int y){
	uchar	b;
	uchar	mask;
	mask = 0x01 << (7 - (x & 0x07));
	b = *(image->data + (int32)(x>>3) + (int32)y * image->bytes_per_line) & mask;
	return( (uchar)(b != 0) );
}


				/* Put pixel value on bitmap image */
uchar sip_putbit(SIPImage *image,int x,int y,uchar b){
	uchar	*p;
	uchar	mask;
	mask = 0x01 << (7 - (x & 0x07));
	if ( b != 0 )  b = (uchar)0xff;
	p = (uchar *)image->data + (int32)(x>>3) + (int32)y * image->bytes_per_line;
	*p = (*p & ~mask) | (b & mask);
	return(b);
}


				/* OR & put pixel value on bitmap image */
uchar sip_orbit(SIPImage *image,int x,int y,uchar b){
	uchar	*p;
	uchar	mask;
	mask = 0x01 << (7 - (x & 0x07));
	if ( b != 0 )  b = (uchar)0xff;
	p = (uchar *)image->data + (int32)(x>>3) + (int32)y * image->bytes_per_line;
	*p = *p | (b & mask);
	return(b);
}


				/* AND & put pixel value on bitmap image */
uchar sip_andbit(SIPImage *image,int x,int y,uchar b){
	uchar	*p;
	uchar	mask;
	mask = 0x01 << (7 - (x & 0x07));
	if ( b != 0 )  b = (uchar)0xff;
	p = (uchar *)image->data + (int32)(x>>3) + (int32)y * image->bytes_per_line;
	*p = *p & (b & mask);
	return(b);
}


				/* XOR & put pixel value on bitmap image */
uchar sip_xorbit(SIPImage *image,int x,int y,uchar b){
	uchar	*p;
	uchar	mask;
	mask = 0x01 << (7 - (x & 0x07));
	if ( b != 0 )  b = (uchar)0xff;
	p = (uchar *)image->data + (int32)(x>>3) + (int32)y * image->bytes_per_line;
	*p = *p ^ (b & mask);
	return(b);
}


 

/*------------------------------------------------------
	Data conversion
------------------------------------------------------*/

				/* Depth conversion from 1bit to 8bit */

void sip_cvt1to8(char *src,int bitoff,char *dst,int len,char fc,char bc){
	int	i;
	uchar	d;
	uchar	mask;
	if ( bitoff ){
		mask = 0x80 >> bitoff;
		bitoff = 8 - bitoff;
		d = (uchar)*src++;
		for ( i=0 ; (i < bitoff) && (len > 0) ; i++, len--, mask >>= 1 ){
			if ( d & mask )  *dst++ = fc;  else *dst++ = bc;
		}
	}
	for ( i = len >> 3 ; i>0 ; i-- ){
		d = (uchar)*src++;
		if ( d & 0x80 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x40 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x20 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x10 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x08 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x04 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x02 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x01 )  *dst++ = fc;  else *dst++ = bc;
	}
	mask = (uchar)0x80;
	d = (uchar)*src++;
	for ( i = len & 0x07 ; i>0 ; i-- ){
		if ( d & mask )  *dst++ = fc;  else *dst++ = bc;
		mask >>= 1;
	}
}


				/* Depth conversion from 1bit to 16bit */

void sip_cvt1to16(char *src,int bitoff,short *dst,int len,short fc,short bc){
	int	i;
	uchar	d;
	uchar	mask;
	if ( bitoff ){
		mask = 0x80 >> bitoff;
		bitoff = 8 - bitoff;
		d = (uchar)*src++;
		for ( i=0 ; (i < bitoff) && (len > 0) ; i++, len--, mask >>= 1 ){
			if ( d & mask )  *dst++ = fc;  else *dst++ = bc;
		}
	}
	for ( i = len >> 3 ; i>0 ; i-- ){
		d = (uchar)*src++;
		if ( d & 0x80 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x40 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x20 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x10 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x08 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x04 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x02 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x01 )  *dst++ = fc;  else *dst++ = bc;
	}
	mask = (uchar)0x80;
	d = (uchar)*src++;
	for ( i = len & 0x07 ; i>0 ; i-- ){
		if ( d & mask )  *dst++ = fc;  else *dst++ = bc;
		mask >>= 1;
	}
}


				/* Depth conversion from 1bit to 32bit */

void sip_cvt1to32(char *src,int bitoff,int32 *dst,int len,int32 fc,int32 bc){
	int	i;
	uchar	d;
	uchar	mask;
	if ( bitoff ){
		mask = 0x80 >> bitoff;
		bitoff = 8 - bitoff;
		d = (uchar)*src++;
		for ( i=0 ; (i < bitoff) && (len > 0) ; i++, len--, mask >>= 1 ){
			if ( d & mask )  *dst++ = fc;  else *dst++ = bc;
		}
	}
	for ( i = len >> 3 ; i>0 ; i-- ){
		d = (uchar)*src++;
		if ( d & 0x80 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x40 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x20 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x10 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x08 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x04 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x02 )  *dst++ = fc;  else *dst++ = bc;
		if ( d & 0x01 )  *dst++ = fc;  else *dst++ = bc;
	}
	mask = (uchar)0x80;
	d = (uchar)*src++;
	for ( i = len & 0x07 ; i>0 ; i-- ){
		if ( d & mask )  *dst++ = fc;  else *dst++ = bc;
		mask >>= 1;
	}
}


				/* Depth conversion from 8bit to 16bit */
				/*                        (unsigned)   */

void sip_cvt8to16u(uchar *src,ushort *dst,int len,ushort mul){
	int	i;
	if ( mul == 1 ){
		for ( i=0 ; i<len ; i++ )  *dst++ = (ushort)*src++;
	}
	else{	for ( i=0 ; i<len ; i++ )  *dst++ = (ushort)*src++ * mul;
	}
}


				/* Depth conversion from 8bit to 16bit */
				/*                         (signed)    */

void sip_cvt8to16(char *src,short *dst,int len,short mul){
	int	i;
	if ( mul == 1 ){
		for ( i=0 ; i<len ; i++ )  *dst++ = (short)*src++;
	}
	else{	for ( i=0 ; i<len ; i++ )  *dst++ = (short)*src++ * mul;
	}
}


				/* Depth conversion from 8bit to 32bit */
				/*                        (unsigned)   */

void sip_cvt8to32u(uchar *src,uint32 *dst,int len,uint32 mul){
	int	i;
	if ( mul == 1 ){
		for ( i=0 ; i<len ; i++ )  *dst++ = (uint32)*src++;
	}
	else{	for ( i=0 ; i<len ; i++ )  *dst++ = (uint32)*src++ * mul;
	}
}


				/* Depth conversion from 8bit to 32bit */
				/*                         (signed)    */

void sip_cvt8to32(char *src,int32 *dst,int len,int32 mul){
	int	i;
	if ( mul == 1 ){
		for ( i=0 ; i<len ; i++ )  *dst++ = (int32)*src++;
	}
	else{	for ( i=0 ; i<len ; i++ )  *dst++ = (int32)*src++ * mul;
	}
}


				/* Depth conversion from 16bit to 32bit */
				/*                         (unsigned)   */

void sip_cvt16to32u(ushort *src,uint32 *dst,int len,uint32 mul){
	int	i;
	if ( mul == 1 ){
		for ( i=0 ; i<len ; i++ )  *dst++ = (uint32)*src++;
	}
	else{	for ( i=0 ; i<len ; i++ )  *dst++ = (uint32)*src++ * mul;
	}
}


				/* Depth conversion from 16bit to 32bit */
				/*                          (signed)    */

void sip_cvt16to32(short *src,int32 *dst,int len,int32 mul){
	int	i;
	if ( mul == 1 ){
		for ( i=0 ; i<len ; i++ )  *dst++ = (int32)*src++;
	}
	else{	for ( i=0 ; i<len ; i++ )  *dst++ = (int32)*src++ * mul;
	}
}


				/* Depth conversion from 8bit to 1bit */
				/*                          (signed)  */

void sip_cvt8to1(char *src,char *dst,int bitoff,int len,char th){
	int	i,len8;
	char	d;
	uchar	m;
	if ( bitoff != 0 ){
		m = (uchar)((int)0x80 >> bitoff);
		d = *dst;
		for ( i = 8 - bitoff ; i > 0 && len > 0 ; i--, len-- ){
			if ( *src > th )  d |= m;  else d &= ~m;
			++src;
			m >>= 1;
		}
		*dst++ = d;
	}
	len8 = len / 8;
	len = len - len8 * 8;
	for ( ; len8 > 0 ; len8-- ){
		d = 0;
		if ( *src > th )  d |= (char)0x80;
		++src;
		if ( *src > th )  d |= (char)0x40;
		++src;
		if ( *src > th )  d |= (char)0x20;
		++src;
		if ( *src > th )  d |= (char)0x10;
		++src;
		if ( *src > th )  d |= (char)0x08;
		++src;
		if ( *src > th )  d |= (char)0x04;
		++src;
		if ( *src > th )  d |= (char)0x02;
		++src;
		if ( *src > th )  d |= (char)0x01;
		++src;
		*dst++ = d;
	}
	if ( len != 0 ){
		d = *dst;
		m = 0x80;
		for ( ; len > 0 ; len-- ){
			if ( *src > th )  d |= m;  else d &= ~m;
			++src;
			m >>= 1;
		}
		*dst++ = d;
	}
}


				/* Depth conversion from 8bit to 1bit */
				/*                         (unsigned) */

void sip_cvt8to1u(uchar *src,uchar *dst,int bitoff,int len,uchar th){
	int	i,len8;
	uchar	d;
	uchar	m;
	if ( bitoff != 0 ){
		m = 0x80 >> bitoff;
		d = *dst;
		for ( i = 8 - bitoff ; i > 0 && len > 0 ; i--, len-- ){
			if ( *src > th )  d |= m;  else d &= ~m;
			++src;
			m >>= 1;
		}
		*dst++ = d;
	}
	len8 = len / 8;
	len = len - len8 * 8;
	for ( ; len8 > 0 ; len8-- ){
		d = 0;
		if ( *src > th )  d |= (uchar)0x80;
		++src;
		if ( *src > th )  d |= (uchar)0x40;
		++src;
		if ( *src > th )  d |= (uchar)0x20;
		++src;
		if ( *src > th )  d |= (uchar)0x10;
		++src;
		if ( *src > th )  d |= (uchar)0x08;
		++src;
		if ( *src > th )  d |= (uchar)0x04;
		++src;
		if ( *src > th )  d |= (uchar)0x02;
		++src;
		if ( *src > th )  d |= (uchar)0x01;
		++src;
		*dst++ = d;
	}
	if ( len != 0 ){
		d = *dst;
		m = 0x80;
		for ( ; len > 0 ; len-- ){
			if ( *src > th )  d |= m;  else d &= ~m;
			++src;
			m >>= 1;
		}
		*dst++ = d;
	}
}


				/* Depth conversion from 16bit to 1bit */
				/*                          (signed)    */

void sip_cvt16to1(short *src,char *dst,int bitoff,int len,short th){
	int	i,len8;
	char	d;
	char	m;
	if ( bitoff != 0 ){
		m = 0x80 >> bitoff;
		d = *dst;
		for ( i = 8 - bitoff ; i > 0 && len > 0 ; i--, len-- ){
			if ( *src > th )  d |= m;  else d &= ~m;
			++src;
			m >>= 1;
		}
		*dst++ = d;
	}
	len8 = len / 8;
	len = len - len8 * 8;
	for ( ; len8 > 0 ; len8-- ){
		d = 0;
		if ( *src > th )  d |= (char)0x80;
		++src;
		if ( *src > th )  d |= (char)0x40;
		++src;
		if ( *src > th )  d |= (char)0x20;
		++src;
		if ( *src > th )  d |= (char)0x10;
		++src;
		if ( *src > th )  d |= (char)0x08;
		++src;
		if ( *src > th )  d |= (char)0x04;
		++src;
		if ( *src > th )  d |= (char)0x02;
		++src;
		if ( *src > th )  d |= (char)0x01;
		++src;
		*dst++ = d;
	}
	if ( len != 0 ){
		d = *dst;
		m = (char)0x80;
		for ( ; len > 0 ; len-- ){
			if ( *src > th )  d |= m;  else d &= ~m;
			++src;
			m >>= 1;
		}
		*dst++ = d;
	}
}


				/* Depth conversion from 16bit to 8bit */
				/*                        (unsigned)   */

void sip_cvt16to8u(ushort *src,uchar *dst,int len,ushort mul,ushort div){
	int	i;
	if ( mul == 1 ){
		if ( div == 1 )
			for ( i=0 ; i<len ; i++ )  *dst++ = (uchar)*src++;
		else	for ( i=0 ; i<len ; i++ )  *dst++ = (uchar)(*src++ / div);
	}
	else{	if ( div == 1 )
			for ( i=0 ; i<len ; i++ )  *dst++ = (uchar)(*src++ * mul);
		else	for ( i=0 ; i<len ; i++ )  *dst++ = (uchar)((*src++ * mul) / div);
	}
}




/*------------------------------------------------------
	Clipping
------------------------------------------------------*/

void clip8(char *src,char *dst,int len,char lo,char hi){
	char	d;
	for ( ; len > 0 ; len-- ){
		d = *src++;
		if ( d < lo )  d = lo;
		if ( d > hi )  d = hi;
		*dst++ = d;
	}
}


void clip8u(uchar *src,uchar *dst,int len,uchar lo,uchar hi){
	uchar	d;
	for ( ; len > 0 ; len-- ){
		d = *src++;
		if ( d < lo )  d = lo;
		if ( d > hi )  d = hi;
		*dst++ = d;
	}
}


void clip16(short *src,short *dst,int len,short lo,short hi){
	short	d;
	for ( ; len > 0 ; len-- ){
		d = *src++;
		if ( d < lo )  d = lo;
		if ( d > hi )  d = hi;
		*dst++ = d;
	}
}


void clip16u(ushort *src,ushort *dst,int len,ushort lo,ushort hi){
	ushort	d;
	for ( ; len > 0 ; len-- ){
		d = *src++;
		if ( d < lo )  d = lo;
		if ( d > hi )  d = hi;
		*dst++ = d;
	}
}


void clipint(int *src,int *dst,int len,int lo,int hi){
	int	d;
	for ( ; len > 0 ; len-- ){
		d = *src++;
		if ( d < lo )  d = lo;
		if ( d > hi )  d = hi;
		*dst++ = d;
	}
}


void clipuint(uint *src,uint *dst,int len,uint lo,uint hi){
	uint	d;
	for ( ; len > 0 ; len-- ){
		d = *src++;
		if ( d < lo )  d = lo;
		if ( d > hi )  d = hi;
		*dst++ = d;
	}
}




