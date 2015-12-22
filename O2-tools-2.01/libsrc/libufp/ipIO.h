/*--------------------------------------------------------------
    IP-format file I/O class Header Include File
		Written  by H.Goto , Jan. 1994
		Modified by H.Goto , Feb. 1997
		Modified by H.Goto , Apr. 2000
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1994-2000  Hideaki Goto

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


#ifndef ipIO_h
#define ipIO_h

#include	"utypes.h"


/*------------------------------------------------------
	IP-format file Load Class
------------------------------------------------------*/

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




/*------------------------------------------------------
	IP-format file Save Class
------------------------------------------------------*/

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


#endif		//  ipIO_h

