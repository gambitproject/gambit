//
// FILE: nfdommix.cc -- Elimination of dominated strategies in nfg
//
// $Id$
//

#include "base/gstream.h"
#include "game/nfg.h"
#include "game/nfgiter.h"
#include "game/nfgciter.h"
#include "game/nfstrat.h"
#include "lpsolve.h"
#include "gstatus.h"
#include "nfdom.h"

#include "nfdommix.imp"

NFSupport *NFSupport::MixedUndominated(bool strong, gPrecision precision,
				       const gArray<int> &players,
				       gOutput &tracefile,
				       gStatus &status) const
{
  NFSupport *newS = new NFSupport(*this);
  bool any = false;
  
  if(precision == precRATIONAL) {
    
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      
      any |= ComputeMixedDominated(*this, *newS, pl, strong,
				  (gRational)0, tracefile, status);
    }
  }
  else if (precision == precDOUBLE) {
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      
      any |= ComputeMixedDominated(*this, *newS, pl, strong,
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
    ret =  IsMixedDominated(S, str, strong, (gRational)0, tracefile);
  else if (precision == precDOUBLE) 
    ret =  IsMixedDominated(S, str, strong, (double)0, tracefile);
  
  return ret;
}

#include "game/mixedsol.h"

bool IsMixedDominated(const MixedProfile<gNumber> &sol,
		 bool strong, gPrecision precision, gOutput &tracefile)
{
  bool ret = false;
  int n = (sol.Game()).NumPlayers();
  int i=1;

  while (i<=n && !ret) {
    ret = IsMixedDominated(sol,i,strong, precision, tracefile);
    i++;
  }
  return ret;
}

bool IsMixedDominated(const MixedProfile<gNumber> &sol, int pl,
		 bool strong, gPrecision precision, gOutput &tracefile)
{
  bool ret = false;

  if(precision == precRATIONAL) {
    MixedProfile<gRational> p(sol.Support());
    for (int i = 1; i <= p.Length(); i++)
      p[i] = sol[i];
    ret =  IsMixedDominated(p, pl, strong, tracefile);
  }
  else if (precision == precDOUBLE) {
    MixedProfile<double> p(sol.Support());
    for (int i = 1; i <= p.Length(); i++)
      p[i] = sol[i];
    
    ret =  IsMixedDominated(p, pl, strong, tracefile);
  }
  
  return ret;
}

// Note: junk is dummy arg so the functions can be templated. 
// There is probably a cleaner way to do this.  
#ifndef __BCC55__
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

template bool 
IsMixedDominated(const MixedProfile<gRational> &pr, int pl,
		 bool strong, gOutput &tracefile);

template bool 
IsMixedDominated(const MixedProfile<double> &pr, int pl,
		 bool strong, gOutput &tracefile);
#endif  // __BCC55__




