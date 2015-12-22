/*--------------------------------------------------------------
	Image object library  libimgo ,   H.Goto Dec.1995
	  Class: CRects
		Last modified  Nov. 1997
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1995-1997  Hideaki Goto

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


#ifndef	CRect_h
#define	CRect_h


#define		CRSTAT_Active	0x8000
#define		CRSTAT_NonLink	-1


class CRect {
    public:
	int		stat;
	int		link;
	int		attr;
	void*		attrp;
	int		x1,y1;
	int		x2,y2;
	inline int	width (){  return(abs(x2-x1)+1); }
	inline int	height(){  return(abs(y2-y1)+1); }

	inline void	enable(){  stat |= CRSTAT_Active; }
	inline void	disable(){  stat &= ~CRSTAT_Active; }
	inline int	isactive(){  return( stat & CRSTAT_Active ); }

	inline void	init();
	void		init(int x1,int y1,int x2,int y2);
	void		set (int x1,int y1,int x2,int y2);
		/* CRect should not have constructor for SPEED reason. */
};


inline void CRect :: init(){
	stat  = 0;
	link  = CRSTAT_NonLink;
	attr  = 0;
	attrp = 0;
	x1 = y1 = x2 = y2 = 0;
}




/*------------------------------------------------------
	Rectangle Object List
------------------------------------------------------*/


class CRects {
    private:
	int		maxcrs;
	int		inc_step;
	int		incalloc();
    protected:
	CRect*		crlist;	
    public:
	int		counts;

	CRect*		alloc(int size);
	CRect*		realloc(int size);
	int		clear(void);
	int		truncate(void);
	void		set_incstep(int step){  inc_step = step;  }

	int		create(int x1,int y1,int x2,int y2);
	int		destroy(int crn);
	int		findparent(int crn);
	int		cat(int cr1,int cr2);
	int		set(int crn,int x1,int y1,int x2,int y2);
	int		expand(int crn,int x1,int y1,int x2,int y2);
	int		setstat(int crn,int stat);
	int		getstat(int crn);
	int		setlink(int crn,int stat);
	int		getlink(int crn);
	int		setattrp(int crn,void *attr);
	void*		getattrp(int crn);
	int		setattr(int crn,int attr);
	int		getattr(int crn);
	CRect*		getrect(int crn){  return(&crlist[crn]);  }

	int		isactive(int crn);
	int		width(int crn);
	int		height(int crn);

			CRects(void);
			CRects(int incstep);
	virtual		~CRects(void);
};


#endif	/* CRect_h */
