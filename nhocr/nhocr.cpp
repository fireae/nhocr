/*----------------------------------------------------------------------
    NHocr - Japanese OCR     nhocr  ver 0.22
        Written by H.Goto, Jan. 2008
        Revised by H.Goto, Feb. 2008
        Revised by H.Goto, Sep. 2008
        Revised by H.Goto, Oct. 2008
        Revised by H.Goto, Dec. 2008
        Revised by H.Goto, May 15, 2009
        Revised by H.Goto, May 29, 2009
        Revised by H.Goto, July 13, 2009
        Revised by H.Goto, Oct. 2009
        Revised by H.Goto, Dec. 2009
        Revised by H.Goto, Feb. 2010
        Revised by H.Goto, Feb. 2013
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


#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <math.h>

#include	"utypes.h"

#include	"ufilep.h"
#include	"siplib.h"
#include	"imgobj.h"
#include	"objgrp.h"
#include	"CRect.h"

#include	"segline.h"

#include	"nhocr.h"

static char	programTitle[] = "NHocr - Japanese OCR  v0.22";

static int	sw_debug = 0;
static int	sw_verbose = 0;
static int	sw_alpha = 0;	// Alphabet segmentation mode
static int	sw_mchar = 0;	// Multi-character recognition mode
static int	sw_line = 0;	// Line recognition mode
static int	sw_block = 0;	// Text block recognition mode
static int	sw_gramd = 0;	// Use gramd for language processing
static int	sw_interpolate = 1;
static int	sw_ntop = 10;
static int	sw_vert = 0;	// vertical writing mode
static int	wdir = WrtDir_H;

		// for text line recognition
#define	MaxLineLength	1024
char	resultline[MaxLineLength +1];

		// for text line segmentation
static CRects	rect_pool;
static GRPLST	tlgroup;


#define	debugprintf	if(sw_debug)printf




/*------------------------------------------------------
    Recognize multi-character image
------------------------------------------------------*/

static void auto_invert_char(SIPImage *image){
	int	x,y,n;
	int	a = 0;
	uchar	*p;
	p = (uchar *)image->pdata[0];
	for ( x=0 ; x<image->width ; x++ ){
		if ( p[x] > 127 )  a++;
	}
	p = (uchar *)image->pdata[image->height -1];
	for ( x=0 ; x<image->width ; x++ ){
		if ( p[x] > 127 )  a++;
	}
	for ( y=1 ; y < image->height -1 ; y++ ){
		p = (uchar *)image->pdata[y];
		if ( p[0] > 127 )  a++;
		if ( p[image->width -1] > 127 )  a++;
	}
	n = 2 * (image->width + image->height) -4;
	if ( n < 1 )  n = 1;
	if ( (100 * a) / n > 50 )  return;
	for ( y=0 ; y<image->height ; y++ ){
		p = (uchar *)image->pdata[y];
		for ( x=0 ; x<image->width ; x++ ){
			p[x] = ~p[x];
		}
	}
	return;
}




static int rec_multichar(SIPImage *image, NHrec *NHrec, \
	int n_top, FILE *fp){
	int	n, cid, r;
	int	nchar;
	int	csize = image->width;
	int	w,h,i;
	uchar	**p;
	uint	psum = 0;
	RecResultItem	*resultTable;
	SIPRectangle	srect, drect;
	SIPImage	*bimage = 0;
	char	candchar[16];
	int	retcode = 0;

	if ( 0 > fprintf(fp,"# Character candidates table\n#   produced by: %s \n", programTitle) ){
		return(-1);
	}

	if ( image->height < csize )  return(-1);
	if ( n_top > NHrec->n_top )  n_top = NHrec->n_top;
	nchar = image->height / csize;
	p = (uchar **)image->pdata;
	if ( 0 == (resultTable = new RecResultItem[NHrec->n_top]) ){
		return(-1);
	}

	srect.x = srect.y = 0;
	srect.width = srect.height = csize;
	drect = srect;
	for ( n=0 ; n<nchar ; n++ ){
		if ( 0 > fprintf(fp,"IMG\t%d\n",n) ){
			retcode = -1;  break;
		}
		srect.y = n * csize;

		// clipping
		w = h = csize;
		do {	for ( i=0, psum=0 ; i<csize ; i++ ){
				psum += (uint)p[srect.y + i][w-1];
			}
			w--;
		} while ( w > 0 && psum == 255 * csize );
		w++;
		do {	for ( i=0, psum=0 ; i<csize ; i++ ){
				psum += (uint)p[srect.y + h -1][i];
			}
			h--;
		} while ( h > 0 && psum == 255 * csize );
		h++;

		if ( 0 == (bimage = sip_CreateImage(w, h, 8)) ){
			retcode = -1;
			break;
		}

		srect.width  = drect.width  = w;
		srect.height = drect.height = h;
		sip_CopyArea(image, &srect, bimage, &drect);
		NHrec->binarize_otsu(bimage,bimage, \
			(w > h ? w : h), 0);

		auto_invert_char(bimage);

		NHrec->rec_character(bimage, \
			0, 0, w, h, WrtDir_H, resultTable);

		sip_DestroyImage(bimage);
		bimage = 0;

		for ( r=0 ; r<n_top ; r++ ){
			cid = resultTable[r].id;
			if ( cid < 0 )  break;
			strncpy(candchar, NHrec->cclist[cid].ccode, 14);
			if ( strcmp(candchar,"\\") == 0 ){
				strcpy(candchar, "\\\\");
			}
			if ( strcmp(candchar," ") == 0 ){
				strcpy(candchar, "\\ ");
			}
			if ( 0 > fprintf(fp, "R\t%d\t%s\t0\t0\t%.7e\n", \
				r+1, candchar, resultTable[r].dist) ){
				retcode = -1;  break;
			}
		}
		if ( retcode )  break;
		if ( 0 > fprintf(fp, "\n") ){
			retcode = -1;  break;
		}
	}

	if ( bimage )  sip_DestroyImage(bimage);
	delete []resultTable;
	return(retcode);
}




/*------------------------------------------------------
	Load / Save Image File
------------------------------------------------------*/

#if 0
static int LoadImage(char *fname,SIPImage **image,int depthconv, \
		int fgcol,int bgcol){
	PBMFILE	PBMLD;
	int	retcode,depth,x,y;
	SIPImage	*imgbuf;
	uchar	*lbuf,*p;
	*image = 0;	/* default value = NULL */
	PBMLD.setpal_fb((uchar)fgcol,(uchar)bgcol);
	retcode = PBMLD.open(fname,"r");
	if ( retcode )  return(retcode);
	switch ( PBMLD.getfiletype() ){
	  case 0:	depth = 1;  break;
	  case 1:	depth = 8;  break;
	  case 2:	depth = 32;  break;
			/* 24bit in file, but 32bit on memory */
	  default:	return(-5);
	}
	if ( depthconv == 24 )  depthconv = 32;
	switch ( depthconv ){
	  case 0:	depthconv = depth;  break;
	  case 1:	if ( depth > 1 )  return(-5);
			break;
	  case 8:	if ( depth > 8 )  return(-5);
			break;
	  case 32:	break;
	  default:	return(-5);
	}
	if ( 0 == (imgbuf = sip_CreateImage(PBMLD.getwidth(), \
		PBMLD.getheight(),depthconv)) )  return(-6);
	if ( 0 == (lbuf = new uchar [3 * PBMLD.getwidth() ]) )  return(-6);
	for ( y=0 ; y < PBMLD.getheight() ; y++ ){
		if ( depthconv == depth && depth < 32 ){
			if ( 0 != PBMLD.readline(-1,\
				(void *)imgbuf->pdata[y]) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
		}
		else if ( depthconv == depth && depth == 32 ){
			if ( 0 != PBMLD.readline(-1,(void *)lbuf) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
			p = (uchar *)imgbuf->pdata[y];
			for ( x=0 ; x < PBMLD.getwidth() ; x++ ){
				p[0] = lbuf[3*x  ];	/* R */
				p[1] = lbuf[3*x+1];	/* G */
				p[2] = lbuf[3*x+2];	/* B */
				p += 4;
			}
		}
		else if ( depthconv < 32 ){
			if ( 0 != PBMLD.readline_gray(-1,\
				(void *)imgbuf->pdata[y]) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
		}
		else{	if ( 0 != PBMLD.readline_gray(-1,(void *)lbuf) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
			p = (uchar *)imgbuf->pdata[y];
			for ( x=0 ; x < PBMLD.getwidth() ; x++ ){
				p[0] = lbuf[x];	/* R=Y */
				p[1] = lbuf[x];	/* G=Y */
				p[2] = lbuf[x];	/* B=Y */
				p += 4;
			}
		}
	}
	delete []lbuf;
	*image = imgbuf;
	return(0);
}
#endif


static int SaveImage(char *fname,SIPImage *image){
	int	x,y,retcode;
	uchar	*lbuf,*p;
	int	depth;
	PBMFILE	PBMSV;
	if ( image->depth != 32 )  depth = image->depth;  else  depth = 24;
	if ( PBMSV.setsize(image->width,image->height,depth) ){
		return(-5);	/* illegal size */
	}
	if ( 0 == (lbuf = new uchar [3 * image->width]) ){
		return(-6);	/* memory error */
	}
	retcode = PBMSV.open(fname,"w");
	if ( 0 > retcode )  return(retcode);
	retcode = 0;
	if ( image->depth == 1 || image->depth == 8 ){
		for ( y=0 ; y < image->height ; y++ ){
			if ( PBMSV.writeline(-1,(void *)image->pdata[y]) ){
				retcode = -1;  break;
			}
		}
	}
	else{	for ( y=0 ; y < image->height ; y++ ){
			p = (uchar *)image->pdata[y];
			for ( x=0 ; x<image->width ; x++ ){
				lbuf[3*x  ] = p[0];
				lbuf[3*x+1] = p[1];
				lbuf[3*x+2] = p[2];
				p += 4;
			}
			if ( PBMSV.writeline(-1,(void *)lbuf) ){
				retcode = -1;
				break;
			}
		}
	}
	delete []lbuf;
	if ( retcode )  PBMSV.close();  else  retcode = PBMSV.close();
	return(retcode);
}




/*----------------------------------------------
    Load image file and convert it to grayscale
----------------------------------------------*/

static int load_image(char *fname,SIPImage **image,int *width,int *height){
	PBMFILE	PBMLD;
	int	retcode,depth,x,y;
	SIPImage	*imgbuf;
	uchar	*lbuf, *p;
	double	gray;
	int	bufwidth,bufheight;
	*image = 0;	/* default value = NULL */
	PBMLD.setpal_fb(0,0xff);
	retcode = PBMLD.open(fname,"r");
	if ( retcode )  return(retcode);
	switch ( PBMLD.getfiletype() ){
	  case 0:	depth = 1;  break;
	  case 1:	depth = 8;  break;
	  case 2:	depth = 24;  break;
	  default:	return(-5);
	}
	bufwidth  = *width  = PBMLD.getwidth();
	bufheight = *height = PBMLD.getheight();

	if ( 0 == (imgbuf = sip_CreateImage(bufwidth,bufheight,8)) )  return(-6);
	if ( 0 == (lbuf = new uchar[*width * 3]) ){
		sip_DestroyImage(imgbuf);
		return(-6);
	}
	sip_ClearImage(imgbuf,0xff);
	for ( y=0 ; y < *height ; y++ ){
		p = (uchar *)imgbuf->pdata[y];
		if ( 24 == depth ){
			if ( 0 != PBMLD.readline(-1,\
				(void *)lbuf) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
			for ( x=0 ; x<*width ; x++ ){
				gray  = 0.299 * (double)lbuf[3*x];
				gray += 0.587 * (double)lbuf[3*x+1];
				gray += 0.114 * (double)lbuf[3*x+2];
				if ( gray>255 )  gray=255;
				p[x] = (uchar)((uint)gray);
			}
		}
		else if ( 8 == depth ){
			if ( 0 != PBMLD.readline(-1,(void *)p) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
		}
		else{	if ( 0 != PBMLD.readline_gray(-1,(void *)p) ){
				delete []lbuf;
				return(-1);	/* read error */
			}
		}
		for ( x=*width ; x<bufwidth ; x++ )  p[x] = p[*width-1];
	}
	for ( ; y < bufheight ; y++ ){
		memcpy((void*)imgbuf->pdata[y], (void*)imgbuf->pdata[*height-1],bufwidth);
	}

	*image = imgbuf;
	PBMLD.close();
	delete []lbuf;
	return(0);
}




/*----------------------------------------------
    Convert gray image into binary and save it
----------------------------------------------*/

static int save_bin_image(char *fname, SIPImage *image){
	int	x,y;
	uchar	*p;
	SIPImage	*bimage;

	if ( 0 == (bimage = sip_CreateImage(image->width, image->height,1)) ){
		fputs("Low memory.\n",stderr);
		return(-1);
	}

	for ( y=0 ; y<image->height ; y++ ){
		sip_cvt8to1u((uchar *)image->pdata[y], \
			(uchar *)bimage->pdata[y],0,image->width,127);
		p = (uchar *)bimage->pdata[y];
		for ( x=0 ; x < (image->width +7) /8 ; x++ )  p[x] = ~p[x];
	}

	if ( 0 > SaveImage(fname,bimage) ){
		sip_DestroyImage(bimage);
		return(-1);
	}
	sip_DestroyImage(bimage);
	return(0);
}




/*----------------------------------------------
    Print usage
----------------------------------------------*/

static void usage(){
	fprintf(stderr,"%s  Copyright (C) 2008-2014 Hideaki Goto \n",programTitle);
	fputs("usage: nhocr [-options] [-o result.txt] img_file [img_file2 img_file3 ...]\n",stderr);
	fputs("         -o -      :  write text data to stdout \n",stderr);
	fputs("         -alpha    :  force alphabet segmentation mode \n",stderr);
	fputs("         -vert     :  vertical writing mode (for line/block modes only) \n",stderr);
	fputs("         -line     :  line recognition mode \n",stderr);
	fputs("         -block    :  text block recognition mode \n",stderr);
	fputs("         -mchar    :  multi-character recognition mode \n",stderr);
#ifdef HAVE_LIBGRAMDCLIENT
	fputs("         -gramd    :  enable language post-processing by gramd \n",stderr);
	fputs("                       (default: disabled) \n",stderr);
#endif
	fputs("         -top N    :  produce up to N character candidates (default: 10) \n",stderr);
	fputs("         -csize N  :  cell size in adaptive binarization (default: 16) \n",stderr);
//	fputs("         -verbose  :  show process information \n",stderr);
	fputs("\ndescription:\n",stderr);
	fputs("  The current version of nhocr does not support \"Page recognition\n  mode\". One of -line, -block, and -mchar is always required.\n",stderr);
	fputs("  nhocr reads PBM/PGM/PPM image file(s), recognizes the text\n",stderr);
	fputs("  line image for each file, and produces text data in UTF-8.\n",stderr);
	fputs("  Each file should contain only ONE horizontal text line image\n",stderr);
	fputs("  in line recognition mode, or only ONE text block in block\n",stderr); 
	fputs("  recognition mode, without any surrounding lines or dirt.\n\n",stderr);
}




/*----------------------------------------------
    Main routine
----------------------------------------------*/

int main(int ac,char *av[]){
	char	*infile  = NULL;
	char	*outfile = NULL;
	FILE	*fp_out = stdout;
	int	width=0,height=0;
	int	csize = 16;
	int	i,k;
	int	argn = 1;
	SIPImage	*image, *bimage;
	NHrec	NHrec;

	for ( k=1, i=1 ; i<ac && k ; i++ ){
		if ( 0 == strcmp("-top",av[i]) ){
			if ( ++i >= ac ){  k=0; continue; }
			sw_ntop = atoi(av[i]);
			if ( sw_ntop < 1 ){  k=0; continue; }
			continue;
		}
		if ( 0 == strcmp("-csize",av[i]) ){
			if ( ++i >= ac ){  k=0; continue; }
			csize = atoi(av[i]);
			if ( csize < 4 ){  k=0; continue; }
			continue;
		}
		if ( 0 == strcmp("-o",av[i]) ){
			if ( ++i >= ac ){  k=0; continue; }
			outfile = av[i];
			continue;
		}
		if ( 0 == strcmp("-debug",av[i]) ){  sw_debug = -1;  continue; }
		if ( 0 == strcmp("-i",av[i]) ){  sw_interpolate = -1;  continue; }
		if ( 0 == strcmp("-alpha",av[i]) ){
			sw_alpha = -1;  continue; }
		if ( 0 == strcmp("-vert",av[i]) ){
			wdir = WrtDir_V;  sw_vert = -1;  continue; }
		if ( 0 == strcmp("-page",av[i]) ){
			sw_mchar = sw_block = sw_line = 0;  continue; }
		if ( 0 == strcmp("-mchar",av[i]) ){
			sw_line = sw_block = 0;
			sw_mchar = -1;  continue; }
		if ( 0 == strcmp("-line",av[i]) ){
			sw_mchar = sw_block = 0;
			sw_line = -1;  continue; }
		if ( 0 == strcmp("-block",av[i]) ){
			sw_mchar = sw_line = 0;
			sw_block = -1;  continue; }
#ifdef HAVE_LIBGRAMDCLIENT
		if ( 0 == strcmp("-gramd",av[i]) ){  sw_gramd = -1;  continue; }
#endif
		if ( 0 == strcmp("-verbose",av[i]) ){  sw_verbose = -1;  continue; }
		if ( 0 == strcmp("-v",av[i]) ){  sw_verbose = -1;  continue; }
		if ( 0 == strcmp("-h",av[i]) ){  usage(); return(0); }
		if ( av[i][0] == '-' && strlen(av[i]) > 1 ){  k=0;  continue; }
		if ( NULL == infile ){
			infile = av[ argn = i ];
			continue;
		}
	}
	if ( k == 0 || infile == NULL ){  usage();  return(1); }
	if ( k == 0 || outfile == NULL ){
		fprintf(stderr,"Output file is not specified.\n");
		return(1);
	}

	if ( sw_mchar == 0 && sw_line == 0 && sw_block == 0 ){
		fputs("The current version of nhocr does not support \"Page recognition mode\".\n", stderr);
		fputs("Please specify one of -line, -block, and -mchar options.\n", stderr);
		return(1);
	}


/* ---- Initialize line recognizer ---- */

	if ( sw_debug )  NHrec.debug = 1;
	if ( sw_alpha )  NHrec.force_alpha = -1;
	if ( sw_gramd )  NHrec.gramd_enabled = 1;

	if ( getenv("NHOCR_DICDIR") ){
		NHrec.setlibdir(getenv("NHOCR_DICDIR"));
		debugprintf("NHOCR_DICDIR= %s\n",getenv("NHOCR_DICDIR"));
	}
	if ( getenv("NHOCR_DICCODES") ){
		NHrec.setdiccodes(getenv("NHOCR_DICCODES"));
		debugprintf("NHOCR_DICCODES= %s\n",getenv("NHOCR_DICCODES"));
	}
	if ( getenv("NHOCR_GRAMDPORTFILE") ){
		NHrec.setgramdportfile(getenv("NHOCR_GRAMDPORTFILE"));
		debugprintf("NHOCR_GRAMDPORTFILE= %s\n",getenv("NHOCR_GRAMDPORTFILE"));
	}

	if ( NHrec.open() ){
		return(2);
	}

	if ( 0 != strcmp(outfile, "-") ){
		if ( NULL == (fp_out = fopen(outfile,"wb")) ){
			fprintf(stderr,"Can't write on \"%s\".\n",outfile);
			return(2);
		}
	}




  for ( ; argn < ac ; argn++ ){
	infile = av[argn];
	if ( infile[0] == '-' )  continue;

/* ---- Read Image ---- */

	if ( 0 > load_image(infile,&image,&width,&height) ){
		fprintf(stderr,"Can't read \"%s\".\n",infile);
		return(2);
	}

	if ( sw_verbose ){
		fprintf(stderr,"Input image size = %d x %d\n",width,height);
	}

	if ( sw_mchar == 0 && sw_vert ){
		SIPImage *image_r;
		if ( 0 == (image_r = NHrec.rotate90(image, 90)) ){
			fputs("Low memory.\n",stderr);
			return(1);
		}
		sip_DestroyImage(image);
		image = image_r;
	}

	width  = image->width;
	height = image->height;


/* ---- Binarize image ---- */

	if ( 0 == (bimage = sip_DuplicateImage(image)) ){
		fputs("Low memory.\n",stderr);
		return(1);
	}

	if ( sw_mchar == 0 ){
		NHrec.binarize_otsu(bimage,bimage,csize,sw_interpolate);
		NHrec.auto_invert(bimage);
	}

	if ( sw_debug )  SaveImage("out.pgm",image);


/* ---- Layout analysis ---- */

	if ( sw_block ){
		tlgroup.clearall();
		rect_pool.clear();
		if ( 0 != segment_lines(bimage, &rect_pool, &tlgroup, 0, 1, 1) ){
			fprintf(stderr,"Failed in text segmentation in the text block.\n");
		}
	}


/* ---- Recognize character image(s) ---- */

	if ( sw_mchar ){	// multi-character recognition mode
		if ( rec_multichar(image, &NHrec, sw_ntop, fp_out) ){
			fprintf(stderr,"Can't write on \"%s\".\n",outfile);
			return(2);
		}
	}


/* ---- Recognize text line(s) ---- */
			
	else if ( sw_line ){	// line recognition mode
		NHrec.rec_line(bimage, resultline, MaxLineLength, wdir);

		strcat(resultline, "\n");
		if ( EOF == fputs(resultline,fp_out) ){
			fprintf(stderr,"Can't write on \"%s\".\n",outfile);
			return(2);
		}
	}

	else{			// text block recognition mode
		SIPImage	*TLimage;
		for ( int tlid=0 ; tlid<tlgroup.groups ; tlid++ ){
			TLimage = cutout_textline(bimage, &rect_pool, &tlgroup, tlid);
			if ( TLimage == 0 )  continue;
			NHrec.rec_line(TLimage, resultline, MaxLineLength, wdir);
			strcat(resultline, "\n");
			if ( EOF == fputs(resultline,fp_out) ){
				fprintf(stderr,"Can't write on \"%s\".\n",outfile);
				return(2);
			}
			fflush(fp_out);
		}
	}


/* ---- Save the binary image (debug, only the last image) ---- */

	if ( sw_debug && sw_mchar == 0 ){
		if ( 0 > save_bin_image("out2.pbm",bimage) ){
			fprintf(stderr,"Can't write on \"%s\".\n","out2.pbm");
		}
		return(2);
	}

	sip_DestroyImage(bimage);
	sip_DestroyImage(image);
  }


	if ( fp_out != stdout ){
		if ( 0 != fclose(fp_out) ){
			fprintf(stderr,"Can't write on \"%s\".\n",outfile);
		}
	}


/* ---- Close line recognizer ---- */

	NHrec.close();

	return(0);
}


