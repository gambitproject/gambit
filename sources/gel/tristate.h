//
// FILE: tristate.h -- Implementation of three-state boolean logic
//
// $Id$
//

#ifndef TRISTATE_H
#define TRISTATE_H

#include "gstream.h"

typedef enum  { triFALSE = 0, triTRUE = 1, triMAYBE = 2 }  gTriState;



inline gOutput& gTriState_Output( gOutput& out, gTriState s )
{
  switch( s )
  {
  case triFALSE: out << "False"; break;
  case triTRUE:  out << "True";  break;
  case triMAYBE: out << "Maybe"; break;
  }
  return out;
}



inline gTriState TriStateAnd(gTriState x, gTriState y)
{
  if (x == triFALSE || y == triFALSE)  return triFALSE;
  if (x == triMAYBE || y == triMAYBE)  return triMAYBE;
  if (x == triTRUE && y == triTRUE)  return triTRUE;
  return triFALSE;
}

inline gTriState TriStateOr(gTriState x, gTriState y)
{
  if (x == triTRUE || y == triTRUE)   return triTRUE;
  if (x == triMAYBE || y == triMAYBE) return triMAYBE;
  if (x == triFALSE && y == triFALSE) return triFALSE;
  return triTRUE;
}

inline gTriState TriStateNot(gTriState x)
{
  if (x == triTRUE)  return triFALSE;
  if (x == triFALSE) return triTRUE;
  return triMAYBE;
}

#endif  // TRISTATE_H

