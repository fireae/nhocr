/*--------------------------------------------------------------
	Common Library for libufp
		Written by H.Goto , Jan. 1995
		Revised by H.Goto , Feb. 1996
		Revised by H.Goto , May  1996
		Revised by H.Goto , July 1996
		Revised by H.Goto , Feb. 1997
		Revised by H.Goto , Apr. 2000
		Revised by H.Goto , Apr. 2000
		Revised by H.Goto , Nov. 2008
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1995-2008  Hideaki Goto

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


#ifdef	__cplusplus
extern "C" {
#endif

#ifndef	comlib_h
#define	comlib_h

#include	<utypes.h>

// for endian check
static int	_endian1 = 1;
static char	*_endianLSB = (char*)&_endian1;
#define	ifLSB	if ( *_endianLSB == 1 )
#define	ifMSB	if ( *_endianLSB == 0 )

void		_uf_wordswap(ushort *d);
void		_uf_dwordswap(uint32 *d);
void		_uf_convert_1to8(uchar *src,uchar *dst,int size,uchar fg,uchar bg);
void		_uf_convert_8to1(uchar *src,uchar *dst,int size,uchar threshold);

#endif	/* comlib_h */

#ifdef	__cplusplus
}
#endif
