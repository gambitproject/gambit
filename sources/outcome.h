//#
//# FILE: outcome.h -- defines Outcome class
//#
//# $Id$
//#

#ifndef OUTCOME_H
#define OUTCOME_H

#include "gstring.h"
#include "gmap.h"
#include "gtuple.h"

template <class T> class Outcome    {
  private:
    gString name;
    gSparseSet<T> values;

  public:
    Outcome(void);
    Outcome(const Outcome &);

    Outcome<T> &operator=(const Outcome<T> &);
    void SetOutcomeName(const gString &s);
    const gString &GetOutcomeName(void) const;

    // Return a vector containing the values of outcomes for each player
    // from (1..num_players).  If a player's value has not been set, it
    // is assumed to be 0.
    gTuple<T> GetOutcomeVector(int num_players) const;

    // Return the value of the outcome for player p
    T operator[](int p) const;

    // Set a player's value
    void SetOutcome(int p, T value);

    // Set players' values according to a vector
    void SetOutcome(const gTuple<T> &v);
};

  
#endif    // OUTCOME_H

