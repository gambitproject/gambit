//#
//# FILE: grid.h -- Interface to grid-solve module
//#
//# $Id$
//#

#ifndef GRID_H
#define GRID_H

#include "grarray.h"
#include "gstatus.h"

template <class T> class GridParams   {
  public:
    T minLam, maxLam, delLam, delp, tol;
    int powLam, trace;
    gOutput *tracefile, *pxifile;
    gStatus &status;

    GridParams(void);
    GridParams(const GridParams<T> &p);
    GridParams(gStatus &st);
    int Ok(void) const;	// check the validity of the paramters
};

template <class T> class PayoffClass   {
  public:
    T row, col;
};
template <class T> gOutput &operator<<(gOutput &, const PayoffClass<T> &);

template <class T> class GridSolveModule  {
  private:
    const Nfg<T> &nf;
    const NFSupport &support;
    gVector<T> p, x, q_calc, y;
    const GridParams<T> &params;
    gRectArray<PayoffClass<T> > matrix;

    int CheckEqu(gVector<T> &q,T l);
    void OutputResult(gOutput &out,T l,T dist,gVector<T> &q,gVector<T> &p);
    void OutputHeader(gOutput &out);

  public:
    GridSolveModule(const Nfg<T> &, const GridParams<T> &, const NFSupport &);
    ~GridSolveModule();
    int GridSolve(void);
};

#endif    // GRID_H

