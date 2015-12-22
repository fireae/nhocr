/*----------------------------------------------------------------------
    Character segmentation function    segchar_adhoc.cpp
      (Ad-hoc version)
        Written by H.Goto, Jan. 2008
        Revised by H.Goto, Feb. 2008
        Revised by H.Goto, Sep. 2008
        Revised by H.Goto, Jan. 2009
        Revised by H.Goto, May  2009
        Revised by H.Goto, July 2009
        Revised by H.Goto, Oct. 2009
        Revised by H.Goto, Aug. 2014
----------------------------------------------------------------------*/

/*--------------
  Copyright 2008-2014  Hideaki Goto

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
--------------*/


#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <math.h>

#include	"utypes.h"
#include	"siplib.h"

#include	"segchar_adhoc.h"


#define	CharOverlap	0.4
#define	MergeWidthLimit	1.3


/*----------------------------------------------
    CharBox class definition
----------------------------------------------*/

CharBox :: CharBox(void){
	charcore = 0;
	conn_R = 0;
	nbox = 0;
	parent = -1;
	poshint = PosHint_None;
	sizehint = SizeHint_None;
}



int CharBox :: iscore(int refsize){
	int	w = xe-xs+1;
//(Sep.29, 2008)	if ( w <= 2 * refsize /3 )  return(0);	// too small
	if ( w <= 3 * refsize /5 )  return(0);	// too small
	if ( w >= 2 * refsize )  return(0);	// too big
	return(1);
}




/*----------------------------------------------
    Get local line height (and vertical ends)
----------------------------------------------*/

static int lineheight_local(SIPImage *lineimage, int x0, int w, \
	int *ys, int *ye){
	int	x,y;
	int	height = lineimage->height;
	int	tys,tye;
	uchar	**p;
	uint	psum;

	w += x0;
	tys = height;
	tye = -1;
	p = (uchar **)lineimage->pdata;
	for ( y=0 ; y<height ; y++ ){
		psum = 0 ;
		for ( x=x0 ; x<w ; x++ ){
			if ( p[y][x] == 0 )  psum++;
		}
		if ( psum ){  tys = y;  break; }
	}
	for ( y=height -1 ; y>=0 ; y-- ){
		psum = 0 ;
		for ( x=x0 ; x<w ; x++ ){
			if ( p[y][x] == 0 )  psum++;
		}
		if ( psum ){  tye = y;  break; }
	}
	if ( ys )  *ys = tys;
	if ( ye )  *ye = tye;
	if ( tys > tye )  return(0);
	return( tye-tys+1 );
}




/*----------------------------------------------
    Get left and right ends of text string
----------------------------------------------*/

static int LRend(SIPImage *image, int *left, int *right){
	int	x,y;
	uchar	**p;
	uint	psum;

	*left = *right = 0;
	p = (uchar **)image->pdata;
	for ( x=0 ; x<image->width ; x++ ){
		psum = 0;
		for ( y=0 ; y<image->height ; y++ ){
			if ( p[y][x] == 0 )  psum++;
		}
		if ( psum ){
			*left = x;
			break;
		}
	}
	if ( x >= image->width ){
		return(0);
	}
	for ( x=image->width-1 ; x>=0 ; x-- ){
		psum = 0;
		for ( y=0 ; y<image->height ; y++ ){
			if ( p[y][x] == 0 )  psum++;
		}
		if ( psum ){
			*right = x;
			break;
		}
	}
	return( *right - *left +1 );
}




/*----------------------------------------------
    Get average line height
----------------------------------------------*/

static int lineheight_avr(SIPImage *lineimage){
	int	width;
	int	xs,xe;
	int	h,hmax,havr,cw;
	int	n;
	int	i,j;
	if ( 0 == (width = LRend(lineimage,&xs,&xe)) )  return(0);
	cw = 8;
	for ( j=0 ; j<2 ; j++ ){
		n = width / cw;
		if ( n < 2 ){	// if the line is very short
			return( lineheight_local(lineimage,xs,width,0,0) );
		}
		hmax = havr = 0;
		for ( i=0 ; i<n ; i++ ){
			h = lineheight_local(lineimage,xs+(cw*i),cw,0,0);
			havr += h;
			if ( hmax < h )  hmax = h;
		}
		havr /= n;
		if ( n < 4 ){
			// if the line is not long enough
			// compared with column width
			return(hmax);
		}
//		cw = (int)(1.5 * (hmax + havr) /2);
		cw = (int)(3.0 * (hmax + havr) /2);
	}
	return(havr);
}




/*----------------------------------------------
    Get top and bottom lines
----------------------------------------------*/

static int gettopbottomlines(SIPImage *image, int w, int *top, int *bottom){
	int	x,ys,ye;
	ys = ye = 0;
	if ( w >= image->width ){
		lineheight_local(image, 0, image->width , &ys, &ye);
		for ( x=0 ; x<image->width ; x++ ){
			top[x] = ys;
			bottom[x] = ye;
		}
		return(0);
	}
	for ( x=0 ; x<image->width - w ; x++ ){
		lineheight_local(image, x, w, &ys, &ye);
		top[x+(w/2)] = ys;
		bottom[x+(w/2)] = ye;
	}
	for ( ; x<image->width -(w/2) ; x++ ){
		top[x+(w/2)] = ys;
		bottom[x+(w/2)] = ye;
	}
	for ( x=0 ; x<(w/2) ; x++ ){
		top[x] = top[w/2];
		bottom[x] = bottom[w/2];
	}
	return(0);
}




/*----------------------------------------------
    Get average box width
----------------------------------------------*/

static double average_boxwidth(SIPImage *lineimage, \
	CharBox *cba, int lineheight){
	int	i,n1;
	CharBox	*cb;
	int	xs,xe,w;
	double	avr_boxwidth;

	avr_boxwidth = 0;  n1 = 0;
	for ( i=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox ){
		cb = &cba[i];
		xs = cb->xs;
		xe = cb->xe;
		w = xe - xs +1;
		lineheight_local(lineimage,xs,w, \
			&cb->ys,&cb->ye);
		if ( cb->height() < lineheight /2 \
		  && cb->width() < lineheight /2 ){
			cb->sizehint = SizeHint_Tiny;
		}
#if 0
		printf("(%d,%d) - (%d,%d)  %dx%d\n", \
			xs, cb->ys, xe, cb->ye, \
			w, cb->ye - cb->ys +1);
#endif

		if ( w < lineheight /2 )  continue;
		if ( w > 2*lineheight )  continue;

		avr_boxwidth += (double)w;
		n1++;
	}
	if ( n1 < 1 ){	/* use an alternative (degraded) parameter */
		avr_boxwidth = 0;  n1 = 0;
		for ( i=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox ){
			cb = &cba[i];
			w = cb->xe - cb->xs +1;
			if ( w < lineheight /3 )  continue;
			avr_boxwidth += (double)w;
			n1++;
		}
		if ( n1 < 1 ){
			avr_boxwidth = lineheight /2.0;
			n1 = 1;
		}
	}
	avr_boxwidth /= (double)n1;

	// set charcore flags
	for ( i=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox ){
		cb = &cba[i];
		w = cb->xe - cb->xs +1;
		if ( cb->iscore(lineheight) ){
			cb->charcore = 10;
		}
	}

	return(avr_boxwidth);
}




/*----------------------------------------------
    Get average box pitch
----------------------------------------------*/

static double average_pitch(CharBox *cba, \
	int lineheight, double avr_boxwidth){
	int	i, n1, n2;
	double	avr_pitch;
	CharBox	*cb, *cb_next;
	int	c1,c2;

	avr_pitch = 0;  n1 = n2 = 0;
	for ( i=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox ){
		cb = &cba[i];
		c1 = cb->xc();
		if ( cb->charcore != 0 )  n1++;
		cb_next = &cba[i+cba[i].nbox];
		if ( cb_next->nbox == 0 )  break;
		c2 = cb_next->xc();
		if ( cb->charcore == 0 || cb_next->charcore == 0 )  continue;
		c2 -= c1;
		if ( c2 < avr_boxwidth )  continue;
//		printf("c2=%d\n",c2);
		avr_pitch += (double)c2;
		n2++;
	}
	if ( n2 < 1 || n1 < 1 ){
		avr_pitch = avr_boxwidth;  n2 = 1;
	}
	if ( n1 == 1 ){
		if ( avr_pitch < lineheight )  avr_pitch = lineheight;
	}
	avr_pitch /= (double)n2;
	return(avr_pitch);
}




/*----------------------------------------------
    Clean-up small dots
----------------------------------------------*/

static int clean_dots(CharBox *cba, int lineheight){
	int	th = 1;
	int	i,j;

	if ( lineheight <= 12 )  return(0);	// no change
	if ( lineheight > 20 )  th = 2;		// 1x2, 2x1
	if ( lineheight > 32 )  th = 4;		// 1x4, 2x2, 4x1

	for ( i=j=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox ){
		if ( cba[i].width() * cba[i].height() <= th )  continue;
		cba[j++] = cba[i];
	}
	cba[j].nbox = 0;	// terminator
	return(0);
}




/*----------------------------------------------
    Count boxes
----------------------------------------------*/

static int boxcount_merged(CharBox *cba){
	int	i,n;
	for ( i=n=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox )  n++;
	return(n);
}


static int boxcount(CharBox *cba){
	int	i;
	for ( i=0 ; cba[i].nbox != 0 ; i++ )  ;
	return(i);
}




/*----------------------------------------------
    Get character box overlap
----------------------------------------------*/

static double charbox_overlap(CharBox *cb1, CharBox *cb2){
	int	w1 = cb1->width();
	int	w2 = cb2->width();
	w1 = w1 < w2 ? w1 : w2;		// MIN
	if ( cb1->xs <= cb2->xs ){
		if ( cb1->xe < cb2->xs )  return(0);
		if ( cb1->xe > cb2->xe )  return(1.0);
		w2 = cb1->xe - cb2->xs +1;
	}
	else{
		if ( cb2->xe < cb1->xs )  return(0);
		if ( cb2->xe > cb1->xe )  return(1.0);
		w2 = cb2->xe - cb1->xs +1;
	}
	return((double)w2/(double)w1);
}




/*----------------------------------------------
    Merge character boxes
----------------------------------------------*/

static int parent_charbox(CharBox *cba, int bid){
	while( cba[bid].parent >= 0 )  bid = cba[bid].parent;
	return(bid);
}


#if 0
static void merge_charbox(CharBox *cb1, CharBox *cb2){
	if ( cb1 == cb2 )  return;
	cb1->xs = cb1->xs < cb2->xs ? cb1->xs : cb2->xs;
	cb1->xe = cb1->xe > cb2->xe ? cb1->xe : cb2->xe;
	cb1->ys = cb1->ys < cb2->ys ? cb1->ys : cb2->ys;
	cb1->ye = cb1->ye > cb2->ye ? cb1->ye : cb2->ye;
	cb1->nbox += cb2->nbox;
	cb2->parent = 1;
}
#endif


static int merge_charbox(CharBox *cba, int bid1, int bid2){
	bid1 = parent_charbox(cba,bid1);
	bid2 = parent_charbox(cba,bid2);
	if ( bid1 == bid2 )  return(0);
	CharBox	*cb1 = &cba[bid1];
	CharBox	*cb2 = &cba[bid2];
	cb1->xs = cb1->xs < cb2->xs ? cb1->xs : cb2->xs;
	cb1->xe = cb1->xe > cb2->xe ? cb1->xe : cb2->xe;
	cb1->ys = cb1->ys < cb2->ys ? cb1->ys : cb2->ys;
	cb1->ye = cb1->ye > cb2->ye ? cb1->ye : cb2->ye;
	cb1->nbox += cb2->nbox;
	cb2->parent = bid1;
	return(1);
}


static int merged_width(CharBox *cba, int bid1, int bid2){
	bid1 = parent_charbox(cba,bid1);
	bid2 = parent_charbox(cba,bid2);
	CharBox	*cb1 = &cba[bid1];
	CharBox	*cb2 = &cba[bid2];
	int	xs = cb1->xs < cb2->xs ? cb1->xs : cb2->xs;
	int	xe = cb1->xe > cb2->xe ? cb1->xe : cb2->xe;
	return( xe-xs+1 );
}




/*----------------------------------------------
    Refine segmentation
----------------------------------------------*/

static int refine_charbox_1(SIPImage *lineimage, \
	CharBox *cba, \
	int lineheight, \
	double *avr_boxwidth, double *avr_pitch, \
	int corelevel, double wmul){
	CharBox	*cb;
	CharBox	cwindow0;
	CharBox	cwindow;
	CharBox	cwindowL;
	double	pitch = *avr_pitch;
	double	ww = wmul * (*avr_boxwidth + *avr_pitch) /2;	// window width
	int	bid, bid2, nbox;

	nbox = boxcount(cba);

	// find a core box
	for ( bid=0 ; bid<nbox ; bid++ ){
		if ( cba[bid].charcore >= corelevel \
		  && cba[bid].parent < 0 )  break;
	}
	if ( bid>=nbox ){
		// Skip the process if no core exists.
		return(0);
	}

	SIPImage	*tmpimg = sip_DuplicateImage(lineimage);
	uchar		**p = (uchar **)tmpimg->pdata;

	for ( bid=0 ; bid<nbox ; bid++ ){
		// search for the next core box
		for ( ; bid<nbox ; bid++ ){
			if ( cba[bid].charcore >=corelevel \
			  && cba[bid].parent < 0 )  break;
		}
		if ( bid >= nbox )  break;

		cwindow0 = *(cb = &cba[bid]);
		cwindow0.xs = cb->xc() - (int)(ww/2);
		cwindow0.xe = cwindow0.xs + (int)ww -1;

		int	pbid_m;

		// rightside (+1)
		cwindow = cwindow0;
		cwindow.xs += (int)(pitch + .5);
		cwindow.xe += (int)(pitch + .5);
p[0][cwindow.xs] = 0;
p[1][cwindow.xe] = 127;
		pbid_m = -1;
		for ( bid2=bid ; bid2<nbox ; bid2++ ){
			int	pbid = parent_charbox(cba,bid2);
			cb = &cba[pbid];
			if ( cb->xe < cwindow.xs )  continue;
			if ( charbox_overlap(cb,&cwindow) < CharOverlap )  continue;
			if ( cb->xs > cwindow.xe )  break;
			if ( pbid_m < 0 )  pbid_m = pbid;

			// merger trial
			if ( merged_width(cba,pbid_m,pbid) > 1.2 * ww )  break;

			if ( merge_charbox(cba,pbid_m,pbid) )  cb->charcore = 0;
		}
		if ( pbid_m >= 0 ){
			CharBox	*cbm = &cba[pbid_m];
			if ( cbm->iscore(lineheight) && cbm->charcore == 0 ){
				cbm->charcore = corelevel-1;
			}
		}

#if 1
		// rightside (+2)
		cwindow = cwindow0;
		// We use 1.9 instead of 2.0.
		// The small shift to the left is useful for avoiding
		// inappropriate merger.
		cwindow.xs += (int)(1.9*pitch + .5);
		cwindow.xe += (int)(1.9*pitch + .5);
		pbid_m = -1;
		for ( bid2=bid ; bid2<nbox ; bid2++ ){
			int	pbid = parent_charbox(cba,bid2);
			cb = &cba[pbid];
			if ( cb->xe < cwindow.xs )  continue;
			if ( charbox_overlap(cb,&cwindow) < CharOverlap )  continue;
			if ( cb->xs > cwindow.xe )  break;
			if ( pbid_m < 0 )  pbid_m = pbid;

			// merger trial
			if ( merged_width(cba,pbid_m,pbid) > 1.2 * ww )  break;

			if ( merge_charbox(cba,pbid_m,pbid) )  cb->charcore = 0;
		}
		if ( pbid_m >= 0 ){
			CharBox	*cbm = &cba[pbid_m];
			if ( cbm->iscore(lineheight) && cbm->charcore == 0 ){
				cbm->charcore = corelevel-1;
			}
		}
#endif

		// leftside (-1)
		cwindowL = cwindow0;
		cwindowL.xs -= (int)(pitch + .5);
		cwindowL.xe -= (int)(pitch + .5);
		pbid_m = -1;
		for ( bid2=0 ; bid2<nbox ; bid2++ ){
			int	pbid = parent_charbox(cba,bid2);
			cb = &cba[pbid];
			if ( cb->xe < cwindowL.xs )  continue;
			if ( charbox_overlap(cb,&cwindowL) < CharOverlap )  continue;
			if ( cb->xs > cwindowL.xe )  break;
			if ( pbid_m < 0 )  pbid_m = pbid;

			// merger trial
			if ( merged_width(cba,pbid_m,pbid) > 1.2 * ww )  break;

			if ( merge_charbox(cba,pbid_m,pbid) )  cb->charcore = 0;
		}
		if ( pbid_m >= 0 ){
			CharBox	*cbm = &cba[pbid_m];
			if ( cbm->iscore(lineheight) && cbm->charcore == 0 ){
				cbm->charcore = corelevel-1;
			}
		}

#if 1
		// leftside (-2)
		cwindowL = cwindow0;
		// We use 1.9 instead of 2.0.
		// The small shift to the right is useful for avoiding
		// inappropriate merger.
		cwindowL.xs -= (int)(1.9*pitch + .5);
		cwindowL.xe -= (int)(1.9*pitch + .5);
		pbid_m = -1;
		for ( bid2=0 ; bid2<nbox ; bid2++ ){
			int	pbid = parent_charbox(cba,bid2);
			cb = &cba[pbid];
			if ( cb->xe < cwindowL.xs )  continue;
			if ( charbox_overlap(cb,&cwindowL) < CharOverlap )  continue;
			if ( cb->xs > cwindowL.xe )  break;
			if ( pbid_m < 0 )  pbid_m = pbid;

			// merger trial
			if ( merged_width(cba,pbid_m,pbid) > 1.2 * ww )  break;

			if ( merge_charbox(cba,pbid_m,pbid) )  cb->charcore = 0;
		}
		if ( pbid_m >= 0 ){
			CharBox	*cbm = &cba[pbid_m];
			if ( cbm->iscore(lineheight) && cbm->charcore == 0 ){
				cbm->charcore = corelevel-1;
			}
		}
#endif
	}

	sip_DestroyImage(tmpimg);
	return(0);
}




static int refine_charbox_1(SIPImage *lineimage, 
	CharBox *cba, \
	int lineheight, double *avr_boxwidth, double *avr_pitch){
	int	rs;
	rs = refine_charbox_1(lineimage, cba, \
		lineheight, avr_boxwidth, avr_pitch, 10, 1.0);
	if ( rs )  return(rs);
	rs = refine_charbox_1(lineimage, cba, \
		lineheight, avr_boxwidth, avr_pitch, 9, 1.2);
	return(rs);
}




static int refine_charbox_2(SIPImage *lineimage, \
	CharBox *cba, \
	int lineheight, double *avr_boxwidth, double *avr_pitch){
	CharBox	*cb;
	int	*charbox;
	int	i, bid, bid2, nbox;
	int	h,w1,w2,wlimit;

	nbox = boxcount_merged(cba);
	charbox = new int[ nbox>0 ? nbox:1 ];

	for ( i=bid=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox ){
		cb = &cba[ charbox[bid++] = i ];
		if ( cb->iscore(lineheight) ){
			cb->charcore = 8;
		}
		else{	cb->charcore = 0;
		}
	}

	wlimit = (int)(MergeWidthLimit * (*avr_boxwidth));
	for ( bid=1 ; bid<nbox-1 ; bid++ ){
		cb = &cba[charbox[bid]];
		if ( cb->charcore )  continue;
		if ( cb->sizehint != SizeHint_None )  continue;
		h = cb->height();
		w1 = w2 = 10000;
		if ( cba[charbox[bid-1]].charcore ){
			w1 = cb->xe - cba[charbox[bid-1]].xs +1;
		}
		if ( cba[charbox[bid+1]].charcore ){
			w2 = cba[charbox[bid+1]].xe - cb->xs +1;
		}
		if ( w1 == 10000 && w2 == 10000 )  continue;
		if ( w1 < w2 ){	// merge to the left
			if ( h < cba[charbox[bid-1]].height() ){
				h = cba[charbox[bid-1]].height();
			}
			if ( w1 > MergeWidthLimit * h )  continue;
			if ( w1 > wlimit )  continue;
			merge_charbox(cba, charbox[bid-1], charbox[bid]);
			for ( bid2=bid ; bid2<nbox-1 ; bid2++ ){
				charbox[bid2] = charbox[bid2+1];
			}
			nbox--;
		}
		else{		// merge to the right
			if ( h < cba[charbox[bid+1]].height() ){
				h = cba[charbox[bid+1]].height();
			}
			if ( w2 > MergeWidthLimit * h )  continue;
			if ( w2 > wlimit )  continue;
			merge_charbox(cba, charbox[bid], charbox[bid+1]);
			for ( bid2=bid+1 ; bid2<nbox-1 ; bid2++ ){
				charbox[bid2] = charbox[bid2+1];
			}
			nbox--;
		}
	}

	if ( nbox > 1 ){
		w1 = cba[charbox[1]].xe - cba[charbox[0]].xs +1;
		if ( w1 <= wlimit ){
			merge_charbox(cba, charbox[0], charbox[1]);
			for ( bid=1 ; bid<nbox-1 ; bid++ ){
				charbox[bid] = charbox[bid+1];
			}
			nbox--;
		}
	}
	if ( nbox > 1 ){
		w1 = cba[charbox[nbox-1]].xe - cba[charbox[nbox-2]].xs +1;
		if ( w1 <= wlimit ){
			merge_charbox(cba, charbox[nbox-2], charbox[nbox-1]);
			nbox--;
		}
	}

	delete []charbox;
	return(0);
}




/*----------------------------------------------
    Segment characters in a text line
----------------------------------------------*/

int segmentchars(SIPImage *lineimage, \
	CharBox *cba, CharBox *cba_raw, \
	double *avrcwidth, double *lineheight, double *charpitch, \
	int force_alpha, int wdir, int debug){
	int	x,y;
	int	width,height,charsize;
	uchar	**p;
	uint	psum;
	int	con;
	int	lc;
	int	lheight;
	CharBox	*cb;
	double	avr_boxwidth;
	double	avr_pitch;
	int	amode = 1;
	int	*top = 0;
	int	*bottom;
	int	cbi;

	width  = lineimage->width;
	height = lineimage->height;
	p = (uchar**)lineimage->pdata;

	if ( 0 == (top = new int[2*width]) )  return(-1);
	bottom = top + width;

	// 1st segmentation: find character components
	cbi = 0;
	for ( x=0 ; x<width ; x++ ){
		int	ys = height;
		int	ye = -1;
		// search for the left-end column of the next component
		for ( ; x<width ; x++ ){
			psum = 0;
			for ( y=0 ; y<height ; y++ ){
				if ( 0 != p[y][x] )  continue;
				psum = 1;
				if ( ye < y )  ye = y;
				if ( ys > y )  ys = y;
			}
			if ( psum )  break;
		}
		lc = x;

		// search for the right-end column of the component
		for ( ; x<width-1 ; x++ ){
			con = 0;  
			psum = 0;
			for ( y=0 ; y<height ; y++ ){
				if ( 0 != p[y][x] )  continue;
				psum++;
				if ( ye < y )  ye = y;
				if ( ys > y )  ys = y;
				if ( 0 == p[y][x+1] )  con = 1;
			}
			for ( y=1 ; y<height ; y++ ){
				if ( 0 == p[y][x] && 0 == p[y-1][x+1] )  con = 1;
			}
			for ( y=0 ; y<height-1 ; y++ ){
				if ( 0 == p[y][x] && 0 == p[y+1][x+1] )  con = 1;
			}
			if ( con == 0 ){
				cb = &cba[cbi++];
				cb->nbox = 1;
				cb->xs = lc;	cb->xe = x;
				cb->ys = ys;	cb->ye = ye;
				cb->alphamode = 0;
//	p[0][lc] = p[0][x] = 127;
				break;
			}
		}
		if ( x == width -1 ){
			cb = &cba[cbi++];
			cb->nbox = 1;
			cb->xs = lc;	cb->xe = x;
			cb->ys = ys;	cb->ye = ye;
			cb->alphamode = 0;
//	p[0][lc] = p[0][x] = 127;
		}
	}

	lheight = lineheight_avr(lineimage);
	clean_dots(cba,lheight);
	gettopbottomlines(lineimage, 3 * lheight, top, bottom);
	for ( cbi=0 ; cba[cbi].nbox != 0 ; cbi++ ){
		cba_raw[cbi] = cba[cbi];
	}

	if ( debug ){
		printf("Found %d box(es).\n", boxcount_merged(cba));
		printf("height=%d\n",lheight);
	}

	avr_boxwidth = average_boxwidth(lineimage,cba,lheight);
		// cb->ys and cb->ye are set inside avr_boxwidth().
	avr_pitch = average_pitch(cba, lheight, avr_boxwidth);

	if ( debug ){
		printf("avr_boxwidth = %.1f\n", avr_boxwidth);
		printf("avr_pitch = %.1f\n", avr_pitch);
		printf("pitch/height = %.2f\n",avr_pitch/lheight);
	}

	if ( avr_pitch/lheight > .9 ){
		amode = 0;
	}
	if ( force_alpha )  amode = 1;

	if ( ! amode ){
		// 1st refinement
		refine_charbox_1(lineimage, cba, \
			lheight, &avr_boxwidth, &avr_pitch);

		// 2nd refinement
		refine_charbox_2(lineimage, cba, \
			lheight, &avr_boxwidth, &avr_pitch);
	}

	// position and size estimation
	for ( cbi=0 ; cba[cbi].nbox != 0 ; cbi+=cba[cbi].nbox ){
		cb = &cba[cbi];
		cb->poshint = PosHint_None;
		cb->sizehint = SizeHint_None;
		charsize = cb->width();
		if ( charsize < cb->height() ){
			charsize = cb->height();
		}
		if ( amode ){
			cb->alphamode = 1;
		}

		// character size estimation
		if ( lheight * 0.8 <= charsize ){
			// normal size
			cb->sizehint |= SizeHint_Normal; 
		}
		else if ( lheight * 0.5 <= charsize ){
			// small characters such as in Japanese
			cb->sizehint |= SizeHint_Small; 
		}
		else{	// tiny signs such as dots and commas
			cb->sizehint |= SizeHint_Tiny;
		}

		// character position estimation
		int	yt,yb,h;
		x = (cb->xs + cb->xe) /2;
		y = (cb->ys + cb->ye) /2;
		yt = (top[x] + bottom[x]) /2;
		h = bottom[x] - top[x] +1;
		yb = yt + h /5;
		yt -= h /3;
		if ( wdir & WrtDir_H ){	// horizontal mode
			if ( y < yt ){
				cb->poshint |= PosHint_Top;
			}
			else if ( y > yb ){
				cb->poshint |= PosHint_Bottom;
			}
			else{	cb->poshint |= PosHint_Middle;
			}
		}
		else{	// vertical mode
			if ( y < yt ){
				cb->poshint |= PosHint_Right;
			}
			else if ( y > yb ){
				cb->poshint |= PosHint_Left;
			}
			else{	cb->poshint |= PosHint_Center;
			}
		}
	}

// for monitor
	for ( cbi=0 ; cba[cbi].nbox != 0 ; cbi+=cba[cbi].nbox ){
		cb = &cba[cbi];
		p[0][cb->xs] = 127;
		p[0][cb->xe] = 200;
	}

	if ( avrcwidth )  *avrcwidth = avr_boxwidth;
	if ( lineheight )  *lineheight = lheight;
	if ( charpitch )  *charpitch = avr_pitch;

	// delete objects
	if ( top )  delete []top;

	return(0);
}


