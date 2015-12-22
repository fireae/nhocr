/*--------------------------------------------------------------
	Image Processing function library    libsip
	  (Projection profile  Rev.081230)
		Written by H.Goto , Nov 1995
		Revised by H.Goto , Sep 1997
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

#include	"siplib.h"


long sip_projprofile(SIPImage *image,long *rden,long *cden){
	int	x,y;
	int	width,height,depth;
	ulong	rsum,total;
	ulong	d;
	uchar	*line;
	uchar	mask,dc;
	width  = image->width;
	height = image->height;
	switch ( depth = image->depth ){
	    case 1:
	    case 8:	break;
	    default:	return(-1);
	}
	total = 0;
	if ( cden != 0 ){
		for ( x=0 ; x<width ; x++ )  cden[x] = 0;
		for ( y=0 ; y<height ; y++ ){
			line = (uchar *)sip_getimgptr(image,y);
			rsum = 0;
			mask = dc = 0;
			if ( depth == 1 ){
				for ( x=0 ; x<width ; x++ ){
					if ( mask == 0 ){
						mask = 0x80;
						dc = *line++;
					}
					if ( (mask & dc) != 0 ){  ++rsum;  ++cden[x];  }
					mask >>= 1;
				}
			}
			else{	for ( x=0 ; x<width ; x++ ){
					d = (ulong)*line++;
					rsum += d;
					cden[x] += d;
				}
			}
			if ( rden != 0 )  rden[y] = rsum;
			total += rsum;
		}
	}
	else{	for ( y=0 ; y<height ; y++ ){
			line = (uchar *)sip_getimgptr(image,y);
			rsum = 0;
			mask = dc = 0;
			if ( depth == 1 ){
				for ( x=0 ; x<width ; x++ ){
					if ( mask == 0 ){
						mask = 0x80;
						dc = *line++;
					}
					if ( (mask & dc) != 0 )  ++rsum;
					mask >>= 1;
				}
			}
			else{	for ( x=0 ; x<width ; x++ ){
					rsum += (ulong)*line++;
				}
			}
			if ( rden != 0 )  rden[y] = rsum;
			total += rsum;
		}
	}
	return(total);
}


long sip_projprofile_area(SIPImage *image,long *rden, \
			long *cden,SIPRectangle *area){
	int	x,y;
	int	width,height,depth;
	ulong	rsum,total;
	ulong	d;
	uchar	*line;
	uchar	mask,dc;
	int	xoffb;
	width  = area->width;
	height = area->height;
	xoffb  = area->x;
	switch ( depth = image->depth ){
	    case 1:	xoffb >>= 3;
	    case 8:	break;
	    default:	return(-1);
	}
	total = 0;
	if ( cden != 0 ){
		for ( x=0 ; x<width ; x++ )  cden[x] = 0;
		for ( y=0 ; y<height ; y++ ){
			line = (uchar *)sip_getimgptr(image,y) + xoffb;
			rsum = 0;
			if ( depth == 1 ){
				mask = 0x80;
				dc = *line++;
				mask >>= (area->x & 0x07);
				for ( x=0 ; x<width ; x++ ){
					if ( mask == 0 ){
						mask = 0x80;
						dc = *line++;
					}
					if ( (mask & dc) != 0 ){  ++rsum;  ++cden[x];  }
					mask >>= 1;
				}
			}
			else{	for ( x=0 ; x<width ; x++ ){
					d = (ulong)*line++;
					rsum += d;
					cden[x] += d;
				}
			}
			if ( rden != 0 )  rden[y] = rsum;
			total += rsum;
		}
	}
	else{	for ( y=0 ; y<height ; y++ ){
			line = (uchar *)sip_getimgptr(image,y) + xoffb;
			rsum = 0;
			if ( depth == 1 ){
				mask = 0x80;
				dc = *line++;
				mask >>= (area->x & 0x07);
				for ( x=0 ; x<width ; x++ ){
					if ( mask == 0 ){
						mask = 0x80;
						dc = *line++;
					}
					if ( (mask & dc) != 0 )  ++rsum;
					mask >>= 1;
				}
			}
			else{	for ( x=0 ; x<width ; x++ ){
					rsum += (ulong)*line++;
				}
			}
			if ( rden != 0 )  rden[y] = rsum;
			total += rsum;
		}
	}
	return(total);
}


