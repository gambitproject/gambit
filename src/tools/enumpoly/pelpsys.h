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
**    copyright (c) 1995  Birk Huber
*/

/*
** System.h    interface to psys class 
**             requires Dvector.
*/
#ifndef PSYS_INC
#define PSYS_INC 1
#define DMATRIX_FAST 1

#include "pelutils.h"

#define psys_outfile stdout
#define psys_logfile stdout

typedef struct psys_t *psys;

/* These declarations are added to get compilation.  Linking is another matter... */
int HPK_cont(Dvector, int tweak);
int init_hom(psys);

/* creator/destructor*/
void psys_free(psys);
psys psys_new(int,int,int);
int psys_init_mon(psys);
void psys_save_mon(psys,int);
/*
psys psys_block_init(psys);
*/
psys psys_fprint(FILE *,psys);
#define psys_print(p) (psys_fprint(psys_outfile,p))
psys psys_copy(psys);
/* access functions */
#define FORALL_POLY(sys,stmts) psys_start_poly(sys); do { stmts } while(psys_next_poly(sys)==TRUE);
#define FORALL_MONO(sys,stmts) psys_start_mon(sys); do { stmts } while(psys_next_mon(sys)==TRUE);
#define FORALL_BLOCK(sys,stmts) psys_start_block(sys); do { stmts } while(psys_next_block(sys)==TRUE);

  int psys_d(psys);
  int psys_r(psys);
  int psys_eqno(psys);
  int psys_set_eqno(psys,int);
  int psys_size(psys);
  int psys_start_poly(psys);
  int psys_start_mon(psys);
  int psys_start_block(psys);
  int psys_next_poly(psys);
  int psys_next_block(psys);
  int psys_Bstart_poly(psys,int);
  int psys_Bnext_poly(psys,int);
  int psys_next_mon(psys);
  double *psys_coef_real(psys);
  double *psys_coef_imag(psys);
  void **psys_aux(psys);
  int *psys_exp(psys,int);
  int *psys_homog(psys);
  int *psys_def(psys);
  int psys_block_size(psys);
  int *psys_block_start(psys,int);
  int psys_bkno(psys);
  int psys_eq_size(psys);
  Ivector psys_type(psys);

/* transformations */
Dvector psys_scale(psys);
psys psys_lift(psys,int);                  
psys psys_lead(psys);
psys psys_saturate(psys);
psys psys_norm_sub(psys,Ivector);
aset psys_to_aset(psys P);
psys aset_to_psys(aset A,Ivector T, int seed);
node psys_binsolve(psys sys);
node psys_hom(psys sys, node point_list, int tweak); /* tweaks homotopy */
node psys_solve(psys sys, Imatrix norm, int tweak);

/* evaluators */
Dmatrix psys_jac(psys,Dvector,Dmatrix);
double psys_abs(psys,Dvector);          
Dvector psys_eval(psys,Dvector,Dvector);
Dvector psys_moment(psys,Dvector,Dvector); /*untried*/

#endif
