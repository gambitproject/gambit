//
// FILE: tableau1.h:  new Tableau classes  
//                    To use, rename tableau1.h,cc,imp to tableau.h,cc,imp
//
// $Id$
//

#ifndef TABLEAU_H  
#define TABLEAU_H

#include "rational.h"
#include "ludecomp.h"
#include "bfs.h"

template <class T> class Tableau;
template <class T> class LPTableau;


// ---------------------------------------------------------------------------
//                          BaseTableau Stuff
// ---------------------------------------------------------------------------


template <class T> class BaseTableau {
public:
      // constructors and destructors
//  virtual ~BaseTableau();
  
      // information
  bool ColIndex(int) const;
  bool RowIndex(int) const;
  bool ValidIndex(int) const;
  virtual int MinRow() const = 0;
  virtual int MaxRow() const = 0;
  virtual int MinCol() const = 0;
  virtual int MaxCol() const = 0;

  virtual bool Member(int i) const = 0;
    // is variable i is a member of basis
  virtual int Label(int i) const = 0;   
    // return variable in i'th position of Tableau
  virtual int Find(int i) const = 0;  
    // return position of variable i
  
      // pivoting
  virtual int CanPivot(int outgoing,int incoming) = 0;
  virtual void Pivot(int outrow,int col) = 0;
      // perform pivot operation -- outgoing is row, incoming is column
  void CompPivot(int outlabel,int col);
  virtual long NumPivots() const = 0;
  
      // raw Tableau functions
  virtual  void Refactor() = 0;
};

// ---------------------------------------------------------------------------
//                           TableauInterface Stuff
// ---------------------------------------------------------------------------

template <class T> class TableauInterface : public BaseTableau<T>{
protected:
  const gMatrix<T> *A;  // should this be private?
  const gVector<T> *b;  // should this be private?
  Basis<T> basis; 
  gVector<T> solution;  // current solution vector. should this be private?
  long npivots;
  T eps1,eps2;
public:
  TableauInterface(const gMatrix<T> &A, const gVector<T> &b); 
  TableauInterface(const TableauInterface<T>&);
  virtual ~TableauInterface();

  TableauInterface<T>& operator=(const TableauInterface<T>&);

  // information

  int MinRow() const;
  int MaxRow() const;
  int MinCol() const;
  int MaxCol() const;

  const gMatrix<T> & Get_A(void) const;
  const gVector<T> & Get_b(void) const;
  
  bool Member(int i) const;
  int Label(int i) const;   // return variable in i'th position of Tableau
  int Find(int i) const;  // return Tableau position of variable i

  long NumPivots() const;
  long &NumPivots();
  
  void Mark(int label);     // marks label to block it from entering basis
  void UnMark(int label);   // unmarks label
  bool IsBlocked(int label) const;   // returns true if label is blocked
  
  void BasisVector(gVector<T> &x) const; // solve M x = (*b)
  void GetColumn( int , gVector<T> &) const;  // raw column
  void GetBasis( Basis<T> & ) const; // return Basis for current Tableau

  BFS<T> GetBFS1(void) const; 
  BFS<T> GetBFS(void) const;  // used in lpsolve for some reason
  void Dump(gOutput &) const;
  void BigDump(gOutput &);

  virtual int CanPivot(int outgoing,int incoming) = 0;
  virtual void Pivot(int outrow,int col) = 0; // pivot -- outgoing is row, incoming is column
  virtual void SolveColumn(int, gVector<T> &) = 0;  // column in new basis 
  virtual void Solve(const gVector<T> &b, gVector<T> &x) = 0;  // solve M x = b
  virtual void SolveT(const gVector<T> &c, gVector<T> &y) = 0;  // solve y M = c

  virtual void Refactor() = 0;
  virtual void SetRefactor(int) = 0;

      // miscellaneous functions
  bool EqZero(T x) const;
  bool LtZero(T x) const;
  bool GtZero(T x) const;
  bool LeZero(T x) const;
  bool GeZero(T x) const;
  T Epsilon(int i = 2) const;
};

// ---------------------------------------------------------------------------
// Tableau Stuff
// 
// We have different implementations of Tableau for double and gRational, 
// but with the same interface
// ---------------------------------------------------------------------------

// 
// Tableau<double>
//  

class Tableau<double> : public TableauInterface<double>{
private:
  LUdecomp<double> B;     // LU decomposition
  gVector<double> tmpcol; // temporary column vector, to avoid allocation

public:
      // constructors and destructors
  Tableau(const gMatrix<double> &A, const gVector<double> &b); 
  Tableau(const Tableau<double>&);
  virtual ~Tableau();
  
  Tableau<double>& operator=(const Tableau<double>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void SolveColumn(int, gVector<double> &);  // column in new basis 
  void Solve(const gVector<double> &b, gVector<double> &x);  // solve M x = b
  void SolveT(const gVector<double> &c, gVector<double> &y);  // solve y M = c
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const gVector<double> &bnew);
  void SetBasis( const Basis<double> &); // set new Tableau
  
  bool IsFeasible();
  bool IsLexMin();
};

// 
// Tableau<gRational>  currently in tableau2.h
//  

// ---------------------------------------------------------------------------
//  LPTableau Stuff (For Linear Programming code)
// ---------------------------------------------------------------------------

#include "tableau3.h"

template <class T> class LPTableau : public Tableau<T> {
private:
  gVector<T> dual;
  gVector<T> unitcost;
  gVector<T> cost;
  gBlock<bool> UB,LB;  // does col have upper/lower bound?
  gBlock<T> ub,lb;   // upper/lower bound
  
  void SolveDual();
public:
  LPTableau(const gMatrix<T> &A, const gVector<T> &b); 
  LPTableau(const LPTableau<T>&);
  virtual ~LPTableau();
  
  LPTableau<T>& operator=(const LPTableau<T>&);
  
      // cost information
  void SetCost(const gVector<T>& ); // unit column cost := 0
  void SetCost(const gVector<T>&, const gVector<T>& );
  gVector<T> GetCost() const;
  gVector<T> GetUnitCost() const;
  T TotalCost(); // cost of current solution
  T RelativeCost(int) const; // negative index convention
  void RelativeCostVector(gVector<T> &, gVector<T> &); 
  void DualVector(gVector<T> &) const; // column vector
      // Redefined functions
  void Refactor();
  void Pivot(int outrow,int col);
  void ReversePivots(gList<gArray<int> > &);
  bool IsReversePivot(int i, int j);
  void DualReversePivots(gList<gArray<int> > &);
  bool IsDualReversePivot(int i, int j);
  BFS<T> DualBFS(void) const;

  // Inserts an artificial variable in column col
  void InsertArtificial( int art, int col );

  // Appends an artificial variable
  int AppendArtificial( int art );

  // Removes an artificial variable located at col.
  void RemoveArtificial( int col );

  // returns the index of the last artificial variable
  int LastArtificial( void );

  // Removes all artificial variables
  void FlushArtificial( void );
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









