//#
//# FILE: grid.h -- Interface to grid-solve module
//#
//# $Id$
//#

#ifndef GRID_H
#define GRID_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "normal.h"

class GridParams   {
  public:
    gRational minLam, maxLam, delLam, delp, tol;
    int type, plev;
    gString outfile, errfile;

    GridParams(void);
};

class GridSolver   {
  private:
    const BaseNormalForm &nf;
    GridParams params;
    int nevals;
    gRational time;
 
  public:
    GridSolver(const BaseNormalForm &N, const GridParams &p)
      : nf(N), params(p)   { }
    GridSolver(const BaseNormalForm &N, const gString &param_file,
	       const gString &out_file);
    ~GridSolver()   { }

    int GridSolve(void);

    int NumEvals(void) const    { return nevals; }
    gRational Time(void) const  { return time; }

    GridParams &Parameters(void)   { return params; }
};

#endif    // GRID_H

