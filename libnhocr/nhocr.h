/*----------------------------------------------------------------------
    Character / Text line recognition library   nhocr.h
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


#ifndef	_nhocr_h
#define	_nhocr_h

#include	"ocrbase.h"
#include	"codelist.h"

#ifdef HAVE_LIBGRAMDCLIENT
#include	"gramdclient.h"
#endif


class CharBox {
public:
	int	xs,xe;
	int	ys,ye;
	int	charcore;
	int	conn_R;
	int	poshint, sizehint;
	int	alphamode;
	int	nbox;	// Number of boxes for a character.
			// nbox=0 shows the end of array.
	int	parent;	// -1: The CharBox is the leftmost one or is not merged.
			// non-negative: Parent ID

	int	xc(){ return((xs+xe)/2); }
	int	width(){ return(xe-xs+1); }
	int	height(){ return(ye-ys+1); }
	int	iscore(int refsize);
		CharBox(void);
};




class NHrec {
    private:
	RecBase	Rec;

	int	rec_addstr(char *line, const char *str, int bufsize);
	int	rec_character(SIPImage *image, CharBox *cb, int wdir, \
			RecResultItem *resultTable=0);

#ifdef HAVE_LIBGRAMDCLIENT
	gramd_connection *gramd_handle;
#endif
	// the following functions work as stubs without gramd
	int	rec_gramd_add(RecResultItem *list);
	int	rec_gramd_solve(char *buffer, size_t size);

    protected:
	char	*nhocrlibdir;
	char	*nhocrdiccodes;
	char	*cctablefile;
	char	*dicfile;
	char	*gramdportfile;

    public:
	int	debug;

	int	n_cat;
	int	n_top;
	int	force_alpha;	// set True (non-zero) to force alphabet mode
	int	gramd_enabled;
	CharCode	*cclist;

	// Basic functions
	int	setlibdir(char *path);
	int	setgramdportfile(char *path);
	int	setdiccodes(char *diccodes);
	int	setdicfile(char *dicname, char *cctablename);
	int	open();
	int	close();

	int	rec_character(SIPImage *image, \
			int x0, int y0, int width, int height, int wdir, \
			RecResultItem *resultTable);

	int	rec_line(SIPImage *image, char *resultline, int bufsize, int wdir);

	// Utilities
	int	binarize_otsu(SIPImage *src, SIPImage *dst, int csize, int interpolate);
	int	auto_invert(SIPImage *image);
	SIPImage	*rotate90(SIPImage *image, int angle);

		NHrec();
		~NHrec();
};

#endif	// _nhocr_h
