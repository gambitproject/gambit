//#
//# FILE: lemketab.h:  Lemke Tableau class
//#
//# $Id$
//#

#ifndef LEMKETAB_H
#define LEMKETAB_H

// includes
#include "tableau.h"
#include "gstatus.h"

template <class T> class LTableau : public Tableau<T> {
protected:
//  T eps2;
public:
//   LTableau(void);
  LTableau(const gMatrix<T> &A, const gVector<T> &b);
  LTableau(Tableau<T> &);
  virtual ~LTableau();

  int SF_PivotIn(int i);
  int SF_ExitIndex(int i);
  int SF_LCPPath(int dup, gStatus &status_ = gstatus); // follow a path of ACBFS's from one CBFS to another
  int PivotIn(int i);
  int ExitIndex(int i);
  int LemkePath(int dup); // follow a path of ACBFS's from one CBFS to another
};

#endif     // LEMKETAB_H










