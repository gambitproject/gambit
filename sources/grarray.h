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
    int nrows, ncols;
    T **data;

    bool Check(int row, int col) const;

  public:
    gRectArray(int nrows, int ncols);
    gRectArray(const gRectArray<T> &);
    virtual ~gRectArray();

    gRectArray<T> &operator=(const gRectArray<T> &);

    int NumRows(void) const;
    int NumColumns(void) const;
    
    T &operator()(int r, int c);
    const T &operator()(int r, int c) const;

    virtual void Dump(gOutput &) const;
};

template <class T> gOutput &operator<<(gOutput &, const gRectArray<T> &);

#endif   // GRARRAY_H
