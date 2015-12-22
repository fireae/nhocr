/*--------------------------------------------------------------
	Signal processing function library    libsgp
	  ( Histogram analysis   Rev.20000711 )
		Written by H.Goto , Feb. 1995
		Modified by H.Goto, July 2000
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1995-2000  Hideaki Goto

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
#include	<math.h>

#include	"sgplib.h"


double sgp_gethistmeani(int *hist,int len){	/* Get mean posision */
	int	i,sum,hsum;
	sum = hsum = 0;
	for ( i=0 ; i<len ; i++ ){
		sum += hist[i];
		hsum += i * hist[i];
	}
	if ( !sum )  return(0);
	return( (double)hsum / (double)sum );
}


double sgp_gethistsdi(int *hist,int len,double mean){
	int	i,h,sum;
	double	d,sd;
	sd = 0;
	sum = 0;
	for ( i=0 ; i<len ; i++ ){
		h = *hist++;
		sum += h;
		d = (double)i - mean;
		sd += (double)h * (d * d);
	}
	if ( !sum )  return(0);
	return ( sqrt(sd / sum) );
}


double sgp_gethistmdi(int *hist,int len,double mean){
	int	i,h,sum;
	double	md;
	md = 0;
	sum = 0;
	for ( i=0 ; i<len ; i++ ){
		h = *hist++;
		sum += h;
		md += (double)h * fabs((double)i - mean);
	}
	if ( !sum )  return(0);
	return ( md / sum );
}


