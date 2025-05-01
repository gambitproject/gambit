//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/game.cc
// Implementation of extensive form game representation
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
#include <numeric>
#include <random>

#include "gambit.h"
#include "writer.h"

// The references to the tree representations violate the logic
// of separating implementation types.  This will be fixed when we move
// editing operations into the game itself instead of in the member-object
// classes.
#include "gametree.h"

namespace Gambit {

//========================================================================
//                       class GameOutcomeRep
//========================================================================

GameOutcomeRep::GameOutcomeRep(GameRep *p_game, int p_number) : m_game(p_game), m_number(p_number)
{
  for (const auto &player : p_game->GetPlayers()) {
    m_payoffs[player] = Number();
  }
}

//========================================================================
//                       class GamePlayerRep
//========================================================================

GamePlayerRep::GamePlayerRep(GameRep *p_game, int p_id, int p_strats)
  : m_game(p_game), m_number(p_id)
{
  for (int j = 1; j <= p_strats; j++) {
    m_strategies.push_back(new GameStrategyRep(this, j, ""));
  }
}

GamePlayerRep::~GamePlayerRep()
{
  for (auto infoset : m_infosets) {
    infoset->Invalidate();
  }
  for (auto strategy : m_strategies) {
    strategy->Invalidate();
  }
}

Array<GameStrategy> GamePlayerRep::GetStrategies() const
{
  m_game->BuildComputedValues();
  Array<GameStrategy> ret(m_strategies.size());
  std::transform(m_strategies.cbegin(), m_strategies.cend(), ret.begin(),
                 [](const GameStrategyRep *s) -> GameStrategy { return s; });
  return ret;
}

void GamePlayerRep::MakeStrategy()
{
  Array<int> c(NumInfosets());

  for (size_t i = 1; i <= NumInfosets(); i++) {
    if (m_infosets[i - 1]->flag == 1) {
      c[i] = m_infosets[i - 1]->whichbranch;
    }
    else {
      c[i] = 0;
    }
  }

  auto *strategy = new GameStrategyRep(this, m_strategies.size() + 1, "");
  strategy->m_behav = c;

  // We generate a default labeling -- probably should be changed in future
  if (!strategy->m_behav.empty()) {
    for (size_t iset = 1; iset <= strategy->m_behav.size(); iset++) {
      if (strategy->m_behav[iset] > 0) {
        strategy->m_label += lexical_cast<std::string>(strategy->m_behav[iset]);
      }
      else {
        strategy->m_label += "*";
      }
    }
  }
  else {
    strategy->m_label = "*";
  }
  m_strategies.push_back(strategy);
}

void GamePlayerRep::MakeReducedStrats(GameNodeRep *n, GameNodeRep *nn)
{
  GameNodeRep *m;

  if (!n->GetParent()) {
    n->ptr = nullptr;
  }

  if (n->NumChildren() > 0) {
    if (n->m_infoset->m_player == this) {
      if (n->m_infoset->flag == 0) {
        // we haven't visited this infoset before
        n->m_infoset->flag = 1;
        for (size_t i = 1; i <= n->NumChildren(); i++) {
          GameNodeRep *m = n->m_children[i - 1];
          n->whichbranch = m;
          n->m_infoset->whichbranch = i;
          MakeReducedStrats(m, nn);
        }
        n->m_infoset->flag = 0;
      }
      else {
        // we have visited this infoset, take same action
        MakeReducedStrats(n->m_children[n->m_infoset->whichbranch - 1], nn);
      }
    }
    else {
      n->ptr = nullptr;
      if (nn != nullptr) {
        n->ptr = nn->m_parent;
      }
      n->whichbranch = n->m_children.front();
      if (n->m_infoset) {
        n->m_infoset->whichbranch = 0;
      }
      MakeReducedStrats(n->m_children.front(), n->m_children.front());
    }
  }
  else if (nn) {
    for (;; nn = nn->m_parent->ptr->whichbranch) {
      if (!nn->GetNextSibling()) {
        m = nullptr;
      }
      else {
        m = nn->GetNextSibling();
      }
      if (m || nn->m_parent->ptr == nullptr) {
        break;
      }
    }
    if (m) {
      GameNodeRep *mm = m->m_parent->whichbranch;
      m->m_parent->whichbranch = m;
      MakeReducedStrats(m, m);
      m->m_parent->whichbranch = mm;
    }
    else {
      MakeStrategy();
    }
  }
  else {
    MakeStrategy();
  }
}

GameInfoset GamePlayerRep::GetInfoset(int p_index) const { return m_infosets[p_index - 1]; }

Array<GameInfoset> GamePlayerRep::GetInfosets() const
{
  Array<GameInfoset> ret(m_infosets.size());
  std::transform(m_infosets.cbegin(), m_infosets.cend(), ret.begin(),
                 [](const GameInfosetRep *s) -> GameInfoset { return s; });
  return ret;
}

size_t GamePlayerRep::NumSequences() const
{
  if (!m_game->IsTree()) {
    throw UndefinedException();
  }
  return std::accumulate(
      m_infosets.cbegin(), m_infosets.cend(), 1,
      [](int ct, const GameInfosetRep *s) -> int { return ct + s->m_actions.size(); });
}

//========================================================================
//                            class GameRep
//========================================================================

Array<GamePlayer> GameRep::GetPlayers() const
{
  Array<GamePlayer> ret(NumPlayers());
  for (size_t pl = 1; pl <= NumPlayers(); pl++) {
    ret[pl] = GetPlayer(pl);
  }
  return ret;
}

Array<GameStrategy> GameRep::GetStrategies() const
{
  Array<GameStrategy> ret(MixedProfileLength());
  auto output = ret.begin();
  for (auto player : GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      *output = strategy;
      ++output;
    }
  }
  return ret;
}

GameRep::~GameRep()
{
  for (auto player : m_players) {
    player->Invalidate();
  }
  for (auto outcome : m_outcomes) {
    outcome->Invalidate();
  }
}

//------------------------------------------------------------------------
//                     GameRep: Writing data files
//------------------------------------------------------------------------

///
/// Write the game to a savefile in .nfg payoff format.
///
/// This implements writing a game to a .nfg savefile.  As it uses
/// only publicly-accessible operations, it is in principle valid
/// for any game.  It writes the payoff format, as it is not required
/// that a game representation implements an outcome for each possible
/// pure strategy profile.  For example, in extensive games with chance
/// moves, the outcome from a pure strategy profile is not deterministic,
/// but rather a probability distribution over outcomes.
///
void GameRep::WriteNfgFile(std::ostream &p_file) const
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

  for (auto iter :
       StrategyContingencies(StrategySupportProfile(Game(const_cast<GameRep *>(this))))) {
    p_file << FormatList(
                  players,
                  [&iter](const GamePlayer &p) {
                    return lexical_cast<std::string>(iter->GetPayoff(p));
                  },
                  false, false)
           << std::endl;
  };
}

//========================================================================
//                     MixedStrategyProfileRep<T>
//========================================================================

template <class T>
MixedStrategyProfileRep<T>::MixedStrategyProfileRep(const StrategySupportProfile &p_support)
  : m_probs(p_support.MixedProfileLength()), m_support(p_support),
    m_gameversion(p_support.GetGame()->GetVersion())
{
  int index = 1;
  for (auto player : p_support.GetGame()->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      if (p_support.Contains(strategy)) {
        m_profileIndex[strategy] = index++;
      }
      else {
        m_profileIndex[strategy] = -1;
      }
    }
  }
  SetCentroid();
}

template <class T> void MixedStrategyProfileRep<T>::SetCentroid()
{
  for (auto player : m_support.GetGame()->GetPlayers()) {
    T center = T(1) / T(m_support.GetStrategies(player).size());
    for (auto strategy : m_support.GetStrategies(player)) {
      (*this)[strategy] = center;
    }
  }
}

template <class T> MixedStrategyProfileRep<T> *MixedStrategyProfileRep<T>::Normalize() const
{
  auto norm = Copy();
  for (auto player : m_support.GetGame()->GetPlayers()) {
    T sum = static_cast<T>(0);
    for (auto strategy : m_support.GetStrategies(player)) {
      sum += (*this)[strategy];
    }
    if (sum == static_cast<T>(0)) {
      continue;
    }
    for (auto strategy : m_support.GetStrategies(player)) {
      (*norm)[strategy] /= sum;
    }
  }
  return norm;
}

template <class T> T MixedStrategyProfileRep<T>::GetRegret(const GameStrategy &p_strategy) const
{
  const GamePlayer player = p_strategy->GetPlayer();
  T payoff = GetPayoffDeriv(player->GetNumber(), p_strategy);
  T brpayoff = payoff;
  for (auto strategy : player->GetStrategies()) {
    if (strategy != p_strategy) {
      brpayoff = std::max(brpayoff, GetPayoffDeriv(player->GetNumber(), strategy));
    }
  }
  return brpayoff - payoff;
}

template <class T> T MixedStrategyProfileRep<T>::GetRegret(const GamePlayer &p_player) const
{
  auto strategies = p_player->GetStrategies();
  T br_payoff = std::accumulate(std::next(strategies.begin()), strategies.end(),
                                GetPayoff(*strategies.begin()),
                                [this](const T &x, const GameStrategy &strategy) {
                                  return std::max(x, GetPayoff(strategy));
                                });
  return br_payoff - GetPayoff(p_player);
}

template <class T> T MixedStrategyProfileRep<T>::GetMaxRegret() const
{
  auto players = m_support.GetGame()->GetPlayers();
  return std::accumulate(players.begin(), players.end(), T(0),
                         [this](const T &x, const GamePlayer &player) {
                           return std::max(x, this->GetRegret(player));
                         });
}

//========================================================================
//                 MixedStrategyProfile<T>: Lifecycle
//========================================================================

template <class T>
MixedStrategyProfile<T>::MixedStrategyProfile(const MixedBehaviorProfile<T> &p_profile)
  : m_rep(new TreeMixedStrategyProfileRep<T>(p_profile))
{
  auto *efg = dynamic_cast<GameTreeRep *>(p_profile.GetGame().operator->());
  for (const auto &player : efg->m_players) {
    for (const auto &strategy : player->m_strategies) {
      auto prob = static_cast<T>(1);
      for (const auto &infoset : player->m_infosets) {
        if (strategy->m_behav[infoset->GetNumber()] > 0) {
          prob *= p_profile[infoset->GetAction(strategy->m_behav[infoset->GetNumber()])];
        }
      }
      (*m_rep)[strategy] = prob;
    }
  }
}

template <class T>
MixedStrategyProfile<T> &
MixedStrategyProfile<T>::operator=(const MixedStrategyProfile<T> &p_profile)
{
  if (this != &p_profile) {
    InvalidateCache();
    m_rep.reset(p_profile.m_rep->Copy());
  }
  return *this;
}

//========================================================================
//             MixedStrategyProfile<T>: General data access
//========================================================================

template <class T> Vector<T> MixedStrategyProfile<T>::operator[](const GamePlayer &p_player) const
{
  CheckVersion();
  auto strategies = m_rep->m_support.GetStrategies(p_player);
  Vector<T> probs(strategies.size());
  std::transform(strategies.begin(), strategies.end(), probs.begin(),
                 [this](const GameStrategy &s) { return (*m_rep)[s]; });
  return probs;
}

template <class T> MixedStrategyProfile<T> MixedStrategyProfile<T>::ToFullSupport() const
{
  CheckVersion();
  MixedStrategyProfile<T> full(m_rep->m_support.GetGame()->NewMixedStrategyProfile(T(0)));

  for (const auto &player : m_rep->m_support.GetGame()->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      full[strategy] =
          (m_rep->m_support.Contains(strategy)) ? (*m_rep)[strategy] : static_cast<T>(0);
    }
  }
  return full;
}

//========================================================================
//    MixedStrategyProfile<T>: Computation of interesting quantities
//========================================================================

template <class T> void MixedStrategyProfile<T>::ComputePayoffs() const
{
  if (!map_profile_payoffs.empty()) {
    // caches (map_profile_payoffs and map_strategy_payoffs) are valid,
    // so don't compute anything, simply return
    return;
  }
  for (const auto &player : m_rep->m_support.GetPlayers()) {
    map_profile_payoffs[player] = GetPayoff(player);
    // values of the player's strategies
    for (const auto &strategy : m_rep->m_support.GetStrategies(player)) {
      map_strategy_payoffs[player][strategy] = GetPayoff(strategy);
    }
  }
};

template <class T> T MixedStrategyProfile<T>::GetLiapValue() const
{
  CheckVersion();
  ComputePayoffs();

  auto liapValue = static_cast<T>(0);
  for (auto [player, payoff] : map_profile_payoffs) {
    for (auto v : map_strategy_payoffs[player]) {
      liapValue += sqr(std::max(v.second - payoff, static_cast<T>(0)));
    }
  }
  return liapValue;
}

template class MixedStrategyProfileRep<double>;
template class MixedStrategyProfileRep<Rational>;

template class MixedStrategyProfile<double>;
template class MixedStrategyProfile<Rational>;

} // end namespace Gambit
