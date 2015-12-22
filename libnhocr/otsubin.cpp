/*----------------------------------------------------------------------
    Global/Adaptive Image Binarization
      based on Discriminant Criterion (Otsu's method)  otsubin.cpp
    (C) 2000-2009  Hideaki Goto  (see accompanying LICENSE file)
        Written by H.Goto, Apr 2000
        Revised by H.Goto, Sep 2003
        Revised by H.Goto, Dec 2005
        Revised by H.Goto, May 2007
        Revised by H.Goto, Apr 2009
        Revised by H.Goto, May 2009
----------------------------------------------------------------------*/


#define		GrayLevels		256

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>

#include	"utypes.h"

#include	"ufilep.h"
#include	"siplib.h"

#include	"discrim.h"
#include	"otsubin.h"




/*----------------------------------------------
    Local Binarization
----------------------------------------------*/

static int binarize_local_interp(SIPImage *src, SIPImage *dst, \
	int x0,int y0, int w,int h,double **tharray){
	int	x,y;
	int	mx,my,hw,hh;
	uchar	*sp,*dp;
	int	th;
	double	s,t,th1,th2;
	hw = w /2;
	hh = h /2;
	mx = (x0 / w) +1;
	my = (y0 / h) +1;
	th = (int)(tharray[my][mx] + 0.5);
	for ( y=y0 ; y < y0+hh ; y++ ){
		if ( y < 0 || y >= src->height )  continue;
		sp = (uchar *)src->pdata[y];
		dp = (uchar *)dst->pdata[y];
		t = 0.5 + (double)(y-y0)/(double)hh/2.0;
		for ( x=x0 ; x < x0+hw ; x++ ){
			if ( x < 0 || x >= src->width )  continue;
			s = 0.5 + (double)(x-x0)/(double)hw/2.0;
			th2 = (1-s) * tharray[my-1][mx-1] \
				+ s * tharray[my-1][mx];
			th1 = (1-s) * tharray[my][mx-1] \
				+ s * tharray[my][mx];
			th = (int)((1-t) * th2 + t * th1 + 0.5);
			if ( (int)((uint)sp[x]) > th )  dp[x] = 255;  else dp[x] = 0;
		}
		for ( x=x0+hw ; x < x0+h ; x++ ){
			if ( x < 0 || x >= src->width )  continue;
			s = 1.0 - (double)(x-x0-hw)/(double)hw/2.0;
			th2 = (1-s) * tharray[my-1][mx+1] \
				+ s * tharray[my-1][mx];
			th1 = (1-s) * tharray[my][mx+1] \
				+ s * tharray[my][mx];
			th = (int)((1-t) * th2 + t * th1 + 0.5);
			if ( (int)((uint)sp[x]) > th )  dp[x] = 255;  else dp[x] = 0;
		}
	}
	for ( y=y0+hh ; y < y0+h ; y++ ){
		if ( y < 0 || y >= src->height )  continue;
		sp = (uchar *)src->pdata[y];
		dp = (uchar *)dst->pdata[y];
		t = 1.0 - (double)(y-y0-hh)/(double)hh/2.0;
		for ( x=x0 ; x < x0+hw ; x++ ){
			if ( x < 0 || x >= src->width )  continue;
			s = 0.5 + (double)(x-x0)/(double)hw/2.0;
			th2 = (1-s) * tharray[my+1][mx-1] \
				+ s * tharray[my+1][mx];
			th1 = (1-s) * tharray[my][mx-1] \
				+ s * tharray[my][mx];
			th = (int)((1-t) * th2 + t * th1 + 0.5);
			if ( (int)((uint)sp[x]) > th )  dp[x] = 255;  else dp[x] = 0;
		}
		for ( x=x0+hw ; x < x0+h ; x++ ){
			if ( x < 0 || x >= src->width )  continue;
			s = 1.0 - (double)(x-x0-hw)/(double)hw/2.0;
			th2 = (1-s) * tharray[my+1][mx+1] \
				+ s * tharray[my+1][mx];
			th1 = (1-s) * tharray[my][mx+1] \
				+ s * tharray[my][mx];
			th = (int)((1-t) * th2 + t * th1 + 0.5);
			if ( (int)((uint)sp[x]) > th )  dp[x] = 255;  else dp[x] = 0;
		}
	}
	return(0);
}




/*----------------------------------------------
    Local Binarization
----------------------------------------------*/

static int binarize_local(SIPImage *src, SIPImage *dst, \
	int x0,int y0,int w,int h,int th){
	int	x,y;
	uchar	*sp,*dp;
	if ( (w += x0) > src->width  )  w = src->width;
	if ( (h += y0) > src->height )  h = src->height;
	if ( x0 < 0 )  x0 = 0;
	if ( y0 < 0 )  y0 = 0;
	for ( y=y0 ; y<h ; y++ ){
		sp = (uchar *)src->pdata[y];
		dp = (uchar *)dst->pdata[y];
		for ( x=x0 ; x<w ; x++ ){
			if ( (uint)sp[x] > (uint)th )  dp[x] = 255;  else dp[x] = 0;
		}
	}
	return(0);
}




/*----------------------------------------------
    Smoothing by moving average
----------------------------------------------*/

static int average7x(int *hist){
	int	i,j;
	int	sum;
	int	*tmphist;
	if ( 0 == (tmphist = new int[GrayLevels +6]) )  return(-1);
	memcpy((void *)(tmphist + 3),(void *)hist,sizeof(int) * GrayLevels);
	for ( i=0 ; i<3 ; i++ )  tmphist[i] = tmphist[3];
	for ( i=GrayLevels +5 ; i>=GrayLevels +3; i-- ) \
		tmphist[i] = tmphist[GrayLevels +2];
	for ( i=0 ; i<GrayLevels ; i++ ){
		sum = 0;
		for ( j=0 ; j<7 ; j++ )  sum += tmphist[i+j];
		hist[i] = sum;
	}
	delete []tmphist;
	return(0);
}




/*----------------------------------------------
    Find peaks in an array
----------------------------------------------*/

typedef struct {
	int	pos;
	int	value;
} peaklisti;


static int getpeaks(int *src,peaklisti *pks, \
		int size,int listsize,int thlow,int thhigh){
	int	ix,l;
	int	dx,dx0,x0;
	int	count;
	count = 0;
	l = 0;
	x0 = dx0 = 0;
	for ( ix=0 ; ix<size ; ++ix ){
		dx = src[ix] - x0;
		if ( dx == 0 ){
			if ( dx0 > 0 )  ++l;  else  l = 0;
		}
		else{	if ( dx < 0 ){
				if ( l != 0 ){
					if ( (x0 >= thlow) && (x0 <= thhigh) ){
						if ( (++count) > listsize )  break;
						pks->pos = ix - ((l+1)/2) -1;
						pks->value = x0;
						++pks;
					}
					l = 0;
				}
				else{	if ( dx0 > 0 ){
						if ( (x0 >= thlow) && (x0 <= thhigh) ){
							if ( (++count) > listsize )  break;
							pks->pos = ix -1;
							pks->value = x0;
							++pks;
						}
					}
				}
			}
			else{	l = 0;
			}
			dx0 = dx;
		}
		x0 = src[ix];
	}
	return (count);
}




/*----------------------------------------------
    Find the number of peaks in histogram
----------------------------------------------*/

#define		MinCursorWidth	10
#define		MAXPEAKS	(GrayLevels /4)


static int find_npeaks(long *hist0){
	int	n_cluster;
	int	i,c,l;
	int	hist[GrayLevels +1];
	int	cw = MinCursorWidth;	/* cursor width */
	peaklisti	peaklist[MAXPEAKS];

	for ( i=0 ; i<GrayLevels ; i++ )  hist[i] = (int)hist0[i];
	hist[GrayLevels] = 0;
	average7x(hist);

	/* hysteresis smoothing */
	c = 0;  l = 0;
	for ( i=0 ; i<GrayLevels ; i++ ){
		cw = (int)(0.2 * (double)hist[i]);
		if ( cw < MinCursorWidth )  cw = MinCursorWidth;

		/* force drop to background level for very long roof */
		if ( ++l > 16 ){
			c = hist[i];
			l = 0;
		}

		if ( hist[i] < c - cw ){
			c = hist[i] + cw;
			l = 0;
		}
		if ( hist[i] > c + cw ){
			c = hist[i] - cw;
			l = 0;
		}
		hist[i] = c;
	}

	/* peak detection */
	n_cluster = getpeaks(hist,peaklist,\
		GrayLevels +1,MAXPEAKS,0,0x7fffffff);
	return(n_cluster);
}




/*----------------------------------------------
    Find a threshold
----------------------------------------------*/

double threshold_otsu(SIPImage *image,int x0,int y0,int w,int h){
	int	i,x,y;
	uchar	*p;
	long	*hist;
	double	*rlvl;
	double	th;
	int	npeaks;
	DiscBinParams	params;

	if ( 0 == (hist = new long[GrayLevels]) ){
		return(-1);
	}
	if ( 0 == (rlvl = new double[GrayLevels]) ){
		delete	[]hist;
		return(-1);
	}

	for ( i=0 ; i<GrayLevels ; i++ ){
		hist[i] = 0;
		rlvl[i] = (double)i;
	}

	if ( (w += x0) > image->width  )  w = image->width;
	if ( (h += y0) > image->height )  h = image->height;
	if ( x0 < 0 )  x0 = 0;
	if ( y0 < 0 )  y0 = 0;
	for ( y=y0 ; y<h ; y++ ){
		p = (uchar *)image->pdata[y];
		for ( x=x0 ; x<w ; x++ ){
			hist[ (uint)p[x] ]++;
		}
	}

	npeaks = find_npeaks(hist);

	if ( npeaks > 1 ){
		bin_disc(GrayLevels, NULL, hist, &params);
		th = params.threshold;
	}
	else{	th = -1.0;
	}

	delete	[]rlvl;
	delete	[]hist;
	return(th);
}




/*----------------------------------------------
    2-dim array allocation
----------------------------------------------*/

static double **alloc_double2D(int w,int h){
	double	**a;
	double	*p;
	int	i;
	if ( w <= 0 || h <= 0 )  return(0);
	if ( 0 == (a = new double* [h]) )  return(0);
	if ( 0 == (p = new double [w*h]) ){
		delete []a;
		return(0);
	}
	for ( i=0 ; i<h ; i++ )  a[i] = p + (w*i);
	return(a);
}


static int dealloc_double2D(double **a){
	if ( ! a )  return(0);
	delete []a[0];
	delete []a;
	return(0);
}




/*----------------------------------------------
    Fix the threshold array
----------------------------------------------*/

#define		DilateCounts		8


static int fix_tharray1(double **tharray,int mwidth,int mheight){
	int	mx,my,i,j,count;
	double	sum;
	double	**tharray0;
	if ( 0 == (tharray0 = alloc_double2D(mwidth,mheight)) )  return(-1);

	/* copy original array to temporary array */
	for ( my=0 ; my<mheight ; my++ ){
		memcpy((void *)tharray0[my],(void *)tharray[my], \
			sizeof(double) * mwidth);
		tharray0[my][0] = tharray0[my][mwidth-1] = -1;
	}
	for ( mx=0 ; mx<mwidth ; mx++ ){
		tharray0[0][mx] = tharray0[1][mx];
		tharray0[mheight-1][mx] = tharray0[mheight-2][mx];
	}

	/* interpolate */
	for ( my=1 ; my < mheight -1 ; my++ ){
		for ( mx=1 ; mx < mwidth -1 ; mx++ ){
			if ( tharray0[my][mx] >= 0.0 )  continue;
			count = 0;
			sum = 0.0;
			for ( j = -1 ; j <= 1 ; j++ ){
				for ( i = -1 ; i <= 1 ; i++ ){
					if ( tharray0[my+j][mx+i] < 0.0 )  continue;
					sum += tharray0[my+j][mx+i];
					count++;
				}
			}
			if ( count ){
				tharray[my][mx] = sum / count;
			}
			/* else:  leave tharray[][] < 0 */
		}
	}
	dealloc_double2D(tharray0);
	return(0);
}




static int fix_tharray(double **tharray,int mwidth,int mheight){
	int	mx,my,i,j,count;
	double	sum;
	double	**tharray0;

	/* dilate bilevel area */
	for ( i=0 ; i<DilateCounts ; i++ ){
		if ( 0 > fix_tharray1(tharray,mwidth,mheight) )  return(-1);
	}

	/* create a temporary array */
	if ( 0 == (tharray0 = alloc_double2D(mwidth,mheight)) )  return(-1);

	/* copy original array to temporary array */
	for ( my=0 ; my<mheight ; my++ ){
		memcpy((void *)tharray0[my],(void *)tharray[my], \
			sizeof(double) * mwidth);
		tharray0[my][0] = tharray0[my][mwidth-1] = -1;
	}
	for ( mx=0 ; mx<mwidth ; mx++ ){
		tharray0[0][mx] = tharray0[1][mx];
		tharray0[mheight-1][mx] = tharray0[mheight-2][mx];
	}

	/* interpolate */
	for ( my=1 ; my < mheight -1 ; my++ ){
		for ( mx=1 ; mx < mwidth -1 ; mx++ ){
			if ( tharray0[my][mx] >= 0.0 )  continue;
			count = 0;
			sum = 0.0;
			for ( j = -1 ; j <= 1 ; j++ ){
				for ( i = -1 ; i <= 1 ; i++ ){
					if ( tharray0[my+j][mx+i] < 0.0 )  continue;
					sum += tharray0[my+j][mx+i];
					count++;
				}
			}
			if ( count ){
				tharray[my][mx] = sum / count;
			}
			else{	tharray[my][mx] = 127.0;
				/* force inserting a medium threshold */
			}
		}
	}
	dealloc_double2D(tharray0);

	/* interpolate edge cells */
	for ( my=0 ; my<mheight ; my++ ){
		tharray[my][0] = tharray[my][mwidth-1] = -1;
	}
	for ( mx=0 ; mx<mwidth ; mx++ ){
		tharray[0][mx] = tharray[1][mx];
		tharray[mheight-1][mx] = tharray[mheight-2][mx];
	}

	return(0);
}




/*----------------------------------------------
    Adaptive binarization
----------------------------------------------*/

int adpt_binarize_otsu(SIPImage *src, SIPImage *dst, int csize, int interpolate){
	int	width,height;
	int	mwidth,mheight;
	int	mx,my;
	double	**tharray;

	if ( csize < 4 )  return(-1);

	width  = src->width;
	height = src->height;
	mwidth  = (width  + csize -1) / csize + 2;
	mheight = (height + csize -1) / csize + 2;
	if ( 0 == (tharray = alloc_double2D(mwidth,mheight)) ){
		return(-1);
	}

	for ( my=1 ; my < mheight -1 ; my++ ){
		for ( mx=1 ; mx < mwidth -1 ; mx++ ){
			tharray[my][mx] = threshold_otsu(src,\
				(mx-1)*csize, (my-1)*csize, \
				csize,csize);
		}
	}
	fix_tharray(tharray,mwidth,mheight);
	for ( my=1 ; my < mheight -1 ; my++ ){
		for ( mx=1 ; mx < mwidth -1 ; mx++ ){
			if ( ! interpolate ){
				binarize_local(src, dst, \
				  (mx-1)*csize,(my-1)*csize, \
				  csize,csize,(int)(tharray[my][mx] + .5));
			}
			else{	binarize_local_interp(src, dst, \
				  (mx-1)*csize,(my-1)*csize, \
				  csize,csize,tharray);
			}
		}
	}

	dealloc_double2D(tharray);
	return(0);
}




/*----------------------------------------------
    Global binarization
----------------------------------------------*/

int binarize_otsu(SIPImage *src, SIPImage *dst, double *threshold){
	double	th;
	th = threshold_otsu(src,0,0,src->width,src->height);
	if ( threshold )  *threshold = th;
	if ( th < 0 ){  th = 127; }
	binarize_local(src,dst,0,0,src->width,src->height,(int)(th + .5));
	return(0);
}




