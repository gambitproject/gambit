//
// FILE: efgcsum.cc -- Constant Sum Extensive Form Game Solution Module
//
// $Id$
//

#include "efgcsum.imp"
#include "rational.h"

//---------------------------------------------------------------------------
//                        EFCSumParams: member functions
//---------------------------------------------------------------------------

CSSeqFormParams::CSSeqFormParams(gStatus &status_) 
  :  trace(0), stopAfter(0), tracefile(&gnull), status(status_)
{ }


template class CSSeqFormModule<double>;
template class CSSeqFormModule<gRational>;
template class CSSeqFormBySubgame<double>;
template class CSSeqFormBySubgame<gRational>;








