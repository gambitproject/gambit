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


#ifdef __GNUG__
template class CSSeqFormModule<double>;
template class CSSeqFormModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class CSSeqFormModule<double>;
class CSSeqFormModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__










