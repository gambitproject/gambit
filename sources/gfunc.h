//#
//# FILE: gfunc.h -- Abstract base classes for implementation of functions
//#
//# $Id$
//#


#ifndef GFUNC_H
#define GFUNC_H
#include "gmisc.h"
template <class T> class gVector;
template <class T> class gMatrix;

template <class T> class gFunction   {
  public:
    virtual ~gFunction()    { }
    virtual T Value(const gVector<T> &) = 0;
};


template <class T> class gC2Function : public gFunction<T>   {
  public:
    virtual ~gC2Function()   { }

    virtual bool Deriv(const gVector<T> &, gVector<T> &) = 0;
    virtual bool Hessian(const gVector<T> &, gMatrix<T> &) = 0;
};


#endif   // GFUNC_H
