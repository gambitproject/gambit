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
    gMap<double> values;

  public:
    Outcome(void)   { }
    Outcome(const Outcome &);
    ~Outcome()  { }

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

inline Outcome::Outcome(const Outcome &o)
{
  name = o.name;
  values = o.values;
}

inline Outcome &Outcome::operator=(const Outcome &o)
{
  name = o.name;
  values = o.values;
  return *this;
}

inline gVector<double>
Outcome::GetOutcomeVector(int num_players) const
{
  gVector<double> outVect(1, num_players);
  for (uint i = 1; i <= num_players; i++)
    outVect[i] = (*this)[i];
  return outVect;
}

inline double Outcome::operator[](int player) const
{
  if (values.Contains(player)) 
    return values[player];

  return 0;  // If no value found for that player
  
}

inline void Outcome::SetOutcome(int player, double value) 
{
  if (value != 0)
    values.Insert(value, player);
  else  if (values.Contains(player)) values.Remove(player);
}

inline void Outcome::SetOutcome(const gVector<double> &vals)
{
  for (uint i = 1; i <= vals.Length(); i++)
    if (vals[i] != 0) 
      values.Insert(vals[i], i);
    else if (values.Contains(i))
      values.Remove(i);
}
  
#endif    // OUTCOME_H
