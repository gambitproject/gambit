//#
//# FILE: gdpvect.h -- Doubly-partitioned vector class
//#
//# $Id$
//#

#ifndef GDPVECT_H
#define GDPVECT_H

#include "gpvector.h"

template <class T> class gDPVector : public gPVector<T>  {
  private:
    gTuple<int> Flatten(const gPVector<int> &V) const;
    int sum(int part, const gPVector<int> &v) const;
    void setindex(void);

  protected:
    T ***dvptr;
    gTuple<int> dvlen, dvidx;

  public:
    gDPVector(void);
    gDPVector(const gPVector<int> &sig);
    gDPVector(const gVector<T> &val, const gPVector<int> &sig);
    gDPVector(const gDPVector<T> &v);
    virtual ~gDPVector();

    T &operator()(int a, int b, int c);
    const T &operator()(int a, int b, int c) const;

	// extract a subvector
    void CopySubRow(int row, int col,  const gDPVector<T> &v);

    gDPVector<T> &operator=(const gDPVector<T> &v);
    gDPVector<T> &operator=(const gPVector<T> &v);
    gDPVector<T> &operator=(const gVector<T> &v);
    gDPVector<T> &operator=(T c);

    gDPVector<T> operator+(const gDPVector<T> &v) const;
    gDPVector<T> &operator+=(const gDPVector<T> &v);
    gDPVector<T> operator-(void) const;
    gDPVector<T> operator-(const gDPVector<T> &v) const;
    gDPVector<T> &operator-=(const gDPVector<T> &v);
    T operator*(const gDPVector<T> &v) const;
    gDPVector<T> &operator*=(const T &c);
    gDPVector<T> operator/(const T &c) const;
    
    int operator==(const gDPVector<T> &v) const;
    int operator!=(const gDPVector<T> &v) const;

    const gTuple<int> &DPLengths(void) const;

    void Dump(gOutput &) const;
};

template <class T> gOutput &operator<<(gOutput &to, const gDPVector<T>&v);

#endif   // GDPVECTOR_H



