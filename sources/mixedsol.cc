//#
//# FILE: mixedsol.cc -- Mixed strategy solution classes
//#
//# $Id$
//#



#include "mixedsol.imp"


gString NameNfgAlgType(NfgAlgType i)
{
  switch(i)
  {
  case NfgAlg_USER:
    return "User"; 
  case NfgAlg_GOBIT: // GobitSolve
    return "Gobit"; 
  case NfgAlg_LIAP: // LiapSolve
    return "Liap"; 
  case NfgAlg_LEMKE: // LcpSolve
    return "LCP(Lemke)"; 
  case NfgAlg_ZSUM: // LpSolve
    return "LP(ZSum)"; 
  case NfgAlg_ENUM: // EnumMixedSolve
    return "Enum"; 
  case NfgAlg_GOBITGRID: // GobitGridSOlve
    return "GobitGrid"; 
  case NfgAlg_SIMPDIV:  // SimpDivSolve
    return "SimpDiv"; 
  case NfgAlg_PURENASH: // EnumPureSolve
    return "PureNash"; 
  case NfgAlg_SEQFORM: // LcpSolve
    return "LCP(Seq)"; 
  default:
    return "ERROR" ;
  }
}


void DisplayNfgAlgType(gOutput& o, NfgAlgType i)
{
  o << NameNfgAlgType(i);
}



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"
TEMPLATE class MixedSolution<double>;
TEMPLATE class MixedSolution<gRational>;

TEMPLATE gOutput &operator<<(gOutput &, const MixedSolution<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const MixedSolution<gRational> &);
