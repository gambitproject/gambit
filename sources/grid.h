//
// FILE: grid.h -- Interface to grid-solve module
//
// $Id$
//

#ifndef GRID_H
#define GRID_H

#include "mixedsol.h"
#include "glist.h"
#include "algutils.h"

class GridParams : public AlgParams  {
public:
  double minLam, maxLam, delLam, delp1, delp2, tol1, tol2;
  int multi_grid;
  int powLam;
  bool fullGraph;
  gOutput *pxifile;
  
  GridParams(void);
};

int GridSolve(const NFSupport &, const GridParams &, gList<MixedSolution> &,
	      gStatus &);


#endif    // GRID_H

