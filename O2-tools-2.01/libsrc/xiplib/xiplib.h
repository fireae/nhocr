/*--------------------------------------------------------------
	Extended Image Processing functions    v1.1
		Written  by H.Goto , Dec 1998
		Modified by H.Goto , May 2002
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1998-2002  Hideaki Goto

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


#ifndef xiplib_h
#define xiplib_h

#include	<utypes.h>	/* define unsigned variables */

#include	<siplib.h>
#include	<imgobj.h>
#include	<objgrp.h>
#include	<ufilep.h>

#ifdef	__cplusplus
extern "C" {
#endif


/*------------------------------------------------------
	Load / Save Image File
		depthconv = 0: use file type
			  = 8: convert image to 8bit
------------------------------------------------------*/

extern	int	xip_LoadImage(char *fname,SIPImage **image,int depthconv, \
		int fgcol,int bgcol);
extern	int	xip_SaveImage(char *fname,SIPImage *image);


/*------------------------------------------------------
	Create Connected Components (multilevel)

		mode = 0: 4-connection mode
		     = 1: 8-connection mode
------------------------------------------------------*/

/*
  Note: The function xip_CreateCC() does not create 4-connected
  components and 8-connected components simultaneously.  As well
  known, 4-connected components for foreground are not complements
  of 4-connected components for background topologically.
  8-connected components for foreground are not complements of
  8-connected components for background as well.

  Users are encouraged to use another function, xip_CreateCC2(),
  if topology analysis is required in bilevel images.
*/

extern	int		xip_CreateCC(SIPImage *image,GRPLST *ccpool,\
				CRects *runpool,int mode);


/*------------------------------------------------------
	Create Connected Components (bilevel)

		mode = 0: 8-connection for background
			  4-connection for foreground
		     = 1: 4-connection for background
			  8-connection for foreground
------------------------------------------------------*/

/*
  Note: The function xip_CreateCC2() creates 8-connected
  components and 4-connected components simultaneously. Mode for 
  foreground and background images can be swapped by setting
  mode=1. The function xip_CreateCC2() assumes that the input
  image is bilevel.
*/

extern	int		xip_CreateCC2(SIPImage *image,GRPLST *ccpool,\
				CRects *runpool,int mode);


/*------------------------------------------------------
	Create Connected Components (monolevel)

		mode = 0: 4-connection for foreground
		     = 1: 8-connection for foreground
------------------------------------------------------*/

/*
  Note: The function xip_CreateCC1() creates only connected
  components for foreground image. Background image is ignored.
  This function is far faster and more convenient than
  xip_CreateCC2() when we use only connected components for
  foreground image.
*/

extern	int		xip_CreateCC1(SIPImage *image,GRPLST *ccpool,\
				CRects *runpool,int mode);



#ifdef	__cplusplus
}
#endif

#endif	/* xiplib_h */
