//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute dominated mixed strategies on normal forms
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "base/gstream.h"
#include "base/gstatus.h"
#include "numerical/lpsolve.h"
#include "nfg.h"
#include "nfgiter.h"
#include "nfgciter.h"
#include "nfstrat.h"
#include "nfdom.h"

#include "nfdommix.imp"

NFSupport NFSupport::MixedUndominated(bool strong, gPrecision precision,
				      const gArray<int> &players,
				      gOutput &tracefile,
				      gStatus &status) const
{
  NFSupport newS(*this);
  bool any = false;
  
  if (precision == precRATIONAL) {
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      ComputeMixedDominated(*this, newS, pl, strong,
			    (gRational)0, tracefile, status);
    }
  }
  else if (precision == precDOUBLE) {
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      ComputeMixedDominated(*this, newS, pl, strong,
			    (double)0, tracefile, status);
    }
  }

  tracefile << "\n";
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




