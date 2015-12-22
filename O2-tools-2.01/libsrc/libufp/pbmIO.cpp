/*--------------------------------------------------------------
	IP-format file I/O class Library File
		Written  by H.Goto , Jan. 1995
		Modified by H.Goto , May. 1996
		Modified by H.Goto , Jun. 1996
		Modified by H.Goto , Jul. 1996
		Modified by H.Goto , Feb. 1997
		Modified by H.Goto , Aug. 2014
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1995-2014  Hideaki Goto

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
  author, and (iv) the notice of the modification is specified in
  case of that the modified copies of this software are distributed.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
  THE AUTHOR WILL NOT BE RESPONSIBLE FOR ANY DAMAGE CAUSED BY THIS
  SOFTWARE.
--------------------------------------------------------------------*/


#define		WidthLIMIT	32768
#define		HeightLIMIT	32768

#define		FMODE_READ	0
#define		FMODE_WRITE	1

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"comlib.h"
#include	"pbmIO.h"



/*------------------------------------------------------
	PBM (raw) file I/O Class
------------------------------------------------------*/

#if 0
class PBMFILE {
    private:
	FILE		*fp;
	char		*wfname;
	int		mode;
	int		cline;
	int		width,height,pixel;
	int		flbytes,linebytes;
	uchar		*linebuffer;
 	int		filetype;	/* = 0 : 1bit/pixel     */
					/*   1 : 8bit grayscale */
	uchar		fgcol,bgcol;
	int		checksize();
	int		write_header();
	int		read_header();
	int		seekto(int line);
    public:
	int		open(char *path,char *type);
	int		close(void);

	int		setpal_fb(uchar fg,uchar bg);
	int		readline(int line,void *buf);
	int		readline_gray(int line,void *buf);
	int		getwidth(void)  {  return width; }
	int		getheight(void) {  return height; }
	int		getfiletype(void) {  return filetype; }

	int		writeline(int line,void *buf);
	int		writeline_bilevel(int line,void *buf,int threshold);
	int		setsize(int width,int height,int pixel);

			PBMFILE();
	virtual		~PBMFILE();
};
#endif


PBMFILE :: PBMFILE(){
	fp = 0;
	wfname = 0;
	linebuffer = 0;
	fgcol = 0x00;
	bgcol = 0xff;
}


PBMFILE :: ~PBMFILE(){
	if ( fp != 0 ){
		fclose(fp);
		if ( wfname != 0 )  remove(wfname);
	}
	if ( wfname != 0 )  delete []wfname;
	if ( linebuffer != 0 )  delete []linebuffer;
}


int PBMFILE :: checksize(){
	switch(filetype){
	    case 0:
	    case 1:
	    case 2:
		break;
	    default:
		return(-1);
	}
	if ( (uint)width  > WidthLIMIT  )  return(-1);
	if ( (uint)height > HeightLIMIT )  return(-1);
	return(0);
}


int PBMFILE :: read_header(){
	char	*buf;
	int	ft = -1;
	int	maxgray;
	int	errflag = 0;
	if ( 0 == (buf = new char[80]) )  return(-1);
	if ( NULL == fgets(buf,80,fp) ){  delete []buf;  return(-1); }
	if ( 0 == strncmp("P4",buf,2) )  ft = 0;
	if ( 0 == strncmp("P5",buf,2) )  ft = 1;
	if ( 0 == strncmp("P6",buf,2) )  ft = 2;
	if ( -1 == ft ){  delete []buf;  return(-5); }
	filetype = ft;
	while (1){
		if ( NULL == fgets(buf,80,fp) ){  delete []buf;  return(-1); }
		if ( *buf != '#' && *buf != '\n' )  break;
	}
	if ( 2 != sscanf(buf,"%d %d",&width,&height) ){  delete []buf;  return(-4); }
	if ( filetype == 1 || filetype == 2 ){
		while (1){
			if ( NULL == fgets(buf,80,fp) ){  errflag = -1;  break; }
			if ( *buf != '#' && *buf != '\n' )  break;
		}
		if ( 1 != sscanf(buf,"%d",&maxgray) )  errflag = -4;
		if ( maxgray > 255 )  errflag = -5;
	}
	delete []buf;
	if ( errflag )  return(errflag);
	if ( checksize() )  return(-5);		/* illegal data */
	switch ( filetype ){
	    case 0:
		flbytes = linebytes = (width + 7) /8;  break;
	    case 1:
		flbytes = linebytes = width;  break;
	    case 2:
		flbytes = linebytes = 3 * width;  break;
	}
	if ( linebuffer != 0 )  delete []linebuffer;
	if ( 0 == (linebuffer = new uchar[flbytes]) )  return(-6);
	return(0);
}


int PBMFILE :: write_header(){
	switch ( filetype ){
	    case 0:
		if ( EOF == fprintf(fp,"P4\n%d %d\n",width,height) )  return(-1);
		flbytes = linebytes = (width + 7) /8;
		break;
	    case 1:
		if ( EOF == fprintf(fp,"P5\n%d %d\n255\n",width,height) )  return(-1);
		flbytes = linebytes = width;
		break;
	    case 2:
		if ( EOF == fprintf(fp,"P6\n%d %d\n255\n",width,height) )  return(-1);
		flbytes = linebytes = 3 * width;
		break;
	}
	if ( linebuffer != 0 )  delete linebuffer;
	if ( 0 == (linebuffer = new uchar[flbytes]) )  return(-6);
	return(0);
}


int PBMFILE :: open(char *path,char *type){
	int	mode = -1;
	int	rc;
	if ( fp != 0 )  return(-3);	/* file reopened */
	if ( 0 == strncmp("r",type,2) )  mode = FMODE_READ;
	if ( 0 == strncmp("w",type,2) )  mode = FMODE_WRITE;
	if ( -1 == mode )  return(-4);	/* type unknown */
	PBMFILE::mode = mode;
	if ( 0 == strcmp("-",path) || *path == 0 ){
		if ( mode == FMODE_READ ){
			fp = stdin;
			if ( 0 > (rc = read_header()) )  return(rc);
		}
		else{	fp = stdout;
			if ( write_header() )  return(-2);	/* r/w error */
		}
		cline = 0;
		return(0);
	}
	switch (mode){
	    case FMODE_READ:
		if ( 0 == (fp = fopen(path,"rb")) )  return(-1);	/* open error */
		if ( 0 > (rc = read_header()) ){
			fclose(fp);	fp = 0;
			return(rc);
		}
		break;
	    case FMODE_WRITE:
		if ( checksize() )  return(-5);		/* illegal data */
		if ( 0 == (wfname = new char[ strlen(path)+1 ]) )  return(-6);
							/* memory error */
		strcpy(wfname,path);
		if ( 0 == (fp = fopen(path,"wb")) ){
			delete []wfname;  wfname = 0;
			return(-1);
		}
		if ( write_header() ){
			fclose(fp);	fp = 0;
			remove(wfname);
			delete []wfname;  wfname = 0;
			return(-2);
		}
		break;
	}
	cline = 0;
	return(0);
}


int PBMFILE :: seekto(int line){
	if ( -1 != line ){
		if ( line != cline )  return(-1);
			/* Seeking is not yet implemented */
	}
	return(0);
}


int PBMFILE :: setpal_fb(uchar fg,uchar bg){
	fgcol = fg;  bgcol = bg;
	return(0);
}


int PBMFILE :: close(void){
	int	rc;
	if ( fp == 0 )  return(-1);
	if ( linebuffer != 0 ){
		delete []linebuffer;  linebuffer = 0;
	}
	switch(mode){
	    case FMODE_READ:
		if ( 0 != fclose(fp) )  return(-1);
		fp = 0;
		break;
	    case FMODE_WRITE:
		rc = fclose(fp);  fp = 0;
		if ( rc != 0 && wfname != 0 )  remove(wfname);
		if ( wfname != 0 ){
			delete []wfname;  wfname = 0;
		}
		if ( rc != 0 )  return(-1);
		break;
	}
	return(0);
}


int PBMFILE :: readline(int line,void *buf){
	if ( seekto(line) )  return(-1);
	if ( mode != FMODE_READ )  return(-3);	/* phase error */
	if ( 1 != fread(buf,flbytes,1,fp) )  return(-1);
	++cline;
	return(0);
}


int PBMFILE :: readline_gray(int line,void *buf){
	int     retcode;
	if ( filetype == 1 )  return( readline(line,buf) );
	if ( filetype == 2 )  return(-1);
	retcode = readline(line,linebuffer);
	if ( retcode != 0 )  return(retcode);
	_uf_convert_1to8((uchar *)linebuffer,(uchar *)buf,width,fgcol,bgcol);
	return(0);
}


int PBMFILE :: writeline(int line,void *buf){
	if ( seekto(line) )  return(-1);
	if ( mode != FMODE_WRITE )  return(-3);	/* phase error */
	if ( 1 != fwrite(buf,flbytes,1,fp) )  return(-1);
	++cline;
	return(0);
}


int PBMFILE :: writeline_bilevel(int line,void *buf,int threshold){
	_uf_convert_8to1((uchar *)buf,(uchar *)linebuffer,width,(uchar)threshold);
	return( writeline(line,linebuffer) );
}


int PBMFILE :: setsize(int width,int height,int pixel){
	PBMFILE::width  = width;
	PBMFILE::height = height;
	PBMFILE::pixel  = pixel;
	switch(pixel){
	    case 1:
		filetype = 0;
		break;
	    case 8:
		filetype = 1;
		break;
	    case 24:
		filetype = 2;
		break;
	}
	if ( checksize() )  return(-1);
	return(0);
}


