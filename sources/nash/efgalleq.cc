//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerates all Nash equilibria of a game, via polynomial equations
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

#include "game/efgensup.h"
#include "efgalleq.h"
#include "epolenum.h"

efgPolEnum::efgPolEnum(void)
  : m_stopAfter(0)
{ }

gList<BehavSolution> efgPolEnum::Solve(const EFSupport &p_support,
				       gStatus &p_status)
{
  p_status.SetProgress(0.0);
  p_status << "Step 1 of 2: Enumerating supports";
  gList<const EFSupport> supports = PossibleNashSubsupports(p_support,
							    p_status);

  p_status.SetProgress(0.0);
  p_status << "Step 2 of 2: Computing equilibria";

  gList<const EFSupport> singularSupports;
  gList<BehavSolution> solutions;

  for (int i = 1; (i <= supports.Length() &&
		   (m_stopAfter == 0 || m_stopAfter > solutions.Length()));
       i++) {
    p_status.Get();
    p_status.SetProgress((double) (i-1) / (double) supports.Length());
    long newevals = 0;
    double newtime = 0.0;
    gList<BehavSolution> newsolns;
    bool is_singular = false;

    EfgPolEnumParams params;
    params.stopAfter = 0;
    EfgPolEnum(supports[i], params, newsolns, p_status, 
	       newevals, newtime, is_singular);
    for (int j = 1; j <= newsolns.Length(); j++) {
      if (newsolns[j].IsANFNash()) {
	solutions += newsolns[j];
      }
    }

    if (is_singular) { 
      singularSupports += supports[i];
    }
  }

  return solutions;
}

