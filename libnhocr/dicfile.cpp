/*--------------------------------------------------------------
  OCR base library: dictionary file I/O    dicfile
  (C) 2008-2013  Hideaki Goto  (see accompanying LICENSE file)
    Written by  H.Goto,  Feb. 2008
    Revised by  H.Goto,  May  2009
    Revised by  H.Goto,  Oct. 2009
    Revised by  H.Goto,  Aug. 2013
--------------------------------------------------------------*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#include	"utypes.h"
#include	"siplib.h"

#include	"ocrbase.h"


/*----------------------------------------------
  Load character vector dictionary
----------------------------------------------*/

static int ocrbase_mergedic(RecBase *Rec, char *dicfile, int dim, int pos, int n_cat){
	FILE	*fp;
	int	cid;
	struct stat	statinfo;
	int	n;
	if ( stat(dicfile,&statinfo) ){
		return(-1);
	}
	n = statinfo.st_size / sizeof(double) / dim;
	if ( n <= 0 ){
		return(-1);
	}
	if ( NULL == (fp = fopen(dicfile,"rb")) ){
		return(-1);
	}
	for ( cid=0 ; cid<n ; cid++ ){
		if ( Rec->dic[cid+pos].read_vector(fp) ){
			fclose(fp);
			return(-1);
		}
	}
	fclose(fp);
	return(n+pos);
}




int ocrbase_loaddic(RecBase *Rec, char *dicfile, int dim, int n_top, int debug){
	char	*dcode;
	char	**codelist;
	struct stat	statinfo;
	int	n, n_cat;
	int	ndiccode;

	ndiccode = 1;
	for ( int i=0 ; dicfile[i] != '\0' ; i++ ){
		if ( dicfile[i] == ':' )  ndiccode++;
	}

	if ( 0 == (dcode = new char[strlen(dicfile)+1]) )  return(-1);
	strcpy(dcode, dicfile);
	if ( 0 == (codelist = new char*[ndiccode]) ){
		delete []dcode;
		return(-1);
	}

	ndiccode = 0;
	codelist[ ndiccode++ ] = dcode;
	for ( int i=0 ; dcode[i] != '\0' ; i++ ){
		if ( dcode[i] == ':' ){
			codelist[ndiccode++] = dcode + i+1;
			dcode[i] = 0;
		}
	}

	n_cat = 0;
	for ( int i=0 ; i<ndiccode ; i++ ){
		if ( debug ){
			fprintf(stderr,"Checking file \"%s\".\n", codelist[i]);
		}
		if ( stat(codelist[i],&statinfo) ){
			delete []codelist;
			delete []dcode;
			return(-1);
		}
		n = statinfo.st_size / sizeof(double) / dim;
		if ( n <= 0 ){
			delete []codelist;
			delete []dcode;
			return(-1);
		}
		n_cat += n;
	}

	n_cat++;	// add a room for SPACE

	if ( Rec->alloc(n_cat, dim, n_top) ){
		delete []codelist;
		delete []dcode;
		return(-1);
	}

	// insert SPACE
	Rec->dic[0].zeroVector();

	n = 1;
	for ( int i=0 ; i<ndiccode ; i++ ){
		if ( debug ){
			fprintf(stderr,"Loading file \"%s\".\n", codelist[i]);
		}
		n = ocrbase_mergedic(Rec, codelist[i], dim, n, n_cat);
		if ( n < 0 )  break;
	}

	delete []codelist;
	delete []dcode;
	return(n);
}




int ocrbase_loaddic_bydiccodes(RecBase *Rec, char *dir, char *diccodes, int dim, int n_top, int debug){
	char	*fname;
	char	*dcode;
	char	**codelist;
	struct stat	statinfo;
	int	n, n_cat;
	int	ndiccode;
	int	err;

	ndiccode = 1;
	for ( int i=0 ; diccodes[i] != '\0' ; i++ ){
		if ( diccodes[i] == ':' )  ndiccode++;
	}

	if ( 0 == (dcode = new char[strlen(diccodes)+1]) )  return(-1);
	strcpy(dcode, diccodes);
	if ( 0 == (codelist = new char*[ndiccode]) ){
		delete []dcode;
		return(-1);
	}
	if ( 0 == (fname = new char[strlen(dir)+10+10]) ){
		delete []dcode;
		return(-1);
	}

	ndiccode = 0;
	codelist[ ndiccode++ ] = dcode;
	for ( int i=0 ; dcode[i] != '\0' ; i++ ){
		if ( dcode[i] == ':' ){
			codelist[ndiccode++] = dcode + i+1;
			dcode[i] = 0;
		}
	}

	n_cat = 0;
	err = 0;
	for ( int i=0 ; i<ndiccode ; i++ ){
		if ( 8 < strlen(codelist[i]) ){  err=1; break; }
		strcpy(fname, dir);
		strcat(fname, "/PLM-");
		strcat(fname, codelist[i]);
		strcat(fname, ".dic");
		if ( debug ){
			fprintf(stderr,"Checking file \"%s\".\n", fname);
		}
		if ( stat(fname,&statinfo) ){  err=1; break; }
		n = statinfo.st_size / sizeof(double) / dim;
		if ( n <= 0 ){  err=1; break; }
		n_cat += n;
	}
	if ( err ){
		delete []fname;
		delete []codelist;
		delete []dcode;
		return(-1);
	}

	n_cat++;	// add a room for SPACE

	if ( Rec->alloc(n_cat, dim, n_top) ){
		delete []fname;
		delete []codelist;
		delete []dcode;
		return(-1);
	}

	// insert SPACE
	Rec->dic[0].zeroVector();

	n = 1;
	for ( int i=0 ; i<ndiccode ; i++ ){
		strcpy(fname, dir);
		strcat(fname, "/PLM-");
		strcat(fname, codelist[i]);
		strcat(fname, ".dic");
		if ( debug ){
			fprintf(stderr,"Loading file \"%s\".\n", fname);
		}
		n = ocrbase_mergedic(Rec, fname, dim, n, n_cat);
		if ( n < 0 )  break;
	}

	delete []fname;
	delete []codelist;
	delete []dcode;
	return(n);
}





