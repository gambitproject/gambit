//#
//# FILE: lhtab.h:  Lemke-Howson Tableau class
//#
//# $Id$
//#

#ifndef LHTAB_H
#define LHTAB_H

// includes
#include "lemketab.h"

template <class T> class Nfg;

template <class T> class LHTableau : public LTableau<T> {
public:
  LHTableau(const Nfg<T> &);
  LHTableau(Tableau<T> &);
  virtual ~LHTableau();
};



#endif     // LHTAB_H
