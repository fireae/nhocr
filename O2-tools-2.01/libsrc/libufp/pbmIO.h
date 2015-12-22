/*--------------------------------------------------------------
    PBM (raw) file I/O class Header Include File   Rev.970218
		Written  by H.Goto , Jan. 1995
		Modified by H.Goto , July 1996
		Modified by H.Goto , Feb. 1997
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1995-1997  Hideaki Goto

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


#ifndef pbmIO_h
#define pbmIO_h

#include	<utypes.h>


/*------------------------------------------------------
	PBM (raw) file I/O Class
------------------------------------------------------*/

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
					/*   2 : 24bit color */
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

			PBMFILE(void);
	virtual		~PBMFILE(void);
};


#endif		//  pbmIO_h

