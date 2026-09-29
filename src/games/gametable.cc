//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/gametable.cc
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

#include "games.h"
#include "core/lazy.h"
#include "gametable.h"
#include "writer.h"

namespace Gambit {

//========================================================================
//                  class TablePureStrategyProfileRep
//========================================================================

class TablePureStrategyProfileRep : public PureStrategyProfileRep {
protected:
  std::shared_ptr<PureStrategyProfileRep> Copy() const override
  {
    return std::make_shared<TablePureStrategyProfileRep>(*this);
  }

public:
  explicit TablePureStrategyProfileRep(const Game &p_game) : PureStrategyProfileRep(p_game) {}
  GameOutcome GetOutcome() const override;
  Rational GetPayoff(const GamePlayer &) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

Game NewTable(const std::vector<int> &p_dim, bool p_sparseOutcomes /*= false*/)
{
  return std::make_shared<GameTableRep>(p_dim, p_sparseOutcomes);
}

//------------------------------------------------------------------------
//       TablePureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

GameOutcome TablePureStrategyProfileRep::GetOutcome() const
{
  return dynamic_cast<GameTableRep &>(*m_game).m_results.at(m_index)->shared_from_this();
}

Rational TablePureStrategyProfileRep::GetPayoff(const GamePlayer &p_player) const
{
  return dynamic_cast<GameTableRep &>(*m_game).m_results.at(m_index)->GetPayoff<Rational>(
      p_player);
}

Rational TablePureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  const auto &player = p_strategy->GetPlayer();
  const auto &[m_radices, m_strides] = m_game->m_pureStrategies;
  const size_t index = player->GetNumber() - 1;
  const long stride = m_strides[index];
  const long digit_old = (m_index / stride) % m_radices[index];
  const long digit_new = p_strategy->GetNumber() - 1;
  const long new_index = m_index + (digit_new - digit_old) * stride;
  return dynamic_cast<GameTableRep &>(*m_game).m_results[new_index]->GetPayoff<Rational>(player);
}

PureStrategyProfile GameTableRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(std::make_shared<TablePureStrategyProfileRep>(
      std::const_pointer_cast<GameRep>(shared_from_this())));
}

template <class T> GameTableRep::PayoffTable<T> GameTableRep::BuildPayoffTable() const
{
  PayoffTable<T> table(m_players.size(), std::vector<T>(m_results.size()));
  for (size_t pl = 0; pl < m_players.size(); pl++) {
    const GamePlayer player = m_players[pl];
    auto &payoffs = table[pl];
    for (size_t index = 0; index < m_results.size(); index++) {
      payoffs[index] = m_results[index]->GetPayoff<T>(player);
    }
  }
  return table;
}

template <>
std::shared_ptr<const GameTableRep::PayoffTable<double>>
GameTableRep::GetPayoffTable<double>() const
{
  const std::scoped_lock lock(m_payoffTableMutex);
  if (!m_doublePayoffs || m_doublePayoffsVersion != GetVersion()) {
    m_doublePayoffs = std::make_shared<const PayoffTable<double>>(BuildPayoffTable<double>());
    m_doublePayoffsVersion = GetVersion();
  }
  return m_doublePayoffs;
}

template <>
std::shared_ptr<const GameTableRep::PayoffTable<Rational>>
GameTableRep::GetPayoffTable<Rational>() const
{
  const std::scoped_lock lock(m_payoffTableMutex);
  if (!m_rationalPayoffs || m_rationalPayoffsVersion != GetVersion()) {
    m_rationalPayoffs =
        std::make_shared<const PayoffTable<Rational>>(BuildPayoffTable<Rational>());
    m_rationalPayoffsVersion = GetVersion();
  }
  return m_rationalPayoffs;
}

//========================================================================
//                   TableMixedStrategyProfileRep<T>
//========================================================================

/// Visit each contingency of the strategies of all players other than p_skip1 and p_skip2 (player
/// numbers, or 0) which has positive probability under p_probs, calling p_visit(index, prob) with
/// its position in the table and its probability.  Contingencies are visited with the strategy
/// of the lowest-numbered player varying fastest; each probability is the product of the
/// players' probabilities in order of player number.
template <class T, class Visit>
void ForEachContingency(const SegmentedVector<T> &p_probs, const SegmentedArray<long> &p_offsets,
                        size_t p_skip1, size_t p_skip2, Visit p_visit)
{
  std::vector<const T *> probs;
  std::vector<const long *> offsets;
  std::vector<size_t> radix;
  for (size_t pl = 1; pl <= p_probs.GetShape().size(); pl++) {
    if (pl == p_skip1 || pl == p_skip2) {
      continue;
    }
    const auto segment = p_probs.segment(pl);
    if (segment.empty()) {
      return;
    }
    probs.push_back(segment.data());
    offsets.push_back(p_offsets.segment(pl).data());
    radix.push_back(segment.size());
  }

  const size_t numActive = probs.size();
  std::vector<size_t> digit(numActive, 0);
  while (true) {
    T prob{1};
    long index = 0;
    for (size_t k = 0; k < numActive; k++) {
      prob *= probs[k][digit[k]];
      index += offsets[k][digit[k]];
    }
    if (prob != T{0}) {
      p_visit(index, prob);
    }
    size_t k = 0;
    for (; k < numActive; k++) {
      if (++digit[k] < radix[k]) {
        break;
      }
      digit[k] = 0;
    }
    if (k == numActive) {
      return;
    }
  }
}

template <class T> class TableMixedStrategyProfileRep : public MixedStrategyProfileRep<T> {
public:
  explicit TableMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  {
  }
  ~TableMixedStrategyProfileRep() override = default;

  std::unique_ptr<MixedStrategyProfileRep<T>> Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  bool GetPayoffDerivs(int pl, Vector<T> &p_derivs) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;

private:
  Lazy<std::shared_ptr<const GameTableRep::PayoffTable<T>>> m_payoffTable;

  /// The payoffs to player p_player (a player number) in every contingency
  const std::vector<T> &GetPayoffs(int p_player) const
  {
    const auto &table = m_payoffTable.Get([this] {
      return dynamic_cast<const GameTableRep &>(*this->GetSupport().GetGame())
          .template GetPayoffTable<T>();
    });
    return (*table)[p_player - 1];
  }
};

template <class T>
std::unique_ptr<MixedStrategyProfileRep<T>> TableMixedStrategyProfileRep<T>::Copy() const
{
  return std::make_unique<TableMixedStrategyProfileRep>(*this);
}

template <class T> T TableMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  const auto &payoffs = GetPayoffs(pl);
  T value{0};
  ForEachContingency(this->m_probs, this->m_offsets, 0, 0,
                     [&](long index, const T &prob) { value += prob * payoffs[index]; });
  return value;
}

template <class T>
T TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &strategy) const
{
  const auto &payoffs = GetPayoffs(pl);
  const long base = this->StrategyOffset(strategy);
  T value{0};
  ForEachContingency(this->m_probs, this->m_offsets, strategy->GetPlayer()->GetNumber(), 0,
                     [&](long index, const T &prob) { value += prob * payoffs[base + index]; });
  return value;
}

template <class T>
bool TableMixedStrategyProfileRep<T>::GetPayoffDerivs(int pl, Vector<T> &p_derivs) const
{
  const auto &payoffs = GetPayoffs(pl);
  p_derivs = T{0};
  const auto segment = this->m_offsets.segment(pl);
  ForEachContingency(this->m_probs, this->m_offsets, pl, 0, [&](long index, const T &prob) {
    auto deriv = p_derivs.begin();
    for (const long base : segment) {
      *deriv += prob * payoffs[base + index];
      ++deriv;
    }
  });
  return true;
}

template <class T>
T TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &strategy1,
                                                  const GameStrategy &strategy2) const
{
  if (strategy1->GetPlayer() == strategy2->GetPlayer()) {
    return T{0};
  }
  const auto &payoffs = GetPayoffs(pl);
  const long base = this->StrategyOffset(strategy1) + this->StrategyOffset(strategy2);
  T value{0};
  ForEachContingency(this->m_probs, this->m_offsets, strategy1->GetPlayer()->GetNumber(),
                     strategy2->GetPlayer()->GetNumber(),
                     [&](long index, const T &prob) { value += prob * payoffs[base + index]; });
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
    const auto pl = m_players.size() + 1;
    m_players.push_back(
        std::make_shared<GamePlayerRep>(this, pl, lexical_cast<std::string>(pl), nstrat));
    std::for_each(m_players.back()->m_strategies.begin(), m_players.back()->m_strategies.end(),
                  [st = 1](const std::shared_ptr<GameStrategyRep> &s) mutable {
                    s->m_label = std::to_string(st++);
                  });
  }
  IndexStrategies();

  if (p_sparseOutcomes) {
    std::fill(m_results.begin(), m_results.end(), m_nullOutcome.get());
  }
  else {
    m_outcomes = std::vector<std::shared_ptr<GameOutcomeRep>>(m_results.size());
    std::generate(m_outcomes.begin(), m_outcomes.end(), [this, outc = 1]() mutable {
      const auto number = outc++;
      return std::make_shared<GameOutcomeRep>(this, number, std::to_string(number));
    });
    std::transform(m_outcomes.begin(), m_outcomes.end(), m_results.begin(),
                   [](const std::shared_ptr<GameOutcomeRep> &c) { return c.get(); });
    IndexOutcomeLabels();
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
  auto payoff_sum = [&](const PureStrategyProfile &p) {
    return sum_function(m_players, [&](const auto &player) { return p->GetPayoff(player); });
  };
  const Rational sum = payoff_sum(NewPureStrategyProfile());

  auto contingencies = StrategyContingencies(std::const_pointer_cast<GameRep>(shared_from_this()));
  return std::all_of(contingencies.begin(), contingencies.end(),
                     [&](const PureStrategyProfile &p) { return payoff_sum(p) == sum; });
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
  p_file << std::quoted(GetDescription()) << std::endl << std::endl;

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

  for (auto iter : StrategyContingencies(
           StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this())))) {
    p_file << iter->GetOutcome()->m_number << ' ';
  }
  p_file << std::endl;
}

//------------------------------------------------------------------------
//                       GameTableRep: Players
//------------------------------------------------------------------------

//------------------------------------------------------------------------
//                        GameTableRep: Outcomes
//------------------------------------------------------------------------

std::set<long> GameTableRep::ResolveContingencies(
    const std::vector<std::vector<GameStrategy>> &p_contingencies) const
{
  const auto &strides = m_pureStrategies.m_strides;
  std::set<long> selected;
  for (const auto &contingency : p_contingencies) {
    if (contingency.size() != m_players.size()) {
      throw ValueException("Each contingency must give one strategy per player");
    }
    long index = 0;
    for (const auto &[pl, strategy] : enumerate(contingency)) {
      if (strategy->m_player != m_players[pl].get()) {
        if (strategy->m_player->m_game != this) {
          throw MismatchException();
        }
        throw ValueException("Each contingency must give one strategy per player");
      }
      index += (strategy->m_number - 1) * strides[pl];
    }
    if (!selected.insert(index).second) {
      throw ValueException("Each contingency may be referenced only once");
    }
  }
  return selected;
}

std::set<const GameOutcomeRep *>
GameTableRep::ComputeAbsorbedOutcomes(const std::set<long> &p_selected,
                                      const std::set<const GameOutcomeRep *> &p_covered) const
{
  std::set<const GameOutcomeRep *> absorbed;
  if (!p_covered.empty()) {
    absorbed = p_covered;
    for (size_t index = 0; index < m_results.size() && !absorbed.empty(); index++) {
      if (!p_selected.contains(static_cast<long>(index))) {
        absorbed.erase(m_results[index]);
      }
    }
  }
  return absorbed;
}

GameOutcome
GameTableRep::MakeOutcome(const std::vector<std::vector<GameStrategy>> &p_contingencies,
                          const std::vector<Number> &p_payoffs, const std::string &p_label)
{
  if (p_contingencies.empty()) {
    throw ValueException("At least one contingency must be specified");
  }
  if (p_payoffs.size() != m_players.size()) {
    throw ValueException("A payoff must be specified for each player");
  }
  const auto selected = ResolveContingencies(p_contingencies);
  std::set<const GameOutcomeRep *> covered;
  for (const auto index : selected) {
    if (!m_results[index]->IsNull()) {
      covered.insert(m_results[index]);
    }
  }
  const auto absorbed = ComputeAbsorbedOutcomes(selected, covered);
  CheckOutcomeLabel(p_label, absorbed);

  IncrementVersion();
  auto outcome = std::make_shared<GameOutcomeRep>(this, m_outcomes.size() + 1, p_label);
  AddOutcome(outcome);
  for (const auto &[pl, player] : enumerate(m_players)) {
    outcome->SetPayoff(player, p_payoffs[pl]);
  }
  for (const auto index : selected) {
    m_results[index] = outcome.get();
  }
  if (!absorbed.empty()) {
    EraseOutcomes(absorbed);
  }
  return outcome;
}

void GameTableRep::MakeOutcomeNull(const std::vector<std::vector<GameStrategy>> &p_contingencies)
{
  if (p_contingencies.empty()) {
    throw ValueException("At least one contingency must be specified");
  }
  const auto selected = ResolveContingencies(p_contingencies);
  std::set<const GameOutcomeRep *> covered;
  for (const auto index : selected) {
    if (!m_results[index]->IsNull()) {
      covered.insert(m_results[index]);
    }
  }
  const auto absorbed = ComputeAbsorbedOutcomes(selected, covered);

  IncrementVersion();
  for (const auto index : selected) {
    m_results[index] = m_nullOutcome.get();
  }
  if (!absorbed.empty()) {
    EraseOutcomes(absorbed);
  }
}

//------------------------------------------------------------------------
//                        GameTableRep: Strategies
//------------------------------------------------------------------------

void GameTableRep::RelabelStrategies(const GamePlayer &p_player,
                                     const std::map<std::string, std::string> &p_labels)
{
  if (p_player->GetGame().get() != this) {
    throw MismatchException();
  }
  std::map<GameStrategyRep *, std::string> assignment;
  std::set<const GameStrategyRep *> relabeled;
  for (const auto &[old_label, new_label] : p_labels) {
    GameStrategyRep *match = nullptr;
    for (const auto &strategy : p_player->m_strategies) {
      if (strategy->GetLabel() == old_label) {
        if (match) {
          throw ValueException("Strategy label '" + old_label + "' is ambiguous for this player");
        }
        match = strategy.get();
      }
    }
    if (!match) {
      throw ValueException("No strategy with label '" + old_label + "' for this player");
    }
    assignment[match] = new_label;
    relabeled.insert(match);
  }
  std::set<std::string> targets;
  for (const auto &[strategy, new_label] : assignment) {
    p_player->CheckStrategyLabel(new_label, relabeled);
    if (!targets.insert(new_label).second) {
      throw ValueException("Strategy label '" + new_label +
                           "' would be duplicated by the relabelling");
    }
  }
  for (const auto &[strategy, new_label] : assignment) {
    strategy->m_label = new_label;
  }
}

void GameTableRep::SetStrategies(const GamePlayer &p_player,
                                 const std::vector<std::string> &p_labels)
{
  if (p_player->GetGame().get() != this) {
    throw MismatchException();
  }
  if (p_labels.empty()) {
    throw ValueException("At least one strategy must be specified");
  }
  std::set<std::string> declared;
  for (const auto &label : p_labels) {
    if (label.empty()) {
      throw ValueException("Strategy label must not be empty");
    }
    CheckLabel(label);
    if (!declared.insert(label).second) {
      throw ValueException("Strategy label '" + label + "' appears more than once");
    }
  }
  // Match declared labels against current strategies.
  std::map<std::string, long> current;
  for (const auto &strategy : p_player->m_strategies) {
    if (!current.emplace(strategy->GetLabel(), strategy->GetNumber() - 1).second) {
      throw ValueException("Strategy label '" + strategy->GetLabel() +
                           "' is ambiguous for this player");
    }
  }
  std::vector<long> source;
  source.reserve(p_labels.size());
  for (const auto &label : p_labels) {
    const auto it = current.find(label);
    source.push_back((it != current.end()) ? it->second : -1);
  }
  std::vector<long> old_to_new(p_player->m_strategies.size(), -1);
  for (size_t i = 0; i < source.size(); ++i) {
    if (source[i] >= 0) {
      old_to_new[source[i]] = static_cast<long>(i);
    }
  }
  std::vector<long> old_radices;
  for (const auto &player : m_players) {
    old_radices.push_back(player->m_strategies.size());
  }

  IncrementVersion();
  std::vector<std::shared_ptr<GameStrategyRep>> newStrategies;
  newStrategies.reserve(p_labels.size());
  for (size_t i = 0; i < p_labels.size(); ++i) {
    if (source[i] >= 0) {
      newStrategies.push_back(p_player->m_strategies[source[i]]);
    }
    else {
      newStrategies.push_back(
          std::make_shared<GameStrategyRep>(p_player.get(), static_cast<int>(i) + 1, p_labels[i]));
    }
  }
  for (const auto &strategy : p_player->m_strategies) {
    if (declared.count(strategy->GetLabel()) == 0) {
      strategy->Invalidate();
    }
  }
  p_player->m_strategies = std::move(newStrategies);
  RebuildTable(old_radices, p_player->GetNumber() - 1, old_to_new);
}

void GameTableRep::SetPlayers(const std::vector<std::string> &p_labels)
{
  if (p_labels.empty()) {
    throw ValueException("At least one player must be specified");
  }
  std::map<std::string, long> current;
  for (const auto &player : m_players) {
    if (!current.emplace(player->GetLabel(), player->GetNumber() - 1).second) {
      throw ValueException("Player label '" + player->GetLabel() + "' is ambiguous in this game");
    }
  }
  std::set<std::string> declared;
  for (const auto &label : p_labels) {
    if (!declared.insert(label).second) {
      throw ValueException("Player label '" + label + "' appears more than once");
    }
    if (current.count(label) == 0) {
      CheckPlayerLabel(label);
    }
  }
  std::vector<long> old_radices;
  old_radices.reserve(m_players.size());
  for (const auto &player : m_players) {
    if (declared.count(player->GetLabel()) == 0 && player->m_strategies.size() != 1) {
      throw UndefinedException("A player with more than one strategy cannot be deleted");
    }
    old_radices.push_back(player->m_strategies.size());
  }
  std::vector<long> source;
  source.reserve(p_labels.size());
  for (const auto &label : p_labels) {
    const auto it = current.find(label);
    source.push_back((it != current.end()) ? it->second : -1);
  }

  IncrementVersion();
  std::vector<std::shared_ptr<GamePlayerRep>> newPlayers;
  newPlayers.reserve(p_labels.size());
  for (size_t j = 0; j < p_labels.size(); ++j) {
    if (source[j] >= 0) {
      newPlayers.push_back(m_players[source[j]]);
      continue;
    }
    auto player = std::make_shared<GamePlayerRep>(this, static_cast<int>(j) + 1, p_labels[j], 1);
    player->m_strategies.front()->m_label = "1";
    for (const auto &outcome : m_outcomes) {
      outcome->m_payoffs[player.get()] = Number();
    }
    newPlayers.push_back(player);
  }
  for (const auto &player : m_players) {
    if (declared.count(player->GetLabel()) == 0) {
      for (const auto &outcome : m_outcomes) {
        outcome->m_payoffs.erase(player.get());
      }
      player->Invalidate();
    }
  }
  m_players = std::move(newPlayers);
  for (size_t j = 0; j < m_players.size(); ++j) {
    m_players[j]->m_number = static_cast<int>(j) + 1;
  }
  // Permute the outcome table into the new player order.
  std::vector<long> old_strides(old_radices.size());
  long stride = 1;
  for (size_t i = 0; i < old_radices.size(); ++i) {
    old_strides[i] = stride;
    stride *= old_radices[i];
  }
  const long old_size = stride;
  std::vector<long> new_strides(m_players.size());
  long new_size = 1;
  for (size_t j = 0; j < m_players.size(); ++j) {
    new_strides[j] = new_size;
    new_size *= m_players[j]->m_strategies.size();
  }
  std::vector<GameOutcomeRep *> newResults(new_size, m_nullOutcome.get());
  for (long old_index = 0; old_index < old_size; ++old_index) {
    if (m_results[old_index]->IsNull()) {
      continue;
    }
    long new_index = 0;
    for (size_t j = 0; j < m_players.size(); ++j) {
      if (source[j] >= 0) {
        new_index +=
            ((old_index / old_strides[source[j]]) % old_radices[source[j]]) * new_strides[j];
      }
    }
    newResults[new_index] = m_results[old_index];
  }
  m_results.swap(newResults);
  IndexStrategies();
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
/// redimensioned (change in the number of strategies).  See the declaration
/// in gametable.h for the meaning of p_player/p_oldToNew.
void GameTableRep::RebuildTable(const std::vector<long> &old_radices, long p_player,
                                const std::vector<long> &p_oldToNew)
{
  std::vector<long> old_strides(old_radices.size());
  long stride = 1;
  for (size_t i = 0; i < old_radices.size(); ++i) {
    old_strides[i] = stride;
    stride *= old_radices[i];
  }
  const long old_size = stride;

  long new_size = 1;
  std::vector<long> new_strides(m_players.size());
  for (size_t i = 0; i < m_players.size(); ++i) {
    new_strides[i] = new_size;
    new_size *= m_players[i]->m_strategies.size();
  }

  std::vector<GameOutcomeRep *> newResults(new_size, m_nullOutcome.get());
  for (long old_index = 0; old_index < old_size; ++old_index) {
    if (m_results[old_index]->IsNull()) {
      continue;
    }
    long new_index = 0;
    bool dropped = false;
    for (size_t i = 0; i < m_players.size(); ++i) {
      long digit = (old_index / old_strides[i]) % old_radices[i];
      if (static_cast<long>(i) == p_player) {
        digit = p_oldToNew[digit];
        if (digit < 0) {
          // This contingency used a strategy that was removed.
          dropped = true;
          break;
        }
      }
      new_index += digit * new_strides[i];
    }
    if (!dropped) {
      newResults[new_index] = m_results[old_index];
    }
  }
  m_results.swap(newResults);
  IndexStrategies();
}

} // end namespace Gambit
