/*--------------------------------------------------------------
  OCR base library
  (C) 2005-2014  Hideaki Goto  (see accompanying LICENSE file)
    Written by H.Goto,  Dec. 2005
    Revised by H.Goto,  Feb. 2008
    Revised by H.Goto,  Apr. 2008
    Revised by H.Goto,  Sep. 2008
    Revised by H.Goto,  May  2009
    Revised by H.Goto,  Oct. 2009
    Revised by H.Goto,  Dec. 2009
    Revised by H.Goto,  Aug. 2014
--------------------------------------------------------------*/

#ifndef	ocrbase_h
#define	ocrbase_h


// Position hint
#define	PosHint_Mask	0x00ff
#define	PosHint_None	0x00
#define	PosHint_Left	0x40
#define	PosHint_Center	0x20
#define	PosHint_Right	0x10
#define	PosHint_Top	0x04
#define	PosHint_Middle	0x02
#define	PosHint_Bottom	0x01

// Geometry hint
#define	SizeHint_Mask	0x0f00
#define	SizeHint_None	0x0000
#define	SizeHint_Normal	0x0100
#define	SizeHint_Small	0x0200
#define	SizeHint_Tiny	0x0400
#define	SizeHint_Ascender	0x0100
#define	SizeHint_Descender	0x0200
#define	SizeHint_Wide	0x8000

// Writing direction (Horizontal/Vertical font specification)
#define	WrtDir_Mask	0x0f0000
#define	WrtDir_None	0x000000
#define	WrtDir_H	0x010000
#define	WrtDir_V	0x020000


class OCRPrep {
  public:
	int	edge(SIPImage *src, SIPImage *dst);
	int	thin(SIPImage *src, SIPImage *dst){ return( thin(src,dst,0) ); }
	int	thin(SIPImage *src, SIPImage *dst, int maxiter);
	int	normalize(SIPImage *src, SIPImage *dst, double alimit);
};


class FeatureVector {
  public:
	int	dim;
	double	*e;
	int	gHint;

	int	alloc(int dim);
	int	zeroVector();
	int	setVector(double *vec);
	int	getVector(double *vec);
	double	distEuclidean2(FeatureVector& vec);
	double	distEuclidean2(FeatureVector& vec, double limit);
	double	distEuclidean(FeatureVector& vec){  return(sqrt(distEuclidean2(vec))); }
	double	distManhattan(FeatureVector& vec);
	double	distChessboard(FeatureVector& vec);
	int	write_vector(FILE *fp);
	int	read_vector(FILE *fp);

	FeatureVector&	operator=(FeatureVector& obj);
	FeatureVector&	operator+=(FeatureVector& obj);

		FeatureVector();
		FeatureVector(int dim);
		~FeatureVector();
};


class RecResultItem {
  public:
	int	id;
	double	dist;
};


class RecBase {
  protected:
	void	initResultTable();

  public:
	int	n_top;
	int	n_cat;
	FeatureVector	*dic;
	RecResultItem	*resultTable;

	int	dealloc();
	int	alloc(int n_cat, int dim, int n_top);
	int	recognizeEuclidean(FeatureVector& charvec, int gHint);
//	int	recognizeManhattan(FeatureVector& charvec, int gHint);
//	int	recognizeChessboard(FeatureVector& charvec, int gHint);

		RecBase(){ n_cat = n_top = 0; };
		~RecBase();
};


typedef int (*feature_fn_t)(SIPImage*, FeatureVector*);


#ifdef	__cplusplus
extern "C" {
#endif

int	ocrbase_loaddic(RecBase *Rec, char *dicfile, int dim, int n_top, int debug);
int	ocrbase_loaddic_bydiccodes(RecBase *Rec, char *dir, char *diccodes, int dim, int n_top, int debug);

#ifdef	__cplusplus
 }
#endif

#endif	/* ocrbase_h */
