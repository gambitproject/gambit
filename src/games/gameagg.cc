//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gameagg.cc
// Implementation of action-graph game representation
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
#include "gameagg.h"

namespace Gambit {

//========================================================================
//                  class AGGPureStrategyProfileRep
//========================================================================

class AGGPureStrategyProfileRep : public PureStrategyProfileRep {
public:
  explicit AGGPureStrategyProfileRep(const Game &p_game) : PureStrategyProfileRep(p_game) {}
  PureStrategyProfileRep *Copy() const override { return new AGGPureStrategyProfileRep(*this); }

  void SetStrategy(const GameStrategy &) override;
  GameOutcome GetOutcome() const override { throw UndefinedException(); }
  void SetOutcome(GameOutcome p_outcome) override { throw UndefinedException(); }
  Rational GetPayoff(int pl) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//       AGGPureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void AGGPureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

Rational AGGPureStrategyProfileRep::GetPayoff(int pl) const
{
  std::shared_ptr<agg::AGG> aggPtr = dynamic_cast<GameAGGRep &>(*m_nfg).aggPtr;
  std::vector<int> s(aggPtr->getNumPlayers());
  for (int i = 1; i <= aggPtr->getNumPlayers(); i++) {
    s[i - 1] = m_profile[i]->GetNumber() - 1;
  }
  return Rational(aggPtr->getPurePayoff(pl - 1, s));
}

Rational AGGPureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  int player = p_strategy->GetPlayer()->GetNumber();
  std::shared_ptr<agg::AGG> aggPtr = dynamic_cast<GameAGGRep &>(*m_nfg).aggPtr;
  std::vector<int> s(aggPtr->getNumPlayers());
  for (int i = 1; i <= aggPtr->getNumPlayers(); i++) {
    s[i - 1] = m_profile[i]->GetNumber() - 1;
  }
  s[player - 1] = p_strategy->GetNumber() - 1;
  return Rational(aggPtr->getPurePayoff(player - 1, s));
}

//========================================================================
//                   AGGMixedStrategyProfileRep<T>
//========================================================================

template <class T> class AGGMixedStrategyProfileRep : public MixedStrategyProfileRep<T> {

public:
  explicit AGGMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  {
  }
  ~AGGMixedStrategyProfileRep() override = default;

  MixedStrategyProfileRep<T> *Copy() const override
  {
    return new AGGMixedStrategyProfileRep(*this);
  }
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

template <class T> T AGGMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  auto &g = dynamic_cast<GameAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s(g.aggPtr->getNumActions());
  for (int i = 0; i < g.aggPtr->getNumPlayers(); ++i) {
    for (int j = 0; j < g.aggPtr->getNumActions(i); ++j) {
      GameStrategy strategy = this->m_support.GetGame()->GetPlayer(i + 1)->GetStrategy(j + 1);
      int ind = this->m_support.m_profileIndex[strategy->GetId()];
      s[g.aggPtr->firstAction(i) + j] = (ind == -1) ? (T)0 : this->m_probs[ind];
    }
  }
  return (T)g.aggPtr->getMixedPayoff(pl - 1, s);
}

template <class T>
T AGGMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &ps) const
{
  auto &g = dynamic_cast<GameAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s(g.aggPtr->getNumActions());
  for (int i = 0; i < g.aggPtr->getNumPlayers(); ++i) {
    if (i + 1 == ps->GetPlayer()->GetNumber()) {
      for (int j = 0; j < g.aggPtr->getNumActions(i); ++j) {
        s[g.aggPtr->firstAction(i) + j] = (T)0;
      }
      s.at(g.aggPtr->firstAction(i) + ps->GetNumber() - 1) = (T)1;
    }
    else {
      for (int j = 0; j < g.aggPtr->getNumActions(i); ++j) {
        GameStrategy strategy = this->m_support.GetGame()->GetPlayer(i + 1)->GetStrategy(j + 1);
        const int &ind = this->m_support.m_profileIndex[strategy->GetId()];
        s[g.aggPtr->firstAction(i) + j] = (ind == -1) ? (T)0 : this->m_probs[ind];
      }
    }
  }
  return (T)g.aggPtr->getMixedPayoff(pl - 1, s);
}

template <class T>
T AGGMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &ps1,
                                                const GameStrategy &ps2) const
{
  GamePlayerRep *player1 = ps1->GetPlayer();
  GamePlayerRep *player2 = ps2->GetPlayer();
  if (player1 == player2) {
    return (T)0;
  }

  auto &g = dynamic_cast<GameAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s(g.aggPtr->getNumActions());
  for (int i = 0; i < g.aggPtr->getNumPlayers(); ++i) {
    if (i + 1 == player1->GetNumber()) {
      for (int j = 0; j < g.aggPtr->getNumActions(i); ++j) {
        s[g.aggPtr->firstAction(i) + j] = (T)0;
      }
      s.at(g.aggPtr->firstAction(i) + ps1->GetNumber() - 1) = (T)1;
    }
    else if (i + 1 == player2->GetNumber()) {
      for (int j = 0; j < g.aggPtr->getNumActions(i); ++j) {
        s[g.aggPtr->firstAction(i) + j] = (T)0;
      }
      s.at(g.aggPtr->firstAction(i) + ps2->GetNumber() - 1) = (T)1;
    }
    else {
      for (int j = 0; j < g.aggPtr->getNumActions(i); ++j) {
        GameStrategy strategy = this->m_support.GetGame()->GetPlayer(i + 1)->GetStrategy(j + 1);
        const int &ind = this->m_support.m_profileIndex[strategy->GetId()];
        s[g.aggPtr->firstAction(i) + j] = (ind == -1) ? (T)0 : this->m_probs[ind];
      }
    }
  }
  return (T)g.aggPtr->getMixedPayoff(pl - 1, s);
}

template class AGGMixedStrategyProfileRep<double>;
template class AGGMixedStrategyProfileRep<Rational>;

//------------------------------------------------------------------------
//                        GameAGGRep: Lifecycle
//------------------------------------------------------------------------

GameAGGRep::GameAGGRep(std::shared_ptr<agg::AGG> p_aggPtr) : aggPtr(p_aggPtr)
{
  for (int pl = 1; pl <= aggPtr->getNumPlayers(); pl++) {
    m_players.push_back(new GamePlayerRep(this, pl, aggPtr->getNumActions(pl - 1)));
    m_players[pl]->m_label = lexical_cast<std::string>(pl);
    for (int st = 1; st <= m_players[pl]->NumStrategies(); st++) {
      m_players[pl]->m_strategies[st]->SetLabel(lexical_cast<std::string>(st));
    }
  }
  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length();
         m_players[pl]->m_strategies[st++]->m_id = id++)
      ;
  }
}

Game GameAGGRep::Copy() const
{
  std::ostringstream os;
  WriteAggFile(os);
  std::istringstream is(os.str());
  return ReadAggFile(is);
}

//------------------------------------------------------------------------
//                  GameAGGRep: Dimensions of the game
//------------------------------------------------------------------------

Array<int> GameAGGRep::NumStrategies() const
{
  Array<int> ns;
  for (int pl = 1; pl <= aggPtr->getNumPlayers(); pl++) {
    ns.push_back(m_players[pl]->NumStrategies());
  }
  return ns;
}

GameStrategy GameAGGRep::GetStrategy(int p_index) const
{
  for (int pl = 1; pl <= aggPtr->getNumPlayers(); pl++) {
    if (m_players[pl]->NumStrategies() >= p_index) {
      return m_players[pl]->GetStrategy(p_index);
    }
    else {
      p_index -= m_players[pl]->NumStrategies();
    }
  }
  throw IndexException();
}

//------------------------------------------------------------------------
//                    GameAGGRep: Factory functions
//------------------------------------------------------------------------

PureStrategyProfile GameAGGRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(new AGGPureStrategyProfileRep(const_cast<GameAGGRep *>(this)));
}

MixedStrategyProfile<double> GameAGGRep::NewMixedStrategyProfile(double) const
{
  return MixedStrategyProfile<double>(new AGGMixedStrategyProfileRep<double>(
      StrategySupportProfile(const_cast<GameAGGRep *>(this))));
}

MixedStrategyProfile<Rational> GameAGGRep::NewMixedStrategyProfile(const Rational &) const
{
  return MixedStrategyProfile<Rational>(new AGGMixedStrategyProfileRep<Rational>(
      StrategySupportProfile(const_cast<GameAGGRep *>(this))));
}
MixedStrategyProfile<double>
GameAGGRep::NewMixedStrategyProfile(double, const StrategySupportProfile &spt) const
{
  return MixedStrategyProfile<double>(new AGGMixedStrategyProfileRep<double>(spt));
}

MixedStrategyProfile<Rational>
GameAGGRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &spt) const
{
  return MixedStrategyProfile<Rational>(new AGGMixedStrategyProfileRep<Rational>(spt));
}

//------------------------------------------------------------------------
//                  GameAGGRep: General data access
//------------------------------------------------------------------------

bool GameAGGRep::IsConstSum() const
{
  AGGPureStrategyProfileRep profile(const_cast<GameAGGRep *>(this));

  Rational sum(0);
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    sum += profile.GetPayoff(pl);
  }

  for (StrategyProfileIterator iter(StrategySupportProfile(const_cast<GameAGGRep *>(this)));
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
//                   GameAGGRep: Writing data files
//------------------------------------------------------------------------

void GameAGGRep::Write(std::ostream &p_stream, const std::string &p_format /*="native"*/) const
{
  if (p_format == "native" || p_format == "agg") {
    WriteAggFile(p_stream);
  }
  else if (p_format == "nfg") {
    WriteNfgFile(p_stream);
  }
  else {
    throw UndefinedException();
  }
}

void GameAGGRep::WriteAggFile(std::ostream &s) const
{
  s << "#AGG" << std::endl;
  s << aggPtr->getNumPlayers() << std::endl;
  s << aggPtr->getNumActionNodes() << std::endl;
  s << aggPtr->getNumFunctionNodes() << std::endl;
  for (int i = 0; i < aggPtr->getNumPlayers(); i++) {
    s << aggPtr->getNumActions(i) << " ";
  }
  s << std::endl;
  for (int i = 0; i < aggPtr->getNumPlayers(); i++) {
    std::copy(aggPtr->getActionSet(i).begin(), aggPtr->getActionSet(i).end(),
              std::ostream_iterator<int>(s, " "));
    s << std::endl;
  }
  aggPtr->printActionGraph(s);
  s << std::endl;
  // types of function nodes
  aggPtr->printTypes(s);
  s << std::endl;

  // payoffs
  for (int i = 0; i < aggPtr->getNumActionNodes(); i++) {
    s << "1" << std::endl; // type of payoff output
    aggPtr->printPayoffs(s, i);
    s << std::endl;
  }
}

Game GameAGGRep::ReadAggFile(std::istream &in) { return new GameAGGRep(agg::AGG::makeAGG(in)); }

} // end namespace Gambit
