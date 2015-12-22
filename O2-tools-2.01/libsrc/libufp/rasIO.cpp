/*--------------------------------------------------------------
	Rasterfile I/O class Library File
		Written by H.Goto , Sep. 1994
		Revised by H.Goto , Feb. 1996
		Revised by H.Goto , May  1996
		Revised by H.Goto , Feb. 1997
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


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"rasterfile.h"

#include	"comlib.h"
#include	"rasIO.h"
#include	"utypes.h"




/*------------------------------------------------------
	Rasterfile Load Class
------------------------------------------------------*/

#ifndef rasIO_h
class RASLOAD {
    private:
	FILE		*fp;
	int		line;
	int		width,height,pixel;
	int		linebytes;
	uchar		*linebuffer;
	struct rasterfile  raster_header;
    	int		filetype;	/* = 0 : 1bit/pixel     */
					/*   1 : 8bit grayscale */
	uchar		fgcol,bgcol;
    protected:
	void		convert_1to8(uchar *src,uchar *dst,int size);
    public:
	int		fileopen(char *path);
	int		fileclose(void);
	int		readLine(void *buf);
	int		readLine_gray(void *buf);
	int		getWidth(void)  {  return width; }
	int		getHeight(void) {  return height; }
	int		getFiletype(void) {  return filetype; }
	void		setfgcolor(uchar fgcol,uchar bgcol);

    			RASLOAD(void);
	virtual		~RASLOAD(void);
};
#endif


RASLOAD :: RASLOAD(void){
	fp = NULL;
	linebuffer = 0;
	bgcol = 0xff;
	fgcol = 0x00;
}


RASLOAD :: ~RASLOAD(void){
	fileclose();
	return;
}


void RASLOAD :: setfgcolor(uchar fgcol,uchar bgcol){
	RASLOAD::fgcol = fgcol;
	RASLOAD::bgcol = bgcol;
}


int RASLOAD :: fileopen(char *path){
	int	eflag;
	if ( 0 == strcmp(path,"stdin") ){
		fp = stdin;
	}
	else{	if ( NULL == (fp = fopen(path,"rb") ) ){
			return(-1);	// Open failed.
		}
	}
	line = 0;
	if ( 32 != fread((char *)&raster_header,1,32,fp) ){
		return(-2);	// Read failed.
	}

	ifLSB{
		_uf_dwordswap((uint32 *)&raster_header.ras_magic);
		_uf_dwordswap((uint32 *)&raster_header.ras_width);
		_uf_dwordswap((uint32 *)&raster_header.ras_height);
		_uf_dwordswap((uint32 *)&raster_header.ras_depth);
		_uf_dwordswap((uint32 *)&raster_header.ras_length);
		_uf_dwordswap((uint32 *)&raster_header.ras_type);
		_uf_dwordswap((uint32 *)&raster_header.ras_maptype);
		_uf_dwordswap((uint32 *)&raster_header.ras_maplength);
	}

	eflag=0;
	if ( raster_header.ras_magic != RAS_MAGIC  )  eflag=-1;
	if ( raster_header.ras_type != RT_STANDARD )  eflag=-1;
	if ( raster_header.ras_maptype != RMT_NONE )  eflag=-1;
	if ( raster_header.ras_maplength != 0 )  eflag=-1;
	if ( ( raster_header.ras_depth != 1 ) &&  \
	     ( raster_header.ras_depth != 8 ) )  eflag=-1;
	if ( eflag )  return(-3);	// Non-supported file.
	width  = raster_header.ras_width;
	height = raster_header.ras_height;
	if ( raster_header.ras_depth == 1 ){
		filetype = 0;	pixel = 1;	linebytes = ((width+15)/16)*2;
	}
	else{	filetype = 1;	pixel = 8;	linebytes = ((width+1)/2)*2;
	}
	if ( 0 == (linebuffer = new uchar[linebytes]) )  return(-4);
					// Memory not enough.
	return(0);
}


int RASLOAD :: fileclose(void){
	int	retcode;
	if ( linebuffer != 0 ){  delete linebuffer;  linebuffer = 0; }
	if ( (fp == NULL) || (fp == stdin) )  return(0);
	retcode = fclose(fp);
	fp = NULL;
	return( retcode );
}


void RASLOAD :: convert_1to8(uchar *src,uchar *dst,int size){
	int	size2;
	int	cc;
	uchar	fgcol,bgcol;
	size2 = size % 8;
	size /= 8;
	fgcol = RASLOAD :: fgcol;
	bgcol = RASLOAD :: bgcol;
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


int RASLOAD :: readLine(void *buf){
	if ( 1 != fread(buf,linebytes,1,fp) )  return(-1);
	return(0);
}


int RASLOAD :: readLine_gray(void *buf){
	int	retcode;
	if ( filetype == 1 )  return( readLine(buf) );
	retcode = readLine(linebuffer);
	if ( retcode != 0 )  return(retcode);
	convert_1to8((uchar *)linebuffer,(uchar *)buf,width);
	return(0);
}




/*------------------------------------------------------
	Rasterfile Save Class
------------------------------------------------------*/

#ifndef rasIO_h
class RASSAVE {
    private:
	FILE		*fp;
	char		wfname[256];
	int		width,height,pixel;
	int		linebytes;
	uchar		*linebuffer;
	struct rasterfile  raster_header;
    	int		filetype;	/* = 0 : 1bit/pixel     */
					/*   1 : 8bit grayscale */
    protected:
	void		convert_8to1(uchar *src,uchar *dst,int size,uchar threshold);
    public:
	int		filecreat(char *path,int filetype,int width,int height);
	int		fileclose(void);
	int		writeLine(void *buf);
	int		writeLine_mono(void *buf,int threshold);
    			RASSAVE(void);
	virtual		~RASSAVE(void);
};
#endif


RASSAVE :: RASSAVE(void){
	fp = NULL;
	wfname[0]=0;
	linebuffer = 0;
}


RASSAVE :: ~RASSAVE(void){
	if ( fp != NULL ){
		fileclose();
		remove(wfname);
	}
	return;
}


int RASSAVE :: filecreat(char *path,int filetype,int width,int height){
	if ( (filetype != 0) && (filetype != 1) ){
		return(-3);	// Parameter error.
	}
	if ( width  == 0 )  return(-3);
	if ( height == 0 )  return(-3);
	RASSAVE::filetype = filetype;
	RASSAVE::width    = width;
	if ( 0 == strcmp(path,"stdout") ){
		fp = stdout;
	}
	else{	if ( NULL == (fp = fopen(path,"wb")) ){
			return(-1);	// Open failed.
		}
	}
	strcpy(wfname,path);
	if ( filetype == 0 ){
		width = ((width+15)/16)*16;
		linebytes = width/8;
		raster_header.ras_depth = 1;
		raster_header.ras_length = ( width * height ) /8;
	}
	else{	width = ((width+1)/2)*2;
		linebytes = width;
		raster_header.ras_depth = 8;
		raster_header.ras_length = ( width * height );
	}
	raster_header.ras_magic  = RAS_MAGIC;
	raster_header.ras_width  = RASSAVE::width;
	raster_header.ras_height = height;
	raster_header.ras_type   = RT_STANDARD;
	raster_header.ras_maptype = RMT_NONE;
	raster_header.ras_maplength = 0;
	if ( 0 == (linebuffer = new uchar[linebytes]) )  return(-4);
					// Memory not enough.

	ifLSB{
		_uf_dwordswap((uint32 *)&raster_header.ras_magic);
		_uf_dwordswap((uint32 *)&raster_header.ras_width);
		_uf_dwordswap((uint32 *)&raster_header.ras_height);
		_uf_dwordswap((uint32 *)&raster_header.ras_depth);
		_uf_dwordswap((uint32 *)&raster_header.ras_length);
		_uf_dwordswap((uint32 *)&raster_header.ras_type);
		_uf_dwordswap((uint32 *)&raster_header.ras_maptype);
		_uf_dwordswap((uint32 *)&raster_header.ras_maplength);
	}

	if ( 32 != fwrite(&raster_header,1,32,fp) ){
		return(-2);		// Write failed.
	}
	return(0);
}


int RASSAVE :: fileclose(void){
	int	retcode;
	if ( linebuffer != 0 ){  delete linebuffer;  linebuffer = 0; }
	if ( (fp == NULL) || (fp == stdout) ) return(0);
	retcode = fclose(fp);
	fp = NULL;
	if ( retcode != 0 )  remove(wfname);
	wfname[0]=0;
	return( retcode );
}


void RASSAVE :: convert_8to1(uchar *src,uchar *dst,int size,uchar threshold){
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


int RASSAVE :: writeLine(void *buf){
	if ( 1 != fwrite(buf,linebytes,1,fp) )  return(-1);
	return(0);
}


int RASSAVE :: writeLine_mono(void *buf,int threshold){
//	int	retcode;
//	if ( filetype != 0 )  return( writeLine(buf) );
	convert_8to1((uchar *)buf,(uchar *)linebuffer,width,(uchar)threshold);
	return( writeLine(linebuffer) );
}


