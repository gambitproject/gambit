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

/* cly_all.h */

/*
  This file, together with cly_all.c, contains all the code originally in
the files cly* in the original distribution.
 */

#ifndef CLY_ALL_H
#define CLY_ALL_H

#define cly_out Pel_Out
#define cly_err Pel_Err

#include "pelpsys.h"

/************************************************************************/
/******************* header information from cly_int.h ******************/
/************************************************************************/

  #define MM(M,i,j) (* IMref(M,i,j))
  #define MV(M,i)   (* IVref(M,i))
  #define Hint int
  #define HMPrint(P) Imatrix_print(P)
  #define Hprint(P) printf("\d ",P)
  #define HMatrix Imatrix
  #define HMnew(i,j) ((Imatrix_new(i,j)))
  #define HVnew(j) ((Ivector_new(j)))
  #define HMfree(c) ((Ivector_free(c)))
  #define HMget(M,i,j)  ((MM(M,i,j)))
  #define HVget(M,i)  ((MV(M,i)))
  #define HLVget(M,i) ((MV(M,i)))
  #define HLMset(M,i,j,k) (((MM(M,i,j)=k)))
  #define HLVset(M,i,k) (((MV(M,i)=k)))
  #define HHMset(M,i,j,k) (((MM(M,i,j)=k)))
  #define HMresize(M,i,j) (((Imatrix_resize(M,i,j))))
  #define HMsubmat(M,i,j) (((Imatrix_submat(M,i,j))))
  #define HMfactor(M,U) (((Imatrix_hermite(M,U))))
  #define HMbacksolve(M,U) (((Imatrix_backsolve(M,U))))
  #define HMgcd_reduce(M) (((Imatrix_gcd_reduce(M))))
  #define HMdet(M,U,r) {\
           Imatrix_hermite(M,U);\
           r=1; for(i=1;i<=IMrows(M);i++) r*=(MM(M,i,i));\
          }
  #define HLeq(x,y) ((x==y))
  #define HLlt(x,y) ((x<y))
  #define HLgt(x,y) ((x>y))
  #define Hneg(x) ((x*=-1))
  #define HHlt(x,y) ((x<y))
  #define HHgt(x,y) ((x>y))
  #define HHeq(x,y) ((x==y))
  #define Hinit(x,y)  (x=y)
  #define Hfree(x) {;}
  #define HHset(x,y)  (x=y)
  #define HLadd(x,y,z) (x=y+z)
  #define HLmul(x,y,z) (x=y*z)
  #define HHadd(x,y,z) (x=y+z)
  #define HHdiv(x,y,z) (x=y/z)
  #define HHsub(x,y,z) (x=y-z)
  #define HHmul(x,y,z) (x=y*z)
  #define Hnorm_dot(dot,norm,PC) {\
      dot=0;\
      for(i=1;i<=cly_N;i++) dot+=*IVref(norm,i)*Ipnt_coord(PC,i);\
    }
  #define HtoL(x) (x)

/* end cly_int.h */

/************************************************************************/
/***************** header information from cly_globals.h ****************/
/************************************************************************/

/*
** Global  Variables
*/ 
extern int cly_Npts;   /* The number of points in the pt config*/
extern int cly_N;       /* the dimension of the cayley point config */
extern int cly_R;       /* the number of point configs */
extern int cly_Dim;      /* the dimension of the Aset */
extern int next_id;     /* unique id#s for cells (for debugging)*/
extern FILE *cly_out;

/* 
** The matrices here are used for now to avoid problems,
** they will eventually be stored with the individual cells.
*/
extern HMatrix cly_U;   /* factor matrix */
extern HMatrix cly_M;   /* an n+1xn+1 matrix   */
extern HMatrix cly_L;   /* an n+1 vector   */
extern Imatrix cly_T;   /* an R vector */
extern Hint cly_temp;
extern int cly_det;
/*
** controll parameters
**
*/
extern int cly_order;
extern int cly_lift;

/* end cly_globals.h */

/************************************************************************/
/****************** header information from cly_cells.h *****************/
/************************************************************************/

/*
** Ipnt class:  Internal representation of points
**   Ipnt is defined as a pointer to structure Ipnt_t
*/
struct Ipnt_t{
              struct node_t *point;
              char *lable;
              int idx;
              int *coords;
              int lift;
              int seen;
              struct Ipnt_t *next;
              };
typedef struct Ipnt_t *Ipnt;
/* accses macros */
#define Ipnt_coord(p,i) (((p)->coords[(i)-1]))
#define Ipnt_lift(p)    (((p)->lift))
#define Ipnt_lable(p)   (((p)->lable))
#define Ipnt_next(p)    (((p)->next))
#define Ipnt_idx(p)     (((p)->idx))
#define Ipnt_pnt(p)     (((p)->point))
#define Ipnt_seen(p)     (((p)->seen))

void Ipnt_fprint(FILE *fout, Ipnt p);
#define Ipnt_print(P) (Ipnt_fprint(stdout,P))
void Ipnt_list_fprint(FILE *fout,Ipnt p);
#define Ipnt_list_print(P) (Ipnt_list_fprint(stdout,P))
 Ipnt Ipnt_new(struct node_t *pnt,int r);
void Ipnt_free(Ipnt p);
void points_free(Ipnt p);
void lift_original_points(Ipnt p);


/*
** Cells of a triangulation are simplices, represented by a
** vector of n+1 points, and a vector of n+1 pointers,
** the ith pointer pointing to a cell of the triangulation
** which shares the facet oposite the ith vertex, (if no such
** cell exists the pointer is null).
*/
struct cell_t {
     int n;                  /* dimension of cell */
     int r;                  /* number of supports in Aset*/
     int id;                 /* identification number for cell */
     Ipnt *points;           /* points 0 through n */
     struct cell_t **ptrs;   /* the corresponding pointers */
     int freeptrs;           /* count of the number of outer facets*/
     int volume;             /* the normalized volume of the cell */
     Imatrix T;              /* mixture type for cell */
     int Tindx;              /* decimal value of t1+t2D+...+TrD^(R-1)
                                used for quick test of type order */
     HMatrix norm;         /* normal for cell */
     HMatrix U;
     HMatrix H;
     struct cell_t *next;    /* next point in subdivision */
};
typedef struct cell_t *cell;

#define cell_H(c)  (((c)->H))
#define cell_U(c)  (((c)->U))
#define cell_T(c)  (((c)->T))
#define cell_norm(c) (((c)->norm))
#define cell_n(c)   (((c)->n))
#define cell_r(c)   (((c)->r))
#define cell_id(c)   (((c)->id))
#define cell_next(c)  (((c)->next))
#define cell_ptr(c,i)  (((c)->ptrs)[i])
#define cell_fptr(c) (((c)->freeptrs))
#define cell_pnt(c,i)  (((c)->points)[i])
#define cell_point(c,i,j)   (Ipnt_coord(cell_pnt(c,i),j))
#define cell_lift(c,i)   (Ipnt_lift(cell_pnt(c,i)))
#define cell_type(c,i)   ((*IVref(c->T,i)))
#define cell_is_outer(c) ((cell_fptr(c)!=0) ? TRUE : FALSE)
#define cell_volume(c)   ((c->volume))
#define cell_tindex(c)   ((c->Tindx))

cell cell_new(int n,int r);
void cell_free(cell c);
void subdiv_free(cell c);
void cell_fprint(FILE *fout,cell c);
void subdiv_fprint(FILE *fout,cell c);
int fprint_all_volumes(FILE *fout,cell S);
#define cell_print(C) (cell_fprint(stdout,C))
#define subdiv_print(C) (subdiv_fprint(stdout,C))
#define print_all_volumes(C) (fprint_all_volumes(stdout,C))
int point_is_in(Ipnt pt, cell c);
int cell_set_volume(cell c);
int cell_type_cmp(cell c1,cell c2);
void order_subdiv(cell *Subdiv);
cell cly_subdiv_union(cell SD1, cell SD2);

/* end cly_cells.h */

/************************************************************************/
/***************** header information from cly_initial.h ****************/
/************************************************************************/

cell  cly_initial_splx(Ipnt *S1, Ipnt *S2);

/* end cly_initial.h */

/************************************************************************/
/***************** header information from cly_update.h *****************/
/************************************************************************/

 cell cly_new_cells(cell D,cell Dl, Ipnt x);
 int cly_find_lift(cell s, Ipnt pt);

/* end cly_update.h */

/************************************************************************/
/*************** header information from cly_triangulate.h **************/
/************************************************************************/

node cly_triangulate(aset A, Imatrix T, int order, int lift);

/* end cly_triangulate.h */

/************************************************************************/
/***************** header information from cly_continue.h ****************/
/************************************************************************/

psys Cayley_continue(aset A,Imatrix T,node *Sols,int seed,int tweak);

/* end cly_initial.h */

#endif /* CLY_ALL_H */
