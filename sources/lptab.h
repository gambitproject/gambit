//
// FILE: lptab.h:  LPTableau classes  
//
// ---------------------------------------------------------------------------
//  LPTableau Stuff (For Linear Programming code)
// ---------------------------------------------------------------------------

#ifndef LPTAB_H  
#define LPTAB_H

#include "tableau.h"

template <class T> class LPTableau : public Tableau<T> {
private:
  gVector<T> dual;
  gBlock<T> unitcost;
  gBlock<T> cost;
  gBlock<bool> UB,LB;  // does col have upper/lower bound?
  gBlock<T> ub,lb;   // upper/lower bound
  
  void SolveDual();
public:
  LPTableau(const gMatrix<T> &A, const gVector<T> &b); 
  LPTableau(const gMatrix<T> &A, const gBlock<int> &art, const gVector<T> &b); 
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

  // returns the label of the index of the last artificial variable
  int LastLabel( void );

  // select Basis elements according to Tableau rows and cols
  void BasisSelect(const gBlock<T>&rowv, gVector<T> &colv) const;

  // as above, but unit column elements nonzero
  void BasisSelect(const gBlock<T>&unitv, const gBlock<T>&rowv,
		   gVector<T>&colv) const; 

  void BigDump(gOutput &);
};

#endif     // LPTAB_H
