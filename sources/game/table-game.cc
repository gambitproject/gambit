//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of game for table representation
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

#include "game.h"

#include "table-game.h"
#include "table-contingency.h"
#include "table-strategy-mixed.h"
#include "tree-game.h"

#include "nfgiter.h"
#include "nfgciter.h"

//======================================================================
//             Implementation of class gbtTableGameRep
//======================================================================

//----------------------------------------------------------------------
//        class gbtTableGameRep: Constructor and destructor
//----------------------------------------------------------------------

static int Product(const gbtArray<int> &p_dim)
{
  int accum = 1;
  for (int i = 1; i <= p_dim.Length(); accum *= p_dim[i++]);
  return accum;
}

gbtTableGameRep::gbtTableGameRep(const gbtArray<int> &p_dim)
  : m_label("UNTITLED"), m_dimensions(p_dim), m_players(p_dim.Length()),
    m_results(Product(p_dim)), m_efg(0)
{
  for (int pl = 1; pl <= m_players.Length(); pl++)  {
    m_players[pl] = new gbtTablePlayerRep(this, pl, p_dim[pl]);
    m_players[pl]->m_label = ToText(pl);
    for (int st = 1; st <= p_dim[pl]; st++) {
      m_players[pl]->m_infosets[1]->m_actions[st]->m_label = ToText(st);
    }
  }
  IndexStrategies();

  for (int cont = 1; cont <= m_results.Length();
       m_results[cont++] = (gbtTableOutcomeRep *) 0);
}

gbtTableGameRep::~gbtTableGameRep()
{
  for (int pl = 1; pl <= m_players.Length(); delete m_players[pl++]);
  for (int outc = 1; outc <= m_outcomes.Length(); delete m_outcomes[outc++]);
}

//----------------------------------------------------------------------
//            class gbtTableGameRep: Titles and comments
//----------------------------------------------------------------------

void gbtTableGameRep::SetLabel(const std::string &p_label) 
{ m_label = p_label; }

std::string gbtTableGameRep::GetLabel(void) const 
{ return m_label; }

void gbtTableGameRep::SetComment(const std::string &s)
{ m_comment = s; }

std::string gbtTableGameRep::GetComment(void) const
{ return m_comment; }

//----------------------------------------------------------------------
//           class gbtTableGameRep: General information
//----------------------------------------------------------------------

bool gbtTableGameRep::IsConstSum(void) const
{
  int pl, index;
  gbtRational cvalue = (gbtRational) 0;

  if (NumOutcomes() == 0)  return true;

  for (pl = 1; pl <= NumPlayers(); pl++) {
    cvalue += GetOutcome(1)->GetPayoff(GetPlayer(pl));
  }

  for (index = 2; index <= NumOutcomes(); index++)  {
    gbtRational thisvalue = (gbtRational) 0;

    for (pl = 1; pl <= NumPlayers(); pl++) {
      thisvalue += GetOutcome(index)->GetPayoff(GetPlayer(pl));
    }       

    if (thisvalue > cvalue || thisvalue < cvalue) {
      return false;
    }
  }
  
  return true;
}

gbtRational gbtTableGameRep::GetMinPayoff(void) const
{
  if (NumOutcomes() == 0 || NumPlayers() == 0)  return 0;

  gbtRational minpay = m_outcomes[1]->m_payoffs[1];

  for (int index = 1; index <= m_outcomes.Last(); index++)  {
    for (int pl = 1; pl <= NumPlayers(); pl++) {
      if (m_outcomes[index]->m_payoffs[pl] < minpay) {
	minpay = m_outcomes[index]->m_payoffs[pl];
      }
    }
  }
  return minpay;
}

gbtRational gbtTableGameRep::GetMaxPayoff(void) const
{
  if (NumOutcomes() == 0 || NumPlayers() == 0)  return 0;

  gbtRational maxpay = m_outcomes[1]->m_payoffs[1];

  for (int index = 1; index <= m_outcomes.Last(); index++)  {
    for (int pl = 1; pl <= NumPlayers(); pl++) {
      if (m_outcomes[index]->m_payoffs[pl] > maxpay) {
	maxpay = m_outcomes[index]->m_payoffs[pl];
      }
    }
  }
  return maxpay;
}

//----------------------------------------------------------------------
//               class gbtTableGameRep: Game table
//----------------------------------------------------------------------

gbtNfgContingency gbtTableGameRep::NewContingency(void) const
{ return new gbtNfgContingencyTable(const_cast<gbtTableGameRep *>(this)); }

//----------------------------------------------------------------------
//               class gbtTableGameRep: Players
//----------------------------------------------------------------------

int gbtTableGameRep::NumPlayers(void) const 
{ return m_players.Length(); }

gbtGamePlayer gbtTableGameRep::GetPlayer(int pl) const
{ return m_players[pl]; }

//----------------------------------------------------------------------
//               class gbtTableGameRep: Outcomes
//----------------------------------------------------------------------

int gbtTableGameRep::NumOutcomes(void) const
{ return m_outcomes.Length(); }

gbtGameOutcome gbtTableGameRep::GetOutcome(int p_id) const
{ return m_outcomes[p_id]; }

gbtGameOutcome gbtTableGameRep::NewOutcome(void)
{
  gbtTableOutcomeRep *outcome = 
    new gbtTableOutcomeRep(this, m_outcomes.Length() + 1);
  m_outcomes.Append(outcome);
  return outcome;
}

//----------------------------------------------------------------------
//               class gbtTableGameRep: Dimensions
//----------------------------------------------------------------------

int gbtTableGameRep::StrategyProfileLength(void) const
{
  int nprof = 0;
  for (int i = 1; i <= m_players.Length(); i++) {
    nprof += m_players[i]->m_infosets[1]->m_actions.Length();
  }
  return nprof;
}

gbtArray<int> gbtTableGameRep::NumStrategies(void) const
{ return m_dimensions; }

//----------------------------------------------------------------------
//    class gbtTableGameRep: Creating strategy profiles on the game
//----------------------------------------------------------------------

gbtMixedProfile<double> gbtTableGameRep::NewMixedProfile(double) const
{ return gbtMixedProfile<double>(new gbtTableMixedProfileRep<double>(const_cast<gbtTableGameRep *>(this))); }

gbtMixedProfile<gbtRational> 
gbtTableGameRep::NewMixedProfile(const gbtRational &) const
{ return gbtMixedProfile<gbtRational>(new gbtTableMixedProfileRep<gbtRational>(const_cast<gbtTableGameRep *>(this))); }

gbtBehavProfile<double> gbtTableGameRep::NewBehavProfile(double) const
{ throw gbtGameUndefinedException(); }

gbtBehavProfile<gbtRational>
gbtTableGameRep::NewBehavProfile(const gbtRational &) const
{ throw gbtGameUndefinedException(); }


//----------------------------------------------------------------------
//              class gbtTableGameRep: Writing data files
//----------------------------------------------------------------------

static void WriteNfg(const gbtTableGameRep *p_game,
		     gbtNfgIterator &p_iter, int pl, std::ostream &p_file)
{
  p_iter.Set(pl, 1);
  do {
    if (pl == 1) {
      for (int p = 1; p <= p_game->NumPlayers(); p++) {
	p_file << p_iter.GetPayoff(p_game->GetPlayer(p)) << ' ';
      }
      p_file << '\n';
    }
    else {
      WriteNfg(p_game, p_iter, pl - 1, p_file);
    }
  } while (p_iter.Next(pl));
} 

void gbtTableGameRep::WriteNfg(std::ostream &p_file) const
{ 
  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetLabel()) << "\" { ";

  for (int i = 1; i <= NumPlayers(); i++) {
    p_file << '"' << EscapeQuotes(GetPlayer(i)->GetLabel()) << "\" ";
  }

  p_file << "}\n\n{ ";
  
  for (int i = 1; i <= NumPlayers(); i++)   {
    gbtGamePlayer player = GetPlayer(i);
    p_file << "{ ";
    for (int j = 1; j <= player->NumStrategies(); j++)
      p_file << '"' << EscapeQuotes(player->GetStrategy(j)->GetLabel()) << "\" ";
    p_file << "}\n";
  }
  
  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  int ncont = 1;
  for (int i = 1; i <= NumPlayers(); i++)
    ncont *= GetPlayer(i)->NumStrategies();

  if (m_outcomes.Length() > 0) {
    p_file << "{\n";
    for (int outc = 1; outc <= m_outcomes.Length(); outc++)   {
      p_file << "{ \"" << EscapeQuotes(m_outcomes[outc]->m_label) << "\" ";
      for (int pl = 1; pl <= m_players.Length(); pl++)  {
	p_file << m_outcomes[outc]->m_payoffs[pl];
	if (pl < m_players.Length()) {
	  p_file << ", ";
	}
	else {
	  p_file << " }\n";
	}
      }
    }
    p_file << "}\n";
  
    for (int cont = 1; cont <= ncont; cont++)  {
      if (m_results[cont] != 0)
	p_file << m_results[cont]->m_id << ' ';
      else
	p_file << "0 ";
    }
    p_file << "\n";
  }
  else {
    gbtNfgIterator iter(const_cast<gbtTableGameRep *>(this));
    ::WriteNfg(this, iter, NumPlayers(), p_file);
  }
}

//----------------------------------------------------------------------
//              class gbtTableGameRep: Internal members
//----------------------------------------------------------------------

void gbtTableGameRep::DeleteOutcome(gbtTableOutcomeRep *p_outcome)
{
  // Remove references to the outcome from the table
  for (int i = 1; i <= m_results.Length(); i++) {
    if (m_results[i] == p_outcome) {
      m_results[i] = 0;
    }
  }

  // Remove the outcome from the list of defined outcomes
  m_outcomes.Remove(m_outcomes.Find(p_outcome));

  // If no external references, deallocate the memory;
  // otherwise, mark as "deleted"
  p_outcome->m_deleted = true;
 
  // Renumber the remaining outcomes
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_id = outc;
  }
}


void gbtTableGameRep::IndexStrategies(void)
{
  long offset = 1L;

  for (int i = 1; i <= NumPlayers(); i++)  {
    int j;
    for (j = 1; j <= GetPlayer(i)->NumStrategies(); j++)  {
      gbtTableStrategyRep *s = m_players[i]->m_infosets[1]->m_actions[j];
      s->m_id = j;
      s->m_index = (j - 1) * offset;
    }
    offset *= (j - 1);
  }
}

//-------------------------------------------------------------------------
//                           Global functions 
//-------------------------------------------------------------------------

gbtGame NewNfg(const gbtArray<int> &p_dim)
{
  return gbtGame(new gbtTableGameRep(p_dim));
}

