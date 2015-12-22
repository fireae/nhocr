/*--------------------------------------------------------------
	Rasterfile I/O class Header Include File   Rev.970218
		Written  by H.Goto , Jan. 1993
		Modified by H.Goto , Feb. 1997
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1993-1997  Hideaki Goto

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

#ifndef rasIO_h
#define rasIO_h

#include	"rasterfile.h"
#include	"utypes.h"


/*------------------------------------------------------
	Rasterfile Load Class
------------------------------------------------------*/

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




/*------------------------------------------------------
	Rasterfile Save Class
------------------------------------------------------*/

class RASSAVE {
    private:
	FILE		*fp;
	char		wfname[256];
	int		line;
	int		width,height,pixel;
	int		width2,height2;
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



#endif		//  rasIO_h

