/*--------------------------------------------------------------
  OCR base library
  (C) 2005-2014  Hideaki Goto  (see accompanying LICENSE file)
    Written by  H.Goto,  Dec. 2005
    Revised by  H.Goto,  May  2009
    Revised by  H.Goto,  Dec. 2009
    Revised by  H.Goto,  Aug. 2014
--------------------------------------------------------------*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	<limits.h>
#include	<float.h>

#include	"utypes.h"
#include	"ufilep.h"
#include	"siplib.h"

#include	"ocrbase.h"


/*----------------------------------------------
  Preprocessors class
----------------------------------------------*/

int OCRPrep :: edge(SIPImage *src, SIPImage *dst){
	int	x,y,width,height;
	uchar	*p,*dp,*pu,*pd;
	SIPImage	*tmpimage;
	width  = src->width;
	height = src->height;
	if ( 0 == (tmpimage = sip_CreateImage(width +2, height +2,8)) )  return(-1);
	sip_ClearImage(tmpimage,0);
	for ( y=0 ; y<height ; y++ ){
		p = (uchar *)sip_getimgptr(src,y);
		dp = (uchar *)sip_getimgptr(tmpimage,y+1) +1;
		memcpy((void *)dp,(void *)p,width);
	}
	sip_ClearImage(dst,0);
	for ( y=0 ; y<height ; y++ ){
		pu = (uchar *)sip_getimgptr(tmpimage,y);
		p = (uchar *)sip_getimgptr(tmpimage,y+1);
		pd = (uchar *)sip_getimgptr(tmpimage,y+2);
		dp = (uchar *)sip_getimgptr(dst,y);
		for ( x=0 ; x<width ; x++ ){
			if ( p[x+1] != 0 ){
				if ( p[x  ] == 0 )  dp[x] |= 0xff;
				if ( p[x+2] == 0 )  dp[x] |= 0xff;
				if ( pu[x+1] == 0 )  dp[x] |= 0xff;
				if ( pd[x+1] == 0 )  dp[x] |= 0xff;
			}
		}
	}
	sip_DestroyImage(tmpimage);
	return(0);
}


int OCRPrep :: thin(SIPImage *src, SIPImage *dst, int maxiter){
	uchar	**sp, **dp;
	int	x,y,n;
	int	rcode = 0;
	SIPImage	*tmpimg;
	if ( NULL == (tmpimg = sip_CreateImage(src->width+3, src->height+3 , 8)) ){
		return(-1);
	}
	sp = (uchar **)src->pdata;
	dp = (uchar **)tmpimg->pdata;
	for ( y=0 ; y<src->height ; y++ ){
		for ( x=0 ; x<src->width ; x++ ){
			dp[y+2][x+2] = ~sp[y][x];
		}
		dp[y+2][0] = dp[y+2][1] = 255;
		dp[y+2][tmpimg->width-1] = 255;
	}
	for ( x=0 ; x<tmpimg->width ; x++ ){
		dp[0][x] = dp[1][x] = 255;
		dp[tmpimg->height-1][x] = 255;
	}
	if ( maxiter == 0 ){
		maxiter = src->width > src->height ? src->width : src->height;
		maxiter = maxiter/2 +1;
	}
	for ( n=0 ; n<maxiter ; n++ ){
		rcode = sip_thin10neib(tmpimg,0,0);
		for ( x=0 ; x<tmpimg->width ; x++ ){
			dp[tmpimg->height-2][x] = 255;
		}
		if ( 0 == rcode )  break;
	}
	sp = (uchar **)tmpimg->pdata;
	dp = (uchar **)dst->pdata;
	for ( y=0 ; y<src->height ; y++ ){
		for ( x=0 ; x<src->width ; x++ ){
			dp[y][x] = ~sp[y+2][x+2];
		}
	}
	sip_DestroyImage(tmpimg);
	return(rcode);
}


int OCRPrep :: normalize(SIPImage *src, SIPImage *dst, double alimit){
	uchar	**sp, **dp;
	uint	*hhist, *vhist;
	int	x, y, x0, y0, sx, sy, width, height, len;
	int	chr_w, chr_h, ct;
	SIPImage	*src0;
	if ( src->depth != 8 )  return(-1);
	if ( dst->depth != 8 )  return(-1);
	width  = src->width;
	height = src->height;
	len = width > height ? width : height;
	if ( 0 == (hhist = new uint[width + height]) )  return(-1);
	if ( src != dst ){
		src0 = src;
	}
	else{	if ( NULL == (src0 = sip_CreateImage(src->width,src->height,8)) ){
			delete []hhist;
			return(-1);
		}
		for ( y=0 ; y<height ; y++ ){
			memcpy((void *)src0->pdata[y], (void *)src->pdata[y], width);
		}
	}
	for ( x=0 ; x<(width+height) ; x++ )  hhist[x] = 0;
	vhist = hhist + width;
	sp = (uchar **)src0->pdata;
	for ( y=0 ; y<height ; y++ ){
		for ( x=0 ; x<width ; x++ ){
			if ( sp[y][x] != 0 ){  hhist[x]++;  vhist[y]++; }
		}
	}
	for ( x0=0 ; x0<width ; x0++ ){
		if ( 0 != hhist[x0] )  break;
	}
	for ( --width ; width>=0 ; width-- ){
		if ( 0 != hhist[width] )  break;
	}
	chr_w = width - x0 +1;
	if ( chr_w <= 0 ){  x0 = 0;  chr_w = 1; }
	for ( y0=0 ; y0<height ; y0++ ){
		if ( 0 != vhist[y0] )  break;
	}
	for ( --height ; height>=0 ; height-- ){
		if ( 0 != vhist[height] )  break;
	}
	chr_h = height - y0 +1;
	if ( chr_h <= 0 ){  y0 = 0;  chr_h = 1; }
	dp = (uchar **)dst->pdata;
	if ( alimit != 0.0 ){
		if ( chr_w >= chr_h ){
			ct = y0 + chr_h /2;
			if ( (double)chr_h < (double)chr_w / alimit ){
				chr_h = (int)((double)chr_w / alimit);
				if ( chr_h == 0 )  chr_h = 1;
			}
			y0 = ct - chr_h /2;
		}
		else{
			ct = x0 + chr_w /2;
			if ( (double)chr_w < (double)chr_h / alimit ){
				chr_w = (int)((double)chr_h / alimit);
				if ( chr_w == 0 )  chr_w = 1;
			}
			x0 = ct - chr_w /2;
		}
	}
	for ( y=0 ; y<dst->height ; y++ ){
		for ( x=0 ; x<dst->width ; x++ ){
			sx = x0 + (x*chr_w) / dst->width;
			sy = y0 + (y*chr_h) / dst->height;
			if ( sx >= 0 && sx < src->width \
			  && sy >= 0 && sy < src->height ){
				dp[y][x] = sp[sy][sx];
			}
			else{	dp[y][x] = 0;
			}
		}
	}
	if ( src != src0 ){  sip_DestroyImage(src0); }
	delete []hhist;
	return(0);
}




/*----------------------------------------------
  Feature vector class
----------------------------------------------*/

FeatureVector :: FeatureVector(){
	dim = 0;
	e = 0;
	gHint = 0;
}


FeatureVector :: FeatureVector(int dim){
	FeatureVector::dim = dim;
	e = new double[dim];
	gHint = 0;
}


int FeatureVector :: alloc(int dim){
	if ( e )  delete []e;
	FeatureVector::dim = dim;
	gHint = 0;
	if ( 0 == (e = new double[dim]) )  return(-1);
	return(0);
}


FeatureVector :: ~FeatureVector(){
	if ( e )  delete []e;
}


int FeatureVector :: write_vector(FILE *fp){
	int	i, j, etest = 0x12;
	char	*p;
	char	buf[sizeof(double)];
	p = (char*)&etest;
	if ( *p == 0x12 ){
		/* little endian */
		for ( i=0 ; i<dim ; i++ ){
			p = (char *)&e[i];
			for ( j=0 ; j<(int)sizeof(double) ; j++ ){
				buf[j] = p[sizeof(double)-1 - j];
			}
			if ( 1 != fwrite((void *)buf,sizeof(double),1,fp) ){
				return(-1);
			}
		}
	}
	else{
		/* big endian */
		for ( i=0 ; i<dim ; i++ ){
			if ( 1 != fwrite((void *)&e[i],sizeof(double),1,fp) ){
				return(-1);
			}
		}
	}
	return(0);
}


int FeatureVector :: read_vector(FILE *fp){
	int	i, j, etest = 0x12;
	char	*p;
	char	buf[sizeof(double)];
	p = (char*)&etest;
	if ( *p == 0x12 ){
		/* little endian */
		for ( i=0 ; i<dim ; i++ ){
			if ( 1 != fread((void *)buf,sizeof(double),1,fp) ){
				return(-1);
			}
			p = (char *)&e[i];
			for ( j=0 ; j<(int)sizeof(double) ; j++ ){
				p[j] = buf[sizeof(double)-1 - j];
			}
		}
	}
	else{
		/* big endian */
		for ( i=0 ; i<dim ; i++ ){
			if ( 1 != fread((void *)&e[i],sizeof(double),1,fp) ){
				return(-1);
			}
		}
	}
	return(0);
}


FeatureVector& FeatureVector::operator=(FeatureVector& obj){
	memcpy((void *)e, (void *)obj.e, sizeof(double)*dim);
	return(*this);
}


FeatureVector& FeatureVector::operator+=(FeatureVector& obj){
	int	i;
	for ( i=0 ; i<dim ; i++ ){
		e[i] += obj.e[i];
	}
	return(*this);
}


int FeatureVector :: zeroVector(){
	for ( int i=0 ; i<dim ; i++ )  e[i] = 0.0;
	return(0);
}


int FeatureVector :: setVector(double *vec){
	memcpy((void *)e, (void *)vec, sizeof(double)*dim);
	return(0);
}


int FeatureVector :: getVector(double *vec){
	if ( e == 0 )  return(-1);
	memcpy((void *)vec, (void *)e, sizeof(double)*dim);
	return(0);
}


double FeatureVector :: distEuclidean2(FeatureVector& vec){
	int	i;
	double	d = 0;
#if 0
	for ( i=0 ; i<dim ; i++ ){
		d += (e[i] - vec.e[i]) * (e[i] - vec.e[i]);
	}
#else
	for ( i=0 ; i<dim-8 ; i+=8 ){
		d += (e[i  ] - vec.e[i  ]) * (e[i  ] - vec.e[i  ]);
		d += (e[i+1] - vec.e[i+1]) * (e[i+1] - vec.e[i+1]);
		d += (e[i+2] - vec.e[i+2]) * (e[i+2] - vec.e[i+2]);
		d += (e[i+3] - vec.e[i+3]) * (e[i+3] - vec.e[i+3]);
		d += (e[i+4] - vec.e[i+4]) * (e[i+4] - vec.e[i+4]);
		d += (e[i+5] - vec.e[i+5]) * (e[i+5] - vec.e[i+5]);
		d += (e[i+6] - vec.e[i+6]) * (e[i+6] - vec.e[i+6]);
		d += (e[i+7] - vec.e[i+7]) * (e[i+7] - vec.e[i+7]);
	}
	for ( ; i<dim ; i++ ){
		d += (e[i] - vec.e[i]) * (e[i] - vec.e[i]);
	}
#endif
	return(d);
}


double FeatureVector :: distEuclidean2(FeatureVector& vec, double limit){
	int	i;
	double	d = 0;
#if 0
	for ( i=0 ; i<dim ; i++ ){
		d += (e[i] - vec.e[i]) * (e[i] - vec.e[i]);
	}
#else
	for ( i=0 ; i<dim-8 ; i+=8 ){
		d += (e[i  ] - vec.e[i  ]) * (e[i  ] - vec.e[i  ]);
		d += (e[i+1] - vec.e[i+1]) * (e[i+1] - vec.e[i+1]);
		d += (e[i+2] - vec.e[i+2]) * (e[i+2] - vec.e[i+2]);
		d += (e[i+3] - vec.e[i+3]) * (e[i+3] - vec.e[i+3]);
		d += (e[i+4] - vec.e[i+4]) * (e[i+4] - vec.e[i+4]);
		d += (e[i+5] - vec.e[i+5]) * (e[i+5] - vec.e[i+5]);
		d += (e[i+6] - vec.e[i+6]) * (e[i+6] - vec.e[i+6]);
		d += (e[i+7] - vec.e[i+7]) * (e[i+7] - vec.e[i+7]);
		if ( d > limit )  return(d);
	}
	for ( ; i<dim ; i++ ){
		d += (e[i] - vec.e[i]) * (e[i] - vec.e[i]);
	}
#endif
	return(d);
}


double FeatureVector :: distManhattan(FeatureVector& vec){
	int	i;
	double	d = 0;
	for ( i=0 ; i<dim ; i++ ){
		if ( e[i] >= vec.e[i] ){
			d += e[i] - vec.e[i];
		}
		else{
			d += vec.e[i] - e[i];
		}
	}
	return(d);
}


double FeatureVector :: distChessboard(FeatureVector& vec){
	int	i;
	double	diff, d = 0;
	for ( i=0 ; i<dim ; i++ ){
		if ( 0 > (diff = e[i] - vec.e[i]) )  diff = -diff;
		if ( d < diff )  d = diff;
	}
	return(d);
}




/*----------------------------------------------
  Recognizer base class
----------------------------------------------*/

RecBase :: ~RecBase(){
	dealloc();
}


int RecBase :: dealloc(){
	if ( n_top ){
		delete []resultTable;
		n_top = 0;
	}
	if ( n_cat ){
		delete []dic;
		n_cat = 0;
	}
	return(0);
}


int RecBase :: alloc(int n_cat, int dim, int n_top){
	int	cid;
	dealloc();
	if ( 0 == (dic = new FeatureVector[n_cat]) )  return(-1);
	RecBase::n_cat = n_cat;
	for ( cid=0 ; cid<n_cat ; cid++ ){
		if ( 0 == (dic[cid].e = new double[dim]) )  return(-1);
		dic[cid].dim = dim;
	}
	if ( 0 == (resultTable = new RecResultItem[n_top]) )  return(-1);
	RecBase::n_top = n_top;
	return(0);
}


void RecBase :: initResultTable(){
	for ( int i=0 ; i<n_top ; i++ ){
		resultTable[i].id = -1;
		resultTable[i].dist = DBL_MAX;
	}
}


int RecBase :: recognizeEuclidean(FeatureVector& charvec, int gHint){
	int	cat,rank,i;
	double	d,d0;
	initResultTable();
	if ( 0 == (gHint &= WrtDir_Mask) )  gHint = WrtDir_H | WrtDir_V;
	d0 = DBL_MAX;
	for ( cat=0 ; cat<n_cat ; cat++ ){
		// skip the categories in different direction
		if ( 0 == (dic[cat].gHint & gHint) )  continue;

//		d = charvec.distEuclidean2(dic[cat]);
		d = charvec.distEuclidean2(dic[cat],d0);
		if ( d >= d0 )  continue;
		for ( rank=0 ; rank<n_top ; rank++ ){
			if ( d >= resultTable[rank].dist )  continue;
			for ( i=n_top-2 ; i>=rank ; i-- ){
				resultTable[i+1] = resultTable[i];
			}
			resultTable[rank].dist = d;
			resultTable[rank].id = cat;
			d0 = resultTable[n_top-1].dist;
			break;
		}
	}
	for ( i=0 ; i<n_top ; i++ ){
		resultTable[i].dist = sqrt(resultTable[i].dist);
	}
	return(0);
}




