//#
//# FILE: tableau.h:  tableau and basis classes
//#
//# $Id$
//#

#ifndef TABLEAU_H
#define TABLEAU_H

// includes
#include "ludecomp.h"
#include "gtableau.h"

template <class T> class Tableau;
template <class T> class Basis;
template <class T> class BasisCode;

template <class T> class Tableau {
  friend class Basis<T>;
  friend class BasisCode<T>;
 protected:
  gMatrix<T> A;
  gVector<T> b;

 public:
  Tableau(void);
  Tableau(int n1, int n2);
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
  gBlock<bool> unitflag;
  gBlock<int> column;    
 public:
  BasisCode(const Tableau<T> &); // initial basis is unit matrix
  BasisCode(const Tableau<T> &, const gBlock<int> &);
    // use negative index convention
  BasisCode(const Tableau<T> &,
	    const gBlock<bool> &,
	    const gBlock<int> &
	    ); // use separate index and flag
  BasisCode<T>& operator=(const BasisCode<T>&);
  BasisCode(const BasisCode<T> &);
  ~BasisCode();

  void NewBasis();
  void NewBasis(const gBlock<int> &);
  void NewBasis(const gBlock<bool> &, const gBlock<int> &);
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
  int Label(int basisnum) const;
  int Label(int basisnum, bool &unitflag) const;
    // finds label of variable corresponding to basis index
  void BasisSelect(const gVector<T>&rowv, gVector<T> &colv) const;
    // select row elements according to basis (unit column elements are 0)
  void BasisSelect(const gVector<T>&unitv,
		   const gVector<T>&rowv,
		   gVector<T>&colv
		   ) const; // as above, but unit column elements nonzero
  void Dump(gOutput &) const;
};

template <class T> class Basis {
 private:
  //gVector<T> tmpcol; // temporary column vector, to avoid allocation

  void SolveDual();

  bool NegOK() const;
  bool ColIndex(int) const;
  bool RowIndex(int) const;
 protected:
  const Tableau<T> *tableau;
  BasisCode<T> columns;
  LUdecomp<T> B;
  gVector<T> basis;

  gVector<T> unitcost;
  gVector<T> cost;
  gVector<T> dual;
  bool costdefined;

 public:
  Basis(const Tableau<T> &);
  Basis(const Tableau<T> &, const BasisCode<T> &);
  Basis(const Tableau<T> &, const gVector<T> &cost, const BasisCode<T> &);
    // unit column cost == 0
  Basis(const Tableau<T> &,
	const gVector<T> &unitcost,
	const gVector<T> &cost,
	const BasisCode<T> &); // unit column cost given
  Basis(const Basis<T>&);
  Basis<T>& operator=(const Basis<T>&);
  virtual ~Basis();

  int MinCol() const;
  int MaxCol() const;
  int MinRow() const;
  int MaxRow() const;

  // cost-based functions
  void SetCost(const gVector<T>& ); // unit column cost := 0
  void SetCost(const gVector<T>&, const gVector<T>& ); // tableau row
  void GetCost(gVector<T>&, gVector<T>& ) const; // tableau row
  void ClearCost(); // make cost undefined
	T TotalCost() const; // cost of current solution
	T RelativeCost(int) const; // negative index convention
	T RelativeCost(bool,int) const; // flag/index convention
	void RelativeCostVector(gVector<T> &, gVector<T> &) const; // tableau row
	void DualVector(gVector<T> &) const; // column vector

  // non-cost-based functions
  void BasisVector(gVector<T> &) const; // column vector
  void SolveColumn(int, gVector<T> &) const;
  void SolveColumn(bool,int,gVector<T>&) const;
    // tableau column in terms of basis
  bool Member(int i) const;
  bool Member(int i,bool flag) const;
  int Label(int i) const;   // return variable in i'th position of basis
  int Find(int i) const;  // return basis position of variable i
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int inlabel);
  void Pivot(int outrow, bool inflag,int inlabel);
    // perform pivot operation -- outgoing is row, incoming is column
  void CompPivot(int outlabel,int inlabel);
  void SetBasis( const BasisCode<T> &); // set new basis
  void GetBasis( BasisCode<T> & ) const; // return BasisCode for current basis

  // raw basis functions
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
gOutput &operator<<(gOutput &, const Basis<double> &);
gOutput &operator<<(gOutput &, const Basis<gRational> &);
gOutput &operator<<(gOutput &, const BasisCode<double> &);
gOutput &operator<<(gOutput &, const BasisCode<gRational> &);
#elif defined __BORLANDC__
template <class T> gOutput &operator<<(gOutput &, const Basis<T> &);
template <class T> gOutput &operator<<(gOutput &, const BasisCode<T> &);
#endif   // __GNUG__, __BORLANDC__

#endif     // TABLEAU_H
