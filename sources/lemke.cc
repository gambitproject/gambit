//
// FILE: lemke.cc -- Lemke-Howson module
//
// $Id$
//

#include "lemke.imp"

//---------------------------------------------------------------------------
//                       LemkeParams: member functions
//---------------------------------------------------------------------------

LemkeParams::LemkeParams(gStatus &s) 
  : dup_strat(0), trace(0), stopAfter(0), tracefile(&gnull), status(s)
{ }


#include "rational.h"

template class LemkeModule<double>;
template class LemkeModule<gRational>;


