//#
//# FILE: lhtab.h -- Lemke-Howson Tableau class
//#
//# $Id$
//#

#ifndef LHTAB_H
#define LHTAB_H

// includes
#include "lemketab.h"

template <class T> class Nfg;
class NFSupport;

template <class T> class LHTableau : public LTableau<T> {
public:
  LHTableau(const Nfg<T> &, const NFSupport &);
  LHTableau(Tableau<T> &);
  virtual ~LHTableau();
};



#endif     // LHTAB_H
