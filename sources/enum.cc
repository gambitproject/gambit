//
// FILE: enum.cc -- Nash Enum module
//
// $Id$
//

#include "enum.imp"

//---------------------------------------------------------------------------
//                        EnumParams: member functions
//---------------------------------------------------------------------------

EnumParams::EnumParams(gStatus &status_)
  : trace(0), stopAfter(0), tracefile(&gnull), status(status_)
{ }

#include "rational.h"

template class EnumModule<double>;
template class EnumModule<gRational>;











