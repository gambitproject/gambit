//
// FILE: gfunc.h -- Abstract base classes for implementation of functions
//
// $Id$
//


#ifndef GFUNC_H
#define GFUNC_H

#include "base/base.h"

template <class T> class gVector;
template <class T> class gMatrix;

template <class T> class gFunction   {
  public:
    virtual ~gFunction()    { }
    virtual T Value(const gVector<T> &) const = 0;
};

template <class T> class gC1Function : public gFunction<T> {
public:
  virtual ~gC1Function() { }

  virtual bool Gradient(const gVector<T> &, gVector<T> &) const = 0;
};

class gFuncMinError : public gException {
public:
  virtual ~gFuncMinError() { }
  gText Description(void) const 
    { return "Internal error in minimization code"; }
};


#endif   // GFUNC_H
