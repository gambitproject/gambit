//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/gamebagg.cc
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

#include <iostream>

#include "gambit.h"
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
  void SetOutcome(GameOutcome p_outcome) override { throw UndefinedException(); }
  Rational GetPayoff(const GamePlayer &) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//       BAGGPureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

Rational BAGGPureStrategyProfileRep::GetPayoff(const GamePlayer &p_player) const
{
  const std::shared_ptr<agg::BAGG> baggPtr = dynamic_cast<GameBAGGRep &>(*m_nfg).baggPtr;
  std::vector<int> s(m_nfg->NumPlayers());
  for (size_t i = 1; i <= m_nfg->NumPlayers(); i++) {
    s[i - 1] = m_profile.at(m_nfg->GetPlayer(i))->GetNumber() - 1;
  }
  const int bp = dynamic_cast<GameBAGGRep &>(*m_nfg).agent2baggPlayer[p_player->GetNumber()];
  const int tp = p_player->GetNumber() - 1 - baggPtr->typeOffset[bp - 1];
  return Rational(baggPtr->getPurePayoff(bp - 1, tp, s));
}

Rational BAGGPureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  const int player = p_strategy->GetPlayer()->GetNumber();
  const std::shared_ptr<agg::BAGG> baggPtr = dynamic_cast<GameBAGGRep &>(*m_nfg).baggPtr;
  std::vector<int> s(m_nfg->NumPlayers());
  for (size_t i = 1; i <= m_nfg->NumPlayers(); i++) {
    s[i - 1] = m_profile.at(m_nfg->GetPlayer(i))->GetNumber() - 1;
  }
  s[player - 1] = p_strategy->GetNumber() - 1;
  const int bp = dynamic_cast<GameBAGGRep &>(*m_nfg).agent2baggPlayer[player];
  const int tp = player - 1 - baggPtr->typeOffset[bp - 1];
  return Rational(baggPtr->getPurePayoff(bp - 1, tp, s));
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
};

template <class T> T BAGGMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  auto &g = dynamic_cast<GameBAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s(g.GetStrategies().size());
  const auto ns = g.GetStrategies().shape();
  int bplayer = -1, btype = -1;
  for (int i = 0, offs = 0; i < g.baggPtr->getNumPlayers(); ++i) {
    for (int tp = 0; tp < g.baggPtr->getNumTypes(i); ++tp) {
      if (pl == g.baggPtr->typeOffset[i] + tp + 1) {
        bplayer = i;
        btype = tp;
      }
      for (int j = 0; j < ns[g.baggPtr->typeOffset[i] + tp]; ++j, ++offs) {
        const GameStrategy strategy = this->m_support.GetGame()
                                          ->GetPlayer(g.baggPtr->typeOffset[i] + tp + 1)
                                          ->GetStrategy(j + 1);
        const int ind = this->m_profileIndex.at(strategy);
        s.at(offs) = (ind == -1) ? (T)0 : this->m_probs[ind];
      }
    }
  }
  return (T)g.baggPtr->getMixedPayoff(bplayer, btype, s);
}

template <class T>
T BAGGMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &ps) const
{
  auto &g = dynamic_cast<GameBAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s(g.GetStrategies().size());
  int bplayer = -1, btype = -1;
  for (int i = 0; i < g.baggPtr->getNumPlayers(); ++i) {
    for (int tp = 0; tp < g.baggPtr->getNumTypes(i); ++tp) {
      if (pl == g.baggPtr->typeOffset[i] + tp + 1) {
        bplayer = i;
        btype = tp;
      }
      if (g.baggPtr->typeOffset[i] + tp + 1 == ps->GetPlayer()->GetNumber()) {
        for (unsigned int j = 0; j < g.baggPtr->typeActionSets.at(i).at(tp).size(); ++j) {
          s.at(g.baggPtr->firstAction(i, tp) + j) = 0;
        }
        s.at(g.baggPtr->firstAction(i, tp) + ps->GetNumber() - 1) = 1;
      }
      else {
        for (int j = 0; j < g.baggPtr->getNumActions(i, tp); ++j) {
          const GameStrategy strategy = this->m_support.GetGame()
                                            ->GetPlayer(g.baggPtr->typeOffset[i] + tp + 1)
                                            ->GetStrategy(j + 1);
          const int ind = this->m_profileIndex.at(strategy);
          s.at(g.baggPtr->firstAction(i, tp) + j) = (ind == -1) ? Rational(0) : this->m_probs[ind];
        }
      }
    }
  }
  return (T)g.baggPtr->getMixedPayoff(bplayer, btype, s);
}

template <class T>
T BAGGMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &ps1,
                                                 const GameStrategy &ps2) const
{
  const auto player1 = ps1->GetPlayer().get();
  const auto player2 = ps2->GetPlayer().get();
  if (player1 == player2) {
    return (T)0;
  }

  auto &g = dynamic_cast<GameBAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s(g.GetStrategies().size());
  int bplayer = -1, btype = -1;
  for (int i = 0; i < g.baggPtr->getNumPlayers(); ++i) {
    for (int tp = 0; tp < g.baggPtr->getNumTypes(i); ++tp) {
      if (pl == g.baggPtr->typeOffset[i] + tp + 1) {
        bplayer = i;
        btype = tp;
      }

      if (g.baggPtr->typeOffset[i] + tp + 1 == player1->GetNumber()) {
        for (unsigned int j = 0; j < g.baggPtr->typeActionSets.at(i).at(tp).size(); ++j) {
          s.at(g.baggPtr->firstAction(i, tp) + j) = 0;
        }
        s.at(g.baggPtr->firstAction(i, tp) + ps1->GetNumber() - 1) = 1;
      }
      else if (g.baggPtr->typeOffset[i] + tp + 1 == player2->GetNumber()) {
        for (int j = 0; j < g.baggPtr->getNumActions(i, tp); ++j) {
          s.at(g.baggPtr->firstAction(i, tp) + j) = 0;
        }
        s.at(g.baggPtr->firstAction(i, tp) + ps2->GetNumber() - 1) = 1;
      }
      else {
        for (unsigned int j = 0; j < g.baggPtr->typeActionSets.at(i).at(tp).size(); ++j) {
          const GameStrategy strategy = this->m_support.GetGame()
                                            ->GetPlayer(g.baggPtr->typeOffset[i] + tp + 1)
                                            ->GetStrategy(j + 1);
          const int ind = this->m_profileIndex.at(strategy);
          s.at(g.baggPtr->firstAction(i, tp) + j) =
              static_cast<T>((ind == -1) ? T(0) : this->m_probs[ind]);
        }
      }
    }
  }
  return (T)g.baggPtr->getMixedPayoff(bplayer, btype, s);
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
      m_players.push_back(
          std::make_shared<GamePlayerRep>(this, k, baggPtr->getNumActions(pl - 1, j)));
      m_players.back()->m_label = std::to_string(k);
      agent2baggPlayer[k] = pl;
      std::for_each(m_players.back()->m_strategies.begin(), m_players.back()->m_strategies.end(),
                    [st = 1](const std::shared_ptr<GameStrategyRep> &s) mutable {
                      s->m_label = std::to_string(st++);
                    });
    }
  }
}

Game GameBAGGRep::Copy() const
{
  std::ostringstream os;
  WriteBaggFile(os);
  std::istringstream is(os.str());
  return ReadBaggFile(is);
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
