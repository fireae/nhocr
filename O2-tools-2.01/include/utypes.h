/*--------------------------------------------------------------
	User's type definitions     Rev.020527
		Written  by H.Goto , May 1996
		Modified by H.Goto , Sep 1997
		Modified by H.Goto , Apr 2000
		Modified by H.Goto , Jan 2001
		Modified by H.Goto , May 2002
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1996-2002  Hideaki Goto

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

#ifndef utypes_h
#define utypes_h

#if !defined(AIXV3) && !defined(AIXV4) && !defined(__alpha)
typedef unsigned char	uchar;
typedef unsigned long	ulong;
#endif

#if !defined(AIXV4) && !defined(__alpha)
typedef unsigned short	ushort;
typedef unsigned int	uint;
#endif

#if defined(__alpha) || defined(__ia64__) || (defined(__sgi) && (_MIPS_SZLONG == 64))
#if !defined(LONG64)
#define	LONG64
#endif
#endif

typedef unsigned int	uint32;
typedef          int	int32;

#endif	/* utypes_h */
