/*----------------------------------------------------------------------
    Text line segmentation in a text block     segline.cpp
        Written by H.Goto, Nov. 1997
        Revised by H.Goto, May  2009  (License changed, code refined)
----------------------------------------------------------------------*/

/*--------------
  Copyright 1997-2009  Hideaki Goto

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


#define	RectWidth	16	/* Width of PRs */
#define	RectParColumn	4	/* Rectangles par Column */
#define	C_nrect		8

#define	C_clip		0.75	/* Const. for getting reference height */

#define	MAXSEGHEIGHT	332	/* > 40pt (at 600dpi) */

#define	HCS		1.9	/* Const. for connection suppression */
#define	C_dist		0.5
#define	C_rheight	0.70

				/* Vertical Line Elimination */
#define	VLE_Width	2	/*   Threshold for width */
#define	VLE_Length	16	/*   Threshold for length */

				/* Separating character combination */
#define	C_sepchr_height	0.8	/*   Maximum height of candidates */
#define	C_sepchr_dist	0.4	/*   Maximum distance */
#define	C_sepchr_merged	1.2	/*   Maximum merged height */


#define		INITGROUPS	20000



#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <limits.h>
#include        <float.h>

#include	"utypes.h"

#include	"CRect.h"
#include	"objgrp.h"
#include	"ufilep.h"
#include	"siplib.h"

#include	"segline.h"

#define		MAXCONST	0x7fffffff




/*----------------------------------------------
    Calculate Distance between Two Lines
----------------------------------------------*/

static int get_linedistance(GRPLST *tlgroup,CRects *rectlist,int ncolumns,\
		int cwid,int grpid1,int grpid2,int *ulflag){
	int	*llist1,*llist2;
	int	*ulist1,*ulist2;
	int	*idlist;
	CRect	*rect;
	int	i,c,ac,d,d0;
	int	uc,lc;
	int	umin,lmin;
	if ( ! ncolumns )  return(-1);
	if ( 0 == (llist1 = new int[ncolumns * 4]) )  return(-1);
	llist2 = llist1 + ncolumns;
	ulist1 = llist2 + ncolumns;
	ulist2 = ulist1 + ncolumns;
	for ( i=0 ; i<ncolumns ; i++ ){
		llist1[i] = llist2[i] = MAXCONST;
		ulist1[i] = ulist2[i] = -1;
	}
	idlist = tlgroup->getidlist(grpid1);
	for ( i=0 ; i < tlgroup->getidcounts(grpid1) ; i++ ){
		rect = rectlist->getrect(idlist[i]);
		c = rect->x1 / cwid;
		if ( llist1[c] > rect->y1 )  llist1[c] = rect->y1;
		if ( ulist1[c] < rect->y2 )  ulist1[c] = rect->y2;
	}
	idlist = tlgroup->getidlist(grpid2);
	for ( i=0 ; i < tlgroup->getidcounts(grpid2) ; i++ ){
		rect = rectlist->getrect(idlist[i]);
		c = rect->x1 / cwid;
		if ( llist2[c] > rect->y1 )  llist2[c] = rect->y1;
		if ( ulist2[c] < rect->y2 )  ulist2[c] = rect->y2;
	}
	d = 0;		/* first, get reference distance */
	umin = lmin = MAXCONST;
	for ( i=uc=lc=0 ; i<ncolumns ; i++ ){
		if ( ulist1[i] == -1 )  continue;
		if ( ulist2[i] == -1 )  continue;
		if ( (llist1[i] + ulist1[i]) < (llist2[i] + ulist2[i]) ){
			if ( 0 > (d0 = llist2[i] - ulist1[i]) )  d0 = 0;
			if ( umin > d0 )  umin = d0;
			uc++;
		}
		else{	if ( 0 > (d0 = llist1[i] - ulist2[i]) )  d0 = 0;
			if ( lmin > d0 )  lmin = d0;
			lc++;
		}
		d += d0;
	}
	ac = uc + lc;
	if ( ac )  d /= ac;  else d = -1;

	if ( ac && ac < 3 ){	/* use minimum value instead of mean value,  */
				/*              if the overlap is very short */
		if ( umin < lmin ){  uc = 1;  lc = 0;  d = umin; }
		else              {  uc = 0;  lc = 1;  d = lmin; }
	}

	if ( ulflag ){
		if ( uc > lc )  *ulflag = 1;  else *ulflag = -1;
	}
	delete []llist1;
	return(d);
}




/*----------------------------------------------
    Sort line data and Create order table
----------------------------------------------*/

static int get_linepos(GRPLST *tlgroup,CRects *rect_pool,int gid){
	int	i,xpos,ypos;
	CRect	*rect;
	xpos = MAXCONST;
	ypos = 0;	/* default position is y=0 */
	for ( i=0 ; i<tlgroup->getidcounts(gid) ; i++ ){
		rect = rect_pool->getrect( tlgroup->getidlist(gid)[i] );
		if ( xpos > rect->x1 ){
			xpos = rect->x1;
			ypos = (rect->y1 + rect->y2) /2;
		}
	}
	return(ypos);
}


static int get_linepos_v(GRPLST *tlgroup,CRects *rect_pool,int gid){
	int	i,xpos;
	CRect	*rect;
	xpos = MAXCONST;
	for ( i=0 ; i<tlgroup->getidcounts(gid) ; i++ ){
		rect = rect_pool->getrect( tlgroup->getidlist(gid)[i] );
		if ( xpos > rect->x1 ){
			xpos = rect->x1;
		}
	}
	return(xpos);
}


static void sort_horizontal(GRPLST *tlgroup,CRects *rect_pool,int *t_order, \
		int groups,int insertmode){
	int	i,j,k,m;
	for ( i=0 ; i<groups ; i++ ){		/* sort by horizontal position */
		for ( j=i+1 ; j<groups ; j++ ){
			if (  get_linepos(tlgroup,rect_pool,t_order[i]) \
			    > get_linepos(tlgroup,rect_pool,t_order[j]) ){
				k = t_order[j];
				if ( insertmode ){
					for ( m=j-1 ; m>=i ; m-- )
						t_order[m+1] = t_order[m];
				}
				else{	t_order[j] = t_order[i];
				}
				t_order[i] = k;
			}
		}
	}
}


static void sort_vertical(GRPLST *tlgroup,CRects *rect_pool,int *t_order, \
		int groups,int insertmode){
	int	i,j,k,m;
	for ( i=0 ; i<groups ; i++ ){		/* sort by horizontal position */
		for ( j=i+1 ; j<groups ; j++ ){
			if (  get_linepos_v(tlgroup,rect_pool,t_order[i]) \
			    > get_linepos_v(tlgroup,rect_pool,t_order[j]) ){
				k = t_order[j];
				if ( insertmode ){
					for ( m=j-1 ; m>=i ; m-- )
						t_order[m+1] = t_order[m];
				}
				else{	t_order[j] = t_order[i];
				}
				t_order[i] = k;
			}
		}
	}
}




static int sort_linedata(GRPLST *tlgroup,CRects *rect_pool,int ncolumns,\
		int cwid,int *t_order){
	int	i,j,k,m;
	int	gid1,gid2,groups;
	int	ulflag;
	int	*ag;
	int	*rev_idx;
	for ( gid1=gid2=0 ; gid1 < tlgroup->groups ; gid1++ ){
		if ( tlgroup->getidcounts(gid1) == 0 )  continue;
		t_order[gid2++] = gid1;
	}
	groups = gid2;
	if ( groups == 0 )  return(0);
	if ( 0 == (ag = new int[groups * groups]) )  return(groups);
					/* exit doing nothing! */
	if ( 0 == (rev_idx = new int[tlgroup->groups]) ){
		delete []ag;
		return(0);		/* exit doing nothing! */
	}

	sort_horizontal(tlgroup,rect_pool,t_order,groups,0);
	sort_vertical(tlgroup,rect_pool,t_order,groups,1);

					/* create adjacency graph */
	for ( i=0 ; i<groups ; i++ ){
		gid1 = t_order[i];
		rev_idx[gid1] = i;
		for ( j=i+1 ; j<groups ; j++ ){
			k = get_linedistance(tlgroup,rect_pool,\
				ncolumns,cwid,gid1,t_order[j],&ulflag);
			if ( k < 0 ){
				ag[i * groups + j] = ag[j * groups + i] = 0;
				continue;
			}
			ag[j * groups + i] = -(ag[i * groups + j] = ulflag);
		}
	}

	for ( i=0 ; i<groups ; i++ ){		/* sort (insertion) */
		gid1 = t_order[i];
		m = groups - i;
		for ( j=i+1 ; j<groups ; j++ ){
			gid2 = t_order[j];
			ulflag = ag[ rev_idx[gid1] * groups + rev_idx[gid2] ];
			if ( ulflag == 0 )  continue;
			if ( ulflag < 0 ){
				gid1 = t_order[j];
				for ( k=j-1 ; k>=i ; k-- )  t_order[k+1] = t_order[k];
				t_order[i] = gid1;
				if ( m >= 0 ){	/* loop limitter */
					m--;
					j = i;
				}
			}
		}
	}

	delete []rev_idx;
	delete []ag;
	return(groups);
}




/*----------------------------------------------
    Link segments
----------------------------------------------*/

static int link_segs(int width,int height,CRects *rect_pool,int ncolumn,\
		int *cltbl,GRPLST *tlgroup,int ref_height){
	int	i,j,k;
	int	cnum;
	int	dist,minrect;
	CRect	*rect1,*rect2;
	int	ct1,ct2;
	int	gid1,gid2;
	int	h1,h2;

	for ( i=0 ; i<cltbl[1] ; i++ ){
		rect1 = rect_pool->getrect(i);
		if ( 0 > (gid1 = tlgroup->newgrp(i)) )  return(-1);
		rect1->attr = gid1;
	}

	for ( cnum=0 ; cnum < ncolumn -1 ; cnum++ ){

		/* left -> right connection */
		for ( i=cltbl[cnum] ; i<cltbl[cnum+1] ; i++ ){
			dist = height;  minrect = -1;
			rect1 = rect_pool->getrect(i);
			ct1 = (rect1->y1 + rect1->y2) /2;
			gid1 = rect1->attr;
			if ( gid1 < 0 ){
				if ( 0 > (gid1 = tlgroup->newgrp(i)) ){
					return(-1);
				}
				rect1->attr = gid1;
			}
			h1 = (int)(rect1->y2 - rect1->y1 +1);
			if ( h1 > HCS * ref_height )  continue;
			/* find nearest segment */
			for ( j=cltbl[cnum+1] ; j<cltbl[cnum+2] ; j++ ){
				rect2 = rect_pool->getrect(j);
				h2 = (int)(rect2->y2 - rect2->y1 +1);
				if ( h2 > HCS * ref_height )  continue;
				ct2 = (rect2->y1 + rect2->y2) /2;
				if ( dist > abs(ct1-ct2) ){
					dist = abs(ct1-ct2);
					minrect = j;
				}
			}
			if ( minrect == -1 )  continue;
			rect2 = rect_pool->getrect(minrect);
			if ( rect1->y1 > rect2->y2 )  continue;
			if ( rect1->y2 < rect2->y1 )  continue;
			gid2 = rect2->attr;
			if ( gid2 < 0 ){
				if ( 0 > tlgroup->addgrp(gid1,minrect) )  return(-2);
				rect2->attr = gid1;
			}
			else if ( gid1 != gid2 ){
				for ( k=0 ; k<tlgroup->getidcounts(gid2) ; k++ ){
					rect_pool->getrect( tlgroup->getidlist(gid2)[k] )->attr = gid1;
				}
				if ( 0 > tlgroup->mergegrps(gid1,gid2) )  return(-3);
				tlgroup->deletegrp(gid2);
			}
		}

		/* left <- right connection */
		for ( i=cltbl[cnum+1] ; i<cltbl[cnum+2] ; i++ ){
			dist = height;  minrect = -1;
			rect1 = rect_pool->getrect(i);
			ct1 = (rect1->y1 + rect1->y2) /2;
			gid1 = rect1->attr;
			if ( gid1 < 0 ){
				if ( 0 > (gid1 = tlgroup->newgrp(i)) ){
					return(-1);
				}
				rect1->attr = gid1;
			}
			h1 = (int)(rect1->y2 - rect1->y1 +1);
			if ( h1 > HCS * ref_height )  continue;
			/* find nearest segment */
			for ( j=cltbl[cnum] ; j<cltbl[cnum+1] ; j++ ){
				rect2 = rect_pool->getrect(j);
				h2 = (int)(rect2->y2 - rect2->y1 +1);
				if ( h2 > HCS * ref_height )  continue;
				ct2 = (rect2->y1 + rect2->y2) /2;
				if ( dist > abs(ct1-ct2) ){
					dist = abs(ct1-ct2);
					minrect = j;
				}
			}
			if ( minrect == -1 )  continue;
			rect2 = rect_pool->getrect(minrect);
			if ( rect1->y1 > rect2->y2 )  continue;
			if ( rect1->y2 < rect2->y1 )  continue;
			gid2 = rect2->attr;
			if ( gid2 < 0 ){
				if ( 0 > tlgroup->addgrp(gid1,minrect) )  return(-2);
				rect2->attr = gid1;
			}
			else if ( gid1 != gid2 ){
				for ( k=0 ; k<tlgroup->getidcounts(gid2) ; k++ ){
					rect_pool->getrect( tlgroup->getidlist(gid2)[k] )->attr = gid1;
				}
				if ( 0 > tlgroup->mergegrps(gid1,gid2) )  return(-3);
				tlgroup->deletegrp(gid2);
			}
		}

	}
	return(0);
}




/*----------------------------------------------
    Calculate Average Line Height
----------------------------------------------*/

static int get_avrlineheight(GRPLST *tlgroup,CRects *rectlist,int grpid,int clip){
	int	*idlst,nrect,i,n,hsum0,hsum,h;
	CRect	*rect;
	idlst = tlgroup->getidlist(grpid);
	nrect = tlgroup->getidcounts(grpid);
	hsum0 = 0;
	for ( i=0 ; i<nrect ; i++ ){
		rect = rectlist->getrect(idlst[i]);
		h = (int)(rect->y2 - rect->y1 +1);
		hsum0 += h;
	}
	if ( nrect )  hsum0 /= nrect;

	if ( ! clip )  return(hsum0);

	hsum = 0;
	for ( i=n=0 ; i<nrect ; i++ ){
		rect = rectlist->getrect(idlst[i]);
		h = (int)(rect->y2 - rect->y1 +1);
		if ( h > (C_clip * hsum0) ){
			hsum += h;
			n++;
		}
	}
	if ( n == 0 )  return(hsum0);
	return(hsum / n);
}




/*----------------------------------------------
    Calculate Merged Line Height
----------------------------------------------*/

static int get_mergedheight(GRPLST *tlgroup,CRects *rectlist,int ncolumns,\
		int cwid,int grpid1,int grpid2){
	int	*llist1,*llist2;
	int	*ulist1,*ulist2;
	int	*idlist;
	CRect	*rect;
	int	i,c,ac,d;
	int	l,u;
	if ( ! ncolumns )  return(-1);
	if ( 0 == (llist1 = new int[ncolumns * 4]) )  return(-1);
	llist2 = llist1 + ncolumns;
	ulist1 = llist2 + ncolumns;
	ulist2 = ulist1 + ncolumns;
	for ( i=0 ; i<ncolumns ; i++ ){
		llist1[i] = llist2[i] = MAXCONST;
		ulist1[i] = ulist2[i] = -1;
	}
	idlist = tlgroup->getidlist(grpid1);
	for ( i=0 ; i < tlgroup->getidcounts(grpid1) ; i++ ){
		rect = rectlist->getrect(idlist[i]);
		c = rect->x1 / cwid;
		if ( llist1[c] > rect->y1 )  llist1[c] = rect->y1;
		if ( ulist1[c] < rect->y2 )  ulist1[c] = rect->y2;
	}
	idlist = tlgroup->getidlist(grpid2);
	for ( i=0 ; i < tlgroup->getidcounts(grpid2) ; i++ ){
		rect = rectlist->getrect(idlist[i]);
		c = rect->x1 / cwid;
		if ( llist2[c] > rect->y1 )  llist2[c] = rect->y1;
		if ( ulist2[c] < rect->y2 )  ulist2[c] = rect->y2;
	}
	d = 0;
	for ( i=ac=0 ; i<ncolumns ; i++ ){
		if ( ulist1[i] == -1 )  continue;
		if ( ulist2[i] == -1 )  continue;
		l = llist1[i];  if ( l > llist2[i] )  l = llist2[i];
		u = ulist1[i];  if ( u < ulist2[i] )  u = ulist2[i];
		d += u - l;
		ac++;
	}
	if ( ac )  d /= ac;  else d = -1;
	delete []llist1;
	return(d);
}



/*------------------------------------------------------
    Combine elements of separating character
------------------------------------------------------*/

static int combine_sepchr(GRPLST *tlgroup,CRects *rectlist,int ncolumns,\
		int cwid,int ref_height){
	int	i,j,k,gid1,gid2;
	int	*tgid;
	int	ntext;
	int	dist;
	if ( ! tlgroup->groups )  return(0);
	if ( 0 == (tgid = new int[tlgroup->groups]) )  return(-1);
	ntext = 0;
	for ( gid1=0 ; gid1 < tlgroup->groups ; gid1++ ){
		if ( tlgroup->getidcounts(gid1) == 0 )  continue;
		if ( tlgroup->getidcounts(gid1) > 2 )  continue;
		if ( get_avrlineheight(tlgroup,rectlist,gid1,1) \
			> C_sepchr_height * ref_height )  continue;
		tgid[ntext++] = gid1;
	}
	for ( i=0 ; i<ntext ; i++ ){
		gid1 = tgid[i];
		for ( j=i+1 ; j<ntext ; j++ ){
			gid2 = tgid[j];
			dist = get_linedistance(tlgroup,rectlist,ncolumns,\
						cwid,gid1,gid2,NULL);
			if ( dist < 0 )  continue;
			if ( dist > C_sepchr_dist * ref_height )  continue;
			if ( get_mergedheight(tlgroup,rectlist,ncolumns,\
			  cwid,gid1,gid2) > C_sepchr_merged * ref_height )  continue;
			for ( k=0 ; k<tlgroup->getidcounts(gid1) ; k++ ){
				rectlist->getrect( tlgroup->getidlist(gid1)[k] )->attr = gid2;
			}
			if ( 0 > tlgroup->mergegrps(gid2,gid1) )  return(-1);
			tlgroup->deletegrp(gid1);
		}
	}
	delete []tgid;
	return(0);
}




/*------------------------------------------------------
    Combine underlines/overlines/ruby characters
------------------------------------------------------*/

static int combine(GRPLST *tlgroup,CRects *rectlist,int ncolumns,\
		int cwid,int ref_height,int ulflag){
	int	i,j,k,gid1,gid2,mingid;
	int	*avrheight;
	int	*rgid,*tgid;
	int	nruby,ntext;
	int	dist,mindist,mindist_w,w;
	int	ul;
	if ( ! tlgroup->groups )  return(0);
	if ( 0 == (avrheight = new int[tlgroup->groups * 3]) )  return(-1);
	rgid = avrheight + tlgroup->groups;
	tgid = rgid + tlgroup->groups;
	nruby = ntext = 0;
	for ( gid1=0 ; gid1 < tlgroup->groups ; gid1++ ){
		if ( tlgroup->getidcounts(gid1) == 0 )  continue;
		avrheight[gid1] = get_avrlineheight(tlgroup,rectlist,gid1,1);
		if ( avrheight[gid1] >= C_rheight * ref_height ){
			tgid[ntext++] = gid1;
		}
		else{	rgid[nruby++] = gid1;
		}
	}
	for ( i=0 ; i<nruby ; i++ ){
		gid1 = rgid[i];
		mindist = mindist_w = MAXCONST;
		mingid = -1;
		for ( j=0 ; j<ntext ; j++ ){
			gid2 = tgid[j];
			dist = get_linedistance(tlgroup,rectlist,ncolumns,\
						cwid,gid1,gid2,&ul);
			if ( ulflag == 0 )        w = 1;
			else if ( ul == ulflag )  w = 2;
			else                      w = 3;
			if ( dist >= 0 && mindist_w > w * dist ){
				mingid = gid2;
				mindist = dist;
				mindist_w = w * dist;
			}
		}
		if ( mingid != -1 ){
			if ( mindist <= C_dist * ref_height ){
				for ( k=0 ; k<tlgroup->getidcounts(gid1) ; k++ ){
					rectlist->getrect( tlgroup->getidlist(gid1)[k] )->attr = mingid;
				}
				if ( 0 > tlgroup->mergegrps(mingid,gid1) )  return(-1);
				tlgroup->deletegrp(gid1);
			}
		}
	}
	delete []avrheight;
	return(0);
}




/*----------------------------------------------
    Get reference height of rectangle
----------------------------------------------*/

static long get_ref_height(CRects *rectlist,int avr_height){
	int	rid;
	CRect	*rect;
	long	ref_height = 0;
	long	ref_counts = 0;
	int	height;
	for ( rid=0 ; rid < rectlist->counts ; rid++ ){
		rect = rectlist->getrect(rid);
		height = rect->y2 - rect->y1 +1;
		if ( height > (C_clip * avr_height) ){
			ref_height += (long)height;
			ref_counts++;
		}
	}
	if ( ref_counts )  ref_height /= ref_counts;
	return(ref_height);
}




/*----------------------------------------------
    Create Local Rectangles
----------------------------------------------*/

static int filter_proj(long *proj,int len,int th,int th_len){
	int	i,j,l;
	for ( i=l=0 ; i<len ; ){
		for ( ; i<len && (proj[i] == 0 || proj[i] > th) ; i++ ) ;
		for ( l = 0; i<len && proj[i] != 0 && proj[i] <= th ; i++ ) l++;
		if ( l >= th_len ){
			for ( j=i-l ; j < i ; j++ )  proj[j] = 0;
		}
		l = 0;
	}
	if ( l >= th_len ){
		for ( j=i-l ; j < i ; j++ )  proj[j] = 0;
	}
	return(0);
}




static int create_rects(SIPImage *image,int x,int awidth, \
		RUNLIST *runlist,int minpix){
	int	i,y,l,pixsum,height;
	long	*rden;
	SIPRectangle	cbox;
	cbox.x = x;
	cbox.y = 0;
	cbox.width = awidth;
	cbox.height = image->height;
	height = (int)image->height;
	if ( 0 == (rden = new long[height]) )  return(-1);
	sip_projprofile_area(image,rden,0,&cbox);
	if ( image->depth != 1 ){
		for ( y=0 ; y<height ; y++ ){
			rden[y] = awidth - rden[y]/255;
		}
	}
	filter_proj(rden,height,VLE_Width,VLE_Length);
	for ( y=i=0 ; y<height ; ){
		for ( l=0 ; rden[y] <= 0 && y<height ; y++ ) ;
		if ( y >= height )  break;
		runlist[i].pos = y;
		for ( l=pixsum=0 ; rden[y] > 0 && y<height ; y++ ){
			l++;
			pixsum += rden[y];
		}
		runlist[i].len = l;
		if ( pixsum >= minpix )  i++;
	}
	runlist[i].pos = -1;
	delete []rden;
	return(0);
}




/*----------------------------------------------
    Segment text lines
----------------------------------------------*/

int segment_lines(SIPImage *image, \
	CRects *rect_pool, GRPLST *tlgroup, \
	int mode_global, int mode_NRpixels, int mode_combine){
	int	i,k,x;
	int	width  = image->width;
	int	height = image->height;
	int	cwid, cnum;
	int	awidth;
	long	avr_height;
	long	ref_height;
	RUNLIST	*runlist;
	int	rect_par_column = RectParColumn;
	int	rects;
	int	crn;
	int	*cltbl;
	GRPLST	tlgroup_tmp;
	int	actgroups;
	int	*t_order;

/* ---- Create Rectangles ---- */

	if ( 0 == (runlist = new RUNLIST[height +1]) ){
		return(-1);
	}

	if ( mode_global ){
		rect_par_column = (width + RectWidth -1)/RectWidth;
	}

	for ( k=0 ; k<2 ; k++ ){
		cwid = rect_par_column * RectWidth;
		cnum = (width + cwid -1) / cwid;
		if ( 0 == (cltbl = new int[cnum +1]) ){
			delete []runlist;
			return(-1);
		}

		cnum = rects = 0;
		avr_height = 0;
		rect_pool->clear();
		for ( x=0 ; x<width ; x += cwid, cnum++ ){
			cltbl[cnum] = rects;
			awidth = width - x;
			awidth = awidth < cwid ? awidth : cwid;
			create_rects(image,x,awidth,runlist,mode_NRpixels);
			for ( i=0 ; runlist[i].pos >= 0 ; i++ ){
				if ( runlist[i].len > MAXSEGHEIGHT )  continue;
				crn = rect_pool->create(x,runlist[i].pos, \
					x + (awidth -1), runlist[i].pos + runlist[i].len -1);
				avr_height += (long)runlist[i].len;
				if ( 0 > crn ){
					delete []cltbl;
					delete []runlist;
					return(-1);
				}
				rect_pool->setattr(crn,-1);
					/* attribute = -1: not yet linked */

#if 0
				if ( sw_verbose ){
					printf("%3d : %03d %03d %03d\n", \
						rects,x,runlist[i].pos,runlist[i].len);
				}
#endif
				rects++;
			}
		}
		cltbl[cnum] = rects;

		if ( rects )  avr_height /= rects;
		ref_height = get_ref_height(rect_pool,avr_height);
		if ( rect_par_column == RectParColumn && k==0 ){
			if ( (ref_height / C_nrect) > RectParColumn ){
				rect_par_column = ref_height / C_nrect;
				delete []cltbl;
				continue;
			}
		}
		break;
	}

	delete []runlist;

//	if ( sw_verbose ) printf("rect_par_column = %d\n",rect_par_column);

/* ---- Link segments ---- */

	tlgroup_tmp.clearall();
	tlgroup_tmp.alloc(INITGROUPS);
	link_segs(width,height,rect_pool,cnum,cltbl,&tlgroup_tmp,ref_height);

	delete []cltbl;


/* ---- Combine underlines/overlines/ruby characters ---- */

	if ( mode_combine ){
		combine(&tlgroup_tmp,rect_pool,cnum,cwid,ref_height,1);
		combine_sepchr(&tlgroup_tmp,rect_pool,cnum,cwid,ref_height);
	}


/* ---- Sort and copy lines to the output buffer ---- */

	tlgroup->clearall();

	actgroups = 0;
	if ( tlgroup_tmp.groups > 0 ){
		if ( 0 == (t_order = new int[tlgroup_tmp.groups]) ){
			return(-1);
		}
		actgroups = sort_linedata(&tlgroup_tmp,rect_pool,cnum,cwid,t_order);

		for ( int k=0 ; k < actgroups ; k++ ){
			int gid2 = 0;
			int gid = t_order[k];
			CRect *rect;
			for ( i=0 ; i < tlgroup_tmp.getidcounts(gid) ; i++ ){
				int rid = tlgroup_tmp.getidlist(gid)[i];
				rect = rect_pool->getrect(rid);
				if ( i==0 ){
					if ( 0 > (gid2 = tlgroup->newgrp(rid)) ){
						delete []t_order;
						return(-1);
					}
					rect->attr = gid2;
					continue;
				}
				if ( 0 > tlgroup->addgrp(gid2, rid) ){
					delete []t_order;
					return(-1);
				}
				rect->attr = gid2;
			}
		}

		delete []t_order;
	}

	return(0);
}




/*----------------------------------------------
    Cut out a text line
----------------------------------------------*/

SIPImage * cutout_textline(SIPImage *image, \
	CRects *rect_pool, GRPLST *tlgroup, int tlid){
	SIPImage	*TLimage = 0;
	int	i, rid;
	CRect	*rect;
	CRect	TLBB = { 0,0,0,0 };
	SIPRectangle	srect, drect;

	if ( tlid > tlgroup->groups || tlid < 0 )  return(0);

	// get text line bounding box (TLBB)
	for ( i=0 ; i < tlgroup->getidcounts(tlid) ; i++ ){
		rid = tlgroup->getidlist(tlid)[i];
		rect = rect_pool->getrect(rid);
		if ( i==0 ){
			TLBB = *rect;
			continue;
		}
		if ( TLBB.x1 > rect->x1 )  TLBB.x1 = rect->x1;
		if ( TLBB.x2 < rect->x2 )  TLBB.x2 = rect->x2;
		if ( TLBB.y1 > rect->y1 )  TLBB.y1 = rect->y1;
		if ( TLBB.y2 < rect->y2 )  TLBB.y2 = rect->y2;
	}

//	printf("%d %d %d %d\n",TLBB.x1, TLBB.width(), TLBB.y1, TLBB.height());

	if ( 0 == (TLimage = sip_CreateImage(TLBB.width(), TLBB.height(), image->depth)) ){
		return(0);
	}

	if ( image->depth == 1 ){
		sip_ClearImage(TLimage,0);
	}
	else{	sip_ClearImage(TLimage,0xffffffff);
	}

	for ( i=0 ; i < tlgroup->getidcounts(tlid) ; i++ ){
		rid = tlgroup->getidlist(tlid)[i];
		rect = rect_pool->getrect(rid);
		srect.x = rect->x1;
		srect.y = rect->y1;
		srect.width  = rect->width();
		srect.height = rect->height();
		drect = srect;
		drect.x -= TLBB.x1;
		drect.y -= TLBB.y1;
		sip_CopyArea(image, &srect, TLimage, &drect);
	}

	return(TLimage);
}




