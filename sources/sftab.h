//#
//# FILE: sftab.h:  Sequence Form Tableau class
//#
//# $Id$
//#

#ifndef SFTAB_H
#define SFTAB_H

#include "lemketab.h"

template <class T> class ExtForm;

template <class T> class SFTableau : public LTableau<T> {
public:
  SFTableau(const ExtForm<T> &);
  SFTableau(Tableau<T> &);
  virtual ~SFTableau();

  void FillTableau(const ExtForm<T> &E, const Node *n,T prob);
};

#endif     // SFTAB_H

