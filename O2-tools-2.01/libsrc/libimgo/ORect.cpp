/*--------------------------------------------------------------
	Image object library  libimgo ,    H.Goto Nov 1997
	  Class: ORects
		Last modified  Feb 2002
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1997-2002  Hideaki Goto

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

#include	"ORect.h"




/*------------------------------------------------------
	Oriented Rectangle Object
------------------------------------------------------*/


void ORect :: init(int x,int y,int w,int h,double a){
	stat  = 0;
	prev = next = ORSTAT_NonLink;
	link  = ORSTAT_NonLink;
	attr  = 0;
	attrd = 0;
	attrp = 0;
	ORect::x = x;	ORect::y = y;
	ORect::w = w;	ORect::h = h;
	ORect::a = a;
	/* Note: The optional members aren't initialized. */
}




/*------------------------------------------------------
	Rectangle Object List
------------------------------------------------------*/


ORects :: ORects(void){
	rectlist = 0;
	maxrects = counts = 0;
	inc_step = INCALLOC_STEP;
}


ORects :: ORects(int incstep){
	rectlist = 0;
	maxrects = counts = 0;
	inc_step = incstep;
}


ORects :: ~ORects(void){
	clear();
}


int ORects :: alloc(int size){
	if ( 0 == (rectlist = new ORect[size]) )  return(-1);
	maxrects = size;
	counts = 0;
	return(0);
}


int ORects :: realloc(int size){
	int	counts_save;
	ORect	*crtmp;
	if ( maxrects == 0 )  return ( alloc(size) );
	if ( size <= 0 )  size = 1;
	if ( 0 == (crtmp = new ORect[size]) )  return(-1);
	memcpy((char *)crtmp,(char *)rectlist,sizeof(ORect) * counts);
	counts_save = counts;
	clear();
	counts = counts_save;
	maxrects = size;
	rectlist = crtmp;
	return(0);
}


int ORects :: incalloc(){
	if ( realloc( maxrects + inc_step ) )  return(-1);
	return(0);
}


int ORects :: clear(void){
	if ( rectlist != 0 )  delete []rectlist;
	rectlist = 0;
	maxrects = counts = 0;
	return(0);
}


int ORects :: truncate(void){
	int	newcounts;
	int	rid;
	for ( rid=newcounts=0 ; rid<counts ; rid++ ){
		if ( isactive(rid) ){
			rectlist[newcounts++] = rectlist[rid];
		}
	}
	return( counts = newcounts );
}


int ORects :: create(int x,int y,int w,int h,double a){
	if ( counts >= maxrects ){
		if ( incalloc() )  return(-1);
	}
	rectlist[ counts ].init(x,y,w,h,a);
	rectlist[ counts ].enable();
	++counts;
	return ( counts -1 );
}


int ORects :: destroy(int rid){
	if ( rid >= counts )  return(-1);
	rectlist[rid].disable();
	rectlist[rid].attr  = 0;
	rectlist[rid].attrd = 0;
	rectlist[rid].attrp = 0;
	return(0);
}


int ORects :: set(int rid,int x,int y,int w,int h,double a){
	if ( rid >= counts )  return(-1);
	rectlist[rid].x = x;	rectlist[rid].y = y;
	rectlist[rid].w = w;	rectlist[rid].h = h;
	rectlist[rid].a = a;
	return(0);
}


int ORects :: setstat(int rid,int stat){
	if ( rid >= counts )  return(-1);
	rectlist[rid].setstat(stat);
	return(0);
}


int ORects :: getstat(int rid){
	if ( rid >= counts )  return(-1);
	return( rectlist[rid].getstat() );
}


int ORects :: setlink(int rid,int link){
	if ( rid >= counts )  return(-1);
	rectlist[rid].link = link;
	return(0);
}


int ORects :: getlink(int rid){
	if ( rid >= counts )  return(-1);
	return( rectlist[rid].link );
}


int ORects :: setattrp(int rid,void *attr){
	if ( rid >= counts )  return(-1);
	rectlist[rid].attrp = attr;
	return(0);
}


void * ORects :: getattrp(int rid){
	if ( rid >= counts )  return(0);
	return( rectlist[rid].attrp );
}


int ORects :: setattr(int rid,int attr){
	if ( rid >= counts )  return(-1);
	rectlist[rid].attr = attr;
	return(0);
}


int ORects :: getattr(int rid){
	if ( rid >= counts )  return(0);
	return( rectlist[rid].attr );
}


int ORects :: setattrd(int rid,double attr){
	if ( rid >= counts )  return(-1);
	rectlist[rid].attrd = attr;
	return(0);
}


double ORects :: getattrd(int rid){
	if ( rid >= counts )  return(0);
	return( rectlist[rid].attrd );
}


int ORects :: isactive(int rid){
	if ( rid >= counts )  return(0);
	return( rectlist[rid].isactive() );
}
