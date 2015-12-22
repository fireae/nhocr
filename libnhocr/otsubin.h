/*----------------------------------------------------------------------
    Global/Adaptive Image Binarization
      based on Discriminant Criterion (Otsu's method)  otsubin.h
    (C) 2000-2009  Hideaki Goto  (see accompanying LICENSE file)
        Written by H.Goto, Apr 2000
        Revised by H.Goto, Sep 2003
        Revised by H.Goto, Dec 2005
        Revised by H.Goto, May 2007
        Revised by H.Goto, Apr 2009
        Revised by H.Goto, May 2009
----------------------------------------------------------------------*/


#ifndef	_otsubin_h
#define	_otsubin_h

#ifdef	__cplusplus
extern "C" {
#endif

double	threshold_otsu(SIPImage *image,int x0,int y0,int w,int h);
int	adpt_binarize_otsu(SIPImage *src, SIPImage *dst, int csize, int interpolate);
int	binarize_otsu(SIPImage *src, SIPImage *dst, double *threshold);

#ifdef	__cplusplus
}
#endif

#endif	/* _otsubin_h */

