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

  protected:
    int number;
    gString name;
    Efg *E;
    gPolyBlock<gNumber> payoffs;

    EFOutcome(Efg *e, int n) : number(n), E(e),
              payoffs(e->Parameters(), e->ParamOrder(), e->NumPlayers())   { }
    ~EFOutcome()   { }
  
  public:
    Efg *BelongsTo(void) const   { return E; }

    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    int GetNumber(void) const   { return number; }
};

#endif    // OUTCOME_H







