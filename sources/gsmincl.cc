//#
//# FILE: gsmincl.cc   miscellaneous GSM functions
//#
//# $Id$
//#

#include "gsmincl.h"

gString NameID(int i)
{
	switch(i)
	{
	case id_USER:
		return "User"; 
	case id_GOBIT: // GobitSolve
		return "Gobit"; 
	case id_LIAP: // LiapSolve
		return "Liap"; 
	case id_LEMKE: // LcpSolve
		return "LCP(Lemke)"; 
	case id_ZSUM: // LpSolve
		return "LP(ZSum)"; 
	case id_ENUM: // EnumMixedSolve
		return "Enum"; 
	case id_GOBITGRID: // GobitGridSOlve
		return "GobitGrid"; 
	case id_SIMPDIV:  // SimpDivSolve
		return "SimpDiv"; 
	case id_PURENASH: // EnumPureSolve
		return "PureNash"; 
	case id_LEMKESUB: // LcpSolve
		return "LCP(Lemke*)"; 
	case id_SEQFORM: // LcpSolve
		return "LCP(Seq)"; 
	case id_SIMPDIVSUB: // SimpDivSolve
		return "SimpDiv*"; 
	case id_ZSUMSUB: // LpSolve
		return "LP(ZSum*)"; 
	case id_PURENASHSUB: // EnumPureSolve
		return "PureNash*"; 
	case id_SEQFORMSUB: // LcpSolve
		return "LCP(Seq*)"; 
	case id_ELIAPSUB:
		return "EFLiap*"; 
	case id_NLIAPSUB:
		return "NFLiap*"; 
	case id_ENUMSUB:
		return "Enum*"; 
	default:
		return "ERROR" ;
	}
}


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

gString NameTriState(int i)
{
	switch(i)
	{
	case T_DONTKNOW:
		return "DK"; 
	case T_YES:
		return "Y"; 
	case T_NO:
		return "N"; 
	default:
		return "ERROR";
	}
}

void DisplayTriState(gOutput& o, int i)
{
o<<NameTriState(i);
}
