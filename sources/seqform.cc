//
// FILE: seqform.cc -- Sequence Form module
//
// $Id$ 
//

#include "seqform.imp"
#include "rational.h"

//---------------------------------------------------------------------------
//                        SeqFormParams: member functions
//---------------------------------------------------------------------------

SeqFormParams::SeqFormParams(gStatus &status_) 
  : trace(0), stopAfter(0), tracefile(&gnull), status(status_)
{ }

template class SeqFormModule<double>;
template class SeqFormModule<gRational>;
template class SeqFormBySubgame<double>;
template class SeqFormBySubgame<gRational>;












