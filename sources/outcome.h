//#
//# FILE: outcome.h -- defines Outcome class
//#
//# $Id$
//#

#ifndef OUTCOME_H
#define OUTCOME_H

class Outcome   {
  friend class BaseExtForm;

  protected:
    gString name;

    Outcome(void)   { }
    virtual ~Outcome()   { }
  
  public:
    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    virtual void PrintValues(gOutput &) const = 0;
};

#include "gvector.h"

template <class T> class OutcomeVector : public Outcome, public gVector<T>   {
  friend class ExtForm<T>;

  private:
    OutcomeVector(int pl) : gVector<T>(pl)  { }
    ~OutcomeVector()    { }

    void Resize(int pl)    { }

    void PrintValues(gOutput &f) const
      { Dump(f); }
};

#endif    // OUTCOME_H







