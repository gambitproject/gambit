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

/*-------------------------------------------------------------------
Call_Hompack.c    created 9/15/1994         last modified 9/15/1994
                             Birk Huber     (birk@math.cornell.edu
ALL RIGHTS RESERVED

  This File represents the interface between Pelican and Hompacks 
  FIXPNF path tracker. The HOMPACK routines it calls have actually 
  been translated from fortran into c with f2c and then modified a
  little so as not to require the f2c library.

  The two routines the user needs to be aware of are init_HPK 
  which takes a pelican Pvector, converts it to "tableau" format, 
  and initialies all the nescessary global variables to represent 
  the homotopy.   Call_HPK_Hom wich takes a double vector and uses 
  it as a starting point for path continuation.
--------------------------------------------------------------------*/

#ifndef CALL_HPK_H
#define CALL_HPK_H

#include "pelhomot.h"

void print_proj_trans();          

int HPK_cont(Dvector X, int tweak);

#endif  /* CALL_HPK_H */
