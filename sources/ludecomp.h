//#
//# FILE: ludecomp.h -- Implementation of LU decomposition
//#
//# $Id$
//#

#ifndef LUDECOMP_H
#define LUDECOMP_H

#include "gmatrix.h"
#include "gvector.h"
#include "gblock.h"
#include "glist.h"
#include "gambitio.h"


// LUupdatelist is used only by LUdecomp
template <class T> class LUupdate {
 public:
  gBlock<bool> rowp;
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




template <class T> class LUdecomp {

 private:
 protected:
  gMatrix<T> mat;
  gBlock<int> rpp;
  int rpparity;
  gList< LUupdate<T> > updates;

  void null_rpp(); // initialize row permutation to identity
  void identity(); // fill mat with identity matrix (already factored...)

  void factor(); // factor in place
  void permute(const gVector<T>&x, gVector<T>&y) const; // y <- P(x)
  void unpermute(const gVector<T>&x, gVector<T>&y) const; // y <- P^-1(x)
  void Lsolve(const gVector<T>&b, gVector<T>&x) const; // solve: L x = b
  void LsolveT(const gVector<T>&c, gVector<T>&y) const; // solve: y L = c
  void apply(gVector<T> &) const; // apply updates factored into U (Z x)
  void applyT(gVector<T> &) const; // apply updates to row vector (y Z)
  void unapply(gVector<T> &) const; // apply inverse of updates (Z^-1 x)
  void Usolve(const gVector<T>&b, gVector<T>&x) const; // solve U x = b
  void UsolveT(const gVector<T>&c, gVector<T>&y) const; // solve y U = c

 public:
  LUdecomp(int); // create identity matrix [1..length][1..length]
  LUdecomp(int,int); // create identity matrix [a..b][a..b]
  LUdecomp(const gMatrix<T>&); // decompose matrix
  LUdecomp(const gMatrix<T>&, const gBlock<int>&); // decompose select columns
  LUdecomp(const gMatrix<T>&, const gBlock<bool>&, const gBlock<int>&);
    // decompose select unit and matrix columns

  ~LUdecomp();

  void update(int, const gVector<T>&); // replace (update) column with vector
  void refactor(const gMatrix<T>&); // factor a new matrix
  void refactor(const gMatrix<T>&, const gBlock<int>&);
    // reinitialize /w selected indices -- rows (neg) or columns (pos) 
  void solve(const gVector<T>&, gVector<T>&) const; // solve:  M x = b
  void solveT(const gVector<T>&, gVector<T>&) const; // solve:  yt M = ct
  void reconstruct(gMatrix<T>&) const; // reconstruct M from decomposition
  T Determinant() const; // compute determinant

}; // end class LUdecomp

/*
#ifdef __BORLANDC__
#include "ludecomp.imp"
#endif   // __BORLANDC__
*/

#endif     // LUDECOMP_H
