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

#ifdef __GNUG__
template class SeqFormModule<double>;
template class SeqFormModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class SeqFormModule<double>;
class SeqFormModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__


#ifdef __GNUG__
template class SeqFormBySubgame<double>;
template class SeqFormBySubgame<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class SeqFormBySubgame<double>;
class SeqFormBySubgame<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__













