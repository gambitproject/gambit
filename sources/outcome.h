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
  friend class BaseEfg;

  protected:
    int number;
    gString name;
    BaseEfg *E;

    EFOutcome(BaseEfg *e, int n) : number(n), E(e)   { }
    ~EFOutcome()   { }
  
  public:
    BaseEfg *BelongsTo(void) const   { return E; }

    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    int GetNumber(void) const   { return number; }
};

#endif    // OUTCOME_H







