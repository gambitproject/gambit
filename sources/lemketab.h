//#
//# FILE: lemketab.h:  Lemke Tableau class
//#
//# $Id$
//#

#ifndef LEMKETAB_H
#define LEMKETAB_H

// includes
#include "tableau.h"

template <class T> class LTableau : public Tableau<T> {
public:
  LTableau(int rl, int rh, int cl, int ch);
  LTableau(Tableau<T> &);
  virtual ~LTableau();

  int PivotIn(int i);
  int ExitIndex(int i) const;
  int LemkePath(int dup); // follow a path of ACBFS's from one CBFS to another
};

#endif     // LEMKETAB_H
