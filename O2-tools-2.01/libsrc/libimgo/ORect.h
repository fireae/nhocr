/*--------------------------------------------------------------
	Image object library  libimgo ,   H.Goto Nov.1997
	  Class: ORects
		Last modified  Nov. 1997
--------------------------------------------------------------*/

/*--------------------------------------------------------------------
  Copyright (C) 1997  Hideaki Goto

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


#ifndef	ORect_h
#define	ORect_h


#define		ORSTAT_Active	0x8000
#define		ORSTAT_NonLink	-1


/*------------------------------------------------------
	Oriented Rectangle
------------------------------------------------------*/

class ORect {
    private:
	int		stat;
    public:
	int		prev,next;
	/* Note: Don't use pointer, because the data will be relocated. */
	int		link;
	int		attr,attr1,attr2;
	double		attrd,attrd1,attrd2;
	void		*attrp;
	int		x,y,x1,y1;
	int		w,h;
	double		a;
	/* Note: attr1-2,attrd1-2,x1 and y1 are optional members. */

	inline void	setstat(int stat);
	inline int	getstat(){  return( stat & ~ORSTAT_Active ); }
	inline void	enable(){  stat |= ORSTAT_Active; }
	inline void	disable(){  stat &= ~ORSTAT_Active; }
	inline int	isactive(){  return( stat & ORSTAT_Active ); }

	inline void	init();
	void		init(int x,int y,int w,int h,double a);
		/* ORect should not have constructor for SPEED reason. */
};


inline void ORect :: setstat(int stat){
	stat &= ~ORSTAT_Active;
	ORect::stat = (ORect::stat & ORSTAT_Active) | stat;
}


inline void ORect :: init(){
	stat  = 0;
	prev = next = ORSTAT_NonLink;
	link  = ORSTAT_NonLink;
	attr  = 0;
	attrd = 0;
	attrp = 0;
	x = y = w = h = 0;
	a = 0;
	/* Note: The optional members aren't initialized. */
}




/*------------------------------------------------------
	Rectangle Object List
------------------------------------------------------*/


class ORects {
    private:
	int		maxrects;
	int		inc_step;
	int		incalloc();
    protected:
	ORect*		rectlist;	
    public:
	int		counts;

	int		alloc(int size);
	int		realloc(int size);
	int		clear(void);
	int		truncate(void);
	void		set_incstep(int step){  inc_step = step;  }

	int		create(int x,int y,int w,int h,double a);
	int		destroy(int rid);

	/* Warning: Functions truncate() and destroy() don't update */
	/*          prev or next. You should not call any of these  */
	/*          if you're using linked-list.                    */

	int		set(int rid,int x,int y,int w,int h,double a);
	int		setstat(int rid,int stat);
	int		getstat(int rid);
	ORect*		getrect(int rid){  return(&rectlist[rid]);  }

	int		setlink(int rid,int link);
	int		getlink(int rid);
	int		setattrp(int rid,void *attr);
	void*		getattrp(int rid);
	int		setattr(int rid,int attr);
	int		getattr(int rid);
	int		setattrd(int rid,double attr);
	double		getattrd(int rid);

	int		isactive(int rid);

			ORects(void);
			ORects(int incstep);
	virtual		~ORects(void);
};


#endif	/* ORect_h */
