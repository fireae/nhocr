/*--------------------------------------------------------------
  Make character dictionary    makedic  v1.1
  (C) 2005-2010  Hideaki Goto  (see accompanying LICENSE file)
    Written by H.Goto,  Dec. 2005
    Revised by H.Goto,  Feb. 2010
--------------------------------------------------------------*/

//#define		Use_DEfeature

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#include	"utypes.h"
#include	"ufilep.h"
#include	"siplib.h"

#include	"ocrbase.h"
#include	"feature_PLOVE.h"

#ifdef	Use_DEfeature
#include	"feature_DEF.h"
#endif

static int	sw_help = 0;
static int	sw_verbose = 0;
static int	sw_dim = FVECDIM_PLM;

static int	n_cat = 0;
static int	n_set = 1;




/*----------------------------------------------
  Save dictionary
----------------------------------------------*/

static int save_dic(RecBase *Rec, char *dicfile){
	FILE	*fp;
	int	cid;
	if ( NULL == (fp = fopen(dicfile,"wb")) ){
		fprintf(stderr,"Failed to open \"%s\" for output.\n",dicfile);
		return(-1);
	}
	for ( cid=0 ; cid<n_cat ; cid++ ){
		if ( Rec->dic[cid].write_vector(fp) ){
			fprintf(stderr,"Failed to write data to \"%s\".\n",dicfile);
			fclose(fp);
			return(3);
		}
	}
	if ( fclose(fp) ){
		fprintf(stderr,"Failed to write data to \"%s\".\n",dicfile);
		return(3);
	}
	return(0);
}




/*----------------------------------------------
  Load the first vector file
----------------------------------------------*/

static int load_vec(RecBase *Rec, char *vecfile, int dim){
	FILE	*fp;
	int	cid;
	struct stat	statinfo;
	if ( stat(vecfile,&statinfo) ){
		fprintf(stderr,"Failed to load vector file \"%s\".\n",vecfile);
		return(-1);
	}
	n_cat = statinfo.st_size / sizeof(double) / dim;
	if ( n_cat <= 0 ){
		fprintf(stderr,"Vector file \"%s\" is too short.\n",vecfile);
		return(-1);
	}
	if ( Rec->alloc(n_cat, dim, 1) ){
		fprintf(stderr,"Failed to load vector file \"%s\".\n",vecfile);
		return(-1);
	}
	if ( NULL == (fp = fopen(vecfile,"rb")) ){
		fprintf(stderr,"Failed to load vector file \"%s\".\n",vecfile);
		return(-1);
	}
	for ( cid=0 ; cid<n_cat ; cid++ ){
		if ( Rec->dic[cid].read_vector(fp) ){
			fprintf(stderr,"Failed to load vector file \"%s\".\n",vecfile);
			fclose(fp);
			return(-1);
		}
	}
	fclose(fp);
	if ( sw_verbose ){
		fprintf(stderr,"%d categories found.\n",n_cat);
	}
	return(0);
}




/*----------------------------------------------
  Add another vector file
----------------------------------------------*/

static int add_vec(RecBase *Rec, char *vecfile){
	FILE	*fp;
	int	cid;
	FeatureVector	vec(Rec->dic[0].dim);
	if ( NULL == (fp = fopen(vecfile,"rb")) ){
		fprintf(stderr,"Failed to load vector file \"%s\".\n",vecfile);
		return(-1);
	}
	for ( cid=0 ; cid<n_cat ; cid++ ){
//		if ( Rec->dic[cid].read_vector(fp) ){
		if ( vec.read_vector(fp) ){
			fprintf(stderr,"Failed to load vector file \"%s\".\n",vecfile);
			fclose(fp);
			return(-1);
		}
		Rec->dic[cid] += vec;
	}
	fclose(fp);
	n_set++;
	return(0);
}




/*----------------------------------------------
  Main routine
----------------------------------------------*/

int main(int ac, char **av){
	char	*outfile = NULL;
	char	*infile = NULL;
	int	i,k, cid;
	int	rcode;
	RecBase	Rec;
	int	sw_err = 0;

	for ( k=1 ; k<ac ; k++ ){
		if ( 0 == strcmp(av[k],"-h") ){  sw_help = 1;  continue; }
		if ( 0 == strcmp(av[k],"-v") ){  sw_verbose = 1;  continue; }
		if ( 0 == strcmp(av[k],"-dim") ){
			if ( ++k >= ac ){  sw_err = 1;  break; }
			sw_dim = atoi(av[k]);
			if ( sw_dim < 1 ){  sw_err = 1;  break; }
			continue;
		}
		if ( 0 == strcmp(av[k],"-F") ){
			if ( ++k >= ac ){  sw_err = 1;  break; }
#ifdef	Use_DEfeature
			if ( 0 == strcmp(av[k],"DEF") ){ sw_dim = FVECDIM_DEF; continue; }
			if ( 0 == strcmp(av[k],"DEFOL") ){ sw_dim = FVECDIM_DEFOL; continue; }
#endif
			if ( 0 == strcmp(av[k],"P-LOVE") ){ sw_dim = FVECDIM_PLOVE; continue; }
			if ( 0 == strcmp(av[k],"PLOVE") ){ sw_dim = FVECDIM_PLOVE; continue; }
			if ( 0 == strcmp(av[k],"P-LM") ){ sw_dim = FVECDIM_PLM; continue; }
			if ( 0 == strcmp(av[k],"PLM") ){ sw_dim = FVECDIM_PLM; continue; }
			sw_err = 1;  break;
		}
		if ( 0 == strcmp(av[k],"-o") ){
			if ( ++k >= ac ){  sw_err = 1;  break; }
			outfile = av[k];
			continue;
		}
		if ( infile == NULL ){  infile = av[k];  break; }
	}

	if ( sw_err || sw_help || outfile == NULL || infile == NULL ){
		fputs("Make character dictionary\n",stderr);
		fputs("makedic v1.1  Copyright (C) 2005-2010 Hideaki Goto\n",stderr);
		fputs("usage: makedic [options] -o out_dic_file  vec_file1 vec_file2 ...\n",stderr);
#ifdef	Use_DEfeature
		fputs("         -F type : feature type PLOVE/PLM(default)/DEF/DEFOL\n",stderr);
#else
		fputs("         -F type : feature type PLOVE/PLM(default)\n",stderr);
#endif
		fprintf(stderr,"         -dim N : dimension of feature vector (default:%d)\n",FVECDIM_PLM);
		fputs("         -v     : verbose mode\n",stderr);
		return(1);
	}

	rcode = load_vec(&Rec, infile, sw_dim);
	if ( rcode )  return(rcode);

	for ( k++ ; k<ac ; k++ ){
		rcode = add_vec(&Rec, av[k]);
		if ( rcode )  return(rcode);
	}

	if ( sw_verbose ){
		fprintf(stderr,"%d sets loaded.\n",n_set);
	}

	/* normalization */
	for ( cid=0 ; cid<Rec.n_cat ; cid++ ){
		for ( i=0 ; i<Rec.dic[cid].dim ; i++ ){
			Rec.dic[cid].e[i] /= (double)n_set;
		}
	}

	save_dic(&Rec, outfile);

	return(rcode);
}




