//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include "gambit.h"
#include "gametable.h"

namespace Gambit {

//========================================================================
//                  class TablePureStrategyProfileRep
//========================================================================

class TablePureStrategyProfileRep : public PureStrategyProfileRep {
protected:
  long m_index{1L};

  PureStrategyProfileRep *Copy() const override;

public:
  explicit TablePureStrategyProfileRep(const Game &p_game);
  long GetIndex() const override { return m_index; }
  void SetStrategy(const GameStrategy &) override;
  GameOutcome GetOutcome() const override;
  void SetOutcome(GameOutcome p_outcome) override;
  Rational GetPayoff(int pl) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//               TablePureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

TablePureStrategyProfileRep::TablePureStrategyProfileRep(const Game &p_nfg)
  : PureStrategyProfileRep(p_nfg)
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    m_index += m_profile[pl]->m_offset;
  }
}

PureStrategyProfileRep *TablePureStrategyProfileRep::Copy() const
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

GameOutcome TablePureStrategyProfileRep::GetOutcome() const
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
    return static_cast<Rational>(outcome->GetPayoff(pl));
  }
  else {
    return Rational(0);
  }
}

Rational TablePureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  int player = p_strategy->GetPlayer()->GetNumber();
  GameOutcomeRep *outcome =
      dynamic_cast<GameTableRep &>(*m_nfg)
          .m_results[m_index - m_profile[player]->m_offset + p_strategy->m_offset];
  if (outcome) {
    return static_cast<Rational>(outcome->GetPayoff(player));
  }
  else {
    return Rational(0);
  }
}

PureStrategyProfile GameTableRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(new TablePureStrategyProfileRep(const_cast<GameTableRep *>(this)));
}

//========================================================================
//                   TableMixedStrategyProfileRep<T>
//========================================================================

template <class T> class TableMixedStrategyProfileRep : public MixedStrategyProfileRep<T> {
private:
  /// @name Private recursive payoff functions
  //@{
  /// Recursive computation of payoff to player pl
  T GetPayoff(int pl, int index, int i) const;
  /// Recursive computation of payoff derivative
  void GetPayoffDeriv(int pl, int const_pl, int cur_pl, long index, const T &prob, T &value) const;
  /// Recursive computation of payoff second derivative
  void GetPayoffDeriv(int pl, int const_pl1, int const_pl2, int cur_pl, long index, const T &prob,
                      T &value) const;
  //@}

public:
  explicit TableMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  {
  }
  ~TableMixedStrategyProfileRep() override = default;

  MixedStrategyProfileRep<T> *Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

template <class T> MixedStrategyProfileRep<T> *TableMixedStrategyProfileRep<T>::Copy() const
{
  return new TableMixedStrategyProfileRep(*this);
}

template <class T>
T TableMixedStrategyProfileRep<T>::GetPayoff(int pl, int index, int current) const
{
  if (current > this->m_support.GetGame()->NumPlayers()) {
    Game game = this->m_support.GetGame();
    auto &g = dynamic_cast<GameTableRep &>(*game);
    GameOutcomeRep *outcome = g.m_results[index];
    if (outcome) {
      return static_cast<T>(outcome->GetPayoff(pl));
    }
    else {
      return T(0);
    }
  }

  T sum = static_cast<T>(0);
  for (auto s : this->m_support.GetStrategies(this->m_support.GetGame()->GetPlayer(current))) {
    if ((*this)[s] != T(0)) {
      sum += ((*this)[s] * GetPayoff(pl, index + s->m_offset, current + 1));
    }
  }
  return sum;
}

template <class T> T TableMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  return GetPayoff(pl, 1, 1);
}

template <class T>
void TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, int const_pl, int cur_pl, long index,
                                                     const T &prob, T &value) const
{
  if (cur_pl == const_pl) {
    cur_pl++;
  }
  if (cur_pl > this->m_support.GetGame()->NumPlayers()) {
    Game game = this->m_support.GetGame();
    auto &g = dynamic_cast<GameTableRep &>(*game);
    GameOutcomeRep *outcome = g.m_results[index];
    if (outcome) {
      value += prob * static_cast<T>(outcome->GetPayoff(pl));
    }
  }
  else {
    for (auto s : this->m_support.GetStrategies(this->m_support.GetGame()->GetPlayer(cur_pl))) {
      if ((*this)[s] > T(0)) {
        GetPayoffDeriv(pl, const_pl, cur_pl + 1, index + s->m_offset, prob * (*this)[s], value);
      }
    }
  }
}

template <class T>
T TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &strategy) const
{
  T value = T(0);
  GetPayoffDeriv(pl, strategy->GetPlayer()->GetNumber(), 1, strategy->m_offset + 1, T(1), value);
  return value;
}

template <class T>
void TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, int const_pl1, int const_pl2,
                                                     int cur_pl, long index, const T &prob,
                                                     T &value) const
{
  while (cur_pl == const_pl1 || cur_pl == const_pl2) {
    cur_pl++;
  }
  if (cur_pl > this->m_support.GetGame()->NumPlayers()) {
    Game game = this->m_support.GetGame();
    auto &g = dynamic_cast<GameTableRep &>(*game);
    GameOutcomeRep *outcome = g.m_results[index];
    if (outcome) {
      value += prob * static_cast<T>(outcome->GetPayoff(pl));
    }
  }
  else {
    for (auto s : this->m_support.GetStrategies(this->m_support.GetGame()->GetPlayer(cur_pl))) {
      if ((*this)[s] > static_cast<T>(0)) {
        GetPayoffDeriv(pl, const_pl1, const_pl2, cur_pl + 1, index + s->m_offset,
                       prob * (*this)[s], value);
      }
    }
  }
}

template <class T>
T TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &strategy1,
                                                  const GameStrategy &strategy2) const
{
  GamePlayerRep *player1 = strategy1->GetPlayer();
  GamePlayerRep *player2 = strategy2->GetPlayer();
  if (player1 == player2) {
    return T(0);
  }

  T value = T(0);
  GetPayoffDeriv(pl, player1->GetNumber(), player2->GetNumber(), 1,
                 strategy1->m_offset + strategy2->m_offset + 1, T(1), value);
  return value;
}

template class TableMixedStrategyProfileRep<double>;
template class TableMixedStrategyProfileRep<Rational>;

//------------------------------------------------------------------------
//                     GameTableRep: Lifecycle
//------------------------------------------------------------------------

namespace {
/// This convenience function computes the Cartesian product of the
/// elements in dim.
int Product(const Array<int> &dim)
{
  int accum = 1;
  for (auto d : dim) {
    accum *= d;
  }
  return accum;
}

} // end anonymous namespace

GameTableRep::GameTableRep(const Array<int> &dim, bool p_sparseOutcomes /* = false */)
{
  m_results = Array<GameOutcomeRep *>(Product(dim));
  for (int pl = 1; pl <= dim.Length(); pl++) {
    m_players.push_back(new GamePlayerRep(this, pl, dim[pl]));
    m_players[pl]->m_label = lexical_cast<std::string>(pl);
    for (int st = 1; st <= m_players[pl]->NumStrategies(); st++) {
      m_players[pl]->m_strategies[st]->SetLabel(lexical_cast<std::string>(st));
    }
  }
  IndexStrategies();

  if (p_sparseOutcomes) {
    for (int cont = 1; cont <= m_results.Length(); m_results[cont++] = 0)
      ;
  }
  else {
    m_outcomes = Array<GameOutcomeRep *>(m_results.Length());
    for (int i = 1; i <= m_outcomes.Length(); i++) {
      m_outcomes[i] = new GameOutcomeRep(this, i);
    }
    m_results = m_outcomes;
  }
}

Game GameTableRep::Copy() const
{
  std::ostringstream os;
  WriteNfgFile(os);
  std::istringstream is(os.str());
  return ReadGame(is);
}

//------------------------------------------------------------------------
//                  GameTableRep: General data access
//------------------------------------------------------------------------

bool GameTableRep::IsConstSum() const
{
  TablePureStrategyProfileRep profile(const_cast<GameTableRep *>(this));

  Rational sum(0);
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    sum += profile.GetPayoff(pl);
  }

  for (StrategyProfileIterator iter(StrategySupportProfile(const_cast<GameTableRep *>(this)));
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

  for (char c : s) {
    if (c == '"') {
      ret += '\\';
    }
    ret += c;
  }

  return ret;
}

} // end anonymous namespace

///
/// Write the game to a savefile in .nfg outcome format.
///
/// This overrides the .nfg writing in the base GameRep class.
/// It writes out the game in the .nfg outcome format, in which
/// the entries of the N-dimensional payoff table are written as
/// indexes into the list of outcomes, rather than the payoffs
/// directly.  This preserves the outcome structure of the game.
///
void GameTableRep::WriteNfgFile(std::ostream &p_file) const
{
  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";

  for (int i = 1; i <= NumPlayers(); i++) {
    p_file << '"' << EscapeQuotes(GetPlayer(i)->GetLabel()) << "\" ";
  }

  p_file << "}\n\n{ ";

  for (int i = 1; i <= NumPlayers(); i++) {
    GamePlayerRep *player = GetPlayer(i);
    p_file << "{ ";
    for (int j = 1; j <= player->NumStrategies(); j++) {
      p_file << '"' << EscapeQuotes(player->GetStrategy(j)->GetLabel()) << "\" ";
    }
    p_file << "}\n";
  }

  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  int ncont = 1;
  for (int i = 1; i <= NumPlayers(); i++) {
    ncont *= m_players[i]->m_strategies.Length();
  }

  p_file << "{\n";
  for (auto outcome : m_outcomes) {
    p_file << "{ \"" << EscapeQuotes(outcome->m_label) << "\" ";
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      p_file << (const std::string &)outcome->m_payoffs[pl];
      if (pl < m_players.Length()) {
        p_file << ", ";
      }
      else {
        p_file << " }\n";
      }
    }
  }
  p_file << "}\n";

  for (int cont = 1; cont <= ncont; cont++) {
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

GamePlayer GameTableRep::NewPlayer()
{
  IncrementVersion();
  auto player = new GamePlayerRep(this, m_players.size() + 1, 1);
  m_players.push_back(player);
  for (auto outcome : m_outcomes) {
    outcome->m_payoffs.push_back(Number());
  }
  return player;
}

//------------------------------------------------------------------------
//                        GameTableRep: Outcomes
//------------------------------------------------------------------------

void GameTableRep::DeleteOutcome(const GameOutcome &p_outcome)
{
  IncrementVersion();
  for (int i = 1; i <= m_results.Length(); i++) {
    if (m_results[i] == p_outcome) {
      m_results[i] = 0;
    }
  }
  m_outcomes.Remove(m_outcomes.Find(p_outcome))->Invalidate();
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_number = outc;
  }
}

//------------------------------------------------------------------------
//                   GameTableRep: Factory functions
//------------------------------------------------------------------------

MixedStrategyProfile<double> GameTableRep::NewMixedStrategyProfile(double) const
{
  return StrategySupportProfile(const_cast<GameTableRep *>(this))
      .NewMixedStrategyProfile<double>();
}

MixedStrategyProfile<Rational> GameTableRep::NewMixedStrategyProfile(const Rational &) const
{
  return StrategySupportProfile(const_cast<GameTableRep *>(this))
      .NewMixedStrategyProfile<Rational>();
}

MixedStrategyProfile<double>
GameTableRep::NewMixedStrategyProfile(double, const StrategySupportProfile &spt) const
{
  return MixedStrategyProfile<double>(new TableMixedStrategyProfileRep<double>(spt));
}
MixedStrategyProfile<Rational>
GameTableRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &spt) const
{
  return MixedStrategyProfile<Rational>(new TableMixedStrategyProfileRep<Rational>(spt));
}

//------------------------------------------------------------------------
//              GameTableRep: Private auxiliary functions
//------------------------------------------------------------------------

/// This rebuilds a new table of outcomes after the game has been
/// redimensioned (change in the number of strategies).  Strategies
/// numbered -1 are identified as the new strategies.
void GameTableRep::RebuildTable()
{
  long size = 1L;
  Array<long> offsets(m_players.Length());
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    offsets[pl] = size;
    size *= m_players[pl]->NumStrategies();
  }

  Array<GameOutcomeRep *> newResults(size);
  for (int i = 1; i <= newResults.Length(); newResults[i++] = 0)
    ;

  for (StrategyProfileIterator iter(StrategySupportProfile(const_cast<GameTableRep *>(this)));
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

void GameTableRep::IndexStrategies()
{
  long offset = 1L;
  for (auto player : m_players) {
    int st = 1;
    for (auto strategy : player->m_strategies) {
      strategy->m_number = st;
      strategy->m_offset = (st - 1) * offset;
      st++;
    }
    offset *= player->m_strategies.size();
  }

  int id = 1;
  for (auto player : m_players) {
    for (auto strategy : player->m_strategies) {
      strategy->m_id = id++;
    }
  }
}

} // end namespace Gambit
