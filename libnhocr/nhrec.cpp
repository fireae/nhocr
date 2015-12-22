/*----------------------------------------------------------------------
    Character / Text line recognition library   nhrec.cpp
        Written by H.Goto, Jan. 2009
        Revised by H.Goto, Jan. 2009
        Revised by H.Goto, May  2009
        Revised by H.Goto, July 2009
        Revised by H.Goto, Oct. 2009
        Revised by H.Goto, Feb. 2013 (gramd support)
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


#define		N_Top		10

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <math.h>

#include	"utypes.h"

#include	"ufilep.h"
#include	"siplib.h"
#include	"imgobj.h"
#include	"objgrp.h"

#include	"discrim.h"
#include	"otsubin.h"

#include	"nhocr.h"
#include	"feature_PLOVE.h"

#include	"segchar_adhoc.h"	// FIXME


// default file/path names
#ifdef DICDIR
static const char	*nhocrlibdir0 = DICDIR;
#else
static const char	*nhocrlibdir0 = "/opt/nhocr/share";
#endif
static const char	*diccodes0 = "ascii+:jpn";

static const char	*gramdportfile0 = "/tmp/gramd.port";

#define	debugprintf	if(NHrec::debug)printf




/*----------------------------------------------
    Constructor / Destructor
----------------------------------------------*/

NHrec :: NHrec(){
	n_cat = 0;
	n_top = N_Top;
	force_alpha = 0;
	cclist = 0;
	nhocrlibdir = (char *)nhocrlibdir0;
	nhocrdiccodes = (char *)diccodes0;
	gramdportfile = (char *)gramdportfile0;
	gramd_enabled = 0;
	cctablefile = 0;	// default to using diccodes
	dicfile = 0;		// default to using diccodes
	debug = 0;
#ifdef HAVE_LIBGRAMDCLIENT
	gramd_handle = 0;
#endif
}


NHrec :: ~NHrec(){
	close();
}




/*----------------------------------------------
    Set library directory / dictionary file
----------------------------------------------*/

int NHrec :: setlibdir(char *path){
	nhocrlibdir = path;
	return(0);
}


int NHrec :: setgramdportfile(char *path){
	gramdportfile = path;
	return(0);
}


int NHrec :: setdiccodes(char *diccodes){
	nhocrdiccodes = diccodes;
	return(0);
}


int NHrec :: setdicfile(char *dicname, char *cctablename){
	dicfile = dicname;
	cctablefile = cctablename;
	return(0);
}




/*----------------------------------------------
    Automatic image inverter
----------------------------------------------*/

int NHrec :: auto_invert(SIPImage *image){
	uchar	*p;
	int	x,y,fgcount = 0;
	int	width  = image->width;
	int	height = image->height;
	for ( y=0 ; y<height ; y++ ){
		p = (uchar *)image->pdata[y];
		for ( x=0 ; x<width ; x++ ){
			if ( p[x] < 128 )  fgcount++;
		}
	}
	if ( (double)fgcount / (double)(width * height) < .5 )  return(0);

	// invert image
	for ( y=0 ; y<height ; y++ ){
		p = (uchar *)image->pdata[y];
		for ( x=0 ; x<width ; x++ ){
			p[x] = 0xff - p[x];
		}
	}
	return(0);
}




/*----------------------------------------------
    Adaptive binarization
----------------------------------------------*/

int NHrec :: binarize_otsu(SIPImage *src, SIPImage *dst, \
	int csize, int interpolate){
	return( adpt_binarize_otsu(src,dst,csize,interpolate) );
}




/*----------------------------------------------
    Recognize text line image
----------------------------------------------*/

int NHrec :: open(){
	int	n;
	char	*fname;

	// allocate a string buf long enough
	n = 0;
	if ( cctablefile ){
		if ( (size_t)n < strlen(cctablefile) ){
			n = strlen(cctablefile); }
	}
	if ( dicfile ){
		if ( (size_t)n < strlen(dicfile) ){
			n = strlen(dicfile); }
	}
	n += strlen(nhocrlibdir) + 2;
	if ( 0 == (fname = new char[n]) ){
		fprintf(stderr,"libnhocr: Failed to access to library directory %s.\n",nhocrlibdir);
		return(1);
	}

	if ( cctablefile ){
		strcpy(fname,nhocrlibdir);
		strcat(fname,"/");
		strcat(fname,cctablefile);
		n_cat = load_codelist(fname,&cclist,debug);
	}
	else{	// by diccodes
		n_cat = load_codelist_bydiccodes(nhocrlibdir,nhocrdiccodes,&cclist,debug);
	}
	if ( n_cat < 1 ){
		fprintf(stderr,"libnhocr: Failed to load Character code table.\n");
		fprintf(stderr,"libnhocr: Check NHOCR_DICDIR environment variable.\n");
		if ( cclist ){ delete []cclist;  cclist = 0; }
		delete []fname;
		return(1);
	}

#if 0
	CharCode	*cc;
	debugprintf("libnhocr: Found %d character codes.\n",n_cat);
	for ( n=0 ; n<n_cat ; n++ ){
		cc = &cclist[n];
		debugprintf("libnhocr: %d:\t%s\t%X\t\%04X\n", \
			n,cc->ccode,cc->poshint,cc->sizehint);
	}
#endif

	if ( dicfile ){
		strcpy(fname,nhocrlibdir);
		strcat(fname,"/");
		strcat(fname,dicfile);
		n = ocrbase_loaddic(&Rec, fname, \
			FVECDIM_PLM, n_top, debug);
	}
	else{	// by diccodes
		n = ocrbase_loaddic_bydiccodes(&Rec, \
			nhocrlibdir, nhocrdiccodes, \
			FVECDIM_PLM, n_top, debug);
	}
	if ( n<1 ){
		fprintf(stderr,"libnhocr: Failed to load character dictionary.\n");
		fprintf(stderr,"libnhocr: Check NHOCR_DICDIR environment variable.\n");
		if ( cclist ){ delete []cclist;  cclist = 0; }
		delete []fname;
		return(1);
	}
//	printf("Found %d character vectors.\n",n);
	if ( n_cat != n ){
		fprintf(stderr,"libnhocr: Mismatch in the number of characters (%d vs %d).\n",n_cat,n);
		if ( cclist ){ delete []cclist;  cclist = 0; }
		delete []fname;
		return(1);
	}

	// copy the character attributes to the feature vector class
	for ( int i=0 ; i<n ; i++ ){
		Rec.dic[i].gHint = cclist[i].poshint | cclist[i].sizehint | cclist[i].wdir;
	}

#ifdef HAVE_LIBGRAMDCLIENT
	gramd_handle = 0;
	if (gramd_enabled) {
		gramdDebugMode(0);
		if (!(gramd_handle = gramdOpenWithFile(gramdportfile, "127.0.0.1"))) {
			fprintf(stderr,"libnhocr: Failed to connect to gramd via libgramd-client.\nIs the daemon running?\n");
			if ( cclist ){ delete []cclist;  cclist = 0; }
			delete []fname;
			return(1);
		}
	}
#endif

	delete []fname;
	return(0);
}




int NHrec :: close(){
	Rec.dealloc();
	if ( cclist ){ delete []cclist;  cclist = 0; }
#ifdef HAVE_LIBGRAMDCLIENT
	if (gramd_handle) {
		gramdClose(gramd_handle);
		gramd_handle = 0;
	}
#endif
	return(0);
}




int NHrec :: rec_addstr(char *line, const char *str, int bufsize){
	if ( strlen(line) + strlen(str) >= (size_t)bufsize )  return(-1);
	strcat(line,str);
	return( strlen(line) );
}




int NHrec :: rec_gramd_add(RecResultItem *list) {
	if (!gramd_enabled)
		return 0;
#ifdef HAVE_LIBGRAMDCLIENT
	gramdAddSection(gramd_handle);
	int n;
	if (!list) {
		// TODO: No support for fake spaces, yet.
		// gramdAddCandidate(gramd_handle, " ", 1.0);
		return 1;
	}
	double dist_sum = 0.0;
	double boost_sum = 0.0;
	for ( n=0 ; n<n_top ; n++ ){
		if ( Rec.resultTable[n].id >= n_cat \
		  || Rec.resultTable[n].id < 0 ){
			break;
		}
		// dist_sum += pow(1000.0, -Rec.resultTable[n].dist);
		boost_sum += pow(1.5, -(n + 1));
	}
	for ( n=0 ; n<n_top ; n++ ){
		if ( Rec.resultTable[n].id >= n_cat \
		  || Rec.resultTable[n].id < 0 ){
			break;
		}
		// Converting distance to probability.
		double dist_prob = pow(1000.0, -Rec.resultTable[n].dist) / dist_sum;
		double boost_prob = pow(1.5, -(n + 1)) / boost_sum;
		double prob = boost_prob;//0.5 * dist_prob + 0.5 * boost_prob;
		// fprintf(stderr, "Adding candidate %s\t\tP=%f\n", cclist[Rec.resultTable[n].id].ccode, prob);
		gramdAddCandidate(gramd_handle, cclist[Rec.resultTable[n].id].ccode, prob);
	}
	// fprintf(stderr, "\n");
	return 1;
#else
	fprintf(stderr,"libnhocr: Not compiled with libgramd-client library. Language post-processing is unavailable.");
	return(0);
#endif
}




int NHrec :: rec_gramd_solve(char *buffer, size_t size) {
	if (!gramd_enabled)
		return 0;
#ifdef HAVE_LIBGRAMDCLIENT
	return gramdQuery(gramd_handle, buffer, size, 10000 /* timeout */);
#else
	return(0);
#endif
}




SIPImage * NHrec :: rotate90(SIPImage *image, int angle){
	// rotate image by 90deg clockwise
	int	x, y;
	SIPImage	*img_r;
	uchar	*p;
	if ( 0 == (img_r = sip_CreateImage(image->height, image->width, 8)) )  return(0);

	for ( y=0 ; y < img_r->height ; y++ ){
		p = (uchar*)img_r->pdata[y];
		if ( angle == -90 ){
			for ( x=0 ; x < img_r->width ; x++ ){
				p[x] = ((uchar *)image->pdata[image->height - x -1])[y];
			}
		}
		else if ( angle == 90 ){
			for ( x=0 ; x < img_r->width ; x++ ){
				p[x] = ((uchar *)image->pdata[x])[image->width - y -1];

			}
		}
	}
	return(img_r);
}




int NHrec :: rec_character(SIPImage *image, CharBox *cb, int wdir, \
		RecResultItem *resultTable){
	uchar	*p;
	int	cbw,cbh;
	int	n=0, cid;
	int	phmask;
	OCRPrep	OCRPrep;
	FeatureVector	vec(FVECDIM_PLM);
	SIPImage	*cimage;
	SIPImage	*cnorm;
	SIPRectangle	srect,drect;
	drect.x = drect.y = 0;

	if ( 0 == (cnorm = sip_CreateImage(64,64,8)) )  return(-1);

	cbw = cb->width();
	cbh = cb->height();
	srect.x = cb->xs;
	srect.y = cb->ys;
	srect.width = drect.width = cbw;
	srect.height = drect.height = cbh;
	cimage = sip_CreateImage(cbw,cbh,8);
	if ( cimage == 0 )  return(-2);
	sip_CopyArea(image,&srect,cimage,&drect);
	for ( int y=0 ; y<cbh ; y++ ){
		p = (uchar *)cimage->pdata[y];
		for ( int x=0 ; x<cbw ; x++ ){
			p[x] = ~p[x];
		}
	}
	OCRPrep.normalize(cimage,cnorm,2.0);
	sip_DestroyImage(cimage);

	phmask = PosHint_Top | PosHint_Middle | PosHint_Bottom;
	if ( wdir & WrtDir_V ){
		phmask = PosHint_Left | PosHint_Center | PosHint_Right;

		SIPImage *img_r;
		img_r = rotate90(cnorm, -90);
		if ( 0 == img_r )  return(-1);
		sip_DestroyImage(cnorm);
		cnorm = img_r;
	}

	feature_PLM(cnorm, &vec);
	Rec.recognizeEuclidean(vec,wdir);

#if 0
	printf("--------\n");
	for ( cid = -1, n=0 ; n<n_top ; n++ ){
		if ( Rec.resultTable[n].id >= n_cat \
		  || Rec.resultTable[n].id < 0 ){
			break;
		}
		printf("%s\t%04x %04x %04x %04x\n", \
			cclist[Rec.resultTable[n].id].ccode,
			cclist[Rec.resultTable[n].id].poshint,cb->poshint,\
			cclist[Rec.resultTable[n].id].sizehint,cb->sizehint);
	}
#endif

	for ( cid = -1, n=0 ; n<n_top ; n++ ){
		if ( Rec.resultTable[n].id >= n_cat \
		  || Rec.resultTable[n].id < 0 ){
			break;
		}
		if ( cb->alphamode ){
			if ( ! cclist[Rec.resultTable[n].id].alphamode ){
				continue;
			}
		}
		if ( cid < 0 )  cid = n;

		if ( (phmask & cclist[Rec.resultTable[n].id].poshint & cb->poshint) == 0 ){
			continue;
		}

		if ( (cclist[Rec.resultTable[n].id].sizehint & SizeHint_Mask) == SizeHint_None ){
			cid = n;  break;
		}
		if ( ((cclist[Rec.resultTable[n].id].sizehint & SizeHint_Mask) & cb->sizehint) != 0 ){
			cid = n;  break;
		}
	}

	if ( resultTable != 0 ){
		memcpy((void *)resultTable, \
			(void *)(Rec.resultTable), \
			sizeof(RecResultItem) * n_top);
	}

	sip_DestroyImage(cnorm);
	return(cid);
}




int NHrec :: rec_character(SIPImage *image, \
		int x0, int y0, int width, int height, int wdir, \
		RecResultItem *resultTable){
	uchar	*p;
	OCRPrep	OCRPrep;
	FeatureVector	vec(FVECDIM_PLM);
	SIPImage	*cimage;
	SIPImage	*cnorm;
	SIPRectangle	srect,drect;
	drect.x = drect.y = 0;

	if ( 0 == (cnorm = sip_CreateImage(64,64,8)) )  return(-1);

	srect.x = x0;
	srect.y = y0;
	srect.width = drect.width = width;
	srect.height = drect.height = height;
	cimage = sip_CreateImage(width,height,8);
	if ( cimage == 0 )  return(-2);
	sip_CopyArea(image,&srect,cimage,&drect);
	for ( int y=0 ; y<height ; y++ ){
		p = (uchar *)cimage->pdata[y];
		for ( int x=0 ; x<width ; x++ ){
			p[x] = ~p[x];
		}
	}
	OCRPrep.normalize(cimage,cnorm,2.0);
	sip_DestroyImage(cimage);

	if ( wdir & WrtDir_V ){
		SIPImage *img_r;
		img_r = rotate90(cnorm, -90);
		if ( 0 == img_r )  return(-1);
		sip_DestroyImage(cnorm);
		cnorm = img_r;
	}

	feature_PLM(cnorm, &vec);
	Rec.recognizeEuclidean(vec,wdir);
	sip_DestroyImage(cnorm);

	memcpy((void *)resultTable, (void *)(Rec.resultTable), \
		sizeof(RecResultItem) * n_top);

	// Return -1 if there is no matching character.
	return( Rec.resultTable[0].id );
}




int NHrec :: rec_line(SIPImage *image, char *resultline, \
		int bufsize, int wdir){
	int	cid;
	CharBox	*cba = 0;
	CharBox	*cba_raw;
	CharBox	*cb, *cb0;
	CharBox	*cblist = 0;
	double	avrcwidth, lineheight, charpitch;
	int	xe0;
	int	i;
	RecResultItem	recResult_c;	// combined
	RecResultItem	recResult1;
	RecResultItem	recResult2;

	RecResultItem	*recResult_cList = new RecResultItem[n_top];	// combined
	RecResultItem	*recResult1List = new RecResultItem[n_top];
	RecResultItem	*recResult2List = new RecResultItem[n_top];

	resultline[0] = '\0';

	if ( n_cat < 1 )  return(-1);

	// set default as horizontal writing
	if ( 0 == (wdir &= WrtDir_Mask) )  wdir = WrtDir_H;

	if ( 0 == (cba = new CharBox[ 2 * (image->width +1) ]) )  return(-1);
	cba_raw = cba + image->width +1;

	segmentchars(image, cba, cba_raw, \
		&avrcwidth, &lineheight, &charpitch, \
		force_alpha, wdir, NHrec::debug);

#if 0
	for ( i=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox ){
		cb = cba[i];
		if ( cb->poshint & PosHint_Top ){
			printf("T");
		} else if ( cb->poshint & PosHint_Middle ){
			printf("M");
		} else if ( cb->poshint & PosHint_Bottom ){
			printf("B");
		} else if ( cb->poshint & PosHint_Left ){
			printf("L");
		} else if ( cb->poshint & PosHint_Center ){
			printf("C");
		} else if ( cb->poshint & PosHint_Right ){
			printf("R");
		}
	}
	printf("\n");
#endif

	xe0 = cba[0].xe;
	cb0 = &cba[0];
	for ( i=0 ; cba[i].nbox != 0 ; i+=cba[i].nbox ){
		cb = &cba[i];

		// cblist does not contain an explicit SPACE character.
		// Spaces are detected and inserted by
		// measuring the distances between adjoining boxes.
		if ( cb0->alphamode == 0 || cb->alphamode == 0 ){
//			if ( (int)(charpitch - avrcwidth) < (cb->xs - xe0) ){
//(Sep.26)		if ( lineheight * .7 < cb->xs - xe0 ){
			if ( (int)avrcwidth * .8 < cb->xs - xe0 ){
				if ( 0 > rec_addstr(resultline, " ", bufsize) ){
					break;
				} else {
					// Added space
				}
			}
		}
		else{	// alphabet mode
// FIXME with a nice proportional/fixed-width detection!
			double	cp;
			double	d;
			d = (double)(cb->xc() - cb0->xc()) / charpitch;
			cp = charpitch * .4;
			if ( d > 1.4 && cp < (cb->xs - xe0) ){
				if ( 0 > rec_addstr(resultline, " ", bufsize) ){
					break;
				} else {
					// Added space
					rec_gramd_add(0);
				}
			}
			else if ( (int)avrcwidth * .8 < cb->xs - xe0 \
				  && lineheight * .4 < cb->xs - xe0 ){
				if ( 0 > rec_addstr(resultline, " ", bufsize) ){
					break;
				} else {
					// Added space
					rec_gramd_add(0);
				}
			}
		}
		xe0 = cb->xe;
		cb0 = cb;

		// Discard very long objects that do not look like text.
		if ( cb->width() / lineheight > 3 ){
			if ( 0 > rec_addstr(resultline, ".", bufsize) ){
				break;
			} else {
				// Added very long object
				rec_gramd_add(0);
			}
			continue;
		}

		// Perform character recognition.
		int	cid2 = 0;
		double	dist1=0;
		double	dist2=0;

		// (suppress compiler warning)
		recResult_c = recResult1 = recResult2 = Rec.resultTable[0];

		int	widechk = SizeHint_None;
		if ( cb->nbox == 2 ){
			cid = rec_character(image, &cba_raw[i], wdir, recResult1List);
			if ( cid >= 0 ){
				recResult1 = Rec.resultTable[cid];
//				dist2 += recResult1.dist * recResult1.dist;
				dist2 += recResult1.dist;
				widechk |= cclist[recResult1.id].sizehint;
			}
			else{ cid2 = -1; }
			cid = rec_character(image, &cba_raw[i+1], wdir, recResult2List);
			if ( cid >= 0 ){
				recResult2 = Rec.resultTable[cid];
//				dist2 += recResult2.dist * recResult2.dist;
				dist2 += recResult2.dist;
				widechk |= cclist[recResult2.id].sizehint;
			}
			else{ cid2 = -1; }
//			dist2 = sqrt(dist2);
			dist2 /= 2;
		}
		else{ cid2 = -1; }

		if ( cid2 == -1 )  dist2 = 1.0e30;
		cid = rec_character(image, cb, wdir, recResult_cList);
		if ( cid == -2 )  break;

		if ( cid >= 0 ){
			recResult_c = Rec.resultTable[cid];
			dist1 = recResult_c.dist;
		}
		else{  dist1 = 1.0e30; }

		if ( dist1 == 1.0e30 && dist2 == 1.0e30 ){
			if ( 0 > rec_addstr(resultline, ".", bufsize) ){
				break;
			} else {
				// Added unknown
				rec_gramd_add(0);
			}
		}
		else if ( dist1 <= 1.3 * dist2 || (widechk & SizeHint_Wide) ){
			if ( 0 > rec_addstr(resultline, \
				cclist[recResult_c.id].ccode, bufsize) ){
				break;
			} else {
				// Added combined
				rec_gramd_add(recResult_cList);
			}
//	printf("%s	%.4f\n",cclist[recResult_c.id].ccode, recResult_c.dist);
		}
		else{	if ( 0 > rec_addstr(resultline, \
				cclist[recResult1.id].ccode, bufsize) ){
				break;
			} else {
				// Added first
				rec_gramd_add(recResult1List);
			}
			if ( 0 > rec_addstr(resultline, \
				cclist[recResult2.id].ccode, bufsize) ){
				break;
			} else {
				// Added second
				rec_gramd_add(recResult2List);
			}

		}
	}

	rec_gramd_solve(resultline, bufsize);

//	if ( cblist )  delete_cblist(cblist);
	delete []cba;
	delete []recResult_cList;
	delete []recResult1List;
	delete []recResult2List;
	return(0);
}




