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


template class ZSumModule<double>;
template class ZSumModule<gRational>;

