//
// FILE: nfdommix.cc -- Elimination of dominated strategies in nfg
//
// $Id$
//

#include "gstream.h"
#include "nfg.h"
#include "nfgiter.h"
#include "nfgciter.h"
#include "nfstrat.h"
#include "lpsolve.h"
#include "gstatus.h"

#include "nfdommix.imp"

NFSupport *ComputeMixedDominated(NFSupport &S,
				 bool strong, gPrecision precision,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status)
{
  NFSupport *newS = new NFSupport(S);
  bool any = false;
  
  if(precision == precRATIONAL) {
    
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      
      any |= ComputeMixedDominated<gRational>(S, *newS, pl, strong,
				  (gRational)0, tracefile, status);
    }
  }
  else if (precision == precDOUBLE) {
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      
      any |= ComputeMixedDominated<double>(S, *newS, pl, strong,
				  (double)0, tracefile, status);
    }
  }


  tracefile << "\n";
  if (!any)  {
    delete newS;
    return 0;
  }
  
  return newS;
}

bool IsMixedDominated(const NFSupport &S,Strategy *str,
			   bool strong, gPrecision precision,
			   gOutput &tracefile)
{
  bool ret = false;
  if(precision == precRATIONAL) 
    ret =  IsMixedDominated<gRational>(S, str, strong, (gRational)0, tracefile);
  else if (precision == precDOUBLE) 
    ret =  IsMixedDominated<double>(S, str, strong, (double)0, tracefile);
  
  return ret;
}

// Note: junk is dummy arg so the functions can be templated. 
// There is probably a cleaner way to do this.  
template bool 
ComputeMixedDominated(const NFSupport &S, NFSupport &R,int pl, bool strong, 
		      gRational junk, gOutput &tracefile, gStatus &status);
template bool 
ComputeMixedDominated(const NFSupport &S, NFSupport &R,int pl, bool strong, 
		      double junk, gOutput &tracefile, gStatus &status);

template bool
IsMixedDominated(const NFSupport &S,Strategy *str,
		 bool strong, gRational junk, gOutput &tracefile);
template bool
IsMixedDominated(const NFSupport &S,Strategy *str,
		 bool strong, double junk, gOutput &tracefile);




