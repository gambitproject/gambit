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
#include "sfstrat.h"
#include "glist.h"
#include "gblock.h"

class Sfg  {
private:
  const Efg &EF;
  const EFSupport &efsupp;
  gArray<SFSequenceSet *> *sequences;
  gNArray<gArray<gNumber> *> *SF;  // sequence form
  gArray<gRectArray<gNumber> *> *E;   // constraint matrices for sequence form.  
  gArray<int> seq;
  gPVector<int> isetFlag,isetRow;
  gArray<gList<Infoset *> > infosets;

  void MakeSequenceForm(const Node *, gNumber,gArray<int>, gArray<int>,
		      gArray<Sequence *>);
  void GetSequenceDims(const Node *);

public:
  Sfg(const EFSupport &);
  virtual ~Sfg();  

  inline int NumSequences(int pl) const {return seq[pl];}
  inline int NumInfosets(int pl) const {return infosets[pl].Length();}
  inline gArray<int> NumSequences() const {return seq;}
  int TotalNumSequences() const;
  int TotalNumInfosets() const;
  inline int NumPlayers() const {return EF.NumPlayers();}
  
  inline gArray<gNumber> Payoffs(const gArray<int> & index) const {return *((*SF)[index]);}
  /*inline*/ gNumber Payoff(const gArray<int> & index,int pl) const {return Payoffs(index)[pl];}

  gRectArray<gNumber> Constraints(int player) const {return *((*E)[player]);};
  int InfosetRowNumber(int pl, int sequence) const;
  int ActionNumber(int pl, int sequence) const;
  const Infoset* GetInfoset(int pl, int sequence) const;
  const Action*  GetAction(int pl, int sequence) const;
  const Efg &GetEfg(void) const {return EF;}
  BehavProfile<gNumber> ToBehav(const gPVector<double> &x) const;
  const Sequence* GetSequence(int pl, int seq) const {return ((*sequences)[pl])->Find(seq);}
  
  void Dump(gOutput &) const;

};

#endif    // SFG_H

