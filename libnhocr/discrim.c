/*----------------------------------------------------------------------
    Discriminant Analysis for Image Binarization      discrim.c
    (Otsu's binarization method)
    (C) 2003-2009  Hideaki Goto  (see accompanying LICENSE file)
        Written by H.Goto, Sep  2003
        Revised by H.Goto, Oct  2004
        Revised by H.Goto, Dec  2005
        Revised by H.Goto, Apr  2009
        Revised by H.Goto, May  2009
----------------------------------------------------------------------*/


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<float.h>

#include	"discrim.h"


int bin_disc(int n, double *rvals, long *hist, DiscBinParams *params){
	long	ntot;
	long	*chist;
	double	*cmom;
	int	lv,i,j;
	double	r,h;
	double	om0,om1,M0,M1,mdiff;
	double	bvar, bvar_max;
	double	*rvals_t;
	long	*hist_t;

	if ( n <= 1 )  return(-1);
	if ( NULL == (chist = (long *)malloc(n * sizeof(long))) )  return(-2);
	if ( NULL == (cmom = (double *)malloc(n * sizeof(double))) ){
		free((void *)chist);
		return(-2);
	}
	if ( NULL == rvals ){
		hist_t = hist;
		rvals_t = rvals;
	}
	else{	if ( NULL == (hist_t = (long *)malloc(n * sizeof(long))) ){
			free((void *)cmom);
			free((void *)chist);
			return(-2);
		}
		if ( NULL == (rvals_t = (double *)malloc(n * sizeof(double))) ){
			free((void *)hist_t);
			free((void *)cmom);
			free((void *)chist);
			return(-2);
		}
		memcpy((void*)hist_t, (void*)hist, n*sizeof(long));
		memcpy((void*)rvals_t, (void*)rvals, n*sizeof(double));
		/* sort (not an efficient way) */
		for ( i=0 ; i<n-1 ; i++ ){
			for ( j=i+1 ; j<n ; j++ ){
				if ( rvals_t[i] > rvals_t[j] ){
					r = rvals_t[i];
					rvals_t[i] = rvals_t[j];
					rvals_t[j] = r;
					lv = hist_t[i];
					hist_t[i] = hist_t[j];
					hist_t[j] = lv;
				}
			}
		}
	}

	params->msqr = 0.0;
	for ( lv=0 ; lv<n ; lv++ ){
		if ( rvals )  r = rvals_t[lv];  else r = (double)lv;
		h = (double)hist_t[lv];
		params->msqr += r * r * h;
		if ( lv == 0 ){
			chist[lv] = hist_t[lv];
			cmom[lv] = r * (double)hist_t[lv];
		}
		else{	/* cumulative histogram */
			chist[lv] = hist_t[lv] + chist[lv -1];
			/* cumulative moment */
			cmom[lv] = r * (double)hist_t[lv] + cmom[lv -1];
		}
	}
	ntot = params->ntotal = chist[n -1];
	if ( ntot <= 0 )  ntot = 1;		/* for safety */
	params->mean = cmom[n -1] / (double)ntot;
	params->msqr /= (double)ntot;
	params->var = params->msqr - params->mean * params->mean;

	bvar_max = 0.0;
	params->seplevel = -1;
	params->threshold = 0.0;
	params->m0 = params->m1 = 0.0;
	for ( lv=0 ; lv < n -1 ; lv++ ){
		om0 = (double)chist[lv];
		om1 = (double)ntot - om0;
		if ( om0 == 0.0 || om1 == 0.0 )  continue;
		M0 = cmom[lv] / om0 ;
		M1 = (cmom[n -1] - cmom[lv]) / om1;
		mdiff = M0 - M1;
		bvar = om0 * om1 * mdiff * mdiff / ((double)ntot * (double)ntot);

		if ( bvar >= bvar_max ){
			bvar_max = bvar;
			params->m0 = M0;
			params->m1 = M1;
			params->seplevel = lv;
			if ( rvals ){
				params->threshold = rvals_t[lv];
			}
			else{	params->threshold = (double)lv;
			}
		}
	}

	if ( params->var != bvar_max ){
		params->varratio = bvar_max / (params->var - bvar_max);
	}
	else{	if ( bvar_max != 0.0 )  params->varratio = DBL_MAX;
		else  params->varratio = 0.0;
	}

	if ( rvals ){
		free((void *)rvals_t);
		free((void *)hist_t);
	}
	free((void *)cmom);
	free((void *)chist);
	return(0);
}


