//
// FILE: gpolyctr.h -- Specialized versions of containers for polynomials
//
// $Id$
//

#ifndef GPOLYCTR_H
#define GPOLYCTR_H

#include "gpoly.h"

template <class T> class gPolyArray  {
  protected:
    int mindex, maxdex;
    gPoly<T> **data;

  public:
    gPolyArray(const gSpace *, const term_order *, int len = 0);
    gPolyArray(const gSpace *, const term_order *, int lo, int hi);
    gPolyArray(const gPolyArray<T> &);
    virtual ~gPolyArray();

    gPolyArray<T> &operator=(const gPolyArray<T> &);
    int Length(void) const;

    int First(void) const;
    int Last(void) const;

    const gPoly<T> &operator[](int index) const;
    gPoly<T> &operator[](int index);
    virtual void Dump(gOutput &) const;
};

template <class T> class gPolyBlock : public gPolyArray<T>   {
  private:
    int InsertAt(const gPoly<T> &t, int where);

  public:
    gPolyBlock(const gSpace *, const term_order *, int len = 0);
    gPolyBlock(const gSpace *, const term_order *, int lo, int hi);
    gPolyBlock(const gPolyBlock<T> &);
    virtual ~gPolyBlock();

    gPolyBlock<T> &operator=(const gPolyBlock<T> &);

    bool operator==(const gPolyBlock<T> &b) const;
    bool operator!=(const gPolyBlock<T> &b) const;

    int Append(const gPoly<T> &);
    int Insert(const gPoly<T> &, int);
    gPoly<T> Remove(int);

    int Find(const gPoly<T> &) const;
    int Contains(const gPoly<T> &t) const;
    void Flush(void);
};

#endif    // GPOLYCTR_H

