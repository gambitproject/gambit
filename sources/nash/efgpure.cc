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
#include "base/gstatus.h"
#include "efgpure.h"

#include "game/game.h"

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

  gbtGameBehavProfileIterator citer = p_game->NewBehavProfileIterator();

  int ncont = 1;
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
    gbtGamePlayer player = p_game->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      ncont *= player->GetInfoset(iset)->NumActions();
    }
  }

  int contNumber = 1;
  gbtGameBehavContingency cont = p_game->NewBehavContingency();
  try {
    // This loop used to take into account unreached information sets
    // to speed up the search.  It should do so again in the future.
    // (It currently doesn't while the game API is being reorganized).
    do  {
      p_status.Get();
      p_status.SetProgress((double) contNumber / (double) ncont);

      bool flag = true;

      
      for (int pl = 1; flag && pl <= p_game->NumPlayers(); pl++)  {
	for (int i = 1; i <= p_game->NumPlayers(); i++) {
	  for (int iset = 1; iset <= p_game->GetPlayer(i)->NumInfosets(); iset++) {
	    cont->SetAction(citer->GetAction(p_game->GetPlayer(i)->GetInfoset(iset)));
	  }
	}
	gbtGamePlayer player = p_game->GetPlayer(pl);
	gbtRational current = citer->GetPayoff(player);
	for (int iset = 1;
	     flag && iset <= player->NumInfosets();
	     iset++)  {
	  for (int act = 1; act <= player->GetInfoset(iset)->NumActions(); act++)  {
	    cont->SetAction(player->GetInfoset(iset)->GetAction(act));
	    if (cont->GetPayoff(player) > current)  {
	      flag = false;
	      break;
	    }
	  }
	}
      }

      if (flag)  {
	gbtBehavProfile<gbtRational> temp = p_game->NewBehavProfile(gbtRational(0));

	for (int i = 1; i <= p_game->NumPlayers(); i++) {
	  for (int iset = 1; iset <= p_game->GetPlayer(i)->NumInfosets(); iset++) {
	    cont->SetAction(citer->GetAction(p_game->GetPlayer(i)->GetInfoset(iset)));
	  }
	}

	for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
	  gbtGamePlayer player = p_game->GetPlayer(pl);
	  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	    gbtGameInfoset infoset = player->GetInfoset(iset);
	    for (int act = 1; act <= infoset->NumActions(); act++) {
	      if (cont->GetAction(infoset)->GetId() == act) {
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
	      citer->NextContingency());
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
