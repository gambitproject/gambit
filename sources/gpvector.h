//#
//# FILE: gpvector.h -- profile vector class
//#
//# $Id$
//#

#ifndef GPVECTOR_H
#define GPVECTOR_H

#include "gvector.h"

template <class T> class gPVector : public gVector<T> {
 private:
  int sum(const gTuple<int> &V) const;
  void setindex(void);

 protected:
  T **svptr;
  gTuple<int> svlen;

  int Check(const gPVector<T> &v) const;

 public:

  // constructors

  gPVector(void);
  gPVector(const gTuple<int> &sig);
  gPVector(const gVector<T> &val, const gTuple<int> &sig);
  gPVector(const gPVector<T> &v);
  virtual ~gPVector();


  // element access operators
  T& operator()(int a, int b);
  const T& operator()(int a, int b) const;

  // extract a subvector
  gVector<T> GetRow(int row) const;
  void GetRow(int row, gVector<T> &v) const;
  void SetRow(int row, const gVector<T> &v);
  void CopyRow(int row, const gPVector<T> &v);

  // more operators

  gPVector<T>& operator=(const gPVector<T> &v);
  gPVector<T>& operator=(const gVector<T> &v);
  gPVector<T>& operator=(T c);

  gPVector<T> operator+(const gPVector<T> &v) const;
  gPVector<T>& operator+=(const gPVector<T> &v);
  gPVector<T> operator-(void) const;
  gPVector<T> operator-(const gPVector<T> &v) const;
  gPVector<T>& operator-=(const gPVector<T> &v);
  T operator*(const gPVector<T> &v) const;
  gPVector<T>& operator*=(const T c);
  gPVector<T> operator/(T c);
  gPVector<T> operator/(const gPVector<T> &v) const;
  gPVector<T>& operator/=(const gPVector<T> &v);

  int operator==(const gPVector<T> &v) const;
  int operator!=(const gPVector<T> &v) const;

  // parameter access functions
  const gTuple<int>& Lengths(void) const;

  void Dump(gOutput &) const;
};

template <class T> gOutput &operator<<(gOutput &to, const gPVector<T> &v);

#endif   //# GPVECTOR_H



