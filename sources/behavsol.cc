//
// FILE: behavsol.cc -- Behavioral strategy solution classes
//
// $Id$
//

#include "behavsol.h"

gText NameEfgAlgType(EfgAlgType i)
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
  case EfgAlg_ENUMSUB: // EnumSolve
    return "Enum*"; 
  case EfgAlg_CSSEQFORM:
    return "CSSeqForm";
  case EfgAlg_POLENSUB:  // PolEnumSolve
    return "PolEnum*"; 
  case EfgAlg_POLENUM:  // PolEnumSolve
    return "PolEnum"; 
  default:
    return "ERROR" ;
  }
}

void DisplayEfgAlgType(gOutput& o, EfgAlgType i)
{
  o << NameEfgAlgType(i);
}

