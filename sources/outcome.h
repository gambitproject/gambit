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

class Outcome   {
  friend class BaseEfg;
  friend class Efg<double>;
  friend class Efg<gRational>;

  protected:
    int number;
    gString name;
    BaseEfg *E;

    Outcome(BaseEfg *e, int n) : number(n), E(e)   { }
    virtual ~Outcome()   { }
  
  public:
    BaseEfg *BelongsTo(void) const   { return E; }

    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    virtual void Resize(int pl) = 0;
    virtual void PrintValues(gOutput &) const = 0;
};

#include "gvector.h"

template <class T> class OutcomeVector : public Outcome, public gVector<T>   {
  friend class Efg<T>;

  private:
    OutcomeVector(BaseEfg *E, int n, int pl)
      : Outcome(E, n), gVector<T>(pl)  { }
    OutcomeVector(BaseEfg *E, const OutcomeVector<T> &v)
      : Outcome(E, v.number), gVector<T>(v)   { name = v.name; }
    ~OutcomeVector()    { }

    void Resize(int pl);    

    void PrintValues(gOutput &f) const
      { Dump(f); }
};

#endif    // OUTCOME_H







