/*--------------------------------------------------------------
	Object Group Class  Library              Rev.030729
		Written  by H.Goto , Mar 1994
		Modified by H.Goto , Oct 1994
		Rewrote  by H.Goto , Sep 1997
		Modified by H.Goto , Dec 1998
		Modified by H.Goto , May 2002
		Modified by H.Goto , Jul 2003
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1994-2003  Hideaki Goto

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

#include	"objgrp.h"


/*------------------------------------------------------
	Object Group Class
------------------------------------------------------*/


GRPLST :: GRPLST(void){
	groups = 0;
	bufsize = 0;
	grplist = 0;
}


GRPLST :: ~GRPLST(void){
	int	i;
	for ( i=0 ; i<groups ; i++ )  deletegrp(i);
	delete []grplist;
}


int GRPLST :: realloc(int grps){
	int	i;
	grppac	**grptmp;
	if ( grps < 0 )  return(-1);
	if ( bufsize >= grps )  return(0);
	if ( 0 == (grptmp = new grppac*[grps]) )  return(-1);
	memcpy((void *)grptmp,(void *)grplist,sizeof(grppac *) * bufsize);
	for ( i=bufsize ; i<grps ; i++ )  grptmp[i] = 0;
	delete []grplist;
	grplist = grptmp;
	bufsize = grps;
	return(0);
}


int GRPLST :: newgrp(int objid){
	grppac	*gp;
	if ( groups >= bufsize ){
		if ( 0 > realloc(bufsize + 1024) )  return(-1);
	}
	if ( 0 == (gp = new grppac) )  return(-1);
	gp->stat = 0;
	gp->attr = 0;
	gp->counts = 1;
	if ( 0 == (gp->id = new int[1]) ){
		delete gp;  return(-1);
	}
	gp->id[0] = objid;
	grplist[groups++] = gp;
	return( groups -1 );
}


int GRPLST :: addgrp(int grpid,int objid){
	grppac	*gp;
	int	*idtmp;
	int	counts;
	if ( 0 == (gp = _getgp(grpid)) )  return(-1);
	counts = gp->counts;
	if ( 0 == (idtmp = new int[ counts +1 ]) )  return(-1);
	memcpy((void *)idtmp,(void *)gp->id,sizeof(int) * counts);
	idtmp[ gp->counts++ ] = objid;
	delete []gp->id;
	gp->id = idtmp;
	return(0);
}


int GRPLST :: deletegrp(int grpid){
	grppac	*gp;
	if ( 0 == (gp = _getgp(grpid)) )  return(0);
	if ( gp->id != 0 )  delete []gp->id;
	if ( gp->attr != 0 )  delete gp->attr;
	delete gp;
	grplist[grpid] = 0;
	return(0);
}


int GRPLST :: clearall(void){
	int	i;
	for ( i=0 ; i<groups ; i++ )  deletegrp(i);
	groups = 0;
	return(0);
}


int GRPLST :: mergegrps(int dstgrpid,int srcgrpid){
	grppac	*sgp,*dgp;
	int	*idtmp;
	int	counts;
	sgp = _getgp(srcgrpid);
	dgp = _getgp(dstgrpid);
	if ( (sgp == 0) || (dgp == 0) )  return(-1);
	counts = sgp->counts + dgp->counts;
	if ( 0 == (idtmp = new int[counts]) )  return(-1);
	memcpy((void *)idtmp,(void *)dgp->id,sizeof(int) * dgp->counts);
	memcpy((void *)(idtmp + dgp->counts),(void *)sgp->id,sizeof(int) * sgp->counts);
	deletegrp(srcgrpid);
	delete []dgp->id;
	dgp->counts = counts;
	dgp->id = idtmp;
	return(0);
}


int GRPLST :: getidcounts(int grpid){
	grppac	*gp;
	if ( 0 == (gp = _getgp(grpid)) )  return(0);
	return ( gp->counts );
}


int GRPLST :: setidcounts(int grpid,int idcounts){
	grppac	*gp;
	if ( 0 == (gp = _getgp(grpid)) )  return(-1);
	gp->counts = idcounts;
	return(0);
}


int * GRPLST :: getidlist(int grpid){
	grppac	*gp;
	if ( 0 == (gp = _getgp(grpid)) )  return(0);
	return ( gp->id );
}


int GRPLST :: setattr(int grpid,void *attr){
	grppac	*gp;
	if ( 0 == (gp = _getgp(grpid)) )  return(-1);
	gp->attr = attr;
	return(0);
}


void * GRPLST :: getattr(int grpid){
	grppac	*gp;
	if ( 0 == (gp = _getgp(grpid)) )  return(0);
	return ( gp->attr );
}


