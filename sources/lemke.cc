//
// FILE: lemke.cc -- Lemke-Howson module
//
// $Id$
//

#include "lemke.imp"

//---------------------------------------------------------------------------
//                        LemkeParams: member functions
//---------------------------------------------------------------------------

LemkeParams::LemkeParams(gStatus &s) 
  : dup_strat(0), trace(0), stopAfter(0), tracefile(&gnull), status(s)
{ }


#include "rational.h"

#ifdef __GNUG__
template class LemkeModule<double>;
template class LemkeModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class LemkeModule<double>;
class LemkeModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__


