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
    virtual ~EFOutcome()   { }
  
  public:
    BaseEfg *BelongsTo(void) const   { return E; }

    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    int GetNumber(void) const   { return number; }

    virtual void Resize(int pl) = 0;
    virtual void PrintValues(gOutput &) const = 0;
};

#endif    // OUTCOME_H







