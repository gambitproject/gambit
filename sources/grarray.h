//#
//# FILE: grarray.h -- Rectangular array base class
//#
//# $Id$
//#

#ifndef GRARRAY_H
#define GRARRAY_H

#include "gambitio.h"

template <class T> class gRectArray    {
  protected:
    int minrow, maxrow, mincol, maxcol;
    T **data;

    bool Check(int row, int col) const;

  public:
    gRectArray(int nrows, int ncols);
    gRectArray(int minr, int maxr, int minc, int maxc);
    gRectArray(const gRectArray<T> &);
    virtual ~gRectArray();

    gRectArray<T> &operator=(const gRectArray<T> &);

    int NumRows(void) const;
    int NumColumns(void) const;
    int MinRow(void) const;
    int MaxRow(void) const;
    int MinColumn(void) const;
    int MaxColumn(void) const;
    
    T &operator()(int r, int c);
    const T &operator()(int r, int c) const;

    void RotateUp(int lo, int hi);
    void RotateDown(int lo, int hi);

    void SwitchRows(int, int);

    virtual void Dump(gOutput &) const;
};

template <class T> gOutput &operator<<(gOutput &, const gRectArray<T> &);

#endif   // GRARRAY_H
