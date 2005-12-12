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

extern double PN_dt0;        
extern double PN_maxdt;
extern double PN_mindt;
extern double PN_scaledt;
extern double PN_cfac;
extern double PN_NYtol;
extern double PN_NDtol;
extern double PN_Nratio;
extern double PN_tfinal;
extern double PN_FYtol;
extern double PN_FDtol;
extern double PN_Fratio; 
extern int    PN_maxsteps;

