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
  case id_GOBIT:
    o << "Gobit"; break;
  case id_LIAP:
    o << "Liap"; break;
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
