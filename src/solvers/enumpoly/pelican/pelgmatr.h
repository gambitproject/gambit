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

#ifndef PELICAN_GMATRIX_H
#define PELICAN_GMATRIX_H

#include "pelgntyp.h"
#include "pelpred.h"

struct Gmatrix_t {
    int store;           
    int topc;           
    int topr;          
    int ncols;     
    Gen_node *coords;
};

typedef struct Gmatrix_t *Gmatrix;           

/*
** matrix access macroes
*/
int GMstore(Gmatrix M);    /* maximum #elts available*/
int GMMrows(Gmatrix M);    /* maximum #rows          */
int GMrows(Gmatrix M);     /* number rows stored */   
int GMcols(Gmatrix M);     /* number cols stored */   
Gen_node *GMref1(Gmatrix M, int i, int j);
#define GMref0(M,i,j) (GMref(M,(i+1),(j+1)))    /* acces starting at 0
*/
#define GMref(M,i,j)  (GMref1((M),i,j))        /* use Mref1 by default
*/


/*
**  Constructor/Gestructors/Gisplay
*/
Gmatrix Gmatrix_new(int r, int c);
Gmatrix Gmatrix_resize(Gmatrix M, int r, int d);
void    Gmatrix_free(Gmatrix V);
Gmatrix Gmatrix_print(Gmatrix M);

/*
** Arithmatic and other operations on Gmatrices
*/
Gmatrix Gmatrix_Dop(Gmatrix M1, Gmatrix M2,Gen_node (*)(Gen_node));
Gmatrix Gmatrix_Sop(Gen_node g,Gmatrix M1 ,Gen_node (*)(Gen_node));
Gmatrix Gmatrix_Mop(Gmatrix M1, Gmatrix M2,Gen_node identity,
                    Gen_node (*)(Gen_node),Gen_node (*)(Gen_node));
Gmatrix Gmatrix_Transpose(Gmatrix M1);
Gen_node GMND(Gmatrix);

Gmatrix Gmatrix_copy(Gmatrix M);
Gmatrix Gen_Mtx(Gen_node g);
int Gen_Mtx_Specs(Gen_node,int *, int *, int *);

/* The following were moved from gennode.h to here on 3/26/00 - AMM */

Gen_node Link(Gen_node g1, Gen_node g2);

Gen_node XPLND(Gen_node g);

Gen_node SYSND(Gmatrix M);

#endif /* PELICAN_GMATRIX_H */
