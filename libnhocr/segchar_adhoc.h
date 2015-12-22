/*----------------------------------------------------------------------
    Character segmentation function    segchar_adhoc.h
      (an Ad-hoc version)
        Written by H.Goto, Jan. 2008
        Revised by H.Goto, Feb. 2008
        Revised by H.Goto, Apr. 2008
        Revised by H.Goto, Sep. 2008
        Revised by H.Goto, Jan. 2009
        Revised by H.Goto, May  2009
        Revised by H.Goto, July 2009
        Revised by H.Goto, Oct. 2009
        Revised by H.Goto, Jan. 2013
        Revised by H.Goto, Aug. 2014
----------------------------------------------------------------------*/

/*--------------
  Copyright 2008-2014 Hideaki Goto

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


#ifndef	_segchar_adhoc_h
#define	_segchar_adhoc_h

#include	"utypes.h"
#include	"siplib.h"

#include	"nhocr.h"


extern "C" {

int	segmentchars(SIPImage *lineimage, \
		CharBox *cba, CharBox *cba_raw, \
		double *avrcwidth, double *lineheight, double *charpitch, \
		int force_alpha, int wdir, int debug);
//void	delete_cblist(CharBox *cblist);

}

#endif	// _segchar_adhoc_h
