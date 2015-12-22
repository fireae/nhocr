/*--------------------------------------------------------------
	Object Group Class  Header File    Rev.030729
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


#ifndef	objgrp_h
#define	objgrp_h


typedef struct {
	int	stat;
	int	counts;
	int	*id;
	void	*attr;
} grppac;


class GRPLST {
    private:
	int		bufsize;
	grppac		**grplist;
	inline grppac	*_getgp(int grpid);
    public:
	int		groups;

	int		realloc(int grps);
	int		alloc(int grps){  return( realloc(grps) ); }
	int		newgrp(int objid);
	int		addgrp(int grpid,int objid);
	int		deletegrp(int grpid);
	int		clearall(void);
	int		mergegrps(int dstgrpid,int srcgrpid);
	int		getidcounts(int grpid);
	int		setidcounts(int grpid,int idcounts);
	int		*getidlist(int grpid);
	int		setattr(int grpid,void *attr);
	void		*getattr(int grpid);
			GRPLST(void);
	virtual		~GRPLST(void);
};


inline grppac * GRPLST :: _getgp(int grpid){
	if ( (grpid < 0) || (grpid >= bufsize) )  return(0);
	return(grplist[grpid]);
}


#endif		/* objgrp_h */
