/*--------------------------------------------------------------
	Signal processing function library    libsgp
	  ( Walsh transform functions  Rev.020527 )
		Written  by H.Goto , May 1993
		Modified by H.Goto , May 2002
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1993-2002  Hideaki Goto

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
#include	<memory.h>

#include	"sgplib.h"




/*------------------------------------------------------
	Get Walsh sequences
------------------------------------------------------*/

int sgp_wal(int n,int m,int nn){
	if ( m == 0 )  return(1);
	n = n % nn;
	if ( m == 1 ){
		if ( n < (nn/2) )  return(1);  else  return(-1);
	}
	if ( m & 0x02 )
		return ( sgp_wal(2 * n, (m/2), nn) * sgp_wal(n, 1-(m & 0x01), nn) );
	return ( sgp_wal(2 * n, (m/2), nn) * sgp_wal(n, m & 0x01, nn) );
	
}


void sgp_walseq(int *buf,int m,int nn){
	int	i;
	for ( i=0 ; i<nn ; i++ )  *buf++ = sgp_wal(i,m,nn);
}


void sgp_walseqb(char *buf,int m,int nn){
	int	i;
	for ( i=0 ; i<nn ; i++ )
		if ( sgp_wal(i,m,nn) +1 )  *buf++ = 0xff;  else  *buf++ = 0;
}




/*------------------------------------------------------
	Create Walsh Matrix
------------------------------------------------------*/

void sgp_walmatrix(int *buf,int nn){
	int	i;
	for ( i=0 ; i<nn ; i++ ){
		sgp_walseq(buf,i,nn);
		buf += nn;
	}
}


void sgp_walmatrixb(char *buf,int nn){
	int	i;
	for ( i=0 ; i<nn ; i++ ){
		sgp_walseqb(buf,i,nn);
		buf += nn;
	}
}




/*------------------------------------------------------
	Walsh Transform
------------------------------------------------------*/

int sgp_WHT(int *data,int *sequency,int len,int *wmtx){
	int	m,n,sum;
	int	*wal,*wp;
	if ( 0 == (wal = wmtx) ){
		if ( 0 == (wal = (int *)malloc(len * len * sizeof(int))) )  return(-1);
		sgp_walmatrix(wal,len);
	}
	for ( m=0 ; m<len ; m++ ){
		sum = 0;
		wp = &wal[len * m];
		for ( n=0 ; n<len ; n++ )  sum += data[n] * (*wp++);
		*sequency++ = sum;
	}
	if ( 0 != wmtx )  free(wal);
	return(0);
}


int sgp_WHTb(char *data,int *sequency,int len,char *wmtx){
	int	m,n,sum;
	char	*wal,*wp;
	if ( 0 == (wal = wmtx) ){
		if ( 0 == (wal = (char *)malloc(len * len * sizeof(char))) )  return(-1);
		sgp_walmatrixb(wal,len);
	}
	for ( m=0 ; m<len ; m++ ){
		sum = 0;
		wp = &wal[len * m];
		for ( n=0 ; n<len ; n++ )  if ( ! (data[n] ^ (*wp++)) )  ++sum;
		*sequency++ = (2 * sum) - len;
	}
	if ( 0 != wmtx )  free(wal);
	return(0);
}


void sgp_WHTpower(int *wseries,int *pow,int len){
	int	i;
	*pow++ = *wseries * *wseries;
	++wseries;
	for ( i=1 ; i<(len/2) ; i++ ){
		*pow++ = (wseries[0] * wseries[0]) + (wseries[1] * wseries[1]);
		wseries += 2;
	}
	*pow++ = *wseries * *wseries;
}


