//
// FILE: outcome.h -- defines Outcome class
//
// $Id$
//

#ifndef OUTCOME_H
#define OUTCOME_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

class EFOutcome   {
  friend class Efg;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class PureBehavProfile<double>;
  friend class PureBehavProfile<gRational>;
  friend class PureBehavProfile<gNumber>;
  protected:
    int number;
    gText name;
    Efg *E;
    gBlock<gNumber> payoffs;
    gBlock<double> double_payoffs;

    EFOutcome(Efg *e, int n) : number(n), E(e),
              payoffs(e->NumPlayers()),double_payoffs(e->NumPlayers())   { }
    ~EFOutcome()   { }
  
  public:
    Efg *BelongsTo(void) const   { return E; }

    const gText &GetName(void) const   { return name; }
    void SetName(const gText &s)       { name = s; }

    int GetNumber(void) const   { return number; }
};

#endif    // OUTCOME_H
