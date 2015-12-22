/*--------------------------------------------------------------
	Image Processing function library    libsip
	  (Rotate image Rev.081230)
		Written by H.Goto , Nov 1995
		Revised by H.Goto , May 1996
		Revised by H.Goto , May 2002
		Revised by H.Goto , Dec 2008
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
#include	<math.h>

#include	"siplib.h"


/*------------------------------------------------------
	Rotate image
------------------------------------------------------*/

int sip_rotate1(SIPImage *src,SIPImage *dst,int x0,int y0,double angle,int mode){
					/* mode = BOP_XXX */
	int	sx,sy,dx,dy;
	int	sx0,sy0;
	int	dx1,dy1,dx2,dy2,dx10;
	double	rc,rs,rsy,rcy;
	double	*ctbl,*stbl;
	int	lnlen;
	if ( src->depth != 1 )  return(-1);
	if ( dst->depth != 1 )  return(-1);
	rc = cos(-angle);
	rs = sin(-angle);
	sx0 = (unsigned int)(src->width  +1) /2;
	sy0 = (unsigned int)(src->height +1) /2;
	dx2 = (int)((double)sx0 * fabs(rc) + (double)sy0 * fabs(rs));
	if ( 0 > (dx1 = x0 - dx2) )  dx1 = 0;
	if ( dst->width <= (dx2 += x0) )  dx2 = dst->width -1;
	dy2 = (int)((double)sx0 * fabs(rs) + (double)sy0 * fabs(rc));
	if ( 0 > (dy1 = y0 - dy2) )  dy1 = 0;
	if ( dst->height <= (dy2 += y0) )  dy2 = dst->height -1;
	dx10 = dx1;
	dx1 -= x0;	dx2 -= x0;
	lnlen = dx2 - dx1 +1;
	if ( NULL == (ctbl = (double *)malloc(2 * lnlen * sizeof(double))) )  return(-1);
	stbl = ctbl + lnlen;
	for ( dx=0 ; dx<lnlen ; dx++ ){
		ctbl[dx] = rc * (double)(dx + dx1);
		stbl[dx] = rs * (double)(dx + dx1);
	}
	for ( dy=dy1 ; dy<=dy2 ; dy++ ){
		rsy = rs * (double)(dy - y0);
		rcy = rc * (double)(dy - y0);
		switch ( mode ){
		    case BOP_PUT:
			for ( dx=0 ; dx<lnlen ; dx++ ){
				sx = sx0 + (int)(ctbl[dx] + rsy);
				if ( sx < 0 )  continue;
				if ( sx >= src->width  )  continue;
				sy = sy0 - (int)(stbl[dx] - rcy);
				if ( sy < 0 )  continue;
				if ( sy >= src->height )  continue;
				sip_putbit(dst,(dx + dx10),dy,_sip_getbit(src,sx,sy));
			}
			break;
		    case BOP_OR:
			for ( dx=0 ; dx<lnlen ; dx++ ){
				sx = sx0 + (int)(ctbl[dx] + rsy);
				if ( sx < 0 )  continue;
				if ( sx >= src->width  )  continue;
				sy = sy0 - (int)(stbl[dx] - rcy);
				if ( sy < 0 )  continue;
				if ( sy >= src->height )  continue;
				sip_orbit(dst,(dx + dx10),dy,_sip_getbit(src,sx,sy));
			}
			break;
		    case BOP_AND:
			for ( dx=0 ; dx<lnlen ; dx++ ){
				sx = sx0 + (int)(ctbl[dx] + rsy);
				if ( sx < 0 )  continue;
				if ( sx >= src->width  )  continue;
				sy = sy0 - (int)(stbl[dx] - rcy);
				if ( sy < 0 )  continue;
				if ( sy >= src->height )  continue;
				sip_andbit(dst,(dx + dx10),dy,_sip_getbit(src,sx,sy));
			}
			break;
		}
	}
	free((char *)ctbl);
	return(0);
}




int sip_rotate8(SIPImage *src,SIPImage *dst,int x0,int y0,double angle,int mode){
					/* mode = BOP_XXX */
	int	sx,sy,dx,dy;
	int	sx0,sy0;
	int	dx1,dy1,dx2,dy2;
	double	rc,rs,rsy,rcy;
	double	*ctbl,*stbl;
	uchar	*dstp;
	int	lnlen;
	if ( src->depth != 8 )  return(-1);
	if ( dst->depth != 8 )  return(-1);
	rc = cos(-angle);
	rs = sin(-angle);
	sx0 = (unsigned int)(src->width  +1) /2;
	sy0 = (unsigned int)(src->height +1) /2;
	dx2 = (int)((double)sx0 * fabs(rc) + (double)sy0 * fabs(rs));
	if ( 0 > (dx1 = x0 - dx2) )  dx1 = 0;
	if ( dst->width <= (dx2 += x0) )  dx2 = dst->width -1;
	dy2 = (int)((double)sx0 * fabs(rs) + (double)sy0 * fabs(rc));
	if ( 0 > (dy1 = y0 - dy2) )  dy1 = 0;
	if ( dst->height <= (dy2 += y0) )  dy2 = dst->height -1;
	dx1 -= x0;	dx2 -= x0;
	lnlen = dx2 - dx1 +1;
	if ( NULL == (ctbl = (double *)malloc(2 * lnlen * sizeof(double))) )  return(-1);
	stbl = ctbl + lnlen;
	for ( dx=0 ; dx<lnlen ; dx++ ){
		ctbl[dx] = rc * (double)(dx + dx1);
		stbl[dx] = rs * (double)(dx + dx1);
	}
	for ( dy=dy1 ; dy<=dy2 ; dy++ ){
		dstp = (uchar *)sip_getimgptr(dst,dy) + x0 + dx1;
		rsy = rs * (double)(dy - y0);
		rcy = rc * (double)(dy - y0);
		switch ( mode ){
		    case BOP_PUT:
			for ( dx=0 ; dx<lnlen ; dx++ ){
				sx = sx0 + (int)(ctbl[dx] + rsy);
				if ( sx < 0 )  continue;
				if ( sx >= src->width  )  continue;
				sy = sy0 - (int)(stbl[dx] - rcy);
				if ( sy < 0 )  continue;
				if ( sy >= src->height )  continue;
				dstp[dx] = sip_getimgptr(src,sy)[sx];
			}
			break;
		    case BOP_OR:
			for ( dx=0 ; dx<lnlen ; dx++ ){
				sx = sx0 + (int)(ctbl[dx] + rsy);
				if ( sx < 0 )  continue;
				if ( sx >= src->width  )  continue;
				sy = sy0 - (int)(stbl[dx] - rcy);
				if ( sy < 0 )  continue;
				if ( sy >= src->height )  continue;
				dstp[dx] |= sip_getimgptr(src,sy)[sx];
			}
			break;
		    case BOP_AND:
			for ( dx=0 ; dx<lnlen ; dx++ ){
				sx = sx0 + (int)(ctbl[dx] + rsy);
				if ( sx < 0 )  continue;
				if ( sx >= src->width  )  continue;
				sy = sy0 - (int)(stbl[dx] - rcy);
				if ( sy < 0 )  continue;
				if ( sy >= src->height )  continue;
				dstp[dx] &= sip_getimgptr(src,sy)[sx];
			}
			break;
		}
	}
	free((char *)ctbl);
	return(0);
}


