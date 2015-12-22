/*--------------------------------------------------------------
	4-dim Data I/O class Library File
		Written by H.Goto , Sep. 1993
		Revised by H.Goto , Feb. 1996
		Revised by H.Goto , May  1996
		Revised by H.Goto , Feb. 1997
		Revised by H.Goto , Apr. 2000
		Revised by H.Goto , May  2002
		Revised by H.Goto , Nov. 2008
		Revised by H.Goto , Aug. 2014
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1993-2014  Hideaki Goto

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
  author, and (iv) the notice of modification is specified in cases
  where modified copies of this software are distributed.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
  THE AUTHOR WILL NOT BE RESPONSIBLE FOR ANY DAMAGE CAUSED BY THIS
  SOFTWARE.
--------------------------------------------------------------------*/


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"d8IO.h"
#include	"comlib.h"




/*------------------------------------------------------
	4-dim Data Load Class
------------------------------------------------------*/

D4LOAD :: D4LOAD(void){
	fp = NULL;
	buffer = 0;
	bufferptr = 0;
	header8 = (d8file *)&header;
}


D4LOAD :: ~D4LOAD(void){
	fileclose();
	return;
}


int D4LOAD :: fileopen(char *path){
	int	eflag;
	if ( 0 == strcmp(path,"stdin") ){
		fp = stdin;
	}
	else{	if ( NULL == (fp = fopen(path,"rb") ) ){
			return(-1);	// Open failed.
		}
	}
	if ( sizeof(header) != fread((char *)&header,1,sizeof(header),fp) ){
		return(-2);	// Read failed.
	}
	eflag=0;
	ndim = 0;
	if ( 0 == strncmp(header.d4_magic,"DATA0004",8) )  ndim = 4;
	if ( 0 == strncmp(header.d4_magic,"DATA0008",8) )  ndim = 8;
	if ( ndim == 0 )  eflag=-1;

	ifLSB {
		_uf_dwordswap((uint32 *)&header.d4_attrb);
		_uf_dwordswap((uint32 *)&header.d4_count);
		for ( int i=0 ; i < ndim * 2 ; i++ ){
			_uf_wordswap((ushort *)&header.d4_llimit[i]);
		}
	}

	if ( 0 != header.d4_attrb )  eflag=-1;
	if ( 0 != header.d4_count )  eflag=-1;
	if ( eflag )  return(-3);	// Non-supported file.

	if ( ndim == 4 ){
		if ( NULL == (buffer = (d4pac *)new d4pac[d4_BUFFERS]) )  return(-4);
					// Memory not enough.
	}
	else{	if ( NULL == (buffer = (d8pac *)new d8pac[d8_BUFFERS]) )  return(-4);
					// Memory not enough.
	}

	count = 0;
	flag_eof = 0;
	return(0);
}


int D4LOAD :: fileclose(void){
	int	retcode;
	if ( buffer != 0 ){  delete [](d4pac *)buffer;  buffer = 0; }
	if ( (fp == NULL) || (fp == stdin) )  return(0);
	retcode = fclose(fp);
	fp = NULL;
	return( retcode );
}


int D4LOAD :: getlower(d4pac *buf){
	if ( ndim == 4 )
		*buf = *(d4pac *)header.d4_llimit;
	else	*buf = *(d4pac *)header8->d8_llimit;
	return (0);
}


int D4LOAD :: getupper(d4pac *buf){
	if ( ndim == 4 )
		*buf = *(d4pac *)header.d4_hlimit;
	else	*buf = *(d4pac *)header8->d8_hlimit;
	return (0);
}


int D4LOAD :: getdata(d4pac *buf){
	if ( count == 0 ){
		if ( flag_eof )  return(-2);
		if ( ndim == 4 ){
			count = fread((char *)buffer,sizeof(d4pac),d4_BUFFERS,fp);
			if ( 0 == count )  return(-2);

			ifLSB {
				for ( int i=0 ; i<count ; i++ ){
					_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[0]);
					_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[1]);
					_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[2]);
					_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[3]);
				}
			}

			if ( count != d4_BUFFERS )  flag_eof = -1;
		}
		else{	count = fread((char *)buffer,sizeof(d8pac),d8_BUFFERS,fp);
			if ( 0 == count )  return(-2);

			ifLSB {
				for ( int i=0 ; i<count ; i++ ){
					_uf_wordswap((ushort *)&((d8pac *)buffer)[i].d8_data[0]);
					_uf_wordswap((ushort *)&((d8pac *)buffer)[i].d8_data[1]);
					_uf_wordswap((ushort *)&((d8pac *)buffer)[i].d8_data[2]);
					_uf_wordswap((ushort *)&((d8pac *)buffer)[i].d8_data[3]);
						/* data of index 4-7 are not necessary */
				}
			}

			if ( count != d8_BUFFERS )  flag_eof = -1;
		}
		bufferptr = 0;
	}
	if ( ndim == 4 ){
		*buf = ((d4pac *)buffer)[ (bufferptr++) ];
	}
	else{	memcpy(buf,&((d8pac *)buffer)[ (bufferptr++) ],sizeof(d4pac));
	}
	--count;
	return (0);
}


int D4LOAD :: rewind(void){
	if ( 0 != fseek(fp,sizeof(header),0) )   return (-1);
	count = 0;
	flag_eof = 0;
	return (0);
}




/*------------------------------------------------------
	4-dim Data Save Class
------------------------------------------------------*/

D4SAVE :: D4SAVE(void){
	fp = NULL;
	wfname[0]=0;
	buffer = 0;
}


D4SAVE :: ~D4SAVE(void){
	if ( fp != NULL ){
		fileclose();
		remove(wfname);
	}
	return;
}


int D4SAVE :: filecreat(char *path,d4file *d4_info){
	int	i;
	if ( 0 == strcmp(path,"stdout") ){
		fp = stdout;
	}
	else{	if ( NULL == (fp = fopen(path,"wb")) ){
			return(-1);	// Open failed.
		}
	}
	header = *d4_info;
	strcpy(wfname,path);
	strncpy(header.d4_magic,"DATA0004",8);
	header.d4_attrb = 0;
	header.d4_count = 0;
	for ( i=0 ; i<48 ; i++ )  header.d4_spc[i] = 0;
	if ( NULL == (buffer = (d4pac *)new d4pac[d4_BUFFERS]) )  return(-4);
					// Memory not enough.
	ifLSB {
		_uf_dwordswap((uint32 *)&header.d4_attrb);
		_uf_dwordswap((uint32 *)&header.d4_count);
		for ( i=0 ; i<4 ; i++ ){
			_uf_wordswap((ushort *)&header.d4_llimit[i]);
			_uf_wordswap((ushort *)&header.d4_hlimit[i]);
		}
	}

	if ( sizeof(header) != fwrite(&header,1,sizeof(header),fp) ){
		return(-2);		// Write failed.
	}
	count = 0;
	return(0);
}


int D4SAVE :: fileclose(void){
	int	retcode;
	if ( count != 0 ){

		ifLSB {
			for ( int i=0 ; i<count ; i++ ){
				_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[0]);
				_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[1]);
				_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[2]);
				_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[3]);
			}
		}

		if ( (size_t)count != fwrite((char *)buffer,sizeof(d4pac),count,fp) ){
			return(-2);             // Write failed.
		}
		count = 0;
	}
	if ( (fp == NULL) || (fp == stdout) ) return(0);
	retcode = fclose(fp);
	fp = NULL;
	if ( retcode != 0 )  remove(wfname);
	wfname[0]=0;
	return( retcode );
}


int D4SAVE :: putdata(d4pac *buf){
	((d4pac *)buffer)[count] = *buf;
	if ( (++count) >= d4_BUFFERS ){

		ifLSB {
			for ( int i=0 ; i<d4_BUFFERS ; i++ ){
				_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[0]);
				_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[1]);
				_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[2]);
				_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[3]);
			}
		}

		if ( d4_BUFFERS != fwrite((char *)buffer,sizeof(d4pac),d4_BUFFERS,fp) ){
			return(-2);		// Write failed.
		}
		count = 0;
	}
	return (0);
}


