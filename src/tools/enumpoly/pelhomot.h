/* *****************************************************************

   This file, and all other pel*.h and pel*.cc files in the Gambit
source code, are derived from the source code for Pelican, an
implementation of the Huber-Sturmfels algorithm for solving sparse
systems of polynomials written by Birk Huber.  That code in turn
incorporated the source code for HOMPACK and Qhull, which are included
here after being modified in the construction of both Pelican and
Gambit.  We are grateful to the authors of Pelican, Qhull, and
HOMPACK, for code that has been enormously useful in Gambit.
    More information about Qhull can be obtained from:

http://www.geom.edu/software/qhull/

    For more information concerning HOMPACK see:

http://netlib2.cs.utk.edu/hompack/

***************************************************************** */

/* Homotopies.h */

/* 
   This file and Homotopies.c contain the code that was in the Cont
subdirectory of the source directory of the original Pelican 0.80
*/

#ifndef HOMOTOPIES_H
#define HOMOTOPIES_H

#include <cmath>
#include "pelpsys.h"

/************************************************************************/
/**************** header information from Hom_params.h ******************/
/************************************************************************/

#define Hom_LogFile stdout /* was Pel_Log */
#define Hom_OutFile stdout /* was Pel_Out */
extern int     Hom_defd; 
extern int     Hom_use_proj;
extern int     Hom_use_scale;
extern int     Hom_num_vars;
extern char    Hom_LogName[];
extern FILE   *Pel_Log;
extern FILE   *Pel_Out;
extern double  Hom_tol;
extern int     Hom_use_proj;

void print_Hom_params(FILE *);

/* end Hom_params.h */

/************************************************************************/
/****************** header information from Hom_Mem.h *******************/
/************************************************************************/

double *Dres(int);
int Dtop();                                    
int Dfree(int ntop);

int *Ires(int sz);
int Itop();
int Ifree(int ntop);

/* end Hom_Mem.h */

/************************************************************************/
/******************** header information from f2c.h *********************/
/************************************************************************/

typedef long int integer;
typedef char *address;
typedef short int shortint;
typedef float real;
typedef double doublereal;
typedef struct { real r, i; } complex;
typedef struct { doublereal r, i; } doublecomplex;
typedef long int logical;
typedef short int shortlogical;
typedef char logical1;
typedef char integer1;
/* typedef long long longint; */ /* system-dependent */

#define TRUE_ (1)
#define FALSE_ (0)

/* Extern is for use with -E */
#ifndef Extern
#define Extern extern
#endif

/* I/O stuff */

#ifdef f2c_i2
/* for -i2 */
typedef short flag;
typedef short ftnlen;
typedef short ftnint;
#else
typedef long flag;
typedef long ftnlen;
typedef long ftnint;
#endif

/*external read, write*/
typedef struct
{	flag cierr;
	ftnint ciunit;
	flag ciend;
	char *cifmt;
	ftnint cirec;
} cilist;

/*internal read, write*/
typedef struct
{	flag icierr;
	char *iciunit;
	flag iciend;
	char *icifmt;
	ftnint icirlen;
	ftnint icirnum;
} icilist;

/*open*/
typedef struct
{	flag oerr;
	ftnint ounit;
	char *ofnm;
	ftnlen ofnmlen;
	char *osta;
	char *oacc;
	char *ofm;
	ftnint orl;
	char *oblnk;
} olist;

/*close*/
typedef struct
{	flag cerr;
	ftnint cunit;
	char *csta;
} cllist;

/*rewind, backspace, endfile*/
typedef struct
{	flag aerr;
	ftnint aunit;
} alist;

/* inquire */
typedef struct
{	flag inerr;
	ftnint inunit;
	char *infile;
	ftnlen infilen;
	ftnint	*inex;	/*parameters in standard's order*/
	ftnint	*inopen;
	ftnint	*innum;
	ftnint	*innamed;
	char	*inname;
	ftnlen	innamlen;
	char	*inacc;
	ftnlen	inacclen;
	char	*inseq;
	ftnlen	inseqlen;
	char 	*indir;
	ftnlen	indirlen;
	char	*infmt;
	ftnlen	infmtlen;
	char	*inform;
	ftnint	informlen;
	char	*inunf;
	ftnlen	inunflen;
	ftnint	*inrecl;
	ftnint	*innrec;
	char	*inblank;
	ftnlen	inblanklen;
} inlist;

#define VOID void

union Multitype {	/* for multiple entry points */
	integer1 g;
	shortint h;
	integer i;
	/* longint j; */
	real r;
	doublereal d;
	complex c;
	doublecomplex z;
	};

typedef union Multitype Multitype;

typedef long Long;	/* No longer used; formerly in Namelist */

struct Vardesc {	/* for Namelist */
	char *name;
	char *addr;
	ftnlen *dims;
	int  type;
	};
typedef struct Vardesc Vardesc;

struct Namelist {
	char *name;
	Vardesc **vars;
	int nvars;
	};
typedef struct Namelist Namelist;

#define dblabs(x) ((x) >= 0 ? (x) : -(x))
#define dabs(x) (doublereal)dblabs(x)
/* #define min(a,b) ((a) <= (b) ? (a) : (b)) */
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define dmin(a,b) (doublereal)min(a,b)
#define dmax(a,b) (doublereal)max(a,b)

/* procedure parameter types for -A and -C++ */

#define F2C_proc_par_types 1
#ifdef __cplusplus
typedef int /* Unknown procedure type */ (*U_fp)(...);
typedef shortint (*J_fp)(...);
typedef integer (*I_fp)(...);
typedef real (*R_fp)(...);
typedef doublereal (*D_fp)(...), (*E_fp)(...);
typedef /* Complex */ VOID (*C_fp)(...);
typedef /* Double Complex */ VOID (*Z_fp)(...);
typedef logical (*L_fp)(...);
typedef shortlogical (*K_fp)(...);
typedef /* Character */ VOID (*H_fp)(...);
typedef /* Subroutine */ int (*S_fp)(...);
#else
typedef int /* Unknown procedure type */ (*U_fp)();
typedef shortint (*J_fp)();
typedef integer (*I_fp)();
typedef real (*R_fp)();
typedef doublereal (*D_fp)(), (*E_fp)();
typedef /* Complex */ VOID (*C_fp)();
typedef /* Double Complex */ VOID (*Z_fp)();
typedef logical (*L_fp)();
typedef shortlogical (*K_fp)();
typedef /* Character */ VOID (*H_fp)();
typedef /* Subroutine */ int (*S_fp)();
#endif
/* E_fp is for real functions when -R is not specified */
typedef VOID C_f;	/* complex function */
typedef VOID H_f;	/* character function */
typedef VOID Z_f;	/* double complex function */
typedef doublereal E_f;	/* real function with -R not specified */

/* undef any lower-case symbols that your C compiler predefines, e.g.: */

#ifndef Skip_f2c_Undefs
#undef cray
#undef gcos
#undef mc68010
#undef mc68020
#undef mips
#undef pdp11
#undef sgi
#undef sparc
#undef sun
#undef sun2
#undef sun3
#undef sun4
#undef u370
#undef u3b
#undef u3b2
#undef u3b5
#undef unix
#undef vax
#endif

/* end of f2c.h */

/************************************************************************/
/***************** header information from Pmatrix.h *****************/
/************************************************************************/

typedef struct Pmatrix_t *Pmatrix;           
typedef struct Pmatrix_t *Pvector;           

/*
** matrix access macroes
*/
int PMstore(Pmatrix M);    /* maximum #elts available*/
int PMMrows(Pmatrix M);    /* maximum #rows          */
int PMrows(Pmatrix M);     /* number rows stored */   
int PMcols(Pmatrix M);     /* number cols stored */   
polynomial1 *PMref1(Pmatrix,int,int);
#define PMref0(M,i,j) (PMref(M,(i+1),(j+1)))   
#define PMref(M,i,j)  (PMref1((M),i,j))  
#define PVref0(M,j) (PMref(M,1,(j+1)))   
#define PVref(M,j)  (PMref1((M),1,j))       
#define PVlength(M)  (PMcols(M))



/*
**  Constructor/Pestructors/Pisplay
*/
Pmatrix Pmatrix_new(int r, int c);
Pmatrix Pmatrix_resize(Pmatrix M, int r, int d);
void    Pmatrix_free(Pmatrix V);
Pmatrix Pmatrix_print(Pmatrix M);
#define Pvector_print(V)  (Pmatrix_print(V))
#define Pvector_new(n) (Pmatrix_new(1,n))
#define Pvector_free(V) (Pmatrix_free(V))
#define Pvector_resize(M,d) (Pmatrix_resize(M,1,d))

/*
** Arithmatic and other operations on Pmatrices
*/
Pmatrix Pmatrix_add(Pmatrix M1, Pmatrix M2, Pmatrix *M3);
#define add_Pvector(V1,V2,V3) add_Pmatrix(V1,V2,V3)
/*Pmatrix Pmatrix_mull(Pmatrix M1, Pmatrix M2, Pmatrix *M3);  */
Pmatrix Pmatrix_dot(Pmatrix M1, Pmatrix M2, Pmatrix M3);  
void PMset(Pmatrix M, int i, int j, signed int k);
signed long int PMget(Pmatrix M, int i, int j);    

int Pmatrix_gcd_reduce(Pmatrix M);
int Pmatrix_backsolve(Pmatrix N, Pmatrix S);
int Pmatrix_rref(Pmatrix N,int *det);
Pmatrix Pmatrix_resize(Pmatrix M, int r, int d);
Pmatrix Pmatrix_submat(Pmatrix R, int r, int c);
#define Pvector_copy(M) Pmatrix_copy(M)
Pmatrix Pmatrix_copy(Pmatrix M);

/* end of Pmatrix.h */

/************************************************************************/
/********** header information from the original Homotopies.h ***********/
/************************************************************************/

void print_homog(double *x,double *coord_r,double *coord_i);
int init_hom(psys P);
void Htransform(Dvector X);
void Huntransform(Dvector X);
int HPK_cont(Dvector X);

/* end, original Homotopies.h */

/************************************************************************/
/********************** declaration of fixpnf_(..) **********************/
/************************************************************************/

void print_proj_trans();
int fixpnf_(int    *n, 
	    double *y, 
	    int    *iflag, 
	    double *arcre, 
	    double *arcae, 
	    double *ansre, 
	    double *ansae, 
	    int    *trace, 
	    double *a, 
	    int    *nfe, 
	    double *arclen, 
	    double *yp, 
	    double *yold, 
	    double *ypold, 
	    double *qr, 
	    double *alpha, 
	    double *tz, 
	    int    *pivot, 
	    double *w, 
	    double *wp, 
	    double *z0, 
	    double *z1, 
	    double *sspar, 
	    double *par, 
	    int    *ipar,

	    int    tweak); /* Reduce step size, increase # of steps */


/***********************************************************************/
/******************* header information for tangnf.c *******************/
/***********************************************************************/

int tangnf_(double *rholen, 
	    double *y, 
	    double *yp, 
	    double *ypold, 
	    double *a, 
	    double *qr, 
	    double *alpha, 
	    double *tz, 
	    int    *pivot, 
	    int    *nfe, 
	    int    *n, 
	    int    *iflag, 
	    double *par, 
	    int    *ipar);

#endif /* HOMOTOPIES_H */


