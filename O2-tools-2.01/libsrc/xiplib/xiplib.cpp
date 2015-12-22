/*--------------------------------------------------------------
	Extended Image Processing functions    v1.3
		Written  by H.Goto , Dec 1998
		Modified by H.Goto , Jun 1999
		Modified by H.Goto , May 2002
		Modified by H.Goto , Apr 2007
		Modified by H.Goto , Aug.2014
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1998-2014  Hideaki Goto

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
#include	<string.h>

#include	<xiplib.h>




/*------------------------------------------------------
	Load / Save Image File
------------------------------------------------------*/

int xip_LoadImage(char *fname,SIPImage **image,int depthconv, \
		int fgcol,int bgcol){
	PBMFILE	PBMLD;
	int	retcode,depth,x,y;
	SIPImage	*imgbuf;
	uchar	*lbuf,*p;
	*image = 0;	/* default value = NULL */
	PBMLD.setpal_fb((uchar)fgcol,(uchar)bgcol);
	retcode = PBMLD.open(fname,"r");
	if ( retcode )  return(retcode);
	switch ( PBMLD.getfiletype() ){
	  case 0:	depth = 1;  break;
	  case 1:	depth = 8;  break;
	  case 2:	depth = 32;  break;
			/* 24bit in file, but 32bit on memory */
	  default:	return(-5);
	}
	if ( depthconv == 24 )  depthconv = 32;
	switch ( depthconv ){
	  case 0:	depthconv = depth;  break;
	  case 1:	if ( depth > 1 )  return(-5);
			break;
	  case 8:	if ( depth > 8 )  return(-5);
			break;
	  case 32:	break;
	  default:	return(-5);
	}
	if ( 0 == (imgbuf = sip_CreateImage(PBMLD.getwidth(), \
		PBMLD.getheight(),depthconv)) )  return(-6);
	if ( 0 == (lbuf = new uchar [3 * PBMLD.getwidth() ]) )  return(-6);
	for ( y=0 ; y < PBMLD.getheight() ; y++ ){
		if ( depthconv == depth && depth < 32 ){
			if ( 0 != PBMLD.readline(-1,\
				(void *)sip_getimgptr(imgbuf,y)) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
		}
		else if ( depthconv == depth && depth == 32 ){
			if ( 0 != PBMLD.readline(-1,(void *)lbuf) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
			p = (uchar *)sip_getimgptr(imgbuf,y);
			for ( x=0 ; x < PBMLD.getwidth() ; x++ ){
				p[0] = lbuf[3*x  ];	/* R */
				p[1] = lbuf[3*x+1];	/* G */
				p[2] = lbuf[3*x+2];	/* B */
				p += 4;
			}
		}
		else if ( depthconv < 32 ){
			if ( 0 != PBMLD.readline_gray(-1,\
				(void *)sip_getimgptr(imgbuf,y)) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
		}
		else{	if ( 0 != PBMLD.readline_gray(-1,(void *)lbuf) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
			p = (uchar *)sip_getimgptr(imgbuf,y);
			for ( x=0 ; x < PBMLD.getwidth() ; x++ ){
				p[0] = lbuf[x];	/* R=Y */
				p[1] = lbuf[x];	/* G=Y */
				p[2] = lbuf[x];	/* B=Y */
				p += 4;
			}
		}
	}
	delete []lbuf;
	*image = imgbuf;
	return(0);
}


int xip_SaveImage(char *fname,SIPImage *image){
	int	x,y,retcode;
	uchar	*lbuf,*p;
	int	depth;
	PBMFILE	PBMSV;
	if ( image->depth != 32 )  depth = image->depth;  else  depth = 24;
	if ( PBMSV.setsize(image->width,image->height,depth) ){
		return(-5);	/* illegal size */
	}
	if ( 0 == (lbuf = new uchar [3 * image->width]) ){
		return(-6);	/* memory error */
	}
	retcode = PBMSV.open(fname,"w");
	if ( 0 > retcode )  return(retcode);
	retcode = 0;
	if ( image->depth == 1 || image->depth == 8 ){
		for ( y=0 ; y < image->height ; y++ ){
			if ( PBMSV.writeline(-1,(void *)sip_getimgptr(image,y)) ){
				retcode = -1;  break;
			}
		}
	}
	else{	for ( y=0 ; y < image->height ; y++ ){
			p = (uchar *)sip_getimgptr(image,y);
			for ( x=0 ; x<image->width ; x++ ){
				lbuf[3*x  ] = p[0];
				lbuf[3*x+1] = p[1];
				lbuf[3*x+2] = p[2];
				p += 4;
			}
			if ( PBMSV.writeline(-1,(void *)lbuf) ){
				retcode = -1;
				break;
			}
		}
	}
	delete []lbuf;
	if ( retcode )  PBMSV.close();  else  retcode = PBMSV.close();
	return(retcode);
}




/*------------------------------------------------------
	Merge two groups
------------------------------------------------------*/

static int merge_group(GRPLST *ccpool,CRects *runpool,int dstgrpid,int srcgrpid){
	int	i,n;
	CRect	*r;
	if ( 0 > (n = ccpool->getidcounts(srcgrpid)) )  return(-1);
	for ( i=0 ; i<n ; i++ ){
		r = runpool->getrect( ccpool->getidlist(srcgrpid)[i] );
		if ( r == 0 )  return(-1);
		r->link = dstgrpid;
	}
	return( ccpool->mergegrps(dstgrpid,srcgrpid) );
}




/*------------------------------------------------------
	Create Connected Components (multilevel)
------------------------------------------------------*/

int xip_CreateCC(SIPImage *image,GRPLST *ccpool,CRects *runpool,int mode){
	int	x,y,lx,xs,xe,eflag;
	int	width,height,nrun;
	int	*ccid,*ridlist;
	uchar	*lbuf,*lbuf2,*ul,*cl;
	uchar	d,*p;
	int	rid,gid1,gid2;
	CRect	*rect;
	if ( image->depth != 1 && image->depth != 8 )  return(-1);
	width = image->width;
	height = image->height;
	if ( 0 == (lbuf = new uchar[2 * width]) )  return(-1);
	lbuf2 = lbuf + width;
	if ( 0 == (ccid = new int[2 * width]) ){
		delete []lbuf;
		return(-1);
	}
	ridlist = ccid + width;
	p = (uchar *)sip_getimgptr(image,0);
	if ( image->depth == 1 ){
		sip_cvt1to8((char *)p,0,(char *)lbuf2,(ushort)width,1,0);
		p = lbuf2;
	}
	for ( x=0 ; x<width ; x++ ){
		ccid[x] = -1;
		lbuf[x] = ~p[x];
	}
	ul = lbuf;
	cl = p;
	eflag = 0;
	for ( y=0 ; y<height && !eflag ; y++ ){
		p = (uchar *)sip_getimgptr(image,y);
		if ( image->depth == 1 ){
			sip_cvt1to8((char *)p,0,(char *)cl,(ushort)width,1,0);
			p = cl;
		}
		for ( x=nrun=0 ; x<width ; x++ ){
			d = p[x];
			for ( lx=x+1 ; lx<width ; lx++ ){
				if ( d != p[lx] )  break;
			}
			xs = x;
			xe = x = lx -1;

			/* add new run to runpool */
			if ( 0 > (rid = runpool->create(xs,y,xe,y)) ){
				eflag = -1;
				break;
			}
			runpool->setattr(rid,(int)((uint)d));
			ridlist[nrun++] = rid;

			gid2 = -1;
			if ( mode ){	/* if 8-neighbor mode */
				if ( 0 > --xs )  xs = 0;
				if ( width <= ++xe )  xe = width -1;
			}
			for ( ; xs<=xe ; xs++ ){
				if ( d != ul[xs] )  continue;
				if ( gid2 == ccid[xs] )  continue;
				if ( gid2 == -1 ){
					gid2 = ccid[xs];
					if ( 0 > ccpool->addgrp(gid2,rid) ){
						eflag = -1;  break;
					}
					continue;
				}
				gid1 = ccid[xs];
				if ( 0 > merge_group(ccpool,runpool, \
					gid2,gid1) ){
					eflag = -1;  break;
				}
				for ( lx=xs ; lx<width ; lx++ ){
					if ( ccid[lx] == gid1 )  ccid[lx] = gid2;
				}
			}
			if ( eflag )  break;
			if ( gid2 == -1 ){
				if ( 0 > (gid2 = ccpool->newgrp(rid)) ){
					eflag = -1;  break;
				}
			}
			runpool->setlink(rid,gid2);
		}
		for ( rid=0 ; rid<nrun ; rid++ ){
			rect = runpool->getrect(ridlist[rid]);
			for ( x=rect->x1 ; x<=rect->x2 ; x++ )  ccid[x] = rect->link;
		}
		if ( image->depth == 1 ){
			p = ul;  ul = cl;  cl = p;	/* swap ul,cl */
		}
		else{	ul = p;
		}
	}
	delete []ccid;
	delete []lbuf;
	return(eflag);
}




/*------------------------------------------------------
	Create Connected Components (bilevel)
------------------------------------------------------*/

int xip_CreateCC2(SIPImage *image,GRPLST *ccpool,CRects *runpool,int mode){
	int	x,y,lx,xs,xe,eflag;
	int	width,height,nrun;
	int	*ccid,*ridlist;
	uchar	*lbuf,*lbuf2,*ul,*cl;
	uchar	d,*p;
	int	rid,gid1,gid2;
	CRect	*rect;
	if ( image->depth != 1 && image->depth != 8 )  return(-1);
	width = image->width;
	height = image->height;
	if ( 0 == (lbuf = new uchar[2 * width]) )  return(-1);
	lbuf2 = lbuf + width;
	if ( 0 == (ccid = new int[2 * width]) ){
		delete []lbuf;
		return(-1);
	}
	ridlist = ccid + width;
	p = (uchar *)sip_getimgptr(image,0);
	if ( image->depth == 1 ){
		sip_cvt1to8((char *)p,0,(char *)lbuf2,(ushort)width,1,0);
		p = lbuf2;
	}
	for ( x=0 ; x<width ; x++ ){
		ccid[x] = -1;
		lbuf[x] = ~p[x];
	}
	ul = lbuf;
	cl = p;
	eflag = 0;
	for ( y=0 ; y<height && !eflag ; y++ ){
		p = (uchar *)sip_getimgptr(image,y);
		if ( image->depth == 1 ){
			sip_cvt1to8((char *)p,0,(char *)cl,(ushort)width,1,0);
			p = cl;
		}
		for ( x=nrun=0 ; x<width ; x++ ){
			d = p[x];
			for ( lx=x+1 ; lx<width ; lx++ ){
				if ( d != p[lx] )  break;
			}
			xs = x;
			xe = x = lx -1;

			/* add new run to runpool */
			if ( 0 > (rid = runpool->create(xs,y,xe,y)) ){
				eflag = -1;
				break;
			}
			runpool->setattr(rid,(int)((uint)d));
			ridlist[nrun++] = rid;

			gid2 = -1;
			if ( (mode != 0) ^ (d == 0) ){	/* if 8-4 or 4-8 mode */
				if ( 0 > --xs )  xs = 0;
				if ( width <= ++xe )  xe = width -1;
			}
			for ( ; xs<=xe ; xs++ ){
				if ( d != ul[xs] )  continue;
				if ( gid2 == ccid[xs] )  continue;
				if ( gid2 == -1 ){
					gid2 = ccid[xs];
					if ( 0 > ccpool->addgrp(gid2,rid) ){
						eflag = -1;  break;
					}
					continue;
				}
				gid1 = ccid[xs];
				if ( 0 > merge_group(ccpool,runpool, \
					gid2,gid1) ){
					eflag = -1;  break;
				}
				for ( lx=xs ; lx<width ; lx++ ){
					if ( ccid[lx] == gid1 )  ccid[lx] = gid2;
				}
			}
			if ( eflag )  break;
			if ( gid2 == -1 ){
				if ( 0 > (gid2 = ccpool->newgrp(rid)) ){
					eflag = -1;  break;
				}
			}
			runpool->setlink(rid,gid2);
		}
		for ( rid=0 ; rid<nrun ; rid++ ){
			rect = runpool->getrect(ridlist[rid]);
			for ( x=rect->x1 ; x<=rect->x2 ; x++ )  ccid[x] = rect->link;
		}
		if ( image->depth == 1 ){
			p = ul;  ul = cl;  cl = p;	/* swap ul,cl */
		}
		else{	ul = p;
		}
	}
	delete []ccid;
	delete []lbuf;
	return(eflag);
}




/*------------------------------------------------------
	Create Connected Components (monolevel)
------------------------------------------------------*/

int xip_CreateCC1(SIPImage *image,GRPLST *ccpool,CRects *runpool,int mode){
	int	x,y,lx,xs,xe,eflag;
	int	width,height,nrun;
	int	*ccid,*ridlist;
	uchar	*lbuf,*lbuf2,*ul,*cl;
	uchar	d,*p;
	int	rid,gid1,gid2;
	CRect	*rect;
	if ( image->depth != 1 && image->depth != 8 )  return(-1);
	width = image->width;
	height = image->height;
	if ( 0 == (lbuf = new uchar[2 * width]) )  return(-1);
	lbuf2 = lbuf + width;
	if ( 0 == (ccid = new int[2 * width]) ){
		delete []lbuf;
		return(-1);
	}
	ridlist = ccid + width;
	p = (uchar *)sip_getimgptr(image,0);
	if ( image->depth == 1 ){
		sip_cvt1to8((char *)p,0,(char *)lbuf2,(ushort)width,1,0);
		p = lbuf2;
	}
	for ( x=0 ; x<width ; x++ ){
		ccid[x] = -1;
		lbuf[x] = ~p[x];
	}
	ul = lbuf;
	cl = p;
	eflag = 0;
	for ( y=0 ; y<height && !eflag ; y++ ){
		p = (uchar *)sip_getimgptr(image,y);
		if ( image->depth == 1 ){
			sip_cvt1to8((char *)p,0,(char *)cl,(ushort)width,1,0);
			p = cl;
		}
		for ( x=nrun=0 ; x<width ; x++ ){
			d = p[x];
			for ( lx=x+1 ; lx<width ; lx++ ){
				if ( d != p[lx] )  break;
			}
			xs = x;
			xe = x = lx -1;

			if ( d == 0 )  continue;

			/* add new run to runpool */
			if ( 0 > (rid = runpool->create(xs,y,xe,y)) ){
				eflag = -1;
				break;
			}
			runpool->setattr(rid,(int)((uint)d));
			ridlist[nrun++] = rid;

			gid2 = -1;
			if ( mode ){	/* if 8-neighbor mode */
				if ( 0 > --xs )  xs = 0;
				if ( width <= ++xe )  xe = width -1;
			}
			for ( ; xs<=xe ; xs++ ){
				if ( d != ul[xs] )  continue;
				if ( gid2 == ccid[xs] )  continue;
				if ( gid2 == -1 ){
					gid2 = ccid[xs];
					if ( 0 > ccpool->addgrp(gid2,rid) ){
						eflag = -1;  break;
					}
					continue;
				}
				gid1 = ccid[xs];
				if ( 0 > merge_group(ccpool,runpool, \
					gid2,gid1) ){
					eflag = -1;  break;
				}
				for ( lx=xs ; lx<width ; lx++ ){
					if ( ccid[lx] == gid1 )  ccid[lx] = gid2;
				}
			}
			if ( eflag )  break;
			if ( gid2 == -1 ){
				if ( 0 > (gid2 = ccpool->newgrp(rid)) ){
					eflag = -1;  break;
				}
			}
			runpool->setlink(rid,gid2);
		}
		for ( rid=0 ; rid<nrun ; rid++ ){
			rect = runpool->getrect(ridlist[rid]);
			for ( x=rect->x1 ; x<=rect->x2 ; x++ )  ccid[x] = rect->link;
		}
		if ( image->depth == 1 ){
			p = ul;  ul = cl;  cl = p;	/* swap ul,cl */
		}
		else{	ul = p;
		}
	}
	delete []ccid;
	delete []lbuf;
	return(eflag);
}




