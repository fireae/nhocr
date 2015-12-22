/*--------------------------------------------------------------
	Image Processing functions    v2.4
		Written by H.Goto , Jul 1994
		Revised by H.Goto , Nov 1995
		Revised by H.Goto , May 1996
		Revised by H.Goto , Sep 1997
		Revised by H.Goto , Oct 1999
		Revised by H.Goto , Apr 2000
		Revised by H.Goto , Jul 2000
		Revised by H.Goto , Sep 2000
		Revised by H.Goto , Feb 2002
		Revised by H.Goto , May 2002
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


#ifdef	__cplusplus
extern "C" {
#endif

#ifndef siplib_h
#define siplib_h


#include	<utypes.h>	/* define unsigned variables */



/*------------------------------------------------------
	Common constants
------------------------------------------------------*/

#define	BOP_NOP		0
#define	BOP_PUT		1
#define	BOP_OR		2
#define	BOP_NOT		3
#define	BOP_AND		4
#define	BOP_XOR		5
#define	BOP_SET		6
#define	BOP_CLR		7




/*------------------------------------------------------
	Structure definitions for Mask-Op.
------------------------------------------------------*/

typedef struct {
	int	masksize;	/* odd integer required    */
				/*   ex.  5 for 5x5 matrix */
	int	depth;		/* = 16 */
	short	div;		/* normalize factor */
	short	*mask;
} SIPMASKs;


typedef struct {
	int	masksize;
	int	depth;		/* = 32 */
	int32	div;
	int32	*mask;
} SIPMASKl;


typedef struct {
	int	masksize;
	int	depth;		/* = 32 | 0x8000 */
	float	div;
	float	*mask;
} SIPMASKf;


typedef union {
	SIPMASKs	s;
	SIPMASKl	l;
	SIPMASKf	f;
} SIPMASK;




/*------------------------------------------------------
	Primitives
------------------------------------------------------*/

typedef struct {			/* Rectangle */
	int	x,y;
	int	width,height;
} SIPRectangle;


typedef struct {			/* Line Segment */
	int	x1,y1,x2,y2;
} SIPSegment;




/*------------------------------------------------------
	Image object
------------------------------------------------------*/

typedef struct {			/* Image (Array) Object */
	int	width,height;
	int	depth;			/* 1,8,16,32 */
	int	bitmap_pad;		/* 8,16,32 */
	int	bytes_per_line;
	char	*data;
	void	**pdata;
} SIPImage;


#ifndef siplib_c
extern SIPImage	SIPImage0;		/* Skelton of SIPImage */
#endif


extern	SIPImage*	sip_CreateImage(int width,int height,int depth);
extern	int	 	sip_DestroyImage(SIPImage *image);
extern	SIPImage*	sip_DuplicateImage(SIPImage *image);




/*------------------------------------------------------
	Image Data Manipulation
------------------------------------------------------*/

extern	int		sip_rawClearLine(char *buf,int len,int depth,int32 data);
extern	int		sip_ClearLine(SIPImage *image,int y,int32 data);
extern	int		sip_ClearImage(SIPImage *image,int32 data);
extern	int		sip_CopyArea(SIPImage *src,SIPRectangle *srect,SIPImage *dst,SIPRectangle *drect);
extern	int		sip_GetScanLine(SIPImage *image,void *buf,int x,int y,int len,int depth);
extern	int		sip_PutScanLine(SIPImage *image,void *buf,int x,int y,int len,int depth);
extern	int		sip_GetVertLine(SIPImage *image,void *buf,int x,int y,int len,int depth);
extern	int		sip_PutVertLine(SIPImage *image,void *buf,int x,int y,int len,int depth);
extern	uchar		sip_getbit(SIPImage *image,int x,int y);
extern	uchar		sip_putbit(SIPImage *image,int x,int y,uchar b);
extern	uchar		sip_orbit(SIPImage *image,int x,int y,uchar b);
extern	uchar		sip_andbit(SIPImage *image,int x,int y,uchar b);
extern	uchar		sip_xorbit(SIPImage *image,int x,int y,uchar b);

#define	_sip_getbit(image,x,y) \
	(uchar)(( *((image)->data + (int32)(( (uint)(x) )>>3) \
		+ (int32)((uint)(y)) * (image)->bytes_per_line) \
		& (0x01 << (7 - ((uint)(x) & 0x07))) ) != 0)




/*------------------------------------------------------
	Miscellaneous
------------------------------------------------------*/

extern	char*		sip_getimgptr(SIPImage *image,int y);
extern	int		sip_getlinebytes(int len,int depth,int pad);

extern	void		sip_CopyLine(char *src,char *dst,int width,int depth);




/*------------------------------------------------------
	Data conversion
------------------------------------------------------*/

extern	void		sip_cvt1to8(char *src,int bitoff,char *dst,int len,char fc,char bc);
extern	void		sip_cvt1to16(char *src,int bitoff,short *dst,int len,short fc,short bc);
extern	void		sip_cvt1to32(char *src,int bitoff,int32 *dst,int len,int32 fc,int32 bc);
extern	void		sip_cvt8to16u(uchar *src,ushort *dst,int len,ushort mul);
extern	void		sip_cvt8to16(char *src,short *dst,int len,short mul);
extern	void		sip_cvt8to32u(uchar *src,uint32 *dst,int len,uint32 mul);
extern	void		sip_cvt8to32(char *src,int32 *dst,int len,int32 mul);
extern	void		sip_cvt16to32u(ushort *src,uint32 *dst,int len,uint32 mul);
extern	void		sip_cvt16to32(short *src,int32 *dst,int len,int32 mul);

extern	void		sip_cvt8to1(char *src,char *dst,int bitoff,int len,char th);
extern	void		sip_cvt8to1u(uchar *src,uchar *dst,int bitoff,int len,uchar th);
extern	void		sip_cvt16to1(short *src,char *dst,int bitoff,int len,short th);
extern	void		sip_cvt16to8u(ushort *src,uchar *dst,int len,ushort mul,ushort div);



/*------------------------------------------------------
	Clipping
------------------------------------------------------*/

extern	void		clip8(char *src,char *dst,int len,char lo,char hi);
extern	void		clip8u(uchar *src,uchar *dst,int len,uchar lo,uchar hi);
extern	void		clip16(short *src,short *dst,int len,short lo,short hi);
extern	void		clip16u(ushort *src,ushort *dst,int len,ushort lo,ushort hi);
extern	void		clipint(int *src,int *dst,int len,int lo,int hi);
extern	void		clipuint(uint *src,uint *dst,int len,uint lo,uint hi);




/*------------------------------------------------------
	Image rotation
------------------------------------------------------*/

extern	int		sip_rotate1(SIPImage *src,SIPImage *dst, \
					int x0,int y0,double angle,int mode);
extern	int		sip_rotate8(SIPImage *src,SIPImage *dst, \
					int x0,int y0,double angle,int mode);




/*------------------------------------------------------
	Some useful functions
------------------------------------------------------*/

extern	int		sip_thin10neib(SIPImage *image,char *cflag,char *rflag);
extern	int		sip_distimage(SIPImage *image,SIPImage *distimage,int maxdist);

extern	long		sip_projprofile(SIPImage *image,long *rden,long *cden);
extern	long		sip_projprofile_area(SIPImage *image, \
				long *rden,long *cden,SIPRectangle *cbox);

extern	int		sip_kNNsmooth(SIPImage *src,SIPImage *dst,int pixels,int ctweight);


#endif	/* siplib_h */

#ifdef	__cplusplus
}
#endif
