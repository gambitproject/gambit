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

#ifdef __GNUG__
template class EnumModule<double>;
template class EnumModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class EnumModule<double>;
class EnumModule<gRational>;
#endif   // __GNUG__, __BORLANDC__











