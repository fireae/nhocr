/*--------------------------------------------------------------
	IP-format file I/O class Library File
		Written by H.Goto , Jan. 1994
		Revised by H.Goto , Feb. 1996
		Revised by H.Goto , May  1996
		Revised by H.Goto , Apr. 2000
		Revised by H.Goto , Nov. 2008
		Revised by H.Goto , Aug. 2014
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1994-2014  Hideaki Goto

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

#define		WidthLIMIT	32768
#define		HeightLIMIT	32768


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"ipIO.h"
#include	"utypes.h"
#include	"comlib.h"




/*------------------------------------------------------
	IP-format file Load Class
------------------------------------------------------*/

#ifndef ipIO_h
class IPLOAD {
    private:
	FILE		*fp;
	int		line;
	int		width,height,pixel;
	int		linebytes;
	int		byteorder,headersize;
	uchar		*linebuffer;
	struct {
	  ushort		width_bytes;
	  ushort		height;
	} ip_header;
	struct {
	  uint32	width_bytes;
	  uint32	height;
	} ip_header4;
    	int		filetype;	/* = 0 : 1bit/pixel     */
					/*   1 : 8bit grayscale */
	uchar		fgcol,bgcol;
	void		wordswap(ushort *d);
    protected:
	void		convert_1to8(uchar *src,uchar *dst,int size);
    public:
	int		setmode(int byteorder,int headersize);
					/* byteorder  = 0 : MSB First */
					/*              1 : LSB First */
					/* headersize = 2 : WORD  */
					/*              4 : DWORD */
	int		fileopen(char *path);
	int		fileclose(void);
	int		readLine(void *buf);
	int		readLine_gray(void *buf);
	int		getWidth(void)  {  return width; }
	int		getHeight(void) {  return height; }
	int		getFiletype(void) {  return filetype; }
    			IPLOAD(void);
	virtual		~IPLOAD(void);
};
#endif


IPLOAD :: IPLOAD(void){
	fp = NULL;
	linebuffer = 0;
	bgcol = 0xff;
	fgcol = 0x00;
	byteorder = 0;
	headersize = 2;
}


IPLOAD :: ~IPLOAD(void){
	fileclose();
	return;
}


void IPLOAD :: wordswap(ushort *d){
	unsigned char	H,L;
	L = (uchar)*d;
	H = (uchar)(*d >> 8);
	*d = ((ushort)L << 8) | (ushort)H;
}


int IPLOAD :: setmode(int byteorder,int headersize){
	switch ( byteorder ){
	  case 0:
	  case 1:	IPLOAD :: byteorder = byteorder;
			break;
	  default:	return(-1);
	}
	switch ( headersize ){
	  case 2:
	  case 4:	IPLOAD :: headersize = headersize;
			break;
	  default:	return(-1);
	}
	return(0);
}


int IPLOAD :: fileopen(char *path){
	int	eflag;
	if ( 0 == strcmp(path,"stdin") ){
		fp = stdin;
	}
	else{	if ( NULL == (fp = fopen(path,"rb") ) ){
			return(-1);	// Open failed.
		}
	}
	line = 0;
	if ( headersize == 2 ){
		if ( 1 != fread((char *)&ip_header,sizeof(ip_header),1,fp) ){
			return(-2);	// Read failed.
		}
		ifLSB {
			if ( ! byteorder ){
				wordswap(&ip_header.width_bytes);
				wordswap(&ip_header.height);
			}
		}
		else{
			if ( byteorder ){
				wordswap(&ip_header.width_bytes);
				wordswap(&ip_header.height);
			}
		}
	}
	else{	return(-3);		// Non-supported file.
	}
	eflag=0;
	if ( (ip_header.width_bytes == 0) || (ip_header.height == 0) )  eflag = -1;
	if ( ip_header.width_bytes > ((WidthLIMIT +7) /8) )  eflag = -1;
	if ( ip_header.height      > HeightLIMIT )  eflag = -1;
	if ( eflag )  return(-3);	// Non-supported file.
	width  = (int)ip_header.width_bytes * 8;
	height = (int)ip_header.height;
	filetype = 0;
	pixel = 1;
	linebytes = ((width+7)/8);
	if ( 0 == (linebuffer = new uchar[linebytes]) )  return(-4);
					// Memory not enough.
	return(0);
}


int IPLOAD :: fileclose(void){
	int	retcode;
	if ( linebuffer != 0 ){  delete linebuffer;  linebuffer = 0; }
	if ( (fp == NULL) || (fp == stdin) )  return(0);
	retcode = fclose(fp);
	fp = NULL;
	return( retcode );
}


void IPLOAD :: convert_1to8(uchar *src,uchar *dst,int size){
	int	size2;
	int	cc;
	size2 = size % 8;
	size /= 8;
	for( ; size > 0 ; --size ){
		cc = (int)*(src++);
		if((cc&0x80)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
		if((cc&0x40)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
		if((cc&0x20)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
		if((cc&0x10)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
		if((cc&0x08)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
		if((cc&0x04)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
		if((cc&0x02)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
		if((cc&0x01)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
	}
	if ( size2 != 0 ){
		cc = (int)*src;
		for ( ; size2 > 0 ; --size2 ){
			if((cc&0x80)==0) *(dst++)=bgcol; else *(dst++)=fgcol;
			cc <<= 1;
		}
	}
}


int IPLOAD :: readLine(void *buf){
	if ( 1 != fread(buf,linebytes,1,fp) )  return(-1);
	return(0);
}


int IPLOAD :: readLine_gray(void *buf){
	int	retcode;
	if ( filetype == 1 )  return( readLine(buf) );
	retcode = readLine(linebuffer);
	if ( retcode != 0 )  return(retcode);
	convert_1to8((uchar *)linebuffer,(uchar *)buf,width);
	return(0);
}




/*------------------------------------------------------
	IP-format file Save Class
------------------------------------------------------*/

#ifndef ipIO_h
class IPSAVE {
    private:
	FILE		*fp;
	char		wfname[256];
	int		width,height,pixel;
	int		linebytes;
	int		byteorder,headersize;
	uchar		*linebuffer;
	struct {
	  ushort		width_bytes;
	  ushort		height;
	} ip_header;
    	int		filetype;	/* = 0 : 1bit/pixel     */
					/*   1 : 8bit grayscale */
    protected:
	void		convert_8to1(uchar *src,uchar *dst,int size,uchar threshold);
    public:
	int		setmode(int byteorder,int headersize);
					/* This function is now dummy. */
	int		filecreat(char *path,int filetype,int width,int height);
	int		fileclose(void);
	int		writeLine(void *buf);
	int		writeLine_mono(void *buf,int threshold);
    			IPSAVE(void);
	virtual		~IPSAVE(void);
};
#endif


IPSAVE :: IPSAVE(void){
	fp = NULL;
	wfname[0]=0;
	linebuffer = 0;
	byteorder = 0;
	headersize = 2;
}


IPSAVE :: ~IPSAVE(void){
	if ( fp != NULL ){
		fileclose();
		remove(wfname);
	}
	return;
}


int IPSAVE :: setmode(int byteorder,int headersize){
	switch ( byteorder ){
	  case 0:
	  case 1:	IPSAVE :: byteorder = byteorder;
			break;
	  default:	return(-1);
	}
	switch ( headersize ){
	  case 2:
	  case 4:	IPSAVE :: headersize = headersize;
			break;
	  default:	return(-1);
	}
	return(0);
}


int IPSAVE :: filecreat(char *path,int filetype,int width,int height){
	if ( (filetype != 0) && (filetype != 1) ){
		return(-3);	// Parameter error.
	}
	if ( width  <= 0 )  return(-3);
	if ( height <= 0 )  return(-3);
	if ( (width > WidthLIMIT) || (height > HeightLIMIT) )  return(-3);
	IPSAVE::filetype = filetype;
	IPSAVE::width    = width;
	if ( 0 == strcmp(path,"stdout") ){
		fp = stdout;
	}
	else{	if ( NULL == (fp = fopen(path,"wb")) ){
			return(-1);	// Open failed.
		}
	}
	strcpy(wfname,path);
	if ( filetype == 0 ){
		width = ((width+7)/8)*8;
		linebytes = width/8;
	}
	else{	linebytes = width;
	}
	ip_header.width_bytes = linebytes;
	ip_header.height      = height;
	if ( 0 == (linebuffer = new uchar[linebytes]) )  return(-4);
					// Memory not enough.
	if ( 1 != fwrite(&ip_header,sizeof(ip_header),1,fp) ){
		return(-2);		// Write failed.
	}
	return(0);
}


int IPSAVE :: fileclose(void){
	int	retcode;
	if ( linebuffer != 0 ){  delete linebuffer;  linebuffer = 0; }
	if ( (fp == NULL) || (fp == stdout) ) return(0);
	retcode = fclose(fp);
	fp = NULL;
	if ( retcode != 0 )  remove(wfname);
	wfname[0]=0;
	return( retcode );
}


void IPSAVE :: convert_8to1(uchar *src,uchar *dst,int size,uchar threshold){
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


int IPSAVE :: writeLine(void *buf){
	if ( 1 != fwrite(buf,linebytes,1,fp) )  return(-1);
	return(0);
}


int IPSAVE :: writeLine_mono(void *buf,int threshold){
//	int	retcode;
//	if ( filetype != 0 )  return( writeLine(buf) );
	convert_8to1((uchar *)buf,(uchar *)linebuffer,width,(uchar)threshold);
	return( writeLine(linebuffer) );
}


