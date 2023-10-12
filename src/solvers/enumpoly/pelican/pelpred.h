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

#include "pelgmatr.h"

int Can_Be_Poly(Gen_node g);
int Can_Be_Cpx(Gen_node g);
int Can_Be_Dbl(Gen_node g);
int Can_Be_Int(Gen_node g);
int Can_Be_Aset(Gen_node g);
int Can_Be_Vector(Gen_node g, int Tp);
int Can_Be_Matrix(Gen_node g, int Tp);
int Can_Be_List(Gen_node g);
int Common_Type(int,int);
int Is_Numeric(int t);
int Can_Be_Number(Gen_node g, int t);
int Gen_Can_Be_Aset(Gen_node g,int *r, int *d);
int Can_Be_List_Of(Gen_node ptr, int (*pred)(Gen_node));
int Can_Be_Xpnt(Gen_node ptr);


