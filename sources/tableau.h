//#
//# FILE: tableau.h:  tableau and basis classes
//#
//# $Id$
//#

#ifndef TABLEAU_H
#define TABLEAU_H

// includes
#include "ludecomp.h"

template <class T> class Tableau;
template <class T> class Basis;
template <class T> class BasisCode;

template <class T> class Tableau {
  friend class Basis<T>;
  friend class BasisCode<T>;
 protected:
  const gMatrix<T> A;
  const gVector<T> b;

 public:
  Tableau(const gMatrix<T>&, const gVector<T>&);
  ~Tableau();
};


template <class T>
class BasisCode {
  friend class Basis<T>;
 private:
  bool NegOK() const;
 protected:
  const Tableau<T> *tableau;
  gTuple<bool> unitflag;
  gTuple<int> column;
 public:
  BasisCode(const Tableau<T> &); // initial basis is unit matrix
  BasisCode(const Tableau<T> &, const gTuple<int> &);
    // use negative index convention
  BasisCode(const Tableau<T> &,
	    const gTuple<bool> &,
	    const gTuple<int> &
	    ); // use separate index and flag
  BasisCode<T>& operator=(const BasisCode<T>&);
  BasisCode(const BasisCode<T> &);
  ~BasisCode();

  void NewBasis();
  void NewBasis(const gTuple<int> &);
  void NewBasis(const gTuple<bool> &, const gTuple<int> &);
    // as above ctors
  void change(int basisnum, int column);
  void change(int basisnum, bool unitflag, int column);
    // change basis member basisnum to point to given tableau column
  void swap(int,int); // switch two basis elements

  bool member(int column) const;
  bool member(bool unitflag, int column) const;
    // return true iff column is a basis member
  int find(int column) const;
  int find(bool unitflag, int column) const;
    // finds basis index corresponding to column number,
    // fails assert if column not in basis
  void FindColumn(int basisnum, int &index) const;
  void FindColumn(int basisnum, bool &unitflag, int &column) const;
    // finds column number corresponding to basis index
  void BasisSelect(const gVector<T>&rowv, gVector<T> &colv) const;
    // select row elements according to basis (unit column elements are 0)
  void BasisSelect(const gVector<T>&unitv,
		   const gVector<T>&rowv,
		   gVector<T>&colv
		   ) const; // as above, but unit column elements nonzero
};

template <class T> class Basis {
 private:
  //gVector<T> tmpcol; // temporary column vector, to avoid allocation

  void SolveDual();

  bool NegOK() const;

  int MinRow() const;
  int MaxRow() const;
  bool ColIndex(int) const;

  int MinCol() const;
  int MaxCol() const;
  bool RowIndex(int) const;
 protected:
  Tableau<T> *tableau;
  BasisCode<T> columns;
  LUdecomp<T> B;
  gVector<T> basis;

  gVector<T> unitcost;
  gVector<T> cost;
  gVector<T> dual;
  bool costdefined;

 public:
  Basis(Tableau<T> &, const BasisCode<T> &);
  Basis(Tableau<T> &, const gVector<T> &cost, const BasisCode<T> &);
    // unit column cost == 0
  Basis(Tableau<T> &,
	const gVector<T> &unitcost,
	const gVector<T> &cost,
	const BasisCode<T> &); // unit column cost given
  Basis(Basis<T>&);
  Basis<T>& operator=(Basis<T>&);
  ~Basis();

  // cost-based functions
  void SetCost(const gVector<T>& ); // unit column cost := 0
  void SetCost(const gVector<T>&, const gVector<T>& ); // tableau row
  void GetCost(gVector<T>&, gVector<T>& ) const; // tableau row
  void ClearCost(); // make cost undefined
  T TotalCost(); // cost of current solution
  T RelativeCost(int) const; // negative index convention
  T RelativeCost(bool,int) const; // flag/index convention
  void RelativeCostVector(gVector<T> &, gVector<T> &) const; // tableau row
  void DualVector(gVector<T> &) const; // column vector

  // non-cost-based functions
  void BasisVector(gVector<T> &) const; // column vector
  void SolveColumn(int, gVector<T> &) const;
  void SolveColumn(bool,int,gVector<T>&) const;
    // tableau column in terms of basis
  void Pivot(int outgoing,int incoming);
  void Pivot(int outgoing, bool inflag,int incoming);
    // perform pivot operation -- outgoing is row, incoming is column
  void SetBasis( const BasisCode<T> &); // set new basis
  void GetBasis( BasisCode<T> & ) const; // return BasisCode for current basis

  // raw basis functions
  void Solve(const gVector<T> &, gVector<T> &) const;
  void SolveT(const gVector<T> &, gVector<T> &) const;
  void Multiply(const gVector<T> &, gVector<T>& ) const;
  void MultiplyT(const gVector<T> &, gVector<T> &) const;
};

#endif     // TABLEAU_H
