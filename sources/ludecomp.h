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
#include "gambitio.h"


// LUupdatelist is used only by LUdecomp
template <class T> class LUupdate {
 public:
  gTuple<bool> rowp;
  gVector<T> alpha;

  LUupdate();
  LUupdate(int,int);
  ~LUupdate();

  // required for list class
  int operator==(const LUupdate<T>) const;
  int operator!=(const LUupdate<T>) const;

  int First();
  int Last();
}; // end class updatelist

template <class T>
gOutput& operator<<(gOutput &out, LUupdate<T> u)
{ return out<<u.alpha<<u.rowp; }

template <class T>
LUupdate<T>::LUupdate()
{ assert( 0 ); } // required for list class; currently, shouldn't be called

template <class T>
LUupdate<T>::LUupdate(int lo,int hi): rowp(lo,hi), alpha(lo,hi)
{ }

template <class T>
LUupdate<T>::~LUupdate()
{ }


template <class T>
int LUupdate<T>::operator==(const LUupdate<T> u) const
{ return rowp==u.rowp && alpha==u.alpha; }

template <class T>
int LUupdate<T>::operator!=(const LUupdate<T> u) const
{ return rowp!=u.rowp || alpha!=u.alpha; }

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
  gList< LUupdate<T> > updates;

  void null_rpp(); // initialize row permutation to identity
  void identity(); // fill mat with identity matrix (already factored...)

  void factor(); // factor in place
  void permute(const gVector<T>&x, gVector<T>&y) const; // y <- P(x)
  void unpermute(const gVector<T>&x, gVector<T>&y) const; // y <- P^-1(x)
  void Lsolve(const gVector<T>&b, gVector<T>&x) const; // solve: L x = b
  void apply(gVector<T> &) const; // apply updates factored into U
  void unapply(gVector<T> &) const; // apply inverse of updates factored into U
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

  void solve(const gVector<T>&, gVector<T>&) const; // solve:  M x = b
  void solveT(const gVector<T>&, gVector<T>&) const; // solve:  yt M = ct
  void reconstruct(gMatrix<T>&) const; // reconstruct M from decomposition
  T Determinant() const; // compute determinant

}; // end class LUdecomp


#ifdef __BORLANDC__
#include "ludecomp.imp"
#endif   // __BORLANDC__


#endif     // LUDECOMP_H
