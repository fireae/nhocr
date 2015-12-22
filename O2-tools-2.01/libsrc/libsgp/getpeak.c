/*--------------------------------------------------------------
	Signal processing function library    libsgp
	  ( Get peaks in sequence  Rev.960229)
		Written by H.Goto , May 1994
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1994  Hideaki Goto

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


#include	<stdio.h>

#include	"sgplib.h"


/*------------------------------------------------------
	Get peaks
------------------------------------------------------*/

int sgp_getpeaksuc(uchar *src,peaklisti *pks, \
			int size,int listsize,int thlow,int thhigh){
	int	ix,l;
	int	dx,dx0,x0;
	int	count;
	count = 0;
	l = 0;
	x0 = dx0 = 0;
	for ( ix=0 ; ix<size ; ++ix ){
		dx = (int)((uint)src[ix]) - x0;
		if ( dx == 0 ){
			if ( dx0 > 0 )  ++l;  else  l = 0;
		}
		else{	if ( dx < 0 ){
				if ( l != 0 ){
					if ( (x0 >= thlow) && (x0 <= thhigh) ){
						if ( (++count) > listsize )  break;
						pks->pos = ix - ((l+1)/2) -1;
						pks->value = (int)((uint)x0);
						++pks;
					}
					l = 0;
				}
				else{	if ( dx0 > 0 ){
						if ( (x0 >= thlow) && (x0 <= thhigh) ){
							if ( (++count) > listsize )  break;
							pks->pos = ix -1;
							pks->value = (int)((uint)x0);
							++pks;
						}
					}
				}
			}
			else{	l = 0;
			}
			dx0 = dx;
		}
		x0 = (int)((uint)src[ix]);
	}
	return (count);
}




int sgp_getpeaksi(int *src,peaklisti *pks, \
			int size,int listsize,int thlow,int thhigh){
	int	ix,l;
	int	dx,dx0,x0;
	int	count;
	count = 0;
	l = 0;
	x0 = dx0 = 0;
	for ( ix=0 ; ix<size ; ++ix ){
		dx = src[ix] - x0;
		if ( dx == 0 ){
			if ( dx0 > 0 )  ++l;  else  l = 0;
		}
		else{	if ( dx < 0 ){
				if ( l != 0 ){
					if ( (x0 >= thlow) && (x0 <= thhigh) ){
						if ( (++count) > listsize )  break;
						pks->pos = ix - ((l+1)/2) -1;
						pks->value = x0;
						++pks;
					}
					l = 0;
				}
				else{	if ( dx0 > 0 ){
						if ( (x0 >= thlow) && (x0 <= thhigh) ){
							if ( (++count) > listsize )  break;
							pks->pos = ix -1;
							pks->value = x0;
							++pks;
						}
					}
				}
			}
			else{	l = 0;
			}
			dx0 = dx;
		}
		x0 = src[ix];
	}
	return (count);
}


