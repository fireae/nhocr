/*----------------------------------------------------------------------
    Text line segmentation in a text block     segline.h
        Written by H.Goto, Nov. 1997
        Revised by H.Goto, May  2009
----------------------------------------------------------------------*/

/*--------------
  Copyright 1997-2009  Hideaki Goto

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


#ifndef	_segline_h
#define	_segline_h

typedef struct {
	int	pos,len;
} RUNLIST;


#ifdef	__cplusplus
extern "C" {
#endif

// Note:
//   segment_lines() can handle either 1bpp or 8bpp image.
//   Pixel values must be 0 or 255 in 8bpp.

int segment_lines(SIPImage *image, \
	CRects *rect_pool, GRPLST *tlgroup, \
	int mode_global, int mode_NRpixels, int mode_combine);

// Note:
//   cutout_textline() can handle 8bpp image only.

SIPImage * cutout_textline(SIPImage *image, \
	CRects *rect_pool, GRPLST *tlgroup, int tlid);

#ifdef	__cplusplus
 }
#endif

#endif	// _segline_h
