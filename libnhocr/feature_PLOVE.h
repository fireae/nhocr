/*--------------------------------------------------------------
  Character feature extraction :
     P-LOVE: peripheral local outline vector
   & P-LM: peripheral local moment
  (C) 2005-2009  Hideaki Goto  (see accompanying LICENSE file)
	Written by  H.Goto  Dec 2005
	Revised by  H.Goto  May 2009
--------------------------------------------------------------*/

#ifndef	feature_PLOVE_h
#define	feature_PLOVE_h

#include	"ocrbase.h"

#define		FVECDIM_PLOVE	768
#define		FVECDIM_PLM	576


#ifdef	__cplusplus
extern "C"{
#endif

int	feature_PLOVE(SIPImage *cimage, FeatureVector *vec);
int	feature_PLM(SIPImage *cimage, FeatureVector *vec);

#ifdef	__cplusplus
 }
#endif

#endif	/* feature_PLOVE_h */
