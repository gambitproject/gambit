//
// FILE: simpdiv.cc -- Mixed strategy algorithm for Gambit
//
// $Id$
//

#include "simpdiv.imp"

SimpdivParams::SimpdivParams(gStatus &status_)
  : trace(0), stopAfter(1), nRestarts(20), leashLength(0),
    tracefile(&gnull), status(status_)
{ }

#include "rational.h"

#ifdef __GNUG__
template class SimpdivModule<double>;
template class SimpdivModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class SimpdivModule<double>;
class SimpdivModule<gRational>;
#endif   // __GNUG__, __BORLANDC__
