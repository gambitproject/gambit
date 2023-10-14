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

#ifndef CONV_H
#define CONV_H

#include <cstdio>
#include <cmath>

#include "pelpred.h"

polynomial1 Gen_To_Ply(Gen_node g);
fcomplex  Gen_To_Cpx(Gen_node g);
double Gen_To_Dbl(Gen_node g);
int  Gen_To_Int(Gen_node g);
Imatrix Gen_to_Imatrix(Gen_node g);
Dmatrix Gen_to_Dmatrix(Gen_node g);
Gen_node Int_To_Gen(int i);
Gen_node Dbl_To_Gen(double d);
 Gen_node Cpx_To_Gen(fcomplex c);
 Gen_node Ply_To_Gen(polynomial1 p);
 Gen_node Imatrix_to_Gen(Imatrix M);
 Gen_node Dmatrix_to_Gen(Dmatrix M);
node  Gen_to_Ivector_list(Gen_node ptr);
Gen_node Ivector_list_to_Gen(node DL);
psys Gen_to_psys(Gen_node g);
Gen_node Gen_from_psys(psys sys);
Gen_node Dvector_list_to_Gen(node DL);
Gen_node Xpl_to_Gen(node DL);
node  Gen_to_Dvector_list(Gen_node ptr);
Gen_node List_To_Aset(Gen_node g);

#endif /* CONV_H */
