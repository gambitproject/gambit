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

template class SimpdivModule<double>;
template class SimpdivModule<gRational>;

