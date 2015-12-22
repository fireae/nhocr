/*--------------------------------------------------------------
	8-dim Data I/O class Library File
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

#include	"comlib.h"

#define		d8IO_c
#include	"d8IO.h"



/*------------------------------------------------------
	8-dim Null Data
------------------------------------------------------*/

d8pac	d8pac0 = { {0,0,0,0, 0,0,0,0} };




/*------------------------------------------------------
	8-dim Data Load Class
------------------------------------------------------*/


D8LOAD :: D8LOAD(void){
	fp = NULL;
	buffer = 0;
	bufferptr = 0;
}


D8LOAD :: ~D8LOAD(void){
	close();
}


int D8LOAD :: open(char *path){
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
	if ( 0 == strncmp(header.d8_magic,"DATA0004",8) )  ndim = 4;
	if ( 0 == strncmp(header.d8_magic,"DATA0008",8) )  ndim = 8;
	if ( ndim == 0 )  eflag=-1;

	ifLSB {
		_uf_dwordswap((uint32 *)&header.d8_attrb);
		_uf_dwordswap((uint32 *)&header.d8_count);
		for ( int i=0 ; i < ndim * 2 ; i++ ){
			_uf_wordswap((ushort *)&header.d8_llimit[i]);
		}
	}

	if ( 0 != header.d8_attrb )  eflag=-1;
	if ( 0 != header.d8_count )  eflag=-1;
	if ( eflag )  return(-3);	// Non-supported file.

	if ( NULL == (buffer = (d8pac *)new d8pac[d8_BUFFERS]) )  return(-4);
					// Memory not enough.
	count = 0;
	flag_eof = 0;
	return(0);
}


int D8LOAD :: close(void){
	int	retcode;
	if ( buffer != 0 ){  delete [](d8pac *)buffer;  buffer = 0; }
	if ( (fp == NULL) || (fp == stdin) )  return(0);
	retcode = fclose(fp);
	fp = NULL;
	return( retcode );
}


int D8LOAD :: getlower(d8pac *buf){
	int	i;
	if ( ndim == 4 ){
		*buf = d8pac0;
		for ( i=0 ; i<4 ; i++ )  buf->d8_data[i] = header.d8_llimit[i];
	}
	else{	*buf = *(d8pac *)(header.d8_llimit);
	}
	return (0);
}


int D8LOAD :: getupper(d8pac *buf){
	int	i;
	if ( ndim == 4 ){
		*buf = d8pac0;
		for ( i=0 ; i<4 ; i++ )  buf->d8_data[i] = header.d8_llimit[i + 4];
	}
	else{	*buf = *(d8pac *)(header.d8_hlimit);
	}
	return (0);
}


int D8LOAD :: getdata(d8pac *buf){
	d4pac	*ibuf4;
	d8pac	*ibuf8;
	ibuf4 = (d4pac *)buffer;
	ibuf8 = (d8pac *)buffer;
	if ( count == 0 ){
		if ( flag_eof )  return(-2);
		if ( ndim == 4 ){
			count = fread((char *)buffer,sizeof(d4pac),d8_BUFFERS,fp);

			ifLSB {
				for ( int i=0 ; i<count ; i++ ){
					_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[0]);
					_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[1]);
					_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[2]);
					_uf_wordswap((ushort *)&((d4pac *)buffer)[i].d4_data[3]);
				}
			}

		}
		else{	count = fread((char *)buffer,sizeof(d8pac),d8_BUFFERS,fp);

			ifLSB {
				for ( int i=0 ; i<count ; i++ ){
					for ( int j=0 ; j<8 ; j++ ){
						_uf_wordswap((ushort *)&((d8pac *)buffer)[i].d8_data[j]);
					}
				}
			}

		}
		if ( 0 == count )  return(-2);
		if ( count != d8_BUFFERS )  flag_eof = -1;
		bufferptr = 0;
	}
	if ( ndim == 4 ){
		*buf = d8pac0;
		*((d4pac *)buf) = ibuf4[ (bufferptr++) ];
	}
	else{	*buf = ibuf8[ (bufferptr++) ];
	}
	--count;
	return (0);
}


int D8LOAD :: rewind(void){
	if ( 0 != fseek(fp,sizeof(header),0) )   return (-1);
	count = 0;
	flag_eof = 0;
	return (0);
}




/*------------------------------------------------------
	8-dim Data Save Class
------------------------------------------------------*/


D8SAVE :: D8SAVE(void){
	fp = NULL;
	wfname[0]=0;
	buffer = 0;
}


D8SAVE :: ~D8SAVE(void){
	if ( fp != NULL ){
		close();
		remove(wfname);
	}
}


int D8SAVE :: creat(char *path,d8file *d8_info){
	int	i;
	if ( 0 == strcmp(path,"stdout") ){
		fp = stdout;
	}
	else{	if ( NULL == (fp = fopen(path,"wb")) ){
			return(-1);	// Open failed.
		}
	}
	header = *d8_info;
	strcpy(wfname,path);
	strncpy(header.d8_magic,"DATA0008",8);
	header.d8_attrb = 0;
	header.d8_count = 0;
	for ( i=0 ; i<40 ; i++ )  header.d8_spc[i] = 0;

	ifLSB {
		_uf_dwordswap((uint32 *)&header.d8_attrb);
		_uf_dwordswap((uint32 *)&header.d8_count);
		for ( i=0 ; i<8 ; i++ ){
			_uf_wordswap((ushort *)&header.d8_llimit[i]);
			_uf_wordswap((ushort *)&header.d8_hlimit[i]);
		}
	}

	if ( NULL == (buffer = (d8pac *)new d8pac[d8_BUFFERS]) )  return(-4);
					// Memory not enough.
	if ( sizeof(header) != fwrite(&header,1,sizeof(header),fp) ){
		return(-2);		// Write failed.
	}
	count = 0;
	return(0);
}


int D8SAVE :: close(void){
	int	retcode;
	if ( count != 0 ){

			ifLSB {
				for ( int i=0 ; i<count ; i++ ){
					for ( int j=0 ; j<8 ; j++ ){
						_uf_wordswap((ushort *)&((d8pac *)buffer)[i].d8_data[j]);
					}
				}
			}

		if ( (size_t)count != fwrite((char *)buffer,sizeof(d8pac),count,fp) ){
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


int D8SAVE :: putdata(d8pac *buf){
	d8pac	*ibuf;
	ibuf = (d8pac *)buffer;
	ibuf[count] = *buf;
	if ( (++count) >= d8_BUFFERS ){

		ifLSB {
			for ( int i=0 ; i<d8_BUFFERS ; i++ ){
				for ( int j=0 ; j<8 ; j++ ){
					_uf_wordswap((ushort *)&((d8pac *)buffer)[i].d8_data[j]);
				}
			}
		}

		if ( d8_BUFFERS != fwrite((char *)buffer,sizeof(d8pac),d8_BUFFERS,fp) ){
			return(-2);		// Write failed.
		}
		count = 0;
	}
	return (0);
}


