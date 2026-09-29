//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/gamebagg.cc
// Implementation of Bayesian action-graph game representation
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

#include <algorithm>
#include <iostream>
#include <type_traits>

#include "games.h"
#include "gamebagg.h"

namespace Gambit {

//========================================================================
//                  class BAGGPureStrategyProfileRep
//========================================================================

class BAGGPureStrategyProfileRep : public PureStrategyProfileRep {
public:
  explicit BAGGPureStrategyProfileRep(const Game &p_game) : PureStrategyProfileRep(p_game) {}
  std::shared_ptr<PureStrategyProfileRep> Copy() const override
  {
    return std::make_shared<BAGGPureStrategyProfileRep>(*this);
  }
  GameOutcome GetOutcome() const override { throw UndefinedException(); }
  Rational GetPayoff(const GamePlayer &) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//       BAGGPureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

Rational BAGGPureStrategyProfileRep::GetPayoff(const GamePlayer &p_player) const
{
  const std::shared_ptr<agg::BAGG> baggPtr = dynamic_cast<GameBAGGRep &>(*m_game).baggPtr;
  std::vector<int> s(m_game->NumPlayers());
  for (size_t i = 1; i <= m_game->NumPlayers(); i++) {
    s[i - 1] = GetStrategy(m_game->GetPlayer(i))->GetNumber() - 1;
  }
  const int bp = dynamic_cast<GameBAGGRep &>(*m_game).agent2baggPlayer[p_player->GetNumber()];
  const int tp = p_player->GetNumber() - 1 - baggPtr->typeOffset[bp - 1];
  return baggPtr->getPurePayoff<Rational>(bp - 1, tp, s);
}

Rational BAGGPureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  const int player = p_strategy->GetPlayer()->GetNumber();
  const std::shared_ptr<agg::BAGG> baggPtr = dynamic_cast<GameBAGGRep &>(*m_game).baggPtr;
  std::vector<int> s(m_game->NumPlayers());
  for (size_t i = 1; i <= m_game->NumPlayers(); i++) {
    s[i - 1] = GetStrategy(m_game->GetPlayer(i))->GetNumber() - 1;
  }
  s[player - 1] = p_strategy->GetNumber() - 1;
  const int bp = dynamic_cast<GameBAGGRep &>(*m_game).agent2baggPlayer[player];
  const int tp = player - 1 - baggPtr->typeOffset[bp - 1];
  return baggPtr->getPurePayoff<Rational>(bp - 1, tp, s);
}

//========================================================================
//                  class BAGGMixedStrategyProfileRep
//========================================================================

template <class T> class BAGGMixedStrategyProfileRep : public MixedStrategyProfileRep<T> {

public:
  explicit BAGGMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  {
  }
  ~BAGGMixedStrategyProfileRep() override = default;

  std::unique_ptr<MixedStrategyProfileRep<T>> Copy() const override
  {
    return std::make_unique<BAGGMixedStrategyProfileRep>(*this);
  }
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
  void GetPayoffDerivBlock(int pl, Vector<T> &p_values, Matrix<T> &p_derivs) const override;

private:
  T GetPartialDeriv(int pl, const std::vector<GameStrategy> &p_strategies) const;
};

template <class T> T BAGGMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  auto &g = dynamic_cast<GameBAGGRep &>(*(this->m_support.GetGame()));
  std::vector<T> s(g.GetStrategies().size());
  const auto ns = g.GetStrategies().shape();
  int bplayer = -1, btype = -1;
  for (int i = 0, offs = 0; i < g.baggPtr->getNumPlayers(); ++i) {
    for (int tp = 0; tp < g.baggPtr->getNumTypes(i); ++tp) {
      if (pl == g.baggPtr->typeOffset[i] + tp + 1) {
        bplayer = i;
        btype = tp;
      }
      for (size_t j = 0; j < ns[g.baggPtr->typeOffset[i] + tp]; ++j, ++offs) {
        const GameStrategy strategy = this->m_support.GetGame()
                                          ->GetPlayer(g.baggPtr->typeOffset[i] + tp + 1)
                                          ->GetStrategy(j + 1);
        const int ind = this->m_profileIndex.at(strategy);
        s.at(offs) = (ind == -1) ? T(0) : this->m_probs.GetFlattened()[ind];
      }
    }
  }
  return g.baggPtr->getMixedPayoff(bplayer, btype, s);
}

// The payoff to agent pl is linear in its own mixed strategy and in the mixed strategy each
// other BAGG player induces on its AGG actions, to which each of that player's types t
// contributes with weight P(t); it does not depend on the strategies of the other types of pl's
// own BAGG player.  The partial derivative with respect to the probabilities of p_strategies is
// therefore the payoff when each is played for sure (for another BAGG player, by all of its
// types), times the probabilities of the types of the other BAGG players involved.  It is zero
// if two of the strategies belong to one BAGG player, or one belongs to another type of pl's.
template <class T>
T BAGGMixedStrategyProfileRep<T>::GetPartialDeriv(
    int pl, const std::vector<GameStrategy> &p_strategies) const
{
  const auto game = this->m_support.GetGame();
  auto &g = dynamic_cast<GameBAGGRep &>(*game);
  const auto &bagg = *g.baggPtr;
  auto &agg = *bagg.aggPtr;
  const int bplayer = g.agent2baggPlayer[pl] - 1;
  const int btype = pl - 1 - bagg.typeOffset[bplayer];

  int ownAction = -1;
  // Each other BAGG player involved, with the AGG action it plays for sure
  std::vector<std::pair<int, int>> fixed;
  T weight(1);
  for (const auto &strategy : p_strategies) {
    const int agent = strategy->GetPlayer()->GetNumber();
    const int player = g.agent2baggPlayer[agent] - 1;
    const int type = agent - 1 - bagg.typeOffset[player];
    if (agent == pl) {
      if (ownAction != -1) {
        return T(0);
      }
      ownAction = strategy->GetNumber() - 1;
      continue;
    }
    if (player == bplayer || std::any_of(fixed.begin(), fixed.end(),
                                         [&](const auto &f) { return f.first == player; })) {
      return T(0);
    }
    fixed.emplace_back(player,
                       agg.firstAction(player) +
                           bagg.typeAction2ActionIndex[player][type][strategy->GetNumber() - 1]);
    if constexpr (std::is_same_v<T, Rational>) {
      weight *= static_cast<Rational>(bagg.exactIndepTypeDist[player][type]);
    }
    else {
      weight *= bagg.indepTypeDist[player][type];
    }
  }

  std::vector<T> s(bagg.strategyOffset[bagg.getNumTypes()]);
  for (int k = 1, offset = 0; k <= bagg.getNumTypes(); k++) {
    for (const auto &strategy : game->GetPlayer(k)->GetStrategies()) {
      const int ind = this->m_profileIndex.at(strategy);
      s[offset++] = (ind == -1) ? T(0) : this->m_probs.GetFlattened()[ind];
    }
  }

  std::vector<T> as(agg.getNumActions());
  T value(0);
  for (int action = 0; action < bagg.getNumActions(bplayer, btype); action++) {
    const T prob = (ownAction == -1) ? s[bagg.firstAction(bplayer, btype) + action]
                                     : T((action == ownAction) ? 1 : 0);
    if (prob <= T(0)) {
      continue;
    }
    bagg.getAGGStrat(as, s, bplayer, btype, action);
    for (const auto &[player, index] : fixed) {
      std::fill(as.begin() + agg.firstAction(player),
                as.begin() + agg.firstAction(player) + agg.getNumActions(player), T(0));
      as[index] = T(1);
    }
    value += prob * agg.getV(bplayer, bagg.typeAction2ActionIndex[bplayer][btype][action], as);
  }
  return (fixed.empty()) ? value : weight * value;
}

template <class T>
T BAGGMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &ps) const
{
  return GetPartialDeriv(pl, {ps});
}

template <class T>
T BAGGMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &ps1,
                                                 const GameStrategy &ps2) const
{
  return GetPartialDeriv(pl, {ps1, ps2});
}

// See GetPartialDeriv.  The derivative with respect to an action b of type tq of another BAGG
// player j is P(tq) times the payoff when j plays b's AGG action for sure, which is one entry of
// a row of the AGG payoff Jacobian.  The Jacobian row is available in floating point only; exact
// computation uses the default.
template <class T>
void BAGGMixedStrategyProfileRep<T>::GetPayoffDerivBlock(int pl, Vector<T> &p_values,
                                                         Matrix<T> &p_derivs) const
{
  if constexpr (!std::is_same_v<T, double>) {
    MixedStrategyProfileRep<T>::GetPayoffDerivBlock(pl, p_values, p_derivs);
  }
  else {
    const auto game = this->m_support.GetGame();
    const auto &bagg = *dynamic_cast<GameBAGGRep &>(*game).baggPtr;
    auto &agg = *bagg.aggPtr;
    const int numAgents = bagg.getNumTypes();

    // The BAGG strategy vector, and the profile position of each of its entries (-1 if the
    // strategy is not in the support); agent k's strategies start at strategyOffset[k - 1]
    std::vector<double> s(bagg.strategyOffset[numAgents]);
    std::vector<int> column(s.size());
    for (int k = 1; k <= numAgents; k++) {
      int offset = bagg.strategyOffset[k - 1];
      for (const auto &strategy : game->GetPlayer(k)->GetStrategies()) {
        const int ind = this->m_profileIndex.at(strategy);
        s[offset] = (ind == -1) ? 0.0 : this->m_probs.GetFlattened()[ind];
        column[offset++] = ind;
      }
    }

    int bplayer = 0;
    while (bagg.typeOffset[bplayer + 1] < pl) {
      bplayer++;
    }
    const int btype = pl - 1 - bagg.typeOffset[bplayer];
    std::vector<double> as(agg.getNumActions());
    bagg.getAGGStrat(as, s, bplayer, btype, 0);
    const int ownFirst = agg.firstAction(bplayer);
    const auto &ownActions = bagg.typeAction2ActionIndex[bplayer][btype];
    as[ownFirst + ownActions[0]] = 0.0;

    const auto &strategies = this->m_support.GetStrategies(game->GetPlayer(pl));
    p_values = Vector<double>(strategies.size());
    p_derivs = Matrix<double>(strategies.size(), this->m_probs.GetFlattened().size());
    p_derivs = 0.0;
    std::vector<double> row(agg.getNumActions());
    size_t r = 1;
    for (const auto &strategy : strategies) {
      const int action = ownActions[strategy->GetNumber() - 1];
      as[ownFirst + action] = 1.0;
      const double value = agg.getV(bplayer, action, as);
      p_values[r] = value;
      agg.getPayoffJacobianRow(bplayer, action, as, row);
      as[ownFirst + action] = 0.0;

      for (int j = 0; j < bagg.getNumPlayers(); j++) {
        if (j == bplayer) {
          continue;
        }
        const double *payoffs = row.data() + agg.firstAction(j);
        for (int tq = 0; tq < bagg.getNumTypes(j); tq++) {
          const int first = bagg.strategyOffset[bagg.typeOffset[j] + tq];
          const auto &actions = bagg.typeAction2ActionIndex[j][tq];
          const double weight = bagg.indepTypeDist[j][tq];
          for (int b = 0; b < bagg.getNumActions(j, tq); b++) {
            if (column[first + b] != -1) {
              p_derivs(r, column[first + b]) = weight * payoffs[actions[b]];
            }
          }
        }
      }
      r++;
    }
  }
}

template class BAGGMixedStrategyProfileRep<double>;
template class BAGGMixedStrategyProfileRep<Rational>;

//------------------------------------------------------------------------
//                      GameBAGGRep: Lifecycle
//------------------------------------------------------------------------

GameBAGGRep::GameBAGGRep(std::shared_ptr<agg::BAGG> _baggPtr)
  : baggPtr(_baggPtr), agent2baggPlayer(_baggPtr->getNumTypes())
{
  int k = 1;
  for (int pl = 1; pl <= baggPtr->getNumPlayers(); pl++) {
    for (int j = 0; j < baggPtr->getNumTypes(pl - 1); j++, k++) {
      m_players.push_back(std::make_shared<GamePlayerRep>(this, k, std::to_string(k),
                                                          baggPtr->getNumActions(pl - 1, j)));
      agent2baggPlayer[k] = pl;
      std::for_each(m_players.back()->m_strategies.begin(), m_players.back()->m_strategies.end(),
                    [st = 1](const std::shared_ptr<GameStrategyRep> &s) mutable {
                      s->m_label = std::to_string(st++);
                    });
    }
  }
  IndexStrategies();
}

Game GameBAGGRep::Copy() const
{
  std::ostringstream os;
  WriteBaggFile(os);
  std::istringstream is(os.str());
  return ReadBaggFile(is);
}

//------------------------------------------------------------------------
//                  GameBAGGRep: General data access
//------------------------------------------------------------------------

bool GameBAGGRep::IsConstSum() const
{
  auto payoff_sum = [&](const PureStrategyProfile &p) {
    return sum_function(m_players, [&](const auto &player) { return p->GetPayoff(player); });
  };
  const Rational sum = payoff_sum(NewPureStrategyProfile());

  auto contingencies = StrategyContingencies(std::const_pointer_cast<GameRep>(shared_from_this()));
  return std::all_of(contingencies.begin(), contingencies.end(),
                     [&](const PureStrategyProfile &p) { return payoff_sum(p) == sum; });
}

Rational GameBAGGRep::GetPlayerMinPayoff(const GamePlayer &p_player) const
{
  Rational minpay = NewPureStrategyProfile()->GetPayoff(p_player);
  for (const auto &profile :
       StrategyContingencies(std::const_pointer_cast<GameRep>(shared_from_this()))) {
    minpay = std::min(minpay, profile->GetPayoff(p_player));
  }
  return minpay;
}

Rational GameBAGGRep::GetPlayerMaxPayoff(const GamePlayer &p_player) const
{
  Rational maxpay = NewPureStrategyProfile()->GetPayoff(p_player);
  for (const auto &profile :
       StrategyContingencies(std::const_pointer_cast<GameRep>(shared_from_this()))) {
    maxpay = std::max(maxpay, profile->GetPayoff(p_player));
  }
  return maxpay;
}

//------------------------------------------------------------------------
//                 GameBAGGRep: Dimensions of the game
//------------------------------------------------------------------------

PureStrategyProfile GameBAGGRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(std::make_shared<BAGGPureStrategyProfileRep>(
      std::const_pointer_cast<GameRep>(shared_from_this())));
}

MixedStrategyProfile<double> GameBAGGRep::NewMixedStrategyProfile(double) const
{
  return MixedStrategyProfile<double>(std::make_unique<BAGGMixedStrategyProfileRep<double>>(
      StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this()))));
}

MixedStrategyProfile<Rational> GameBAGGRep::NewMixedStrategyProfile(const Rational &) const
{
  // BAGG supports exact payoff computation on Rational profiles throughout, via the same
  // convolution algorithm as the double engine (getMixedPayoff<Rational> et al.); see
  // BAGGMixedStrategyProfileRep<Rational>::GetPayoff.
  return MixedStrategyProfile<Rational>(std::make_unique<BAGGMixedStrategyProfileRep<Rational>>(
      StrategySupportProfile(std::const_pointer_cast<GameRep>(shared_from_this()))));
}
MixedStrategyProfile<double>
GameBAGGRep::NewMixedStrategyProfile(double, const StrategySupportProfile &spt) const
{
  return MixedStrategyProfile<double>(std::make_unique<BAGGMixedStrategyProfileRep<double>>(spt));
}

MixedStrategyProfile<Rational>
GameBAGGRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &spt) const
{
  return MixedStrategyProfile<Rational>(
      std::make_unique<BAGGMixedStrategyProfileRep<Rational>>(spt));
}

//------------------------------------------------------------------------
//                   GameBAGGRep: Writing data files
//------------------------------------------------------------------------

void GameBAGGRep::Write(std::ostream &p_stream, const std::string &p_format /*="native"*/) const
{
  if (p_format == "native" || p_format == "bagg") {
    WriteBaggFile(p_stream);
  }
  else if (p_format == "nfg") {
    WriteNfgFile(p_stream);
  }
  else {
    throw UndefinedException();
  }
}

void GameBAGGRep::WriteBaggFile(std::ostream &s) const { s << (*baggPtr); }

} // end namespace Gambit
