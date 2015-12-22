/*----------------------------------------------------------------------
    Character code table management function   codetable.cpp
        Written by H.Goto, Feb. 2008
        Revised by H.Goto, Feb. 2008
        Revised by H.Goto, Sep. 2008
        Revised by H.Goto, Jan. 2009
        Revised by H.Goto, May  2009
        Revised by H.Goto, July 2009
        Revised by H.Goto, Oct. 2009
        Revised by H.Goto, Dec. 2009
        Revised by H.Goto, Aug. 2014
----------------------------------------------------------------------*/

/*--------------
  Copyright 2008-2014  Hideaki Goto

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
--------------*/


//#define		BuiltInCCtable

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <math.h>

#include	"siplib.h"
#include	"ocrbase.h"
#include	"codelist.h"


static CharCode	cc_SPACE = {
	" ", " ",
	PosHint_None, SizeHint_None, WrtDir_H | WrtDir_V, 
	1, 'S', "-"
};


#ifdef	BuiltInCCtable
#include	"cctable.h"

static int load_codelist_builtin(CharCode **cclist){
	char	lbuf[CCLineLength+1];
	char	ccode[CCLineLength+1];
	char	csizestr[CCLineLength+1];
	char	charclass[CCLineLength+1];
	char	posH, posV;
	char	*csizep;
	int	ncode, n;
	CharCode	*cc;
	CharCode	*cclist0;

	ncode = cctable_size;

	if ( 0 == (cclist0 = new CharCode[ ncode>0 ? ncode:1 ]) ){
		return(-2);
	}

	for ( n=0 ; n<ncode ; n++ ){
		cc = &cclist0[n];
		strcpy(lbuf, cctable[n]);
		sscanf(lbuf,"%s %s %c%c %s",ccode,csizestr,&posH,&posV,charclass);
		if ( strlen(ccode) > CharCodeSize-1 ){
			delete []cclist0;
			return(n);
		}
		strcpy(cc->ccode, ccode);

		cc->sizehint = SizeHint_None;
		for ( csizep = csizestr ; *csizep ; csizep++ ){
			if ( *csizep == 'U' ){
				cc->sizehint = SizeHint_None;
			}
			else if ( *csizep == 'N' ){
				cc->sizehint |= SizeHint_Normal;
			}
			else if ( *csizep == 'S' ){
				cc->sizehint |= SizeHint_Small;
			}
			else if ( *csizep == 'T' ){
				cc->sizehint |= SizeHint_Tiny;
			}
			else{	delete []cclist0;
				return(n);
			}
		}

		cc->poshint = PosHint_None;
		if ( posH == 'L' ){
			cc->poshint = PosHint_Left;
		}
		else if ( posH == 'C' ){
			cc->poshint = PosHint_Center;
		}
		else if ( posH == 'R' ){
			cc->poshint = PosHint_Right;
		}
		else{	delete []cclist0;
			return(n);
		}
		if ( posV == 'T' ){
			cc->poshint |= PosHint_Top;
		}
		else if ( posV == 'M' ){
			cc->poshint |= PosHint_Middle;
		}
		else if ( posV == 'B' ){
			cc->poshint |= PosHint_Bottom;
		}
		else{	delete []cclist0;
			return(n);
		}

		cc->alphamode = 0;
		if ( strcmp(charclass,"-") == 0 ){
			cc->alphamode = 1;
		}

	}

	*cclist = cclist0;
	return(n);
}

#endif




#ifndef	BuiltInCCtable
static int mergeload_codelist(char *file, CharCode *cclist, \
	int pos, int listlen){
	FILE	*fp;
	char	lbuf[CCLineLength+1];
	char	ccode[CCLineLength+1];
	char	ccode_dic[CCLineLength+1];
	char	csizestr[CCLineLength+1];
	char	charclass[CCLineLength+1];
	char	fontname[CCLineLength+1];
	char	posH, posV, wdir;
	char	*csizep;
	int	ncode, n;
	CharCode	*cc;
	if ( NULL == (fp = fopen(file,"r")) )  return(-1);

	ncode = 0;
	while( fgets(lbuf,CCLineLength,fp) ){
		ncode++;
	}

	fseek(fp,0,SEEK_SET);

	for ( n=0 ; fgets(lbuf,CCLineLength,fp) && n<ncode ; n++ ){
		cc = &cclist[n+pos];
		charclass[0] = fontname[0] = '\0';
		sscanf(lbuf,"%s %s %s %c%c %c %s %s", \
			ccode,ccode_dic, \
			csizestr,&posH,&posV, \
			&wdir,charclass,fontname);
		if ( strlen(ccode) > CharCodeSize-1 ){
			fclose(fp);
			return(n+pos);
		}
		if ( strlen(ccode_dic) > CharCodeSize-1 ){
			fclose(fp);
			return(n+pos);
		}
		strcpy(cc->ccode, ccode);
		strcpy(cc->ccode_dic, ccode_dic);

		cc->sizehint = SizeHint_None;
		for ( csizep = csizestr ; *csizep ; csizep++ ){
			if ( *csizep == 'U' ){
				cc->sizehint = SizeHint_None;
			}
			else if ( *csizep == 'N' ){
				cc->sizehint |= SizeHint_Normal;
			}
			else if ( *csizep == 'S' ){
				cc->sizehint |= SizeHint_Small;
			}
			else if ( *csizep == 'T' ){
				cc->sizehint |= SizeHint_Tiny;
			}
			else if ( *csizep == 'A' ){
				cc->sizehint |= SizeHint_Ascender;
			}
			else if ( *csizep == 'D' ){
				cc->sizehint |= SizeHint_Descender;
			}
			else if ( *csizep == 'W' ){
				cc->sizehint |= SizeHint_Wide;
			}
			else{	fclose(fp);
				return(n+pos);
			}
		}

		cc->poshint = PosHint_None;
		if ( posH == 'L' ){
			cc->poshint = PosHint_Left;
		}
		else if ( posH == 'C' ){
			cc->poshint = PosHint_Center;
		}
		else if ( posH == 'R' ){
			cc->poshint = PosHint_Right;
		}
		else{	fclose(fp);
			return(n+pos);
		}
		if ( posV == 'T' ){
			cc->poshint |= PosHint_Top;
		}
		else if ( posV == 'M' ){
			cc->poshint |= PosHint_Middle;
		}
		else if ( posV == 'B' ){
			cc->poshint |= PosHint_Bottom;
		}
		else{	fclose(fp);
			return(n+pos);
		}

		cc->wdir = WrtDir_None;
		if ( wdir == 'H' ){
			cc->wdir = WrtDir_H;
		}
		else if ( wdir == 'V' ){
			cc->wdir = WrtDir_V;
		}
		else{	cc->wdir = WrtDir_H | WrtDir_V;
		}

		cc->alphamode = 0;
		cc->charclass = charclass[0];
		if ( strcmp(charclass,"A") == 0 ){
			cc->alphamode = 1;
		}
		else if ( strcmp(charclass,"L") == 0 ){
		}
		else if ( strcmp(charclass,"S") == 0 ){
			// narrow symbols
			if ( ! (cc->sizehint & SizeHint_Wide) ){
				cc->alphamode = 1;
			}
		}
		else{	fclose(fp);
			return(n+pos);
		}

		fontname[CCFontNameLen -1] = '\0';	// truncate
		strcpy(cc->fontname, fontname);

//		printf("%s\t%s\t\%c%c\n",ccode,csizestr,posH,posV);
	}

	fclose(fp);
	return(n+pos);
}
#endif	// BuiltInCCtable




int load_codelist(char *file, CharCode **cclist, int debug){

#ifdef	BuiltInCCtable
	return( load_codelist_builtin(cclist) );
#else

	FILE	*fp;
	char	*dcode;
	char	**codelist;
	char	lbuf[CCLineLength+1];
	int	ncode, n;
	int	files;
	CharCode	*cclist0;

	files = 1;
	for ( int i=0 ; file[i] != '\0' ; i++ ){
		if ( file[i] == ':' )  files++;
	}

	if ( 0 == (dcode = new char[strlen(file)+1]) )  return(-1);
	strcpy(dcode, file);
	if ( 0 == (codelist = new char*[files]) ){
		delete []dcode;
		return(-1);
	}

	files = 0;
	codelist[ files++ ] = dcode;
	for ( int i=0 ; dcode[i] != '\0' ; i++ ){
		if ( dcode[i] == ':' ){
			codelist[files++] = dcode + i+1;
			dcode[i] = 0;
		}
	}

	ncode = 0;
	for ( int i=0 ; i<files ; i++ ){
		if ( debug ){
			fprintf(stderr,"Checking file \"%s\".\n",codelist[i]);
		}
		if ( NULL == (fp = fopen(codelist[i],"r")) ){
			delete []dcode;
			delete []codelist;
			return(-1);
		}
		while( fgets(lbuf,CCLineLength,fp) ){
			ncode++;
		}
		fclose(fp);
	}

	ncode++;	// add a room for SPACE
	if ( 0 == (cclist0 = new CharCode[ ncode>0 ? ncode:1 ]) ){
		delete []dcode;
		delete []codelist;
		return(-2);
	}

	// insert SPACE
	cclist0[0] = cc_SPACE;

	n = 1;
	for ( int i=0 ; i<files ; i++ ){
		if ( debug ){
			fprintf(stderr,"Loading file \"%s\".\n",codelist[i]);
		}
		n = mergeload_codelist(codelist[i], cclist0, n, ncode);
		if ( n < 0 )  break;
	}

	*cclist = cclist0;
	delete []codelist;
	delete []dcode;
	return(n);

#endif	// BuiltInCCtable

}




int load_codelist_bydiccodes(char *dir, char *diccodes, \
	CharCode **cclist, int debug){
	FILE	*fp;
	char	*fname;
	char	*dcode;
	char	**codelist;
	char	lbuf[CCLineLength+1];
	int	ncode, n;
	int	ndiccode;
	CharCode	*cclist0;

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

	ncode = 0;
	for ( int i=0 ; i<ndiccode ; i++ ){
		if ( 8 < strlen(codelist[i]) ){
			delete []fname;
			delete []codelist;
			delete []dcode;
			return(-1);
		}
		strcpy(fname, dir);
		strcat(fname, "/cctable-");
		strcat(fname, codelist[i]);
		if ( debug ){
			fprintf(stderr,"Checking file \"%s\".\n",fname);
		}
		if ( NULL == (fp = fopen(fname,"r")) ){
			delete []fname;
			delete []codelist;
			delete []dcode;
			return(-1);
		}
		while( fgets(lbuf,CCLineLength,fp) ){
			ncode++;
		}
		fclose(fp);
	}

	ncode++;	// add a room for SPACE
	if ( 0 == (cclist0 = new CharCode[ ncode>0 ? ncode:1 ]) ){
		delete []fname;
		delete []codelist;
		delete []dcode;
		return(-2);
	}

	// insert SPACE
	cclist0[0] = cc_SPACE;

	n = 1;
	for ( int i=0 ; i<ndiccode ; i++ ){
		strcpy(fname, dir);
		strcat(fname, "/cctable-");
		strcat(fname, codelist[i]);
		if ( debug ){
			fprintf(stderr,"Loading file \"%s\".\n",fname);
		}
		n = mergeload_codelist(fname, cclist0, n, ncode);
		if ( n < 0 )  break;
	}

	*cclist = cclist0;
	delete []fname;
	delete []codelist;
	delete []dcode;
	return(n);
}


