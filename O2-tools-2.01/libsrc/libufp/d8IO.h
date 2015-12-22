/*--------------------------------------------------------------
    8(4)-dim Data I/O class Header Include File
		Written  by H.Goto , Sep. 1993
		Modified by H.Goto , Feb. 1997
		Modified by H.Goto , Apr. 2000
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1993-2000  Hideaki Goto

        All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
  its documentation for any purpose is hereby granted without fee,
  provided that (i) the above copyright notice and this permission
  notice appear in all copies and in supporting documentation, (ii)
  the name of the author, Hideaki Goto, may not be used in any
  advertising or otherwise to promote the sale, use or other
  dealings in this software without prior written authorization
  from the author, (iii) this software may not be used for
  commercial products without prior written permission from the
  author, and (iv) the notice of the modification is specified in
  case of that the modified copies of this software are distributed.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
  THE AUTHOR WILL NOT BE RESPONSIBLE FOR ANY DAMAGE CAUSED BY THIS
  SOFTWARE.
--------------------------------------------------------------------*/


#ifndef d8IO_h
#define d8IO_h

#include	<utypes.h>


/*------------------------------------------------------
	8-dim Data Load/Save Definitions
------------------------------------------------------*/

#ifndef d8_BUFFERS
#define		d8_BUFFERS	512
#endif

struct d8pac {
	short		d8_data[8];
};


struct d8file {
	char		d8_magic[8];
	int32		d8_attrb;
	int32		d8_count;
	short		d8_llimit[8];
	short		d8_hlimit[8];
	short		d8_spc[40];
};


#ifndef	d8IO_c
extern d8pac	d8pac0;
#endif




/*------------------------------------------------------
	8-dim Data Load Class
------------------------------------------------------*/

class D8LOAD {
    private:
	FILE		*fp;
	int		count;
	int		bufferptr;
	int		flag_eof;
	int		ndim;
	void		*buffer;
	struct d8file	header;
    public:
	int		open(char *path);
	int		close(void);
	int		getdim(void){  return ndim; };
	int		getdata(d8pac *buf);
	int		getlower(d8pac *buf);
	int		getupper(d8pac *buf);
	int		rewind(void);
    			D8LOAD(void);
	virtual		~D8LOAD(void);
};




/*------------------------------------------------------
	8-dim Data Save Class
------------------------------------------------------*/

class D8SAVE {
    private:
	FILE		*fp;
	char		wfname[256];
	int		count;
	void		*buffer;
	struct d8file	header;
    public:
	int		creat(char *path,d8file *d8_info);
	int		close(void);
	int		putdata(d8pac *buf);
    			D8SAVE(void);
	virtual		~D8SAVE(void);
};




/*------------------------------------------------------
	4-dim Data Load/Save Definitions
------------------------------------------------------*/

#ifndef d4_BUFFERS
#define		d4_BUFFERS	1024
#endif

struct d4pac {
	short		d4_data[4];
};


struct d4file {
	char		d4_magic[8];
	int32		d4_attrb;
	int32		d4_count;
	short		d4_llimit[4];
	short		d4_hlimit[4];
	short		d4_spc[48];
};




/*------------------------------------------------------
	4-dim Data Load Class
------------------------------------------------------*/

class D4LOAD {
    private:
	FILE		*fp;
	int		count;
	int		bufferptr;
	int		flag_eof;
	int		ndim;
	void	 	*buffer;
	struct d4file	header;
	struct d8file	*header8;
    public:
	int		fileopen(char *path);
	int		fileclose(void);
	int		getdata(d4pac *buf);
	int		getlower(d4pac *buf);
	int		getupper(d4pac *buf);
	int		rewind(void);
    			D4LOAD(void);
	virtual		~D4LOAD(void);
};




/*------------------------------------------------------
	4-dim Data Save Class
------------------------------------------------------*/

class D4SAVE {
    private:
	FILE		*fp;
	char		wfname[256];
	int		count;
	void		*buffer;
	struct d4file	header;
    public:
	int		filecreat(char *path,d4file *d4_info);
	int		fileclose(void);
	int		putdata(d4pac *buf);
    			D4SAVE(void);
	virtual		~D4SAVE(void);
};


#endif		//  _d8IO_h
