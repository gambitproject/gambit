//#
//# FILE: outcome.h -- defines Outcome class
//#
//# $Id$
//#

#ifndef OUTCOME_H
#define OUTCOME_H

#include "gstring.h"
#include "gvector.h"

class Outcome   {
  protected:
    int number;
    gString name;

  public:
    Outcome(int n);
    virtual ~Outcome();

    virtual DataType Type(void) const = 0;

    void SetName(const gString &);
    const gString &GetName(void) const;

    int GetNumber(void) const;
};

template <class T> class OutcomeVector : public Outcome    {
  private:
    gVector<T> *values;

  public:
    OutcomeVector(int n, int pl);
    ~OutcomeVector();

    DataType Type(void) const;

    void SetNumPlayers(int p);

    const gVector<T> &GetOutcomeVector(void) const;

    const T &operator[](int p) const;

    void SetOutcome(int p, const T &value);

    void SetOutcome(const gVector<T> &v);
};

  
#endif    // OUTCOME_H







