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

class Outcome    {
  private:
    gString name;
    gMap<double *> values;

  public:
    Outcome(void)   { }
    Outcome(const Outcome &);
    ~Outcome();

    Outcome &operator=(const Outcome &);
    void SetOutcomeName(const gString &s)  {name = s;}
    gString GetOutcomeName(void) const   {return name;}

    // Return a vector containing the values of outcomes for each player
    // from (1..num_players).  If a player's value has not been set, it
    // is assumed to be 0.
    gVector<double> GetOutcomeVector(int num_players) const;

    // Return the value of the outcome for player p
    double operator[](int p) const;

    // Set a player's value
    void SetOutcome(int p, double value);

    // Set players' values according to a vector
    void SetOutcome(const gVector<double> &v);
};

  
#endif    // OUTCOME_H

