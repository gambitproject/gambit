//
// FILE: sfg.h  -- Declaration of sequence form representation
//
// $Id$
//

#ifndef SFG_H
#define SFG_H

#include "efg.h"
#include "odometer.h"
#include "garray.h"
#include "grarray.h"
#include "gnarray.h"

class Sfg  {
private:
  const Efg &EF;
  const EFSupport &support;
  gNArray<gArray<gNumber> *> *SF;  // sequence form
  gArray<gRectArray<gNumber> *> *E;   // constraint matrices for sequence form.  
  gArray<int> seq,isets;
  gArray<gNumber> values;

  void MakeSequenceForm(const Node *, gNumber,gArray<int>, gArray<int>);

public:
  Sfg(const EFSupport &, const gArray<gNumber> &values);
  virtual ~Sfg();  

  inline int NumSequences(int pl) {return seq[pl];}
  inline int NumInfosets(int pl) {return isets[pl];}
  inline gArray<int> NumSequences() {return seq;}
  inline gArray<int> NumInfosets() {return isets;}
  
  inline gArray<gNumber> Payoffs(const gArray<int> & index) {return *((*SF)[index]);}
  inline gNumber Payoff(const gArray<int> & index,int pl) {return Payoffs(index)[pl];}

  gRectArray<gNumber> Constraints(int player);
  
  void Dump();

};

#endif    // SFG_H

