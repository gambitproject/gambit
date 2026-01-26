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
  std::shared_ptr<PureStrategyProfileRep> Copy() const override
  {
    return std::make_shared<TablePureStrategyProfileRep>(*this);
  }

public:
  explicit TablePureStrategyProfileRep(const Game &p_game) : PureStrategyProfileRep(p_game) {}
  GameOutcome GetOutcome() const override;
  void SetOutcome(GameOutcome p_outcome) override;
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
  if (const auto outcome = dynamic_cast<GameTableRep &>(*m_game).m_results.at(m_index)) {
    return outcome->shared_from_this();
  }
  return nullptr;
}

void TablePureStrategyProfileRep::SetOutcome(GameOutcome p_outcome)
{
  dynamic_cast<GameTableRep &>(*m_game).m_results[m_index] = p_outcome.get();
}

Rational TablePureStrategyProfileRep::GetPayoff(const GamePlayer &p_player) const
{
  if (const auto outcome = dynamic_cast<GameTableRep &>(*m_game).m_results.at(m_index)) {
    return outcome->GetPayoff<Rational>(p_player);
  }
  return Rational(0);
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
  if (const auto outcome = dynamic_cast<GameTableRep &>(*m_game).m_results[new_index]) {
    return outcome->GetPayoff<Rational>(player);
  }
  return Rational(0);
}

PureStrategyProfile GameTableRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(std::make_shared<TablePureStrategyProfileRep>(
      std::const_pointer_cast<GameRep>(shared_from_this())));
}

//========================================================================
//                   TableMixedStrategyProfileRep<T>
//========================================================================

template <class T> class ProductDistribution {
public:
  using index_type = long;
  using prob_type = T;
  using value_type = std::pair<index_type, prob_type>;

  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<index_type, prob_type>;
    using reference = value_type;
    using pointer = void;

    iterator() = default;

    iterator(const SegmentedVector<T> &probs, const SegmentedArray<long> &offsets, size_t skip1,
             size_t skip2, bool end)
      : m_probs(&probs), m_offsets(&offsets), m_done(end)
    {
      if (m_done) {
        return;
      }

      const size_t P = m_probs->GetShape().size();

      // Build active dimension list
      for (size_t p = 1; p <= P; ++p) {
        if (p != skip1 && p != skip2) {
          m_dims.push_back(p);
        }
      }

      m_K = m_dims.size();

      m_digit.assign(m_K, 0);
      m_radix.assign(m_K, 0);
      m_cum_prob.assign(m_K + 1, T{});

      // initialise radices
      for (size_t j = 0; j < m_K; ++j) {
        const size_t p = m_dims[j];
        m_radix[j] = m_probs->segment(p).size();
        if (m_radix[j] == 0) {
          m_done = true;
          return;
        }
      }

      // initial recompute
      recompute_from(0);
      advance_to_next_nonzero();
    }

    reference operator*() const { return {m_index, m_cum_prob[m_K]}; }

    iterator &operator++()
    {
      if (m_done) {
        return *this;
      }

      // increment odometer
      size_t j = 0;
      for (; j < m_K; ++j) {
        if (++m_digit[j] < m_radix[j]) {
          break;
        }
        m_digit[j] = 0;
      }

      if (j == m_K) {
        m_done = true;
        return *this;
      }

      recompute_from(j);
      advance_to_next_nonzero();
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator &other) const { return m_done == other.m_done; }

    bool operator!=(const iterator &other) const { return !(*this == other); }

  private:
    void recompute_from(size_t j0)
    {
      m_index = 0;
      m_cum_prob[0] = T{1};

      for (size_t j = 0; j < m_K; ++j) {
        const size_t p = m_dims[j];
        const size_t d = m_digit[j] + 1;

        const T pi = m_probs->segment(p)[d];
        m_cum_prob[j + 1] = m_cum_prob[j] * pi;
        m_index += m_offsets->segment(p)[d];
      }
    }

    void advance_to_next_nonzero()
    {
      while (!m_done && m_cum_prob[m_K] == T{0}) {
        size_t j = 0;
        for (; j < m_K; ++j) {
          if (++m_digit[j] < m_radix[j]) {
            break;
          }
          m_digit[j] = 0;
        }

        if (j == m_K) {
          m_done = true;
          return;
        }

        recompute_from(j);
      }
    }

    const SegmentedVector<T> *m_probs{nullptr};
    const SegmentedArray<long> *m_offsets{nullptr};

    std::vector<size_t> m_dims; // active player numbers
    size_t m_K{0};
    bool m_done{true};

    std::vector<size_t> m_digit;
    std::vector<size_t> m_radix;
    std::vector<T> m_cum_prob;
    index_type m_index{0};
  };

  ProductDistribution(const SegmentedVector<T> &probs, const SegmentedArray<long> &offsets)
    : m_probs(probs), m_offsets(offsets), m_skip1(0), m_skip2(0)
  {
  }

  ProductDistribution(const SegmentedVector<T> &probs, const SegmentedArray<long> &offsets,
                      size_t skip1)
    : m_probs(probs), m_offsets(offsets), m_skip1(skip1), m_skip2(0)
  {
  }

  ProductDistribution(const SegmentedVector<T> &probs, const SegmentedArray<long> &offsets,
                      size_t skip1, size_t skip2)
    : m_probs(probs), m_offsets(offsets), m_skip1(skip1), m_skip2(skip2)
  {
  }

  iterator begin() const { return iterator(m_probs, m_offsets, m_skip1, m_skip2, false); }

  iterator end() const { return iterator(m_probs, m_offsets, m_skip1, m_skip2, true); }

private:
  const SegmentedVector<T> &m_probs;
  const SegmentedArray<long> &m_offsets;
  size_t m_skip1;
  size_t m_skip2;
};

template <class T> class TableMixedStrategyProfileRep : public MixedStrategyProfileRep<T> {
public:
  explicit TableMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support),
      m_pureStrategies(p_support.GetGame()->m_pureStrategies)
  {
  }
  ~TableMixedStrategyProfileRep() override = default;

  std::unique_ptr<MixedStrategyProfileRep<T>> Copy() const override;
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  bool GetPayoffDerivs(int pl, Vector<T> &p_derivs) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

template <class T>
std::unique_ptr<MixedStrategyProfileRep<T>> TableMixedStrategyProfileRep<T>::Copy() const
{
  return std::make_unique<TableMixedStrategyProfileRep>(*this);
}

template <class T> T TableMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  const auto game = this->GetSupport().GetGame();
  auto &g = dynamic_cast<GameTableRep &>(*game);
  const auto player = game->GetPlayer(pl);
  T value{0};
  for (auto [index, prob] : ProductDistribution<T>(this->m_probs, this->m_offsets)) {
    if (const auto outcome = g.m_results[index]) {
      value += prob * outcome->template GetPayoff<T>(player);
    }
  }
  return value;
}

template <class T>
T TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &strategy) const
{
  const auto game = this->GetSupport().GetGame();
  auto &g = dynamic_cast<GameTableRep &>(*game);
  auto base_index = this->StrategyOffset(strategy);
  const auto player = game->GetPlayer(pl);
  T value{0};
  for (auto [index, prob] : ProductDistribution<T>(this->m_probs, this->m_offsets,
                                                   strategy->GetPlayer()->GetNumber())) {
    if (const auto outcome = g.m_results[base_index + index]) {
      value += prob * outcome->template GetPayoff<T>(player);
    }
  }
  return value;
}

template <class T>
bool TableMixedStrategyProfileRep<T>::GetPayoffDerivs(int pl, Vector<T> &p_derivs) const
{
  const auto game = this->GetSupport().GetGame();
  auto &g = dynamic_cast<GameTableRep &>(*game);
  const auto player = game->GetPlayer(pl);
  p_derivs = T{0};
  auto segment = this->m_offsets.segment(pl);
  for (auto [index, prob] : ProductDistribution<T>(this->m_probs, this->m_offsets, pl)) {
    auto deriv_it = p_derivs.begin();
    for (const auto base_index : segment) {
      if (const auto outcome = g.m_results[base_index + index]) {
        *deriv_it += prob * outcome->template GetPayoff<T>(player);
        ++deriv_it;
      }
    }
  }
  return true;
}

template <class T>
T TableMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &strategy1,
                                                  const GameStrategy &strategy2) const
{
  if (strategy1->GetPlayer() == strategy2->GetPlayer()) {
    return T{0};
  }
  const auto game = this->GetSupport().GetGame();
  auto &g = dynamic_cast<GameTableRep &>(*game);
  auto base_index = this->StrategyOffset(strategy1) + this->StrategyOffset(strategy2);
  const auto player = game->GetPlayer(pl);
  T value{0};
  for (auto [index, prob] :
       ProductDistribution<T>(this->m_probs, this->m_offsets, strategy1->GetPlayer()->GetNumber(),
                              strategy2->GetPlayer()->GetNumber())) {
    if (const auto outcome = g.m_results[base_index + index]) {
      value += prob * outcome->template GetPayoff<T>(player);
    }
  }
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
  std::vector<long> old_radices;
  for (const auto &player : m_players) {
    old_radices.push_back(player->m_strategies.size());
  }
  p_player->m_strategies.push_back(std::make_shared<GameStrategyRep>(
      p_player.get(), p_player->m_strategies.size() + 1, p_label));
  RebuildTable(old_radices);
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
void GameTableRep::RebuildTable(const std::vector<long> &old_radices)
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

  std::vector<GameOutcomeRep *> newResults(new_size, nullptr);
  for (long old_index = 0; old_index < old_size; ++old_index) {
    if (m_results[old_index] == nullptr) {
      continue;
    }
    long new_index = 0;
    for (size_t i = 0; i < m_players.size(); ++i) {
      const long digit = (old_index / old_strides[i]) % old_radices[i];
      new_index += digit * new_strides[i];
    }
    newResults[new_index] = m_results[old_index];
  }
  m_results.swap(newResults);
  IndexStrategies();
}

} // end namespace Gambit
