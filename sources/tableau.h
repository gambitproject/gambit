//
// FILE: tableau.h:  tableau classes
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
// Tableau Stuff
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
  virtual void Refactor() = 0;
};

template <class T> class Tableau : public BaseTableau<T>{
private:
  long npivots;
protected:
  T eps1,eps2;
  gVector<T> tmpcol; // temporary column vector, to avoid allocation
  const gMatrix<T> *A;
  const gVector<T> *b;
  Basis<T> basis;
  LUdecomp<T> B;
  gVector<T> solution;
public:
      // constructors and destructors
  Tableau(const gMatrix<T> &A, const gVector<T> &b); 
  Tableau(const Tableau<T>&);
  virtual ~Tableau();
  
  Tableau<T>& operator=(const Tableau<T>&);
  
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
  
      // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col);
      // perform pivot operation -- outgoing is row, incoming is column
//  void CompPivot(int outlabel,int col);
  long NumPivots() const;
  long &NumPivots();

  // marks/unmarks label to block it from entering basis
  void Mark(int label);
  void UnMark(int label);

  // returns true if label is blocked from entering basis
  bool IsBlocked(int label) const;
  
  
      // raw Tableau functions
  void Refactor();
  void BasisVector(gVector<T> &x) const; // solve M x = (*b)
  void GetColumn( int , gVector<T> &) const;  // raw column
  void SolveColumn(int, gVector<T> &);  // column in new basis 
  void SetConst(const gVector<T> &bnew);
  void SetBasis( const Basis<T> &); // set new Tableau
  void GetBasis( Basis<T> & ) const; // return Basis for current Tableau
  
      // miscellaneous functions
  bool EqZero(T x) const;
  bool LtZero(T x) const;
  bool GtZero(T x) const;
  bool LeZero(T x) const;
  bool GeZero(T x) const;
  T Epsilon(int i = 2) const;
  bool IsFeasible();
  bool IsLexMin();
  BFS<T> GetBFS1(void) const; 
  BFS<T> GetBFS(void) const;  // used in lpsolve for some reason
  void Dump(gOutput &) const;
  void BigDump(gOutput &);
};

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
  void Solve(const gVector<T> &b, gVector<T> &x) const;  // solve M x = b
  void SolveT(const gVector<T> &c, gVector<T> &y) const;  // solve y M = c
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
