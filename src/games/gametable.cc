//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#include "writer.h"

namespace Gambit {

//========================================================================
//                  class TablePureStrategyProfileRep
//========================================================================

class TablePureStrategyProfileRep : public PureStrategyProfileRep {
protected:
  long m_index{0L};

  std::shared_ptr<PureStrategyProfileRep> Copy() const override;

public:
  explicit TablePureStrategyProfileRep(const Game &p_game);
  void SetStrategy(const GameStrategy &) override;
  GameOutcome GetOutcome() const override;
  void SetOutcome(GameOutcome p_outcome) override;
  Rational GetPayoff(const GamePlayer &) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//               TablePureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

TablePureStrategyProfileRep::TablePureStrategyProfileRep(const Game &p_nfg)
  : PureStrategyProfileRep(p_nfg)
{
  for (auto [player, strategy] : m_profile) {
    m_index += strategy->m_offset;
  }
}

std::shared_ptr<PureStrategyProfileRep> TablePureStrategyProfileRep::Copy() const
{
  return std::make_shared<TablePureStrategyProfileRep>(*this);
}

Game NewTable(const std::vector<int> &p_dim, bool p_sparseOutcomes /*= false*/)
{
  return std::make_shared<GameTableRep>(p_dim, p_sparseOutcomes);
}

//------------------------------------------------------------------------
//       TablePureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void TablePureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  m_index += s->m_offset - m_profile.at(s->GetPlayer())->m_offset;
  m_profile[s->GetPlayer()] = s;
}

GameOutcome TablePureStrategyProfileRep::GetOutcome() const
{
  if (const auto outcome = dynamic_cast<GameTableRep &>(*m_nfg).m_results[m_index]) {
    return outcome->shared_from_this();
  }
  return nullptr;
}

void TablePureStrategyProfileRep::SetOutcome(GameOutcome p_outcome)
{
  dynamic_cast<GameTableRep &>(*m_nfg).m_results[m_index] = p_outcome.get();
}

Rational TablePureStrategyProfileRep::GetPayoff(const GamePlayer &p_player) const
{
  if (const auto outcome = dynamic_cast<GameTableRep &>(*m_nfg).m_results[m_index]) {
    return outcome->GetPayoff<Rational>(p_player);
  }
  return Rational(0);
}

Rational TablePureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  const auto &player = p_strategy->GetPlayer();
  const GameOutcomeRep *outcome =
      dynamic_cast<GameTableRep &>(*m_nfg)
          .m_results[m_index - m_profile.at(player)->m_offset + p_strategy->m_offset];
  if (outcome) {
    return outcome->GetPayoff<Rational>(player);
  }
  else {
    return Rational(0);
  }
}

PureStrategyProfile GameTableRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(std::make_shared<TablePureStrategyProfileRep>(
      std::const_pointer_cast<GameRep>(shared_from_this())));
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

  std::unique_ptr<MixedStrategyProfileRep<T>> Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

template <class T>
std::unique_ptr<MixedStrategyProfileRep<T>> TableMixedStrategyProfileRep<T>::Copy() const
{
  return std::make_unique<TableMixedStrategyProfileRep>(*this);
}

template <class T>
T TableMixedStrategyProfileRep<T>::GetPayoff(int pl, int index, int current) const
{
  if (current > static_cast<int>(this->GetSupport().GetGame()->NumPlayers())) {
    const Game game = this->GetSupport().GetGame();
    auto &g = dynamic_cast<GameTableRep &>(*game);
    if (const auto outcome = g.m_results[index]) {
      return outcome->GetPayoff<T>(this->GetSupport().GetGame()->GetPlayer(pl));
    }
    return static_cast<T>(0);
  }

  T sum = static_cast<T>(0);
  for (auto s :
       this->GetSupport().GetStrategies(this->GetSupport().GetGame()->GetPlayer(current))) {
    if ((*this)[s] != T(0)) {
      sum += ((*this)[s] * GetPayoff(pl, index + s->m_offset, current + 1));
    }
  }
  return sum;
}

template <class T> T TableMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  return GetPayoff(pl, 0, 1);
}

template <class T>
void TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, int const_pl, int cur_pl, long index,
                                                     const T &prob, T &value) const
{
  if (cur_pl == const_pl) {
    cur_pl++;
  }
  if (cur_pl > static_cast<int>(this->GetSupport().GetGame()->NumPlayers())) {
    const Game game = this->GetSupport().GetGame();
    auto &g = dynamic_cast<GameTableRep &>(*game);
    if (const auto outcome = g.m_results[index]) {
      value += prob * outcome->GetPayoff<T>(this->GetSupport().GetGame()->GetPlayer(pl));
    }
  }
  else {
    for (auto s :
         this->GetSupport().GetStrategies(this->GetSupport().GetGame()->GetPlayer(cur_pl))) {
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
  GetPayoffDeriv(pl, strategy->GetPlayer()->GetNumber(), 1, strategy->m_offset, T(1), value);
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
  if (cur_pl > static_cast<int>(this->GetSupport().GetGame()->NumPlayers())) {
    const Game game = this->GetSupport().GetGame();
    auto &g = dynamic_cast<GameTableRep &>(*game);
    if (const auto outcome = g.m_results[index]) {
      value += prob * outcome->GetPayoff<T>(this->GetSupport().GetGame()->GetPlayer(pl));
    }
  }
  else {
    for (auto s :
         this->GetSupport().GetStrategies(this->GetSupport().GetGame()->GetPlayer(cur_pl))) {
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
  const auto player1 = strategy1->GetPlayer().get();
  const auto player2 = strategy2->GetPlayer().get();
  if (player1 == player2) {
    return T(0);
  }

  T value = T(0);
  GetPayoffDeriv(pl, player1->GetNumber(), player2->GetNumber(), 1,
                 strategy1->m_offset + strategy2->m_offset, T(1), value);
  return value;
}

template class TableMixedStrategyProfileRep<double>;
template class TableMixedStrategyProfileRep<Rational>;

//------------------------------------------------------------------------
//                     GameTableRep: Lifecycle
//------------------------------------------------------------------------

GameTableRep::GameTableRep(const std::vector<int> &dim, bool p_sparseOutcomes /* = false */)
  : m_results(std::accumulate(dim.begin(), dim.end(), 1, std::multiplies<>()))
{
  for (const auto &nstrat : dim) {
    m_players.push_back(std::make_shared<GamePlayerRep>(this, m_players.size() + 1, nstrat));
    m_players.back()->m_label = lexical_cast<std::string>(m_players.size());
    std::for_each(m_players.back()->m_strategies.begin(), m_players.back()->m_strategies.end(),
                  [st = 1](const std::shared_ptr<GameStrategyRep> &s) mutable {
                    s->m_label = std::to_string(st++);
                  });
  }
  IndexStrategies();

  if (p_sparseOutcomes) {
    std::fill(m_results.begin(), m_results.end(), nullptr);
  }
  else {
    m_outcomes = std::vector<std::shared_ptr<GameOutcomeRep>>(m_results.size());
    std::generate(m_outcomes.begin(), m_outcomes.end(), [this, outc = 1]() mutable {
      return std::make_shared<GameOutcomeRep>(this, outc++);
    });
    std::transform(m_outcomes.begin(), m_outcomes.end(), m_results.begin(),
                   [](const std::shared_ptr<GameOutcomeRep> &c) { return c.get(); });
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
  auto profile = NewPureStrategyProfile();
  Rational sum(0);
  for (const auto &player : m_players) {
    sum += profile->GetPayoff(player);
  }

  for (const auto iter :
       StrategyContingencies(std::const_pointer_cast<GameRep>(shared_from_this()))) {
    Rational newsum(0);
    for (const auto &player : m_players) {
      newsum += iter->GetPayoff(player);
    }
    if (newsum != sum) {
      return false;
    }
  }
  return true;
}

Rational GameTableRep::GetPlayerMinPayoff(const GamePlayer &p_player) const
{
  Rational minpay = NewPureStrategyProfile()->GetPayoff(p_player);
  for (const auto &profile :
       StrategyContingencies(std::const_pointer_cast<GameRep>(shared_from_this()))) {
    minpay = std::min(minpay, profile->GetPayoff(p_player));
  }
  return minpay;
}

Rational GameTableRep::GetPlayerMaxPayoff(const GamePlayer &p_player) const
{
  Rational maxpay = NewPureStrategyProfile()->GetPayoff(p_player);
  for (const auto &profile :
       StrategyContingencies(std::const_pointer_cast<GameRep>(shared_from_this()))) {
    maxpay = std::max(maxpay, profile->GetPayoff(p_player));
  }
  return maxpay;
}

//------------------------------------------------------------------------
//                   GameTableRep: Writing data files
//------------------------------------------------------------------------

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
  auto players = GetPlayers();
  p_file << "NFG 1 R " << std::quoted(GetTitle()) << ' '
         << FormatList(players, [](const GamePlayer &p) { return QuoteString(p->GetLabel()); })
         << std::endl
         << std::endl;
  p_file << "{ ";
  for (auto player : players) {
    p_file << FormatList(player->GetStrategies(), [](const GameStrategy &s) {
      return QuoteString(s->GetLabel());
    }) << std::endl;
  }
  p_file << "}" << std::endl;
  p_file << std::quoted(GetComment()) << std::endl << std::endl;

  p_file << "{" << std::endl;
  for (auto outcome : m_outcomes) {
    p_file << "{ " + QuoteString(outcome->GetLabel()) << ' '
           << FormatList(
                  players,
                  [outcome](const GamePlayer &p) { return outcome->GetPayoff<std::string>(p); },
                  true, false)
           << " }" << std::endl;
  }
  p_file << "}" << std::endl;

  for (auto result : m_results) {
    p_file << ((result) ? result->m_number : 0) << ' ';
  }
  p_file << std::endl;
}

//------------------------------------------------------------------------
//                       GameTableRep: Players
//------------------------------------------------------------------------

GamePlayer GameTableRep::NewPlayer()
{
  IncrementVersion();
  auto player = std::make_shared<GamePlayerRep>(this, m_players.size() + 1, 1);
  m_players.push_back(player);
  for (const auto &outcome : m_outcomes) {
    outcome->m_payoffs[player.get()] = Number();
  }
  return player;
}

//------------------------------------------------------------------------
//                        GameTableRep: Outcomes
//------------------------------------------------------------------------

void GameTableRep::DeleteOutcome(const GameOutcome &p_outcome)
{
  IncrementVersion();
  std::replace(m_results.begin(), m_results.end(), p_outcome.get(),
               static_cast<GameOutcomeRep *>(nullptr));
  m_outcomes.erase(
      std::find(m_outcomes.begin(), m_outcomes.end(), std::shared_ptr<GameOutcomeRep>(p_outcome)));
  p_outcome->Invalidate();
  std::for_each(
      m_outcomes.begin(), m_outcomes.end(),
      [outc = 1](const std::shared_ptr<GameOutcomeRep> &c) mutable { c->m_number = outc++; });
}

//------------------------------------------------------------------------
//                        GameTableRep: Strategies
//------------------------------------------------------------------------

GameStrategy GameTableRep::NewStrategy(const GamePlayer &p_player, const std::string &p_label)
{
  if (p_player->GetGame().get() != this) {
    throw MismatchException();
  }
  IncrementVersion();
  p_player->m_strategies.push_back(std::make_shared<GameStrategyRep>(
      p_player.get(), p_player->m_strategies.size() + 1, p_label));
  RebuildTable();
  return p_player->m_strategies.back();
}

void GameTableRep::DeleteStrategy(const GameStrategy &p_strategy)
{
  const auto player = p_strategy->GetPlayer().get();
  if (player->m_game != this) {
    throw MismatchException();
  }
  if (player->GetStrategies().size() == 1) {
    return;
  }

  IncrementVersion();
  player->m_strategies.erase(std::find(player->m_strategies.begin(), player->m_strategies.end(),
                                       std::shared_ptr<GameStrategyRep>(p_strategy)));
  std::for_each(
      player->m_strategies.begin(), player->m_strategies.end(),
      [st = 1](const std::shared_ptr<GameStrategyRep> &s) mutable { s->m_number = st++; });
  // Note that we do not reindex strategies, and so we do not need to re-build the
  // table of outcomes.
  p_strategy->Invalidate();
}

//------------------------------------------------------------------------
//                   GameTableRep: Factory functions
//------------------------------------------------------------------------

MixedStrategyProfile<double> GameTableRep::NewMixedStrategyProfile(double) const
{
  return StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this()))
      .NewMixedStrategyProfile<double>();
}

MixedStrategyProfile<Rational> GameTableRep::NewMixedStrategyProfile(const Rational &) const
{
  return StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this()))
      .NewMixedStrategyProfile<Rational>();
}

MixedStrategyProfile<double>
GameTableRep::NewMixedStrategyProfile(double, const StrategySupportProfile &spt) const
{
  return MixedStrategyProfile<double>(std::make_unique<TableMixedStrategyProfileRep<double>>(spt));
}
MixedStrategyProfile<Rational>
GameTableRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &spt) const
{
  return MixedStrategyProfile<Rational>(
      std::make_unique<TableMixedStrategyProfileRep<Rational>>(spt));
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
  std::vector<long> offsets;
  for (const auto &player : m_players) {
    offsets.push_back(size);
    size *= player->m_strategies.size();
  }

  std::vector<GameOutcomeRep *> newResults(size);
  std::fill(newResults.begin(), newResults.end(), nullptr);

  for (auto iter : StrategyContingencies(
           StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this())))) {
    long newindex = 0L;
    for (const auto &player : m_players) {
      if (iter->GetStrategy(player)->m_offset < 0) {
        // This is a contingency involving a new strategy... skip
        newindex = -1L;
        break;
      }
      else {
        newindex += (iter->GetStrategy(player)->m_number - 1) * offsets[player->m_number - 1];
      }
    }

    if (newindex >= 0 && iter->GetOutcome() != nullptr) {
      newResults[newindex] = iter->GetOutcome().get();
    }
  }
  m_results.swap(newResults);
  IndexStrategies();
}

void GameTableRep::IndexStrategies() const
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
}

} // end namespace Gambit
