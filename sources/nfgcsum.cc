//
// FILE: nfgcsum.cc -- Constant Sum Game Solution Module
//
// $Id$
//

#include "nfgcsum.imp"
#include "rational.h"

//---------------------------------------------------------------------------
//                        ZSumParams: member functions
//---------------------------------------------------------------------------

ZSumParams::ZSumParams(gStatus &status_) 
  :  trace(0), stopAfter(0), tracefile(&gnull), status(status_)
{ }


#ifdef __GNUG__
template class ZSumModule<double>;
template class ZSumModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class ZSumModule<double>;
class ZSumModule<gRational>;
#endif   // __GNUG__, __BORLANDC__


