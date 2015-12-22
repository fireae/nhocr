/*--------------------------------------------------------------
	Signal processing function library    libsgp
		Written by H.Goto , Jan.1994
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


#ifdef	__cplusplus
extern "C" {
#endif

#ifndef	sgplib_h
#define	sgplib_h

#include	"utypes.h"




/*------------------------------------------------------
	Walsh Transform functions
------------------------------------------------------*/

int		sgp_wal(int n,int m,int nn);
void		sgp_walseq(int *buf,int m,int nn);
void		sgp_walseqb(char *buf,int m,int nn);
void		sgp_walmatrix(int *buf,int nn);
void		sgp_walmatrixb(char *buf,int nn);
int		sgp_WHT(int *data,int *sequency,int len,int *wmtx);
int		sgp_WHTb(char *data,int *sequency,int len,char *wmtx);
void		sgp_WHTpower(int *wseries,int *pow,int len);




/*------------------------------------------------------
	Get peaks
------------------------------------------------------*/

typedef struct {
	int	pos;
	int	value;
} peaklisti;


int		sgp_getpeaksuc(uchar *src,peaklisti *pks, \
			int size,int listsize,int thlow,int thhigh);
int		sgp_getpeaksi(int *src,peaklisti *pks, \
			int size,int listsize,int thlow,int thhigh);




/*------------------------------------------------------
	Convolution
------------------------------------------------------*/

int		sgp_convolutec(char *src,char *dst,int len,char sdata, \
					char *mask,char div,int masklen);
int		sgp_convolutei(int *src,int *dst,int len,int sdata, \
					int *mask,int div,int masklen);




/*------------------------------------------------------
	Get Correlation factor
------------------------------------------------------*/

int		sgp_correlations(short *d1,short *d2,int len,short *corr, \
					int maxmove,int periodic,short zero);
int		sgp_correlationi(int *d1,int *d2,int len,int *corr, \
					int maxmove,int periodic,int zero);
int		sgp_correlationb(char *d1,char *d2,int len,int *corr, \
					int maxmove,int periodic,char zero);




/*------------------------------------------------------
	Histogram analysis
------------------------------------------------------*/

double		sgp_gethistmeani(int *hist,int len);
double		sgp_gethistsdi(int *hist,int len,double mean);
double		sgp_gethistmdi(int *hist,int len,double mean);




#endif	/* sgplib_h */

#ifdef	__cplusplus
}
#endif
