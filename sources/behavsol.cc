//#
//# FILE: behavsol.cc -- Mixed strategy solution classes
//#
//# $Id$
//#


#include "behavsol.imp"


gString NameEfgAlgType(EfgAlgType i)
{
  switch(i)
  {
  case EfgAlg_USER:
    return "User"; 
  case EfgAlg_GOBIT: // GobitSolve
    return "Gobit"; 
  case EfgAlg_LIAP: // LiapSolve
    return "Liap"; 
  case EfgAlg_PURENASH: // ?
    return "PureNash"; 
  case EfgAlg_SEQFORM: // ?
    return "SeqForm"; 
  case EfgAlg_LEMKESUB: // LcpSolve
    return "LCP(Lemke*)"; 
  case EfgAlg_SIMPDIVSUB: // SimpDivSolve
    return "SimpDiv*"; 
  case EfgAlg_ZSUMSUB: // LpSolve
    return "LP(ZSum*)"; 
  case EfgAlg_PURENASHSUB: // EnumPureSolve
    return "PureNash*"; 
  case EfgAlg_SEQFORMSUB: // LcpSolve
    return "LCP(Seq*)"; 
  case EfgAlg_ELIAPSUB:
    return "EFLiap*"; 
  case EfgAlg_NLIAPSUB:
    return "NFLiap*"; 
  case EfgAlg_ENUMSUB:
    return "Enum*"; 
  case EfgAlg_CSSEQFORM:
    return "CSSeqForm";
  default:
    return "ERROR" ;
  }
}

void DisplayEfgAlgType(gOutput& o, EfgAlgType i)
{
  o << NameEfgAlgType(i);
}

unsigned long BehavSolution<double>::MaxId=1;
unsigned long BehavSolution<gRational>::MaxId=1;

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"
TEMPLATE class BehavSolution<double>;
TEMPLATE class BehavSolution<gRational>;

TEMPLATE gOutput &operator<<(gOutput &, const BehavSolution<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const BehavSolution<gRational> &);
