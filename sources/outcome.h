//#
//# FILE: outcome.h -- defines Outcome class
//#
//# $Id$
//#

#ifndef OUTCOME_H
#define OUTCOME_H

class Outcome   {
  friend class BaseExtForm;
  friend class ExtForm<double>;
  friend class ExtForm<gRational>;

  protected:
    int number;
    gString name;
    BaseExtForm *E;

    Outcome(BaseExtForm *e, int n) : number(n), E(e)   { }
    virtual ~Outcome()   { }
  
  public:
    BaseExtForm *BelongsTo(void) const   { return E; }

    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    virtual void PrintValues(gOutput &) const = 0;
};

#include "gvector.h"

template <class T> class OutcomeVector : public Outcome, public gVector<T>   {
  friend class ExtForm<T>;

  private:
    OutcomeVector(BaseExtForm *E, int n, int pl)
      : Outcome(E, n), gVector<T>(pl)  { }
    OutcomeVector(BaseExtForm *E, const OutcomeVector<T> &v)
      : Outcome(E, v.number), gVector<T>(v)   { name = v.name; }
    ~OutcomeVector()    { }

    void Resize(int pl)    { }

    void PrintValues(gOutput &f) const
      { Dump(f); }
};

#endif    // OUTCOME_H







