/*--------------------------------------------------------------
  Make feature vectors from character images    makevec   v1.2
  (C) 2005-2009  Hideaki Goto  (see accompanying LICENSE file)
    Written by H.Goto,  Dec. 2005
    Revised by H.Goto,  June 2009
    Revised by H.Goto,  July 2009
    Revised by H.Goto,  Oct. 2009
--------------------------------------------------------------*/

//#define		Use_DEfeature

#define		CHARSIZE	128
#define		CHARSIZE_Norm	64

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#include	"utypes.h"
#include	"ufilep.h"
#include	"siplib.h"

#include	"ocrbase.h"
#include	"feature_PLOVE.h"

static int	sw_help = 0;
static int	sw_verbose = 0;
static int	sw_feature = 1;


#ifdef	Use_DEfeature
#include	"feature_DEF.h"
static int	vecdim[] = { FVECDIM_PLOVE, FVECDIM_PLM, FVECDIM_DEF, FVECDIM_DEFOL };
static feature_fn_t	featureX[] = {\
		feature_PLOVE, feature_PLM, 
		feature_DEF, feature_DEFOL };
#else
static int	vecdim[] = { FVECDIM_PLOVE, FVECDIM_PLM };
static feature_fn_t	featureX[] = {\
		feature_PLOVE, feature_PLM, };
#endif	// Use_DEfeature




/*----------------------------------------------
  Read/write a character image
----------------------------------------------*/

static int write_char(FILE *fp, SIPImage *charimage){
	int	y;
	char	*lbuf;
	if ( 0 == (lbuf = new char[charimage->width]) )  return(-1);
	for ( y=0 ; y<charimage->height ; y++ ){
		sip_cvt8to1u((uchar *)charimage->pdata[y],(uchar *)lbuf,0,charimage->width,127);
		if ( 1 != fwrite((void *)lbuf, \
			(charimage->width+7)/8,1,fp) )  return(-1);
	}
	delete []lbuf;
	return(0);
}


static int read_char(FILE *fp, SIPImage *charimage){
	int	y;
	char	*lbuf;
	if ( 0 == (lbuf = new char[charimage->width]) )  return(-1);
	for ( y=0 ; y<charimage->height ; y++ ){
		if ( 1 != fread((void *)lbuf, \
			(charimage->width+7)/8,1,fp) )  return(-1);
		sip_cvt1to8(lbuf,0,(char *)charimage->pdata[y],charimage->width,255,0);
	}
	delete []lbuf;
	return(0);
}




/*----------------------------------------------
  Read character images and make feature vectors
----------------------------------------------*/

static int make_vectors(char *infile, char *outfile){
	int	rcode = 0;
	SIPImage	*srcchar;
	SIPImage	*dstchar;
	FILE	*fp;
	FILE	*fp_dst;
	int	n_char;
	OCRPrep	OCRPrep;
	FeatureVector	vec;
	if ( vec.alloc(vecdim[sw_feature]) ){
		fputs("Low memory.\n",stderr);
		return(2);
	}
	if ( 0 == (srcchar = sip_CreateImage(CHARSIZE,CHARSIZE,8)) ){
		fputs("Low memory.\n",stderr);
		return(2);
	}
	if ( 0 == (dstchar = sip_CreateImage(CHARSIZE_Norm,CHARSIZE_Norm,8)) ){
		sip_DestroyImage(srcchar);
		fputs("Low memory.\n",stderr);
		return(2);
	
	}

	if ( NULL == (fp_dst = fopen(outfile,"wb")) ){
		fprintf(stderr,"Failed to open \"%s\" for output.\n",outfile);
		sip_DestroyImage(dstchar);
		sip_DestroyImage(srcchar);
		return(3);
	}
	if ( NULL == (fp = fopen(infile,"rb")) ){
		fprintf(stderr,"Failed to open \"%s\".\n",infile);
		fclose(fp_dst);
		sip_DestroyImage(dstchar);
		sip_DestroyImage(srcchar);
		return(3);
	}

	n_char = 0;
	while( 0 == read_char(fp,srcchar) ){
		OCRPrep.normalize(srcchar,dstchar,2.0);
		featureX[sw_feature](dstchar, &vec);
		if ( vec.write_vector(fp_dst) ){
			fprintf(stderr,"Failed to write data to \"%s\".\n",outfile);
			rcode = 3;
			break;
		}
		n_char++;
	}
	if ( sw_verbose && rcode == 0 ){
		fprintf(stderr,"%d characters found.\n",n_char);
	}

	sip_DestroyImage(dstchar);
	sip_DestroyImage(srcchar);
	fclose(fp);
	if ( fclose(fp_dst) ){
		fprintf(stderr,"Failed to write data to \"%s\".\n",outfile);
		rcode = 3;
	}

	return(rcode);
}




/*----------------------------------------------
  Main routine
----------------------------------------------*/

int main(int ac, char **av){
	char	*infile = NULL;
	char	*outfile = NULL;
	int	k;
	int	rcode;
	int	sw_err = 0;

	for ( k=1 ; k<ac ; k++ ){
		if ( 0 == strcmp(av[k],"-h") ){  sw_help = 1;  continue; }
		if ( 0 == strcmp(av[k],"-v") ){  sw_verbose = 1;  continue; }
		if ( 0 == strcmp(av[k],"-F") ){
			if ( ++k >= ac ){  sw_err = 1;  break; }
			if ( 0 == strcmp(av[k],"P-LOVE") ){ sw_feature = 0; continue; }
			if ( 0 == strcmp(av[k],"PLOVE") ){ sw_feature = 0; continue; }
			if ( 0 == strcmp(av[k],"P-LM") ){ sw_feature = 1; continue; }
			if ( 0 == strcmp(av[k],"PLM") ){ sw_feature = 1; continue; }
#ifdef	Use_DEfeature
			if ( 0 == strcmp(av[k],"DEF") ){ sw_feature = 2; continue; }
			if ( 0 == strcmp(av[k],"DEFOL") ){ sw_feature = 3; continue; }
#endif
			sw_err = 1;  break;
		}
		if ( infile == NULL ){  infile = av[k]; }
		else{	outfile = av[k]; }
	}

	if ( sw_err || sw_help || infile == NULL || outfile == NULL ){
		fputs("Make feature vectors from character images\n",stderr);
		fputs("makevec v1.2  Copyright (C) 2005-2009 Hideaki Goto\n",stderr);
		fputs("usage: makevec [options] in_charfile out_vecfile\n",stderr);
#ifdef	Use_DEfeature
		fputs("         -F type : feature type PLOVE/PLM(default)/DEF/DEFOL\n",stderr);
#else
		fputs("         -F type : feature type PLOVE/PLM(default)\n",stderr);
#endif
		fputs("         -v      : verbose mode\n",stderr);
		return(1);
	}

	rcode = make_vectors(infile,outfile);

	return(rcode);
}


