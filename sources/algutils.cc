//
// FILE: algutils.cc -- Implementation of Algorithm specific classes
//

#include "algutils.h"

AlgParams::AlgParams(gStatus &s)
  : trace(0), accuracy(0), tracefile(&gnull), status(s)
{ }

AlgParams::~AlgParams()
{ }
