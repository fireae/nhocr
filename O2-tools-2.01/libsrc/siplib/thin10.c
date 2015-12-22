/*--------------------------------------------------------------
	 Image Processing function library    libsip
	  ( Thinning by 10-neighbor method   Rev.081230)
		Written by H.Goto , Oct 1994
		Revised by H.Goto , Sep 1997
		Revised by H.Goto , Feb 2002
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


#include	<stdio.h>
#include	<stdlib.h>

#include	"siplib.h"

#include	"tenneib.h"


/*------------------------------------------------------
	Thinning by 10-neighbor method
------------------------------------------------------*/

int sip_thin10neib(SIPImage *image,char *cflag,char *rflag){
	int	i,x,y,y1,y2;
	int	width,height;
	char	*lb1,*lb2,*lb3,*lb4;
	char	*lbuf,*lt1,*lt2,*lbo,*lttmp;
	int	tn,tf,tfa;
	char	*cf,*rf;

	lb1 = lb2 = lb3 = lb4 = 0;	/* (just supress compiler warning) */

	if ( image->depth != 8 )  return(-1);
	width  = image->width;
	height = image->height;
	if ( NULL == (lbuf = (char *)malloc(width * 3)) )  return(-1);
	lt1 = lbuf;  lt2 = lbuf + width;  lbo = lbuf + (2 * width);
	if ( NULL == (cf = (char *)malloc(width + height)) ){
		free(lbuf);
		return(-1);
	}
	rf = cf + width;
	for ( x=0 ; x < (3 * width) ; x++ )  lbuf[x] = (char)0xff;
	if ( rflag != 0 ){
		for ( y=0 ; y<height ; y++ ){  rf[y] = rflag[y];  rflag[y] = 0; }
	}
	else{	for ( y=0 ; y<height ; y++ )  rf[y] = 1;
	}
/*
	if ( cflag != 0 ){
		for ( x=0 ; x<width ; x++ ){  cf[x] = cflag[x];  cflag[x] = 0; }
	}
	else{	for ( x=0 ; x<width ; x++ )  cf[x] = 1;
	}
*/
	tfa = 0;
	for ( y=2 ; y < (height -1) ; y++ ){
		lb1 = (char *)sip_getimgptr(image, y-2);
		lb2 = (char *)sip_getimgptr(image, y-1);
		lb3 = (char *)sip_getimgptr(image, y);
		lb4 = (char *)sip_getimgptr(image, y+1);
		tf = 0;
		if ( rf[y] ){
			for ( x=0 ; x<width ; x++ )  lbo[x] = (char)0xff;
			for ( x=2 ; x < (width -1) ; x++ ){
				tn = 0;
				if ( ! lb1[x] )		tn |= 0x01;
				if ( ! lb2[x-1] )	tn |= 0x02;
				if ( ! lb2[x] )		tn |= 0x04;
				if ( ! lb2[x+1] )	tn |= 0x08;
				if ( ! lb3[x-2] )	tn |= 0x10;
				if ( ! lb3[x-1] )	tn |= 0x20;
				if ( ! lb3[x] )		tn |= 0x40;
				if ( ! lb3[x+1] )	tn |= 0x80;
				if ( ! lb4[x-1] )	tn |= 0x100;
				if ( ! lb4[x] )		tn |= 0x200;
				if ( ! lb4[x+1] )	tn |= 0x400;
				if ( thin(tn) ){
					lbo[x] = 0;
				}
				else{	if ( tn & 0x40 )  tf = 1;
				}
			}
		}
		else{	for ( x=0 ; x<width ; x++ )  lbo[x] = lb3[x];
		}
		if ( (rflag != 0) && (tf != 0) ){
			if ( (y1 = y - 2) < 0 )  y1 = 0;
			if ( (y2 = y + 2) >= height )  y2 = height -1;
			for ( i=y1 ; i<=y2 ; i++ )  rflag[i] = 1;
		}
		tfa |= tf;
		for ( x=0 ; x<width ; x++ )  lb1[x] = lt1[x];
		lttmp = lt1;  lt1 = lt2;  lt2 = lbo;  lbo = lttmp;
	}
	if ( y > 2 ){
		for ( x=0 ; x<width ; x++ )  lb1[x] = lt1[x];
		for ( x=0 ; x<width ; x++ )  lb2[x] = lt2[x];
		for ( x=0 ; x<width ; x++ )  lb3[x] = lbo[x];
	}
	free(cf);
	free(lbuf);
	return(tfa);
}


