//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gametable.cc
// Implementation of strategic game representation
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

#include <iostream>
#include <sstream>

#include "libgambit.h"
#include "gametable.h"

namespace Gambit {

//========================================================================
//                  class TablePureStrategyProfileRep
//========================================================================

class TablePureStrategyProfileRep : public PureStrategyProfileRep {
protected:
  long m_index;

  virtual PureStrategyProfileRep *Copy(void) const;

public:
  TablePureStrategyProfileRep(const Game &p_game);
  virtual long GetIndex(void) const { return m_index; }
  virtual void SetStrategy(const GameStrategy &);
  virtual GameOutcome GetOutcome(void) const;
  virtual void SetOutcome(GameOutcome p_outcome);
  virtual Rational GetPayoff(int pl) const;
  virtual Rational GetStrategyValue(const GameStrategy &) const;
};

//------------------------------------------------------------------------
//               TablePureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

TablePureStrategyProfileRep::TablePureStrategyProfileRep(const Game &p_nfg)
{
  m_index = 1L;
  m_nfg = p_nfg;
  m_profile = Array<GameStrategy>(m_nfg->NumPlayers());
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++)   {
    m_profile[pl] = m_nfg->GetPlayer(pl)->GetStrategy(1);
    m_index += m_profile[pl]->m_offset;
  }
}

PureStrategyProfileRep *TablePureStrategyProfileRep::Copy(void) const
{
  return new TablePureStrategyProfileRep(*this);
}

Game NewTable(const Array<int> &p_dim, bool p_sparseOutcomes /*= false*/)
{
  return new GameTableRep(p_dim, p_sparseOutcomes);
}

//------------------------------------------------------------------------
//       TablePureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void TablePureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  m_index += s->m_offset - m_profile[s->GetPlayer()->GetNumber()]->m_offset;
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

GameOutcome TablePureStrategyProfileRep::GetOutcome(void) const
{ 
  return dynamic_cast<GameTableRep &>(*m_nfg).m_results[m_index]; 
}

void TablePureStrategyProfileRep::SetOutcome(GameOutcome p_outcome)
{
  dynamic_cast<GameTableRep &>(*m_nfg).m_results[m_index] = p_outcome; 
}

Rational TablePureStrategyProfileRep::GetPayoff(int pl) const
{
  GameOutcomeRep *outcome = dynamic_cast<GameTableRep &>(*m_nfg).m_results[m_index];
  if (outcome) {
    return outcome->GetPayoff<Rational>(pl);
  }
  else {
    return 0;
  }
}

Rational
TablePureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  int player = p_strategy->GetPlayer()->GetNumber();
  GameOutcomeRep *outcome = dynamic_cast<GameTableRep &>(*m_nfg).m_results[m_index - m_profile[player]->m_offset + p_strategy->m_offset];
  if (outcome) {
    return outcome->GetPayoff<Rational>(player);
  }
  else {
    return 0;
  }
}

PureStrategyProfile GameTableRep::NewPureStrategyProfile(void) const
{
  return PureStrategyProfile(new TablePureStrategyProfileRep(const_cast<GameTableRep *>(this)));
}



//------------------------------------------------------------------------
//                     GameTableRep: Lifecycle
//------------------------------------------------------------------------

namespace {
/// This convenience function computes the Cartesian product of the
/// elements in dim.
int Product(const Array<int> &dim)
{
  int accum = 1;
  for (int i = 1; i <= dim.Length(); accum *= dim[i++]);
  return accum;
}

} // end anonymous namespace

  
GameTableRep::GameTableRep(const Array<int> &dim, 
			   bool p_sparseOutcomes /* = false */)
{
  m_results = Array<GameOutcomeRep *>(Product(dim));
  for (int pl = 1; pl <= dim.Length(); pl++)  {
    m_players.Append(new GamePlayerRep(this, pl, dim[pl]));
    m_players[pl]->m_label = lexical_cast<std::string>(pl);
    for (int st = 1; st <= m_players[pl]->NumStrategies(); st++) {
      m_players[pl]->m_strategies[st]->SetLabel(lexical_cast<std::string>(st));
    }
  }
  IndexStrategies();

  if (p_sparseOutcomes) {
    for (int cont = 1; cont <= m_results.Length();
	 m_results[cont++] = 0);
  }
  else {
    m_outcomes = Array<GameOutcomeRep *>(m_results.Length());
    for (int i = 1; i <= m_outcomes.Length(); i++) {
      m_outcomes[i] = new GameOutcomeRep(this, i);
    }
    m_results = m_outcomes;
  }
}

Game GameTableRep::Copy(void) const
{
  std::ostringstream os;
  WriteNfgFile(os);
  std::istringstream is(os.str());
  return ReadGame(is);
}

//------------------------------------------------------------------------
//                  GameTableRep: General data access
//------------------------------------------------------------------------

bool GameTableRep::IsConstSum(void) const
{
  TablePureStrategyProfileRep profile(const_cast<GameTableRep *>(this));

  Rational sum(0);
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    sum += profile.GetPayoff(pl);
  }

  for (StrategyIterator iter(StrategySupport(const_cast<GameTableRep *>(this)));
       !iter.AtEnd(); iter++) {
    Rational newsum(0);
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      newsum += (*iter)->GetPayoff(pl);
    }
    
    if (newsum != sum) {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------------
//                   GameTableRep: Writing data files
//------------------------------------------------------------------------

namespace {

std::string EscapeQuotes(const std::string &s)
{
  std::string ret;
  
  for (unsigned int i = 0; i < s.length(); i++)  {
    if (s[i] == '"')   ret += '\\';
    ret += s[i];
  }

  return ret;
}

}  // end anonymous namespace

void GameTableRep::WriteNfgFile(std::ostream &p_file) const
{ 
  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";

  for (int i = 1; i <= NumPlayers(); i++)
    p_file << '"' << EscapeQuotes(GetPlayer(i)->GetLabel()) << "\" ";

  p_file << "}\n\n{ ";
  
  for (int i = 1; i <= NumPlayers(); i++)   {
    GamePlayerRep *player = GetPlayer(i);
    p_file << "{ ";
    for (int j = 1; j <= player->NumStrategies(); j++)
      p_file << '"' << EscapeQuotes(player->GetStrategy(j)->GetLabel()) << "\" ";
    p_file << "}\n";
  }
  
  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  int ncont = 1;
  for (int i = 1; i <= NumPlayers(); i++) {
    ncont *= m_players[i]->m_strategies.Length();
  }

  p_file << "{\n";
  for (int outc = 1; outc <= m_outcomes.Length(); outc++)   {
    p_file << "{ \"" << EscapeQuotes(m_outcomes[outc]->m_label) << "\" ";
    for (int pl = 1; pl <= m_players.Length(); pl++)  {
      p_file << (const std::string &) m_outcomes[outc]->m_payoffs[pl];
      
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
    if (m_results[cont] != 0) {
      p_file << m_results[cont]->m_number << ' ';
    }
    else {
      p_file << "0 ";
    }
  }

  p_file << '\n';
}

//------------------------------------------------------------------------
//                       GameTableRep: Players
//------------------------------------------------------------------------

GamePlayer GameTableRep::NewPlayer(void)
{
  GamePlayerRep *player = 0;
  player = new GamePlayerRep(this, m_players.Length() + 1, 1);
  m_players.Append(player);
  for (int outc = 1; outc <= m_outcomes.Last(); outc++) {
    m_outcomes[outc]->m_payoffs.Append(Number());
  }
  ClearComputedValues();
  return player;
}

//------------------------------------------------------------------------
//                        GameTableRep: Outcomes
//------------------------------------------------------------------------

void GameTableRep::DeleteOutcome(const GameOutcome &p_outcome)
{
  for (int i = 1; i <= m_results.Length(); i++) {
    if (m_results[i] == p_outcome) {
      m_results[i] = 0;
    }
  }
  m_outcomes.Remove(m_outcomes.Find(p_outcome))->Invalidate();
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_number = outc;
  }
  ClearComputedValues();
}

//------------------------------------------------------------------------
//                   GameTableRep: Factory functions
//------------------------------------------------------------------------

MixedStrategyProfile<double> GameTableRep::NewMixedStrategyProfile(double) const
{
  return StrategySupport(const_cast<GameTableRep *>(this)).NewMixedStrategyProfile<double>();
}

MixedStrategyProfile<Rational> GameTableRep::NewMixedStrategyProfile(const Rational &) const
{
  return StrategySupport(const_cast<GameTableRep *>(this)).NewMixedStrategyProfile<Rational>();
}

MixedStrategyProfile<double> GameTableRep::NewMixedStrategyProfile(double, const StrategySupport& spt) const
{
  return new TableMixedStrategyProfileRep<double>(spt);
}
MixedStrategyProfile<Rational> GameTableRep::NewMixedStrategyProfile(const Rational &, const StrategySupport& spt) const
{
  return new TableMixedStrategyProfileRep<Rational>(spt);
}

//------------------------------------------------------------------------
//              GameTableRep: Private auxiliary functions
//------------------------------------------------------------------------

/// This rebuilds a new table of outcomes after the game has been
/// redimensioned (change in the number of strategies).  Strategies
/// numbered -1 are identified as the new strategies.
void GameTableRep::RebuildTable(void)
{
  long size = 1L;
  Array<long> offsets(m_players.Length());
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    offsets[pl] = size;
    size *= m_players[pl]->NumStrategies();
  }

  Array<GameOutcomeRep *> newResults(size);
  for (int i = 1; i <= newResults.Length(); newResults[i++] = 0);

  for (StrategyIterator iter(StrategySupport(const_cast<GameTableRep *>(this)));
       !iter.AtEnd(); iter++) {
    long newindex = 1L;
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      if (iter.m_profile->GetStrategy(pl)->m_offset < 0) {
	// This is a contingency involving a new strategy... skip
	newindex = -1L;
	break;
      }
      else {
	newindex += (iter.m_profile->GetStrategy(pl)->m_number - 1) * offsets[pl];
      }
    }

    if (newindex >= 1) {
      newResults[newindex] = m_results[iter.m_profile->GetIndex()];
    }
  }

  m_results = newResults;

  IndexStrategies();
}

void GameTableRep::IndexStrategies(void)
{
  long offset = 1L;
  for (GamePlayers::const_iterator player = m_players.begin();
       player != m_players.end(); ++player)  {
    int st = 1;
    for (Array<GameStrategyRep *>::const_iterator strategy = player->m_strategies.begin();
	 strategy != player->m_strategies.end(); ++st, ++strategy) {
      strategy->m_number = st;
      strategy->m_offset = (st - 1) * offset;
    }
    offset *= player->m_strategies.size();
  }

  int id = 1;
  for (GamePlayers::const_iterator player = m_players.begin();
       player != m_players.end(); ++player) {
    for (Array<GameStrategyRep *>::const_iterator strategy = player->m_strategies.begin();
	 strategy != player->m_strategies.end(); ++strategy) {
      strategy->m_id = id++;
    }
  }
}


}  // end namespace Gambit
