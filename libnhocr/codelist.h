/*----------------------------------------------------------------------
    Character code table management function   codetable.h
        Written by H.Goto, Feb. 2008
        Revised by H.Goto, Sep. 2008
        Revised by H.Goto, Jan. 2009
        Revised by H.Goto, May  2009
        Revised by H.Goto, July 2009
        Revised by H.Goto, Oct. 2009
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


#ifndef	_codetable_h
#define	_codetable_h

#define	CharCodeSize	16
#define	CCLineLength	80
#define	CCFontNameLen	16

class CharCode {
  public:
	char	ccode[CharCodeSize];
	char	ccode_dic[CharCodeSize];
	int	poshint, sizehint, wdir;
	int	alphamode;
	char	charclass;
	char	fontname[CCFontNameLen];
};

#ifdef	__cplusplus
extern "C" {
#endif

int load_codelist(char *file, CharCode **cclist, int debug);
int load_codelist_bydiccodes(char *dir, char *diccodes, CharCode **cclist, int debug);

#ifdef	__cplusplus
 }
#endif

#endif
