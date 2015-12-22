/*--------------------------------------------------------------
	Image object library  libimgo ,    H.Goto Dec 1995
	  Class: CRects
		Last modified  Feb 2002
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1995-2002  Hideaki Goto

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


#define		INCALLOC_STEP	8


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"CRect.h"




/*------------------------------------------------------
	Rectangle Object
------------------------------------------------------*/


void CRect :: init(int x1,int y1,int x2,int y2){
	stat  = 0;
	link  = CRSTAT_NonLink;
	attr  = 0;
	attrp = 0;
	CRect::x1 = x1;	 CRect::y1 = y1;
	CRect::x2 = x2;	 CRect::y2 = y2;
}


void CRect :: set(int x1,int y1,int x2,int y2){
	CRect::x1 = x1;	 CRect::y1 = y1;
	CRect::x2 = x2;	 CRect::y2 = y2;
}




/*------------------------------------------------------
	Rectangle Object List
------------------------------------------------------*/


CRects :: CRects(void){
	crlist = 0;
	maxcrs = counts = 0;
	inc_step = INCALLOC_STEP;
}


CRects :: CRects(int incstep){
	crlist = 0;
	maxcrs = counts = 0;
	inc_step = incstep;
}


CRects :: ~CRects(void){
	clear();
}


CRect * CRects :: alloc(int size){
	int	i;
	if ( 0 == (crlist = new CRect[size]) )  return(0);
	maxcrs = size;
	counts = 0;
	for ( i=0 ; i<size ; i++ )  crlist[i].init();
//	crlist[0].init();
	return ( crlist );
}


CRect * CRects :: realloc(int size){
	int	i,counts_save;
	CRect	*crtmp;
	if ( maxcrs == 0 )  return ( alloc(size) );
	if ( size <= 0 )  size = 1;
	if ( 0 == (crtmp = new CRect[size]) )  return(0);
	for ( i=counts ; i<size ; i++ )  crtmp[i].init();
	memcpy((char *)crtmp,(char *)crlist,sizeof(CRect) * counts);
	counts_save = counts;
	clear();
	counts = counts_save;
	maxcrs = size;
	return( crlist = crtmp );
}


int CRects :: incalloc(){
	if ( 0 == realloc( maxcrs + inc_step ) )  return(-1);
	return(0);
}


int CRects :: clear(void){
	if ( crlist != 0 )  delete []crlist;
	crlist = 0;
	maxcrs = counts = 0;
	return(0);
}


int CRects :: truncate(void){
	int	newcounts;
	int	crn;
	for ( crn=newcounts=0 ; crn<counts ; crn++ ){
		if ( isactive(crn) ){
			crlist[newcounts++] = crlist[crn];
		}
	}
	return( counts = newcounts );
}


int CRects :: create(int x1,int y1,int x2,int y2){
	if ( counts >= maxcrs ){
		if ( incalloc() )  return(-1);
	}
	crlist[ counts ].init(x1,y1,x2,y2);
	crlist[ counts ].enable();
	++counts;
	return ( counts -1 );
}


int CRects :: destroy(int crn){
	if ( crn >= maxcrs )  return(-1);
	crlist[crn].disable();
	crlist[crn].attr  = 0;
	crlist[crn].attrp = 0;
	return(0);
}


int CRects :: findparent(int crn){
	while ( crlist[crn].link != CRSTAT_NonLink ){
		crn = crlist[crn].link;
		if ( ! crlist[crn].isactive() )  return(-1);
	}
	return(crn);
}


int CRects :: cat(int cr1,int cr2){
	if ( ! crlist[cr2].isactive() )  return(-1);
	if ( (cr1 = findparent(cr1)) < 0 )  return(-1);
	crlist[cr2].disable();
	crlist[cr2].link = cr1;
	if ( crlist[cr1].x1 > crlist[cr2].x1 )  crlist[cr1].x1 = crlist[cr2].x1;
	if ( crlist[cr1].y1 > crlist[cr2].y1 )  crlist[cr1].y1 = crlist[cr2].y1;
	if ( crlist[cr1].x2 < crlist[cr2].x2 )  crlist[cr1].x2 = crlist[cr2].x2;
	if ( crlist[cr1].y2 < crlist[cr2].y2 )  crlist[cr1].y2 = crlist[cr2].y2;
	return(cr1);
}


int CRects :: set(int crn,int x1,int y1,int x2,int y2){
	if ( crn >= maxcrs )  return(-1);
	crlist[crn].x1 = x1;	crlist[crn].y1 = y1;
	crlist[crn].x2 = x2;	crlist[crn].y2 = y2;
	return(0);
}


int CRects :: expand(int crn,int x1,int y1,int x2,int y2){
	if ( crn >= maxcrs )  return(-1);
	if ( crlist[crn].x1 > x1 )  crlist[crn].x1 = x1;
	if ( crlist[crn].y1 > y1 )  crlist[crn].y1 = y1;
	if ( crlist[crn].x2 < x2 )  crlist[crn].x2 = x2;
	if ( crlist[crn].y2 < y2 )  crlist[crn].y2 = y2;
	return(0);
}


int CRects :: setstat(int crn,int stat){
	if ( crn >= maxcrs )  return(-1);
	stat &= ~CRSTAT_Active;
	crlist[crn].stat = (crlist[crn].stat & CRSTAT_Active) | stat;
	return(0);
}


int CRects :: getstat(int crn){
	if ( crn >= maxcrs )  return(-1);
	return( crlist[crn].stat & ~CRSTAT_Active );
}


int CRects :: setlink(int crn,int link){
	if ( crn >= maxcrs )  return(-1);
	crlist[crn].link = link;
	return(0);
}


int CRects :: getlink(int crn){
	if ( crn >= maxcrs )  return(-1);
	return( crlist[crn].link );
}


int CRects :: setattrp(int crn,void *attr){
	if ( crn >= maxcrs )  return(-1);
	crlist[crn].attrp = attr;
	return(0);
}


void * CRects :: getattrp(int crn){
	if ( crn >= maxcrs )  return(0);
	return( crlist[crn].attrp );
}


int CRects :: setattr(int crn,int attr){
	if ( crn >= maxcrs )  return(-1);
	crlist[crn].attr = attr;
	return(0);
}


int CRects :: getattr(int crn){
	if ( crn >= maxcrs )  return(0);
	return( crlist[crn].attr );
}


int CRects :: isactive(int crn){
	if ( crn >= maxcrs )  return(0);
	return( crlist[crn].isactive() );
}


int CRects :: width(int crn){
	if ( crn >= maxcrs )  return(0);
	return( crlist[crn].width() );
}


int CRects :: height(int crn){
	if ( crn >= maxcrs )  return(0);
	return( crlist[crn].height() );
}


