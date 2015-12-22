/*--------------------------------------------------------------
	Signal processing function library    libsgp
	  ( Convolution  Rev.940809)
		Written by H.Goto , Aug.1994
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
#include	<stdlib.h>
#include	<memory.h>

#include	"sgplib.h"


/*------------------------------------------------------
	Convolution
------------------------------------------------------*/

int sgp_convolutec(char *src,char *dst,int len,char sdata, \
				char *mask,char div,int masklen){
	char	*buf,sum;
	int	i,j,off;
	off = (masklen -1) /2;
	if ( NULL == (buf = (char *)malloc(len + masklen)) )  return(-1);
	for ( i=0 ; i<off ; i++ )  buf[i] = sdata;
	for ( i=(off + masklen) ; i<(len + masklen) ; i++ )  buf[i] = sdata;
	memcpy((char *)&buf[off],(char *)src,len * sizeof(char));
	if ( div == 1 ){
		for ( i=0 ; i<len ; i++ ){
			sum = 0;
			for ( j=0 ; j<masklen ; j++ )  sum += mask[j] * buf[i + j];
			*dst++ = sum;
		}
	}
	else{	for ( i=0 ; i<len ; i++ ){
			sum = 0;
			for ( j=0 ; j<masklen ; j++ )  sum += mask[j] * buf[i + j];
			*dst++ = sum / div;
		}
	}
	free(buf);
	return(0);
}


int sgp_convolutei(int *src,int *dst,int len,int sdata, \
				int *mask,int div,int masklen){
	int	*buf,sum;
	int	i,j,off;
	off = (masklen -1) /2;
	if ( NULL == (buf = (int *)malloc((len + masklen) * sizeof(int))) )  return(-1);
	for ( i=0 ; i<off ; i++ )  buf[i] = sdata;
	for ( i=(off + masklen) ; i<(len + masklen) ; i++ )  buf[i] = sdata;
	memcpy((char *)&buf[off],(char *)src,len * sizeof(int));
	if ( div == 1 ){
		for ( i=0 ; i<len ; i++ ){
			sum = 0;
			for ( j=0 ; j<masklen ; j++ )  sum += mask[j] * buf[i + j];
			*dst++ = sum;
		}
	}
	else{	for ( i=0 ; i<len ; i++ ){
			sum = 0;
			for ( j=0 ; j<masklen ; j++ )  sum += mask[j] * buf[i + j];
			*dst++ = sum / div;
		}
	}
	free(buf);
	return(0);
}


