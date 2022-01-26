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

/*
  This file contains the header information that had been in globals.h,
and also the various header files that formerly resided in the Utils
subdirectory of the Pelican distribution.  We ae gathering it all together
with the intent of putting it into a black box, making sure that it has no
dependencies on the rest of the code.
*/

#define LOG_PRINT

#ifndef PELUTILS
#define PELUTILS

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#ifdef __GNUG__
#include <unistd.h>
#endif  // __GNUG__

/*
#define Pel_Out stdout
#define Pel_Err stderr
#define Pel_In  stdin
#define Pel_Log stdout
*/

/**************************************************************************/
/********************* definitions from Atom_Types.h **********************/
/**************************************************************************/

/*
**    copyright (c) 1995  Birk Huber
*/

/* 
**    The types wich can be stored in the left and right fields
**    of a node and the integers that represent them:
** 
** Note type names should all consist of 3 or 4 CAPITOL letters
**      These are not nescessarily constants the interpreter should
**      know about but constants wich the diferent modules may need
**      to interpret the data sent to them.
*/
#ifndef ATOM_INC
#define ATOM_INC 1
  #define NPTR 100  /* A pointer to a node pointer*/
  #define NODE 110  /* A node pointer */
  #define STR  120  /* a string (currently a pntr to a C-string)*/
  #define IDF  121  /* a string (currently a pntr to a C-string)*/
  #define ERR  123  /* a string (currently a pntr to a C-string)*/
  #define PROC 130
  /* Scalor Types */
  #define INT  210  /* an int (will one always fit in a (char *)*/
  #define DBL  220  /* a double (will it fit ) */
  #define CMPX 230  /* NOT SET UP YET a complex  */
  #define POLY 240  /* NOT SET UP YET a pointer to a polynomial */
  /* Vector Types */
  #define NMTX 300  /* a pntr to a mtrx of nodeptrs */
  #define IMTX 310  /* a pointer to an integer matrix (Imatrix)*/
  #define DMTX 320  /* a pntr to a mtrx of doubles */
  #define CMTX 330  /* a pntr to a mtrx of complexes */
  #define PMTX 340  /* a pntr to a mtrx of Polynomials */
  /* Convex Geometry */
  #define PNT  510    /* point in affine space */
  #define PCFG 520 /* point configuration */
  #define ASET 530   /* An Aset */
  #define CELL 540  /* An Icell (from cly_package)*/
  /* End Type Definitions */

/* some other usefull constants */
#define LEFT 20
#define RIGHT 10
#define TRUE 1
#define FALSE 0
#endif


/**************************************************************************/
/************************ definitions from Mem.h **************************/
/**************************************************************************/

/*
**    copyright (c) 1995  Birk Huber
*/

/* 
** elements of type node are pointers to an s-expression structure
** whose elements are a left (right)value and a left (right) type.
** The type is an integer, refering to the constants in XXX.h,
** and the left will be a value of the associated type,(stored as
** a char pointer). 
**
*/

#define RIGHT 10
#define LEFT 20
typedef struct node_t *node;              

typedef struct local_t {
        struct local_t *next;
        node   *val;
} *local_v;

#ifdef Mem_Internal
struct node_t {
    int LT, RT, Mark;
    union {
        void *ptr;
        int ival;
        double dval;
    } L, R;
};     
#define Node_LT(n) ((n->LT))
#define Node_L(n) ((n->L))
#define Node_RT(n) ((n->RT))
#define Node_R(n) ((n->R))
#endif

#define LOCS(n) struct local_t Loc[(n)]; int loc_ct=0;
void node_push_local(local_v loc, node * val);
#define PUSH_LOC(V) node_push_local(Loc+(loc_ct++),(&(V)));
#define POP_LOCS()  while(--loc_ct>=0) node_pop_local();

/* Allocation Functions */
int    node_init_store();   /*reserves space for the node stack */
void    node_free_store();   /*frees node stack*/
node node_new();          /*allocates a node from stack*/

char  *mem_strdup(char *); /*call strdup, with bookkeeping*/ 
void  *mem_malloc(int); /* call malloc,keep running total of calls*/ 
void   mem_free(void *); /* call free, keep running total */

/*
** node_push_local and node_pop_local maintain the 
** the garbage collector's list of starting points.
** any function which (indirectly) calls new_node must
** protect any local variables it uses, by putting their 
** addresses on the list with node_push_local(&ptr) and
** must have a cooresponding call to node_pop_local before
** returning 
*/
void   node_push_local(local_v, node *);                             
void   node_pop_local();         



/* 
** access functions 
*/
node node_set_ptr(node N,void *v, int tp, int side);
node node_set_int(node N,int v, int tp, int side);
node node_set_double(node N,double v, int tp, int side);
int node_get_int(node N, int side);
double node_get_double(node N, int side);
void *node_get_ptr(node N, int side);
int  node_get_type(node g,int side);

node Cons(node,node);
node Car(node);
node Cdr(node);
int node_atomp(node);
int node_nullp(node);
node node_print(node);


/**************************************************************************/
/************************ definitions from error.h ************************/
/**************************************************************************/

void bad_error(const char *);
void warning(const char *);


/**************************************************************************/
/******************* header information from globals.h ********************/
/**************************************************************************/

#ifndef PI
  #define PI (double)3.14159265358979323846264338328
#endif
#ifndef size_t
  #define size_t int
#endif
#define TRUE 1
#define FALSE 0
#define min(i,j) ((i) < (j) ? (i): (j))

#define USE_HOMPACK 1
#define USE_PNEWTON 2

#ifndef IN_GLOBALS_C 
  extern FILE *Pel_Err;
  extern FILE *Pel_Out;
  extern FILE *Pel_Log;
  extern FILE *Pel_In;
  extern char *Pel_LogName;
  extern char *FilePrefix;
  extern int Cont_Alg;
  extern int Show_Sys;
  extern int Show_Xpl;
#endif


/**************************************************************************/
/************************ declarations from Rand.h ************************/
/**************************************************************************/

void rand_seed(long int seedval);
int rand_int(int low, int high);
double rand_double(int low, int high);

/**************************************************************************/
/************************ definitions from Dlist.h ************************/
/**************************************************************************/

/*
** Define a list type for safe protection from garbage collection. 
**
** L is assumed to be a safe local node variable for the procedure
** main(); After initialization (with Dlist_new())
** a doubly linked list of nodes is maintained with two operations:
**
** Dlist_add(L,data) --- crates an entree to hold the node data in
**                       L, inserted at the front of the list, and
**                       returns the pointer to the list entry.
** Dlist_rem(L,node) --- takes a pointer to a list entry unlinks 
**                       it from the list L and returns the 
**                       original data node.
**  Dlist_data(node)  -- takes a list entry and returns the 
**                       original data node.
**
**
**    DList Header     |   Dlist Node               |
**                     |                            |
**  ____________       |    -------------           |
** |Node | Node |      |   | Node | Node |  next    |
** | 0   |  ----+----->|   |   |  | -----+--------> |
**  ------------ first |    ---+---------           |
**                     |       |                    !
**                     |       |link                |
**                     |    _____________           |
**                     |   | Node | Node |          |
**                     |   +---   |   |  |          |
**                     |    ----------+--           |
**                     |              |data         |
**                     |              |             |
*/

#define Dnode_link(D)  (((node)(D))->L.ptr)
#define Dnode_next(D)  (((node)D)->R.ptr)
#define Dnode_prev(D)  (((node)Dnode_link((node)(D)))->L.ptr)
#define Dnode_data(D)  (((node)Dnode_link((node)(D)))->R.ptr)

#define Dlist_new() new_node()
#define Dlist_first(L) Dnode_next(L)

void Dlist_empty(node L);

/*
** invariants: Dnode_next(Dnode_prev(pos)) ==pos 
**             for all pointers to Dlist node entrees.
**             (NOTE: Dnode_next(L) := Dlist_first(L), 
**                    where L is list header) 
**
**             Dnode_prev(Dnode_next(pos))=pos
**             for all non-zero pointers to Dlist node entrees 
**             and also for list header
**             
*/

node Dlist_add(node L, node data);

node Dlist_del(node L, node pos);

node Dlist_data(node pos);


/**************************************************************************/
/********************** declarations from Dmatrix.h ***********************/
/**************************************************************************/

 typedef struct Dmatrix_t *Dmatrix;           
 typedef struct Dmatrix_t *Dvector;           

 #ifdef DMATRIX_FAST
   struct Dmatrix_t {
       int store;
       int nrows;
       int ncols;
       double *coords;
   };
   #define DVstore(V)  (((V)->store))
   #define DVlength(V) (((V)->ncols)) 
   #define DVref1(V,j) (((V)->coords)[(j)-1]) 
   #define DVref0(V,j) (((V)->coords)[j])
   #define DVref(V,i)  DVref1(V,i)
   #define DMstore(V)  (((V)->store))
   #define DMMrows(V)  (((V)->store/(V)->ncols))
   #define DMrows(V)  ((V)->nrows) 
   #define DMcols(V) ((V)->ncols)  
   #define DMelts(V) ((V)->coords)  
   #define DMref1(V,i,j) (((V)->coords)[((i)-1)*DMcols(V)+(j)-1]) 
   #define DMref0(V,i,j) (((V)->coords)[(i)*DMcols(V)+(j)])
   #define DMref(V,i,j)  DMref1((V),i,j)
 #else 
    /*
    ** matrix access macroes
    */
   double DMstore(Dmatrix M);    /* maximum #elts available*/
   double DMMrows(Dmatrix M);    /* maximum #rows          */
   double DMrows(Dmatrix M);     /* number rows stored */   
   double DMcols(Dmatrix M);     /* number cols stored */   
   double *DMref_P(Dmatrix M,int i,int j);  /* acces starting at 1*/
   double *DMelts(Dmatrix M);        /* acces starting at 1*/
   #define DMref0(M,i,j) (*DMref_P(M,i,j))   
   #define DMref1(M,i,j) (*DMref_P(M,(i)-1,(j)-1))   
   #define DMref(M,i,j)    DMref1(M,i,j)
   
   /* 
   ** Vectors are implemented as 1xM matrices, and acces is through 
   ** usual matrix functions via macroes
   */
   #define DVstore(V)  (DMstore(V)) /* maximum #elts available */
   #define DVlength(V) (DMcols(V))  /* actual #elts stored      */
   #define DVref1(V,i)  (DMref1(V,1,i))  /* acces ith elt (starting at 1)*/
   #define DVref0(V,i)  (DMref0(V,0,i))   /* acces ith elt (starting at 0)*/
   #define DVref(V,i)   (DVref1(V,i))
 #endif
 

/*
**  Constructor/Destructors/Display
*/
Dmatrix Dmatrix_new(int r, int c);
Dmatrix Dmatrix_resize(Dmatrix M, int r, int d);
void    Dmatrix_free(Dmatrix V);
Dmatrix Dmatrix_fprint(FILE *fout,Dmatrix M);
#define Dmatrix_print(M)  (Dmatrix_fprint(stdout,M))
#define Dvector_print(V)  (Dmatrix_fprint(stdout,V))
#define Dvector_fprint(F,V)  (Dmatrix_fprint(F,V))
#define Dvector_new(n) (Dmatrix_new(1,n))
#define Dvector_free(V) (Dmatrix_free(V))

/*
** Arithmatic and other operations on Dmatrices
*/
Dmatrix Dmatrix_add(Dmatrix M1, Dmatrix M2, Dmatrix *M3);
#define add_Dvector(V1,V2,V3) add_Dmatrix(V1,V2,V3)
Dmatrix Dmatrix_mull(Dmatrix M1, Dmatrix M2, Dmatrix *M3);  
Dmatrix Dmatrix_dot(Dmatrix M1, Dmatrix M2, Dmatrix M3);  
int Dvector_dot(Dmatrix M1, Dmatrix M2);
int equal_Dmatrix(Dmatrix M1,Dmatrix M2);
void Dmatrix_GQR(Dmatrix,Dmatrix);
void Dmatrix_Solve(Dmatrix,Dmatrix,int);

/* end Dmatrix.h */

/**************************************************************************/
/********************** declarations from Imatrix.h ***********************/
/**************************************************************************/

typedef struct Imatrix_t *Imatrix;           
typedef struct Imatrix_t *Ivector;           

#define IMATRIX_FAST 1
#ifdef IMATRIX_FAST
  struct Imatrix_t {
       int store;
       int topc;
       int topr;
       int ncols;
       int *elts;
  };
  #define IVstore(V)    (((V)->store)) 
  #define IVlength(V)   (((V)->topc)) 
  #define IVref1(V,i)   (&((V)->elts[i-1]))
  #define IVref0(V,i)   (&((V)->elts[i]))
  #define IVref(V,i)    IVref1(V,i)
  #define IMstore(V)    (((V)->store))  
  #define IMMrows(V)    (((V)->store/(V)->ncols))
  #define IMrows(V)     (((V)->topr))  
  #define IMcols(V)     (((V)->topc)) 
  #define IMNcols(V)    (((V)->ncols))      
  #define IMref1(V,i,j) (&(((V))->elts[(i-1)*((V)->ncols)+j-1]))
  #define IMref0(V,i,j) (&((V)->elts[(i*(V)->ncols)+j]))
  #define IMref(V,i,j)  IMref1(V,i,j)
#else

  /*
  ** matrix access macroes
  */
  int IMstore(Imatrix M);                        /* maximum #elts available*/
  int IMMrows(Imatrix M);                        /* maximum #rows          */
  int IMrows(Imatrix M);                         /* number rows stored     */   
  int IMcols(Imatrix M);                         /* number cols stored     */   
  int* IMref1(Imatrix M,int i,int j);            /* acces starting at 1    */
  #define IMref0(M,i,j) (IMref(M,(i+1),(j+1)))   /* acces starting at 0    */
  #define IMref(M,i,j)  (IMref1((M),i,j))        /* use Mref1 by default   */

  /* 
  ** Vectors are implemented as 1xM matrices, and acces is through 
  ** usual matrix functions via macroes
  */
  #define IVstore(V)  (IMstore(V)) /* maximum #elts available */
  #define IVlength(V) (IMcols(V))  /* actual #elts stored      */
  #define IVref1(V,i)  (IMref1(V,1,i))  /* acces ith elt (starting at 1)*/
  #define IVref0(V,i)  (IMref0(V,0,i))   /* acces ith elt (starting at 0)*/
  #define IVref(V,i)   (IVref1(V,i))
#endif 

 /*
 **  Constructor/Destructors/Display
 */
 Imatrix Imatrix_new(int r, int c);
 Imatrix Imatrix_resize(Imatrix M, int r, int d);
 Imatrix Imatrix_submat(Imatrix R, int r, int c); 
 void    Imatrix_free(Imatrix V);
 Imatrix Imatrix_fprint(FILE *fout, Imatrix M);
 #define Imatrix_print(M) (Imatrix_fprint(stdout,M))
 #define Ivector_fprint(F,V) (Imatrix_fprint(F,V))
 #define Ivector_print(V)  (Imatrix_print(V))
 #define Ivector_new(n) (Imatrix_new(1,n))
 #define Ivector_free(V) (Imatrix_free(V))

 /*
 ** Arithmatic and other operations on Imatrices
 */
  Imatrix Imatrix_add(int i1, Imatrix M1,int i2, Imatrix M2, Imatrix M3);
  #define add_Ivector(V1,V2,V3) add_Imatrix(V1,V2,V3)
  Imatrix Imatrix_mul(Imatrix M1, Imatrix M2, Imatrix M3);  
  Imatrix Imatrix_dot(Imatrix M1, Imatrix M2, Imatrix M3);  
  int Ivector_dot(Imatrix M1, Imatrix M2);
  int Imatrix_equal(Imatrix M1,Imatrix M2);
  int Imatrix_rref(Imatrix M,int *);
  int Imatrix_backsolve(Imatrix M,Imatrix Sol);

  int Imatrix_order(Imatrix M1,Imatrix M2);
  int Imatrix_gcd_reduce(Imatrix M);
  int Imatrix_hermite(Imatrix S, Imatrix U);
  Imatrix Imatrix_dup(Imatrix S,Imatrix storeage);
  int Imatrix_is_zero(Imatrix S);

/* end Imatrix.h */

/**************************************************************************/
/*********************** declarations from Lists.h ************************/
/**************************************************************************/

node list_cat(node,node);
node list_push(node,node *);
node list_pop(node *);
node list_first(node);
node list_rest(node);
int  list_insert(node, node *, int (*comp) (node, node),int uniq);
node list_append(node,node *);
int  list_empty(node);
int  list_Imatrix_comp(node g1, node g2);
node list_cat(node l1,node l2);
void xpl_fprint(FILE *fout,node L);

/* end Lists.h */

/**************************************************************************/
/********************** declarations from Pconfig.h ***********************/
/**************************************************************************/

  /* General purpose implementation of points*/
  void *node_get_ptr(node N, int side);
  #define pnt_coords(g) (((Imatrix)(node_get_ptr(g,RIGHT))))
  #define pnt_label(g) ((char *)node_get_ptr(g,LEFT))
  #define pnt_dim(g)   ((IVlength((Imatrix)node_get_ptr(g,RIGHT))))

  /* I don't remember where these are used? */
  #define LABLES_ONLY 1                               
  #define COORDS_ONLY 2                                      
  #define ALL 3                                                 

  node pnt_new(char *s,Imatrix m);
  void pnt_free(node n);
  int pnt_print(node n);      /* Display point */
  int pnt_is_point(node n);   /* return TRUE if node contains a point */
  char * pnt_lable(node n);   /* return lable for point */

  /* General purpose storage of point configurations  */
  int node_get_int(node N, int side);
  #define pcfg_npts(g) ((int)node_get_int(g,LEFT))
  #define pcfg_dim(g) ((pnt_dim(Car((node)node_get_ptr(g,RIGHT)))))
  node pcfg_new();
  int pcfg_in(node,node);
  node pcfg_print(node n);
  node pcfg_print_short(node n);
  int pcfg_add(node point, node config);
  Imatrix point_cfg_coords(node n, Imatrix R);
  Imatrix pcfg_M(node n, Imatrix R);
  node pcfg_face(node pc, Imatrix normal);
  int is_normal_good(Imatrix normal, Imatrix N);
  int pcfg_remove(node, node,node);

/* end Pconfig.h */

/**************************************************************************/
/********************** declarations from Pcomplex.h **********************/
/**************************************************************************/

typedef struct FCOMPLEX {double r,i;} fcomplex; 

#define Real(c) ((c).r)
#define Imag(c) ((c).i)
fcomplex ItoC(int i);
fcomplex DtoC(double );
fcomplex Cadd(fcomplex, fcomplex);
fcomplex Csub(fcomplex, fcomplex);
fcomplex Cmul(fcomplex, fcomplex);
fcomplex Cpow(fcomplex, int);
fcomplex Complex(double, double);
fcomplex Conjg(fcomplex);
fcomplex Cdiv(fcomplex, fcomplex);
double   Cabs(fcomplex);
fcomplex Csqrt(fcomplex);
fcomplex RCmul(double,fcomplex);
fcomplex RootOfOne(int,int);
fcomplex Croot(fcomplex,int);
void printC(fcomplex);

/* end Pcomplex.h */

/**************************************************************************/
/*********************** declarations from Poly.h *************************/
/**************************************************************************/

#define RING_VAR_L 10

struct Pring_tag {
  int n;
  char **vars;
  char *def;};

typedef struct Pring_tag *Pring;
 
struct mono_tag {
        Pring R;
        int *exps;
        int def;
        int homog;
        fcomplex coef;
 
  
       struct mono_tag *next;
      
  
 
    int remaining;}; 

typedef struct mono_tag *monomial;
typedef struct mono_tag *polynomial1;
 
void ring_set_var(Pring R, int n, char *lable);
void ring_set_def(Pring R,  char *lable);      
char *ring_var(Pring R, int n);
char *ring_def(Pring R);      
 int *poly_exp(monomial, int);
 int *poly_def(monomial);
 int *poly_homog(monomial);
 int poly_deg(polynomial1);
 int poly_dim(monomial);
 int ring_dim(Pring);
 Pring poly_ring(monomial);
 fcomplex *poly_coef(monomial);
 monomial poly_next(monomial);
 Pring poly_ring(monomial);
 Pring makePR(int);   
 Pring free_Pring(Pring);
 polynomial1 makeP(Pring);  
 polynomial1 freeP(polynomial1 p);  /* frees space allocated to a polynomial1*/
 polynomial1 copyP(polynomial1 p);
 polynomial1 copyM();
 void printP(polynomial1 P);
 int orderP();
 polynomial1 ItoP(int c,Pring R);
 polynomial1 DtoP(double c, Pring R);
 polynomial1 CtoP(fcomplex c, Pring R);
 polynomial1 addPPP(polynomial1 P1, polynomial1 P2, polynomial1 P3);
 polynomial1 subPPP(polynomial1 P1, polynomial1 P2, polynomial1 P3);
 polynomial1 mulPPP(polynomial1 P1, polynomial1 P2, polynomial1 P3);
 polynomial1 mulCPP(fcomplex c, polynomial1 P1, polynomial1 P2);
 polynomial1 divCPP(fcomplex c, polynomial1 P1, polynomial1 P2);
 polynomial1 mulMPP(polynomial1 mi, polynomial1 P1, polynomial1 P2);
 polynomial1 divMPP(polynomial1 mi, polynomial1 P1, polynomial1 P2);
 polynomial1 expIPP(int x, polynomial1 P, polynomial1 P3);
 polynomial1 unliftP(polynomial1 p);
 
 polynomial1 Homogenize();
 polynomial1 Prog_Eq();
monomial poly_set_next(monomial m,monomial m2);
int orderPP(polynomial1 P1,polynomial1 P2);

/* end Poly.h */

/**************************************************************************/
/*********************** declarations from utime.h ************************/
/**************************************************************************/

int set_mark();
int read_mark(int);

/* end utime.h */

/**************************************************************************/
/************************ declarations from Aset.h ************************/
/**************************************************************************/

/* end Aset.h */

  typedef struct node_t *point;
  typedef struct node_t *aset;
  typedef struct node_t *list;
  typedef struct Imatrix_t *norm_for_Aset;

  /* creation/display/access */
  aset aset_new(int R,int D);
  aset aset_print(aset);
  aset aset_print_short(aset);
  point aset_new_pt(int N,char *lable);
  int aset_add(aset, int, point );
  int aset_r(aset);
  int aset_dim(aset);
  int aset_npts(aset);
  int aset_pnt_set(point, int, int );
  int aset_pnt_get(point, int);
  /* controll for iteration */
  node aset_start_pnt(node ptr);
  point aset_next_pnt(aset *);
  aset  aset_start_cfg(aset);
  aset  aset_next_cfg(aset *);
  /* Manipulators */
  aset aset_face(aset,norm_for_Aset);
  int aset_randlift(node A, int seed, int L, int U);
  int aset_unlift(node A);
Imatrix aset_type(node A, Imatrix T);
Imatrix aset_M(node A, Imatrix M);

/**************************************************************************/
/*********************** declarations from Types.h ************************/
/**************************************************************************/

/* node node_print(node N); APPARENTLY REDUNDANT */
node ERRND(char *);
void atom_free(node N);
node atom_new(char *val, int tp);

/* end Types.h */

/**************************************************************************/
/*********************** definitions from Dtypes.h ************************/
/**************************************************************************/

/* 
** Dtypes.h--- Definition of access to several types represented
**             by Dvectors: (bundeled to make location of fields
**             within vector transparent)
**         
**        xpnt (=<hr,hi,x1r,x1i,.....,xnr,xni, t>)
**             represents a point of CP^nxC, 
**             where 
**             xj=(xjr+I*xji) is the coordinate associated 
**                          to the jth variable
**             h=(hr+I*hi)  is the coordinate associated to the 
**                          homog param
**             t         is the deformation variable.
**
**        ptrans (=<cr,ci,c1r,c1i,.....,cnr,cni>)         
**          represents a relation  h=c+c1x1+...+cnxn
**          usually used to define a random affine chart
**
*/

typedef  Dmatrix xpnt;
typedef  Dmatrix sclvect;

/* 
** access macroes treating an xpntM as a complex matrix:
**                      and an xpnt as a complex vector:
*/
#define xpnt_new(n)        Dvector_new(2*(n)+3)
#define xpnt_n(X)          ((DVlength(X)-3)/2)
#define xpnt_free(X)       Dvector_free(X);
#define xpnt_h_set(X,C)    {DVref(X,1)=(C).r;\
                            DVref(X,2)=(C).i;}
#define xpnt_h(X)          (Complex(DVref((X),1),\
                                    DVref((X),2)))
#define xpnt_xi(X,i)       (Complex(DVref((X),2*(i)+1),\
                                    DVref((X),2*(i)+2)))
#define xpnt_xi_set(X,j,C) {DVref((X),2*(j)+1)=(C).r;\
                            DVref((X),2*(j)+2)=(C).i;}
#define xpnt_t(X)          (DVref((X),DVlength(X)))
#define xpnt_t_set(X,i)    (DVref((X),DVlength(X))=i)



/*
** initializers, and manipulators, misc
*/
void xpnt_unscale(xpnt,Dvector);
void xpnt_affine(xpnt);                 /*verified*/
void xpnt_normalize(xpnt X);

/***********************************************************************/
/****************** header information from Extremal.h *****************/
/***********************************************************************/

node pcfg_extremal(node PC); /*verified*/
node aset_extremal(node A); /*verified*/


/* end Dtypes.h */

/* header information related to what was MSD.c */

node MSD(aset Ast, Ivector T);

typedef struct Aset_Itag{
     int r;
     int n;
     int m;
     node *pts;    /*a vector of points */
     int  *store;
}*Aset_I;

typedef struct Cell_Itag {
  int r;
  int *store;
}*Cell_I;

node set_up_FaceLists(Aset_I A, Cell_I C);

int IsLower();

/***********************************************************************/
/******************* header information for RSimp.c ********************/
/***********************************************************************/

int Rsimp(Dmatrix A, Dvector b, Dvector c,
      Dvector x,Ivector basis,Ivector nonbasis,
      Dmatrix DtypesR, Dmatrix Q, Dvector t1, Dvector t2);

#endif // PELUTILS











