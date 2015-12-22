/*----------------------------------------------------------------------
    Discriminant Analysis for Image Binarization      discrim.h
    (Otsu's binarization method)
    (C) 2003-2009  Hideaki Goto  (see accompanying LICENSE file)
        Written by H.Goto, Sep  2003
        Revised by H.Goto, Oct  2004
        Revised by H.Goto, Dec  2005
        Revised by H.Goto, Apr  2009
        Revised by H.Goto, May  2009
----------------------------------------------------------------------*/


#ifndef	_discrim_h
#define	_discrim_h

typedef struct {
	long	ntotal;		/* total # of data */
	double	mean;		/* total mean */
	double	msqr;		/* total square mean */
	double	var;		/* total variance */
	int	seplevel;	/* separation level */
	double	threshold;	/* threshold value */
	double	m0,m1;		/* mean values of class 0 and 1 */
	double	varratio;	/* variance ratio (=sigma_B^2 / sigma_W^2) */
} DiscBinParams;


#ifdef	__cplusplus
extern "C" {
#endif

int	bin_disc(int n, double *rvals, long *hist, DiscBinParams *params);

#ifdef	__cplusplus
}
#endif

#endif	/* _discrim_h */
