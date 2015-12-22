/*--------------------------------------------------------------
	Signal processing function library    libsgp
	  ( Correlation Functions Rev.941214)
		Written by H.Goto , Dec.1994
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1994  Hideaki Goto

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
#include	<memory.h>

#include	"sgplib.h"


/*------------------------------------------------------
	Get Correlation factor
------------------------------------------------------*/

int sgp_correlations(short *d1,short *d2,int len,short *corr, \
				int maxmove,int periodic,short zero){
	int	i,mv;
	short	sum;
	for ( mv=0 ; mv<maxmove ; mv++ ){
		sum = 0;
		for ( i=0 ; i < (len - mv) ; i++ ){
			sum += d1[i] * d2[i + mv];
		}
		for ( i = (len - mv) ; i<len ; i++ ){
			if ( periodic )  sum += d1[i] * d2[i + mv - len];
			else             sum += d1[i] * zero;
		}
		*corr++ = sum;
	}
	return(0);
}


int sgp_correlationi(int *d1,int *d2,int len,int *corr, \
				int maxmove,int periodic,int zero){
	int	i,mv;
	int	sum;
	for ( mv=0 ; mv<maxmove ; mv++ ){
		sum = 0;
		for ( i=0 ; i < (len - mv) ; i++ ){
			sum += d1[i] * d2[i + mv];
		}
		if ( periodic ){
			for ( i = (len - mv) ; i<len ; i++ )
				sum += d1[i] * d2[i + mv - len];
		}
		else{	for ( i = (len - mv) ; i<len ; i++ )
				sum += d1[i] * zero;
		}
		*corr++ = sum;
	}
	return(0);
}


int sgp_correlationb(char *d1,char *d2,int len,int *corr, \
				int maxmove,int periodic,char zero){
	int	i,mv;
	int	sum;
	for ( mv=0 ; mv<maxmove ; mv++ ){
		sum = 0;
		for ( i=0 ; i < (len - mv) ; i++ ){
			if ( ! (d1[i] ^ d2[i + mv]) )  ++sum;
		}
		if ( periodic ){
			for ( i = (len - mv) ; i<len ; i++ )
				if ( ! (d1[i] ^ d2[i + mv - len]) )  ++sum;
		}
		else{	for ( i = (len - mv) ; i<len ; i++ )
				if ( ! (d1[i] ^ (char)zero) )  ++sum;
		}
		*corr++ = (2 * sum) -len;
	}
	return(0);
}


