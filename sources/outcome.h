//
// FILE: outcome.h -- defines Outcome class
//
// $Id$
//

#ifndef OUTCOME_H
#define OUTCOME_H

#include "gstring.h"
#include "gmap.h"
#include "gvector.h"
#include "gnumber.h"

class Outcome    {
  private:
    gString name;
    gSparseSet<gNumber> values;

  public:
    Outcome(void) : values(gNumber(0.0))  { }
    Outcome(const Outcome &);

    Outcome &operator=(const Outcome &);
    void SetOutcomeName(const gString &s)  { name = s; }
    gString GetOutcomeName(void) const   { return name; }

    // Return a vector containing the values of outcomes for each player
    // from (1..num_players).  If a player's value has not been set, it
    // is assumed to be 0.
    gVector<gNumber> GetOutcomeVector(int num_players) const;

    // Return the value of the outcome for player p
    gNumber operator[](int p) const;

    // Set a player's value
    void SetOutcome(int p, gNumber value);

    // Set players' values according to a vector
    void SetOutcome(const gVector<gNumber> &v);
};

  
#endif    // OUTCOME_H

