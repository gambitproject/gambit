//#
//# FILE: tableau.h:  tableau and basis classes
//#
//# $Id$
//#

#ifndef TABLEAU_H
#define TABLEAU_H

// includes

#include "rational.h"
#include "ludecomp.h"
#include "gtableau.h"

template <class T> class DumbTableau;
template <class T> class Tableau;
template <class T> class Basis;


template <class T> class DumbTableau {
friend class Tableau<T>;
friend class Basis<T>;
protected:
  gMatrix<T> A;
  gVector<T> b;
public:
  DumbTableau(void);
  DumbTableau(int rl, int rh, int cl, int ch);
  DumbTableau(const gMatrix<T>&, const gVector<T>&);
  ~DumbTableau();

  int MinRow(void) const;
  int MinCol(void) const;
  int MaxRow(void) const;
  int MaxCol(void) const;
  T &Set_A(int r,int c);
  T &Set_b(int r);
  const T &Get_A(int r,int c) const;
  const T &Get_b(int r) const;
  void Dump(gOutput &) const;
};


template <class T>
class Basis {
friend class Tableau<T>;
protected:
  const DumbTableau<T> *dtab;
  gBlock<int> label;    
public:
  Basis(void);
  Basis(int min, int max);
  Basis(const DumbTableau<T> &); // initial Tableau is unit matrix
  Basis(const DumbTableau<T> &, const gBlock<int> &);
      // use negative index convention
  Basis<T>& operator=(const Basis<T>&);
  Basis(const Basis<T> &);
  ~Basis();
  
  void NewBasis();
  void NewBasis(const gBlock<int> &);
  void change(int index, int label);
      // change Basis member index to label
  void swap(int,int); // switch two Basis elements
  
  bool member(int label) const;
      // return true iff label is a Basis member
  int find(int label) const;
      // finds Basis index corresponding to label number,
      // fails assert if label not in Basis
  int Label(int index) const;
      // finds label of variable corresponding to Basis index
  void BasisSelect(const gVector<T>&rowv, gVector<T> &colv) const;
      // select Basis elements according to Tableau rows and cols
  void BasisSelect(const gVector<T>&unitv,
		   const gVector<T>&rowv,
		   gVector<T>&colv
		 ) const; // as above, but unit column elements nonzero
  void Dump(gOutput &) const;
};

template <class T> class Tableau {
 private:
  //gVector<T> tmpcol; // temporary column vector, to avoid allocation

//  void SolveDual();

  bool ColIndex(int) const;
  bool RowIndex(int) const;
 protected:
  DumbTableau<T> *dtab;
  Basis<T> basis;
  LUdecomp<T> B;
  gVector<T> solution;

//  gVector<T> unitcost;
//  gVector<T> cost;
//  gVector<T> dual;
//  bool costdefined;
  bool creator;

 public:
  Tableau(DumbTableau<T> &);
  Tableau(int minr, int maxr, int minc, int maxc); // creates dtab
  Tableau(DumbTableau<T> &, const Basis<T> &);
  Tableau(DumbTableau<T> &, const gVector<T> &cost, const Basis<T> &);
    // unit column cost == 0
  Tableau(DumbTableau<T> &,
	const gVector<T> &unitcost,
	const gVector<T> &cost,
	const Basis<T> &); // unit column cost given
  Tableau(const Tableau<T>&);
  Tableau<T>& operator=(const Tableau<T>&);
  virtual ~Tableau();

  int MinCol() const;
  int MaxCol() const;
  int MinRow() const;
  int MaxRow() const;

  // cost-based functions
/*
  void SetCost(const gVector<T>& ); // unit column cost := 0
  void SetCost(const gVector<T>&, const gVector<T>& ); // DumbTableau row
  void GetCost(gVector<T>&, gVector<T>& ) const; // DumbTableau row
  void ClearCost(); // make cost undefined
  T TotalCost() const; // cost of current solution
  T RelativeCost(int) const; // negative index convention
  void RelativeCostVector(gVector<T> &, gVector<T> &) const; 
      // DumbTableau row
  void DualVector(gVector<T> &) const; // column vector
*/

  // non-cost-based functions
  void BasisVector(gVector<T> &) const; // column vector
  void SolveColumn(int, gVector<T> &) const;
    // DumbTableau column in terms of Tableau
  bool Member(int i) const;
  int Label(int i) const;   // return variable in i'th position of Tableau
  int Find(int i) const;  // return Tableau position of variable i
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int inlabel);
    // perform pivot operation -- outgoing is row, incoming is column
  void CompPivot(int outlabel,int inlabel);
  void SetBasis( const Basis<T> &); // set new Tableau
  void GetBasis( Basis<T> & ) const; // return Basis for current Tableau

  // raw Tableau functions
  void Refactor();
  void Solve(const gVector<T> &, gVector<T> &) const;
  void SolveT(const gVector<T> &, gVector<T> &) const;
  void Multiply(const gVector<T> &, gVector<T>& ) const;
  void MultiplyT(const gVector<T> &, gVector<T> &) const;

	BFS<T> GetBFS(void) const;
	bool IsNash(void) const;
  void Dump(gOutput &) const;
};


#ifdef __GNUG__
#include "rational.h"
gOutput &operator<<(gOutput &, const Tableau<double> &);
gOutput &operator<<(gOutput &, const Tableau<gRational> &);
gOutput &operator<<(gOutput &, const Basis<double> &);
gOutput &operator<<(gOutput &, const Basis<gRational> &);
#elif defined __BORLANDC__
template <class T> gOutput &operator<<(gOutput &, const Tableau<T> &);
template <class T> gOutput &operator<<(gOutput &, const Basis<T> &);
#endif   // __GNUG__, __BORLANDC__

#endif     // TABLEAU_H
