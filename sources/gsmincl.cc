//#
//# FILE: gsmincl.cc   miscellaneous GSM functions
//#
//# $Id$
//#

#include "gsmincl.h"


void DisplayID(gOutput& o, int i)
{
  switch(i)
  {
  case id_USER:
    o << "User"; break;
  case id_GOBIT: // GobitSolve
    o << "GobitSolve(Gobit)"; break;
  case id_LIAP: // LiapSolve
    o << "LiapSolve(Liap)"; break;
  case id_LEMKE: // LcpSolve
    o << "LcpSolve(Lemke)"; break;
  case id_ZSUM: // LpSolve
    o << "LpSolve(ZSum)"; break;
  case id_ENUM: // EnumMixedSolve
    o << "EnumMixedSolve(Enum)"; break;
  case id_GOBITGRID: // GobitGridSOlve
    o << "GobitGridSolve(GobitGrid)"; break;
  case id_SIMPDIV:  // SimpDivSolve
    o << "SimpDivSolve(SimpDiv)"; break;
  case id_PURENASH: // EnumPureSolve
    o << "EnumPureSolve(PureNash)"; break;
  case id_LEMKESUB: // LcpSolve
    o << "LcpSolve(LemkeBySubgame)"; break;
  case id_SEQFORM: // LcpSolve
    o << "LcpSolve(SeqForm)"; break;
  case id_SIMPDIVSUB: // SimpDivSolve
    o << "SimpDivSolve(SimpDivBySubgame)"; break;
  case id_ZSUMSUB: // LpSolve
    o << "LpSolve(ZSumBySubgame)"; break;
  case id_PURENASHSUB: // EnumPureSolve
    o << "EnumPureSolve(PureNashBySubgame)"; break;
  case id_SEQFORMSUB: // LcpSolve
    o << "LcpSolve(SeqFormBySubgame)"; break;
  case id_ELIAPSUB:
    o << "(EFLiapBySubgame)"; break;
  case id_NLIAPSUB:
    o << "(NFLiapBySubgame)"; break;
  case id_ENUMSUB:
    o << "(EnumBySubgame)"; break;
  default:
    o << "ERROR" ; break;
  }
}


void DisplayTriState(gOutput& o, int i)
{
  switch(i)
  {
  case T_DONTKNOW:
    o << "DK"; break;
  case T_YES:
    o << "Y"; break;
  case T_NO:
    o << "N"; break;
  default:
    o << "ERROR"; break;
  }
}
