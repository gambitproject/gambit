//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute pure-strategy equilibria in extensive form games
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

#include "base/base.h"
#include "base/gnullstatus.h"
#include "efgpure.h"

#include "game/game.h"
#include "game/efgiter.h"
#include "game/efgciter.h"

class gbtEfgNashEnumPure {
private:
  int m_stopAfter;

public:
  gbtEfgNashEnumPure(void) : m_stopAfter(0) { }
  virtual ~gbtEfgNashEnumPure() { }

  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  std::string GetAlgorithm(void) const { return "EnumPure[EFG]"; }
  gbtList<gbtBehavProfile<gbtRational> > Solve(const gbtGame &, gbtStatus &);
};

gbtList<gbtBehavProfile<gbtRational> > 
gbtEfgNashEnumPure::Solve(const gbtGame &p_game, gbtStatus &p_status)
{
  gbtList<gbtBehavProfile<gbtRational> > solutions;

  gbtEfgContIterator citer(p_game);
  gbtPVector<gbtRational> probs(p_game->NumInfosets());

  int ncont = 1;
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
    gbtGamePlayer player = p_game->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      ncont *= player->GetInfoset(iset)->NumActions();
    }
  }

  int contNumber = 1;
  try {
    do  {
      p_status.Get();
      p_status.SetProgress((double) contNumber / (double) ncont);

      bool flag = true;
      citer.GetProfile().InfosetProbs(probs);

      gbtEfgIterator eiter(citer);
      
      for (int pl = 1; flag && pl <= p_game->NumPlayers(); pl++)  {
	gbtRational current = citer.Payoff(pl);
	for (int iset = 1;
	     flag && iset <= p_game->GetPlayer(pl)->NumInfosets();
	     iset++)  {
	  if (probs(pl, iset) == gbtRational(0))   continue;
	  for (int act = 1; act <= p_game->GetPlayer(pl)->GetInfoset(iset)->NumActions(); act++)  {
	    eiter.Next(pl, iset);
	    if (eiter.Payoff(pl) > current)  {
	      flag = false;
	      break;
	    }
	  }
	}
      }

      if (flag)  {
	gbtBehavProfile<gbtRational> temp = p_game->NewBehavProfile(gbtRational(0));
	const gbtPureBehavProfile &profile = citer.GetProfile();
	for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
	  gbtGamePlayer player = p_game->GetPlayer(pl);
	  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	    gbtGameInfoset infoset = player->GetInfoset(iset);
	    for (int act = 1; act <= infoset->NumActions(); act++) {
	      if (profile.GetAction(infoset)->GetId() == act) {
		temp(pl, iset, act) = 1;
	      }
	      else {
		temp(pl, iset, act) = 0;
	      }
	    }
	  }
	}

	solutions.Append(temp);
      }
      contNumber++;
    }  while ((m_stopAfter == 0 || solutions.Length() < m_stopAfter) &&
	      citer.NextContingency());
  }
  catch (gbtInterruptException &) {
    // catch exception; return list of computed equilibria (if any)
  }

  return solutions;
}

gbtList<gbtBehavProfile<gbtRational> >
gbtNashEnumPureEfg(const gbtGame &p_game, int p_stopAfter)
{
  gbtEfgNashEnumPure algorithm;
  algorithm.SetStopAfter(p_stopAfter);
  gbtNullStatus status;
  return algorithm.Solve(p_game, status);
}
