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
private:
  const ExtForm<T> &E;
  int ns1,ns2,ni1,ni2;
  T maxpay;
public:
  SFTableau(const ExtForm<T> &);
  SFTableau(SFTableau<T> &);
  virtual ~SFTableau();

  void FillTableau(const Node *n,T prob,int s1,int s2,int i1,int i2);
  int LCPPath(); // follow a path of ACBFS's from one CBFS to another
  void GetProfile(gDPVector<T> &, const gVector<T> &, 
		  const Node *n, int,int);
};

#endif     // SFTAB_H

