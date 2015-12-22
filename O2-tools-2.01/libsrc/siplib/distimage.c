/*--------------------------------------------------------------
     Image Processing function library    libsip
     ( Convert binary image into distance image  Rev.020208 )
		Written  by H.Goto , Oct 1999
		Modified by H.Goto , Feb 2002
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1999-2002  Hideaki Goto

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

#include	<stdio.h>
#include	<stdlib.h>

#include	"siplib.h"


/*------------------------------------------------------
	Create Distance Image  (4-neighbor mode)
------------------------------------------------------*/

int sip_distimage(SIPImage *image,SIPImage *distimage,int maxdist){
	int	x,y,n,n0;
	int	width,height;
	uchar	d0;
	uchar	*pi,*pd;
	uchar	*lbuf;
	int	*len;
	width = image->width;
	height = image->height;
	if ( image->depth != 8 || distimage->depth != 8 )  return(-1);
	if ( NULL == (lbuf = malloc(width * sizeof(uchar))) )  return(-1);
	if ( NULL == (len = malloc(width * sizeof(int))) ){
		free((void *)lbuf);
		return(-1);
	}
	sip_ClearImage(distimage,maxdist);

	/* scan forward */
	pi = (uchar *)sip_getimgptr(image,0);
	for ( x=0 ; x<width ; x++ ){
		lbuf[x] = pi[x] +1;
		len[x] = maxdist;
	}
	for ( y=n=0 ; y<height ; y++ ){
		pi = (uchar *)sip_getimgptr(image,y);
		pd = (uchar *)sip_getimgptr(distimage,y);
		d0 = *pi +1;  n0 = maxdist;
		for ( x=0 ; x<width ; x++ ){
			if ( pi[x] != d0 || pi[x] != lbuf[x] )  n = 0;
			n++;
			if ( n > n0+1 )  n = n0+1;
			if ( n > len[x] +1 )  n = len[x] +1;
			if ( (uchar)n > pd[x] )  n = pd[x];
			n0 = len[x] = pd[x] = (uchar)n;
			d0 = lbuf[x] = pi[x];
		}
	}

	/* scan backward */
	pi = (uchar *)sip_getimgptr(image,height -1);
	for ( x=0 ; x<width ; x++ ){
		lbuf[x] = pi[x] +1;
		len[x] = maxdist;
	}
	for ( y=height -1 ; y>=0 ; y-- ){
		pi = (uchar *)sip_getimgptr(image,y);
		pd = (uchar *)sip_getimgptr(distimage,y);
		d0 = pi[width -1] +1;  n0 = maxdist;
		for ( x=width -1 ; x>=0 ; x-- ){
			if ( pi[x] != d0 || pi[x] != lbuf[x] )  n = 0;
			n++;
			if ( n > n0+1 )  n = n0+1;
			if ( n > len[x] +1 )  n = len[x] +1;
			if ( (uchar)n > pd[x] )  n = pd[x];
			n0 = len[x] = pd[x] = (uchar)n;
			d0 = lbuf[x] = pi[x];
		}
	}

	free((void *)len);
	free((void *)lbuf);
	return(0);
}


