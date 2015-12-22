/*--------------------------------------------------------------
	Image Processing function library    libsip
	  (Line-preserving smoothing  Rev.020208)
		Written  by H.Goto , Apr  2000
		Modified by H.Goto , July 2000
		Modified by H.Goto , Oct  2000
		Modified by H.Goto , Dec  2000
		Modified by H.Goto , Feb  2002
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 2000-2002  Hideaki Goto

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

#define		GrayLevels		256

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>

#include	"utypes.h"
#include	"siplib.h"


static int sip_2NNsmooth (SIPImage *src,SIPImage *dst,int ctweight);
static int sip_kNNsmoothc(SIPImage *src,SIPImage *dst,int pixels,int ctweight);




/*------------------------------------------------------
    Line preserving smoothing

    Note: The thin line is never blurred in the
	  direction orthogonal to the line only when
	  pixels=3. Line segments may get shorter by
	  at most 2 pixels.

	  If pixels=9 and ctweight=1, the processing
	  is identical to the moving average by 3x3
	  mask. If pixels=1, this function does
	  nothing except consuming some memory and 
	  enormous CPU power.

    Available ranges of the parameters:
        1 <= pixels <= 9
        0 <= ctweight
    Note: (pixels,ctweight)=(1,0) is prohibited.
------------------------------------------------------*/

int sip_kNNsmooth(SIPImage *src,SIPImage *dst,int pixels,int ctweight){
	int	x,y,i;
	uchar	*pu,*p,*pd,*ptmp;
	uchar	*dp;
	uchar	*lbuf;
	int	c,d;
	int	width,height;
	int	vtbl[8];
	int	dt[8];
	int	minpos;

	if ( src->depth != 8 ){
		return( sip_kNNsmoothc(src,dst,pixels,ctweight) );
	}
	if ( pixels == 3 ){
		return( sip_2NNsmooth(src,dst,ctweight) );
	}

	width = src->width;
	height = src->height;
	if ( 0 == (lbuf = (uchar *)malloc( sizeof(uchar) * 3 * (width+2)) ) )  return(-1);
	pu = lbuf;
	p  = pu + (width+2);
	pd = p  + (width+2);

	/* copy the first line to the line buffer */
	memcpy((void *)(p+1),(void *)sip_getimgptr(src,0),width);
	p[0] = p[1];  p[width+1] = p[width];

	/* the first upper line is as same as the first line */
	memcpy((void *)pu, (void *)p, width +2);

	for ( y=0 ; y<height ; y++ ){

		/* if the next line exists, copy it to the line buffer */
		if ( y < height -1 ){
			memcpy((void *)(pd+1),(void *)sip_getimgptr(src,y+1),width);
			pd[0] = pd[1];  pd[width+1] = pd[width];
		}
		else{	pd = p; 	/* if bottom, use current line instead */
		}

		dp = (uchar *)sip_getimgptr(dst,y);
		for ( x=0 ; x < dst->width ; x++ ){
			c = (int)((uint)p [x+1]);
			d = c - (vtbl[0] = (int)((uint)pu[x  ]));  dt[0] = d * d;
			d = c - (vtbl[1] = (int)((uint)pu[x+1]));  dt[1] = d * d;
			d = c - (vtbl[2] = (int)((uint)pu[x+2]));  dt[2] = d * d;
			d = c - (vtbl[3] = (int)((uint)p [x  ]));  dt[3] = d * d;
			d = c - (vtbl[4] = (int)((uint)p [x+2]));  dt[4] = d * d;
			d = c - (vtbl[5] = (int)((uint)pd[x  ]));  dt[5] = d * d;
			d = c - (vtbl[6] = (int)((uint)pd[x+1]));  dt[6] = d * d;
			d = c - (vtbl[7] = (int)((uint)pd[x+2]));  dt[7] = d * d;

			c *= ctweight;
			for ( i=0 ; i<pixels-1 ; i++ ){
				d = dt[0];  minpos = 0;
				if ( d > dt[1] ){  d = dt[1];  minpos = 1; }
				if ( d > dt[2] ){  d = dt[2];  minpos = 2; }
				if ( d > dt[3] ){  d = dt[3];  minpos = 3; }
				if ( d > dt[4] ){  d = dt[4];  minpos = 4; }
				if ( d > dt[5] ){  d = dt[5];  minpos = 5; }
				if ( d > dt[6] ){  d = dt[6];  minpos = 6; }
				if ( d > dt[7] ){  d = dt[7];  minpos = 7; }
				c += vtbl[minpos];
				dt[minpos] = 0x7fffffff;	/* cancel */
			}
			dp[x] = (uchar)(c / (ctweight + pixels -1));
				/* Note that we don't need any limiter. */
		}

		/* rotate the line buffers */
		ptmp = pu;  pu = p;  p = pd;  pd = ptmp;
	}
	free((void *)lbuf);
	return(0);
}


static int sip_2NNsmooth(SIPImage *src,SIPImage *dst,int ctweight){
	int	x,y,i;
	uchar	*pu,*p,*pd,*ptmp;
	uchar	*dp;
	uchar	*lbuf;
	int	c,d;
	int	width,height;
	int	vtbl[8];
	int	dt[8];
	int	minpos;

	width = src->width;
	height = src->height;
	if ( 0 == (lbuf = (uchar *)malloc( sizeof(uchar) * 3 * (width+2)) ) )  return(-1);
	pu = lbuf;
	p  = pu + (width+2);
	pd = p  + (width+2);

	/* copy the first line to the line buffer */
	memcpy((void *)(p+1),(void *)sip_getimgptr(src,0),width);
	p[0] = p[1];  p[width+1] = p[width];

	/* the first upper line is as same as the first line */
	memcpy((void *)pu, (void *)p, width +2);

	for ( y=0 ; y<height ; y++ ){

		/* if the next line exists, copy it to the line buffer */
		if ( y < height -1 ){
			memcpy((void *)(pd+1),(void *)sip_getimgptr(src,y+1),width);
			pd[0] = pd[1];  pd[width+1] = pd[width];
		}
		else{	pd = p; 	/* if bottom, use current line instead */
		}

		dp = (uchar *)sip_getimgptr(dst,y);
		for ( x=0 ; x < dst->width ; x++ ){
			c = (int)((uint)p [x+1]);
			d = c - (vtbl[0] = (int)((uint)pu[x  ]));  dt[0] = d * d;
			d = c - (vtbl[1] = (int)((uint)pu[x+1]));  dt[1] = d * d;
			d = c - (vtbl[2] = (int)((uint)pu[x+2]));  dt[2] = d * d;
			d = c - (vtbl[3] = (int)((uint)p [x  ]));  dt[3] = d * d;
			d = c - (vtbl[4] = (int)((uint)p [x+2]));  dt[4] = d * d;
			d = c - (vtbl[5] = (int)((uint)pd[x  ]));  dt[5] = d * d;
			d = c - (vtbl[6] = (int)((uint)pd[x+1]));  dt[6] = d * d;
			d = c - (vtbl[7] = (int)((uint)pd[x+2]));  dt[7] = d * d;

			c *= ctweight;
			for ( i=0 ; i<2 ; i++ ){
				d = dt[0];  minpos = 0;
				if ( d > dt[1] ){  d = dt[1];  minpos = 1; }
				if ( d > dt[2] ){  d = dt[2];  minpos = 2; }
				if ( d > dt[3] ){  d = dt[3];  minpos = 3; }
				if ( d > dt[4] ){  d = dt[4];  minpos = 4; }
				if ( d > dt[5] ){  d = dt[5];  minpos = 5; }
				if ( d > dt[6] ){  d = dt[6];  minpos = 6; }
				if ( d > dt[7] ){  d = dt[7];  minpos = 7; }
				c += vtbl[minpos];
				dt[minpos] = 0x7fffffff;	/* cancel */
			}
			dp[x] = (uchar)(c / (ctweight + 2));
				/* Note that we don't need any limiter. */
		}

		/* rotate the line buffers */
		ptmp = pu;  pu = p;  p = pd;  pd = ptmp;
	}
	free((void *)lbuf);
	return(0);
}


static int sip_kNNsmoothc(SIPImage *src,SIPImage *dst,int pixels,int ctweight){
	int	x,y,x4,i;
	uchar	*pu,*p,*pd,*ptmp;
	uchar	*dp;
	uchar	*lbuf;
	int	r,g,b,d;
	int	width,height;
	uchar	*vtblp[8];
	int	dt[8];
	int	minpos;

	if ( src->depth != 32 )  return(-1);
	width = src->width;
	height = src->height;
	if ( 0 == (lbuf = (uchar *)malloc( 3 * 4 * (width+2)) ) )  return(-1);
	pu = lbuf;
	p  = pu + 4 * (width+2);
	pd = p  + 4 * (width+2);

	/* copy the first line to the line buffer */
	memcpy((void *)(p+4),(void *)sip_getimgptr(src,0),4 * width);
	for ( i=0 ; i<4 ; i++ ){
		p[i] = p[4+i];  p[4*(width+1) +i] = p[4*width +i];
	}

	/* the first upper line is as same as the first line */
	memcpy((void *)pu, (void *)p, 4 * (width +2));

	for ( y=0 ; y<height ; y++ ){

		/* if the next line exists, copy it to the line buffer */
		if ( y < height -1 ){
			memcpy((void *)(pd+4),(void *)sip_getimgptr(src,y+1),4 * width);
			for ( i=0 ; i<4 ; i++ ){
				pd[i] = pd[4+i];  pd[4*(width+1) +i] = pd[4*width +i];
			}
		}
		else{	pd = p; 	/* if bottom, use current line instead */
		}

		dp = (uchar *)sip_getimgptr(dst,y);
		for ( x=0 ; x < dst->width ; x++ ){
			x4 = 4 * x;
			r = (int)((uint)p [x4+4]);
			g = (int)((uint)p [x4+5]);
			b = (int)((uint)p [x4+6]);

			vtblp[0] = &pu[x4  ];
			vtblp[1] = &pu[x4+4];
			vtblp[2] = &pu[x4+8];
			vtblp[3] = &p [x4  ];
			vtblp[4] = &p [x4+8];
			vtblp[5] = &pd[x4  ];
			vtblp[6] = &pd[x4+4];
			vtblp[7] = &pd[x4+8];

			for ( i=0 ; i<8 ; i++ ){
				d = r - (int)((uint)vtblp[i][0]);
				dt[i]  = d * d;
				d = g - (int)((uint)vtblp[i][1]);
				dt[i] += d * d;
				d = b - (int)((uint)vtblp[i][2]);
				dt[i] += d * d;
			}

			r *= ctweight;
			g *= ctweight;
			b *= ctweight;
			for ( i=0 ; i<pixels-1 ; i++ ){
				d = dt[0];  minpos = 0;
				if ( d > dt[1] ){  d = dt[1];  minpos = 1; }
				if ( d > dt[2] ){  d = dt[2];  minpos = 2; }
				if ( d > dt[3] ){  d = dt[3];  minpos = 3; }
				if ( d > dt[4] ){  d = dt[4];  minpos = 4; }
				if ( d > dt[5] ){  d = dt[5];  minpos = 5; }
				if ( d > dt[6] ){  d = dt[6];  minpos = 6; }
				if ( d > dt[7] ){  d = dt[7];  minpos = 7; }
				r += (int)((uint)vtblp[minpos][0]);
				g += (int)((uint)vtblp[minpos][1]);
				b += (int)((uint)vtblp[minpos][2]);
				dt[minpos] = 0x7fffffff;	/* cancel */
			}
			dp[x4  ] = (uchar)((uint)(r / (ctweight + pixels -1)));
			dp[x4+1] = (uchar)((uint)(g / (ctweight + pixels -1)));
			dp[x4+2] = (uchar)((uint)(b / (ctweight + pixels -1)));
				/* Note that we don't need any limiter. */
		}

		/* rotate the line buffers */
		ptmp = pu;  pu = p;  p = pd;  pd = ptmp;
	}
	free((void *)lbuf);
	return(0);
}
