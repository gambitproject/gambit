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

#ifndef PROC_H
#define PROC_H

#include "pelsymbl.h"
#include "pelconv.h"

/* Actual signatures called by outside code */
Gen_node Set_Ring(Gen_node g);
Gen_node PROC_MUL(Gen_node g);
Gen_node PROC_ADD(Gen_node g);
Gen_node PROC_SUB(Gen_node g);

/* Signature declarations in C style, in the original header file(s) */
void print_Proc();
extern Gen_node PROC_ADD();
extern Gen_node PROC_SUB();
extern Gen_node PROC_SUBM();
extern Gen_node PROC_MUL();
extern Gen_node PROC_DIV();
extern Gen_node PROC_EXP();
extern Gen_node PROC_SET();
extern Gen_node PROC_LAC();
extern Gen_node PROC_MAC();
extern Gen_node PROC_LUP();
extern Gen_node PROC_QUO();
extern Gen_node PROC_EXIT();
extern Gen_node PROC_MAT();
extern Gen_node Set_Ring();
extern Gen_node PROC_FOR();

#endif /* PROC_H */
