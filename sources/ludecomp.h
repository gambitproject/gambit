//#
//# FILE: ludecomp.h -- Implementation of LU decomposition
//#
//# $Id$
//#

#ifndef LUDECOMP_H
#define LUDECOMP_H

#include "gmatrix.h"
#include "gvector.h"
#include "gtuple.h"
#include "glist.h"


// LUupdatelist is used only by LUdecomp
template <class T> class LUupdate {
 public:
  LUupdate *next, *prev;
  gTuple<bool> rowp;
  gVector<T> alpha;

  LUupdate(int,int);
  ~LUupdate();
  int First();
  int Last();
}; // end class updatelist

template <class T>
LUupdate<T>::LUupdate(int,int): rowp(lo,hi), alpha(lo,hi)
{ next=prev= NULL; }

template <class T>
LUupdate<T>::~LUupdate()
{ }

template <class T>
int LUupdate<T>::First()
{ return alpha.First(); }

template <class T>
int LUupdate<T>::Last()
{ return alpha.Last(); }



template <class T> class LUdecomp {

 private:
 protected:
  gMatrix<T> mat;
  gTuple<int> rpp;
  int rpparity;
  gList<LUupdate<T>> updates;

  void null_rpp(); // initialize row permutation to identity
  void identity(); // fill mat with identity matrix (already factored...)

  void factor(); // factor in place
  void PermuteColumn(const gVector<T>&x, gVector<T>&y) const; // y <- P(x)
  void Lsolve(const gVector<T>&b, gVector<T>&x) const; // solve: L x = b
  void apply(LUupdate<T> &, gVector<T> &) const; // apply 2x2 gauss elims
  void Usolve(const gVector<T>&b, gVector<T>&x) const; // solve U x = b
 public:
  LUdecomp(int); // create identity matrix [1..length][1..length]
  LUdecomp(int,int); // create identity matrix [a..b][a..b]
  LUdecomp(const gMatrix<T>&); // decompose matrix
  LUdecomp(const gMatrix<T>&, const gTuple<int>&); // decompose select columns

  ~LUdecomp();

  void update(int, const gVector<T>&); // replace (update) column with vector
  void refactor(gMatrix<T>&); // factor a new matrix
  void refactor(gMatrix<T>&, gTuple<int>&); // reinitialize /w selected columns

  void solve(gVector<T>&, const gVector<T>&) const; // solve:  M x = b
  void solveT(gVector<T>&, const gVector<T>&) const; // solve:  yt M = ct
  T Determinant() const; // compute determinant

}; // end class LUdecomp


#ifdef __BORLANDC__
#include "ludecomp.imp"
#endif   // __BORLANDC__


#endif     // LUDECOMP_H
