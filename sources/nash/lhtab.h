//
// FILE: lhtab.h -- Lemke-Howson Tableau class
//
// $Id$
//

#ifndef LHTAB_H
#define LHTAB_H

// includes
#include "numerical/lemketab.h"

class Nfg;
class NFSupport;


template <class T> class LHTableau : public BaseTableau<T>{
protected:
  LTableau<T> T1,T2;
  gVector<T> tmp1,tmp2; // temporary column vectors, to avoid allocation
  gVector<T> solution;
public:
      // constructors and destructors
  LHTableau(const gMatrix<T> &A1, const gMatrix<T> &A2, 
	    const gVector<T> &b1, const gVector<T> &b2); 
  LHTableau(const LHTableau<T>&);
  virtual ~LHTableau();
  
  LHTableau<T>& operator=(const LHTableau<T>&);
  
      // information
  int MinRow() const;
  int MaxRow() const;
  int MinCol() const;
  int MaxCol() const;
  T Epsilon() const;
  
  bool Member(int i) const;
  int Label(int i) const;   // return variable in i'th position of Tableau
  int Find(int i) const;  // return Tableau position of variable i
  
      // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int inlabel);
      // perform pivot operation -- outgoing is row, incoming is column
  long NumPivots() const;
  
      // raw Tableau functions
  void Refactor();
  
      // miscellaneous functions
  BFS<T> GetBFS(void);
  void Dump(gOutput &) const;

  int PivotIn(int i);
  int ExitIndex(int i);
  int LemkePath(int dup); // follow a path of ACBFS's from one CBFS to another
};



#endif     // LHTAB_H
