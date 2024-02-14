//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
#include <algorithm>
#include <numeric>
#include <random>

#include "gambit.h"

// The references to the table and tree representations violate the logic
// of separating implementation types.  This will be fixed when we move
// editing operations into the game itself instead of in the member-object
// classes.
#include "gametable.h"
#include "gametree.h"

namespace Gambit {

//========================================================================
//                       class GameOutcomeRep
//========================================================================

GameOutcomeRep::GameOutcomeRep(GameRep *p_game, int p_number)
  : m_game(p_game), m_number(p_number), m_payoffs(m_game->NumPlayers())
{
}

//========================================================================
//                      class GameStrategyRep
//========================================================================

void GameStrategyRep::DeleteStrategy()
{
  if (m_player->GetGame()->IsTree()) {
    throw UndefinedException();
  }
  if (m_player->NumStrategies() == 1) {
    return;
  }

  m_player->GetGame()->IncrementVersion();
  m_player->m_strategies.Remove(m_player->m_strategies.Find(this));
  for (int st = 1; st <= m_player->m_strategies.Length(); st++) {
    m_player->m_strategies[st]->m_number = st;
  }
  // m_player->m_game->RebuildTable();
  this->Invalidate();
}

//========================================================================
//                       class GamePlayerRep
//========================================================================

GamePlayerRep::GamePlayerRep(GameRep *p_game, int p_id, int p_strats)
  : m_game(p_game), m_number(p_id), m_strategies(p_strats)
{
  for (int j = 1; j <= p_strats; j++) {
    m_strategies[j] = new GameStrategyRep(this);
    m_strategies[j]->m_number = j;
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
                 [](GameStrategyRep *s) -> GameStrategy { return s; });
  return ret;
}

GameStrategy GamePlayerRep::NewStrategy()
{
  if (m_game->IsTree()) {
    throw UndefinedException();
  }

  m_game->IncrementVersion();
  auto *strategy = new GameStrategyRep(this);
  m_strategies.push_back(strategy);
  strategy->m_number = m_strategies.size();
  strategy->m_offset = -1; // this flags this action as new
  dynamic_cast<GameTableRep *>(m_game)->RebuildTable();
  return strategy;
}

void GamePlayerRep::MakeStrategy()
{
  Array<int> c(NumInfosets());

  for (int i = 1; i <= NumInfosets(); i++) {
    if (m_infosets[i]->flag == 1) {
      c[i] = m_infosets[i]->whichbranch;
    }
    else {
      c[i] = 0;
    }
  }

  auto *strategy = new GameStrategyRep(this);
  m_strategies.push_back(strategy);
  strategy->m_number = m_strategies.Length();
  strategy->m_behav = c;
  strategy->m_label = "";

  // We generate a default labeling -- probably should be changed in future
  if (!strategy->m_behav.empty()) {
    for (int iset = 1; iset <= strategy->m_behav.Length(); iset++) {
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
}

void GamePlayerRep::MakeReducedStrats(GameTreeNodeRep *n, GameTreeNodeRep *nn)
{
  int i;
  GameTreeNodeRep *m, *mm;

  if (!n->GetParent()) {
    n->ptr = nullptr;
  }

  if (n->NumChildren() > 0) {
    if (n->infoset->m_player == this) {
      if (n->infoset->flag == 0) {
        // we haven't visited this infoset before
        n->infoset->flag = 1;
        for (i = 1; i <= n->NumChildren(); i++) {
          GameTreeNodeRep *m = n->children[i];
          n->whichbranch = m;
          n->infoset->whichbranch = i;
          MakeReducedStrats(m, nn);
        }
        n->infoset->flag = 0;
      }
      else {
        // we have visited this infoset, take same action
        MakeReducedStrats(n->children[n->infoset->whichbranch], nn);
      }
    }
    else {
      n->ptr = nullptr;
      if (nn != nullptr) {
        n->ptr = nn->m_parent;
      }
      n->whichbranch = n->children[1];
      if (n->infoset) {
        n->infoset->whichbranch = 0;
      }
      MakeReducedStrats(n->children[1], n->children[1]);
    }
  }
  else if (nn) {
    for (;; nn = nn->m_parent->ptr->whichbranch) {
      if (!nn->GetNextSibling()) {
        m = nullptr;
      }
      else {
        m = dynamic_cast<GameTreeNodeRep *>(nn->GetNextSibling().operator->());
      }
      if (m || nn->m_parent->ptr == nullptr) {
        break;
      }
    }
    if (m) {
      mm = m->m_parent->whichbranch;
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

GameInfoset GamePlayerRep::GetInfoset(int p_index) const { return m_infosets[p_index]; }

Array<GameInfoset> GamePlayerRep::GetInfosets() const
{
  Array<GameInfoset> ret(m_infosets.size());
  std::transform(m_infosets.cbegin(), m_infosets.cend(), ret.begin(),
                 [](GameTreeInfosetRep *s) -> GameInfoset { return s; });
  return ret;
}

int GamePlayerRep::NumSequences() const
{
  if (!m_game->IsTree()) {
    throw UndefinedException();
  }
  return std::accumulate(
      m_infosets.cbegin(), m_infosets.cend(), 1,
      [](int ct, GameTreeInfosetRep *s) -> int { return ct + s->m_actions.size(); });
}

//========================================================================
//                            class GameRep
//========================================================================

Array<GamePlayer> GameRep::GetPlayers() const
{
  Array<GamePlayer> ret(NumPlayers());
  for (int pl = 1; pl <= NumPlayers(); pl++) {
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

//------------------------------------------------------------------------
//                     GameRep: Writing data files
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
  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";
  for (auto player : players) {
    p_file << '"' << EscapeQuotes(player->GetLabel()) << "\" ";
  }
  p_file << "}\n\n{ ";

  for (auto player : players) {
    p_file << "{ ";
    for (auto strategy : player->GetStrategies()) {
      p_file << '"' << EscapeQuotes(strategy->GetLabel()) << "\" ";
    }
    p_file << "}\n";
  }
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  for (StrategyProfileIterator iter(StrategySupportProfile(Game(const_cast<GameRep *>(this))));
       !iter.AtEnd(); iter++) {
    for (auto player : players) {
      p_file << (*iter)->GetPayoff(player) << " ";
    }
    p_file << "\n";
  }
  p_file << '\n';
}

//========================================================================
//                     MixedStrategyProfileRep<T>
//========================================================================

template <class T>
MixedStrategyProfileRep<T>::MixedStrategyProfileRep(const StrategySupportProfile &p_support)
  : m_probs(p_support.MixedProfileLength()), m_support(p_support),
    m_gameversion(p_support.GetGame()->GetVersion())
{
  SetCentroid();
}

template <class T> void MixedStrategyProfileRep<T>::SetCentroid()
{
  for (auto player : m_support.GetGame()->GetPlayers()) {
    T center = ((T)1) / ((T)m_support.NumStrategies(player->GetNumber()));
    for (auto strategy : m_support.GetStrategies(player)) {
      (*this)[strategy] = center;
    }
  }
}

template <class T> MixedStrategyProfileRep<T> *MixedStrategyProfileRep<T>::Normalize() const
{
  auto norm = Copy();
  for (auto player : m_support.GetGame()->GetPlayers()) {
    T sum = (T)0;
    for (auto strategy : m_support.GetStrategies(player)) {
      sum += (*this)[strategy];
    }
    if (sum == (T)0) {
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
  GamePlayer player = p_strategy->GetPlayer();
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
  Game game = p_profile.GetGame();
  auto *efg = dynamic_cast<GameTreeRep *>(game.operator->());
  for (int pl = 1; pl <= m_rep->m_support.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = m_rep->m_support.GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      T prob = (T)1;

      for (int iset = 1; iset <= efg->GetPlayer(pl)->NumInfosets(); iset++) {
        if (efg->m_players[pl]->m_strategies[st]->m_behav[iset] > 0) {
          GameInfoset infoset = player->GetInfoset(iset);
          prob *=
              p_profile[infoset->GetAction(efg->m_players[pl]->m_strategies[st]->m_behav[iset])];
        }
      }
      (*this)[m_rep->m_support.GetGame()->GetPlayer(pl)->GetStrategy(st)] = prob;
    }
  }
}

template <class T>
MixedStrategyProfile<T>::MixedStrategyProfile(const MixedStrategyProfile<T> &p_profile)
  : m_rep(p_profile.m_rep->Copy())
{
  InvalidateCache();
}

template <class T>
MixedStrategyProfile<T> &
MixedStrategyProfile<T>::operator=(const MixedStrategyProfile<T> &p_profile)
{
  if (this != &p_profile) {
    InvalidateCache();
    delete m_rep;
    m_rep = p_profile.m_rep->Copy();
  }
  return *this;
}

template <class T> MixedStrategyProfile<T>::~MixedStrategyProfile() { delete m_rep; }

//========================================================================
//             MixedStrategyProfile<T>: General data access
//========================================================================

template <class T> Vector<T> MixedStrategyProfile<T>::operator[](const GamePlayer &p_player) const
{
  CheckVersion();
  auto strategies = m_rep->m_support.GetStrategies(p_player);
  Vector<T> probs(strategies.size());
  int st = 1;
  for (auto strategy : strategies) {
    probs[st] = (*this)[strategy];
  }
  return probs;
}

template <class T> MixedStrategyProfile<T> MixedStrategyProfile<T>::ToFullSupport() const
{
  CheckVersion();
  MixedStrategyProfile<T> full(m_rep->m_support.GetGame()->NewMixedStrategyProfile(T(0)));

  for (int pl = 1; pl <= m_rep->m_support.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = m_rep->m_support.GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (m_rep->m_support.Contains(player->GetStrategy(st))) {
        full[player->GetStrategy(st)] = (*this)[player->GetStrategy(st)];
      }
      else {
        full[player->GetStrategy(st)] = static_cast<T>(0);
      }
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
  for (auto player : m_rep->m_support.GetPlayers()) {
    map_profile_payoffs[player] = GetPayoff(player);
    // values of the player's strategies
    for (auto strategy : m_rep->m_support.GetStrategies(player)) {
      map_strategy_payoffs[player][strategy] = GetPayoff(strategy);
    }
  }
};

template <class T> T MixedStrategyProfile<T>::GetLiapValue() const
{
  CheckVersion();
  ComputePayoffs();

  T liapValue = T(0);
  for (auto player : m_rep->m_support.GetPlayers()) {
    for (auto v : map_strategy_payoffs[player]) {
      T regret = v.second - map_profile_payoffs[player];
      if (regret > T(0)) {
        liapValue += regret * regret; // penalty if not best response
      }
    }
  }
  return liapValue;
}

template class MixedStrategyProfileRep<double>;
template class MixedStrategyProfileRep<Rational>;

template class MixedStrategyProfile<double>;
template class MixedStrategyProfile<Rational>;

} // end namespace Gambit
