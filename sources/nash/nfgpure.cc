//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Algorithm to compute pure strategy equilibria on normal forms
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

#include "nfgpure.h"

#include "base/base.h"
#include "base/gnullstatus.h"
#include "game/game.h"

class gbtNfgNashEnumPure {
private:
  int m_stopAfter;

public:
  gbtNfgNashEnumPure(void) : m_stopAfter(0) { }
  virtual ~gbtNfgNashEnumPure() { }

  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  std::string GetAlgorithm(void) const { return "EnumPure"; }
  gbtList<gbtMixedProfile<gbtRational> > Solve(const gbtGame &, gbtStatus &);
};

gbtList<gbtMixedProfile<gbtRational> >
gbtNfgNashEnumPure::Solve(const gbtGame &p_game, gbtStatus &p_status)
{
  gbtList<gbtMixedProfile<gbtRational> > solutions;
  gbtGameContingencyIterator citer = p_game->NewContingencyIterator();

  int ncont = 1;
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
    ncont *= p_game->GetPlayer(pl)->NumStrategies();
  }

  int contNumber = 1;
  try {
    gbtGameContingency cont = p_game->NewContingency();
    do  {
      p_status.Get();
      p_status.SetProgress((double) contNumber / (double) ncont);

      bool flag = true;
    
      for (int pl = 1; flag && pl <= p_game->NumPlayers(); pl++)  {
	for (int i = 1; i <= p_game->NumPlayers(); i++) {
	  cont->SetStrategy(citer->GetStrategy(p_game->GetPlayer(i)));
	}
	gbtRational current = citer->GetPayoff(p_game->GetPlayer(pl));
	for (int st = 1; st <= p_game->GetPlayer(pl)->NumStrategies(); st++)  {
	  cont->SetStrategy(p_game->GetPlayer(pl)->GetStrategy(st));
	  if (cont->GetPayoff(p_game->GetPlayer(pl)) > current)  {
	    flag = false;
	    break;
	  }
	}
      }
      
      if (flag)  {
	gbtMixedProfile<gbtRational> soln = p_game->NewMixedProfile(gbtRational(0));
	for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
	  gbtGamePlayer player = p_game->GetPlayer(pl);
	  for (int st = 1; st <= player->NumStrategies(); st++) {
	    if (citer->GetStrategy(player)->GetId() == st) {
	      soln(pl, st) = 1;
	    }
	    else {
	      soln(pl, st) = 0;
	    }
	  }
	}
	solutions.Append(soln);
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

gbtList<gbtMixedProfile<gbtRational> >
gbtNashEnumPureNfg(const gbtGame &p_game, int p_stopAfter)
{
  gbtNfgNashEnumPure algorithm;
  algorithm.SetStopAfter(p_stopAfter);
  gbtNullStatus status;
  return algorithm.Solve(p_game, status);
}
