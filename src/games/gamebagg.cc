//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
  explicit BAGGPureStrategyProfileRep(const Game &p_game)
    : PureStrategyProfileRep(p_game) { }
  PureStrategyProfileRep *Copy() const override
  {  return new BAGGPureStrategyProfileRep(*this); }
  void SetStrategy(const GameStrategy &) override;
  GameOutcome GetOutcome() const override
  { throw UndefinedException(); }
  void SetOutcome(GameOutcome p_outcome) override
  { throw UndefinedException(); }
  Rational GetPayoff(int pl) const override;
  Rational GetStrategyValue(const GameStrategy &) const override;
};

//------------------------------------------------------------------------
//       BAGGPureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void BAGGPureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

Rational BAGGPureStrategyProfileRep::GetPayoff(int pl) const
{
  std::shared_ptr<agg::BAGG> baggPtr = dynamic_cast<GameBAGGRep &>(*m_nfg).baggPtr;
  std::vector<int> s(m_nfg->NumPlayers());
  for (int i = 1; i <= m_nfg->NumPlayers(); i++) {
    s[i-1] = m_profile[i]->GetNumber() - 1;
  }
  int bp = dynamic_cast<GameBAGGRep &>(*m_nfg).agent2baggPlayer[pl];
  int tp = pl - 1 - baggPtr->typeOffset[bp-1];
  return Rational(baggPtr->getPurePayoff(bp-1,tp,s));
}

Rational
BAGGPureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  int player = p_strategy->GetPlayer()->GetNumber();
  std::shared_ptr<agg::BAGG> baggPtr = dynamic_cast<GameBAGGRep &>(*m_nfg).baggPtr;
  std::vector<int> s(m_nfg->NumPlayers());
  for (int i= 1; i <= m_nfg->NumPlayers(); i++) {
    s[i-1] = m_profile[i]->GetNumber() - 1;
  }
  s[player-1] = p_strategy->GetNumber() - 1;
  int bp = dynamic_cast<GameBAGGRep &>(*m_nfg).agent2baggPlayer[player];
  int tp = player - 1 - baggPtr->typeOffset[bp-1];
  return Rational(baggPtr->getPurePayoff(bp-1,tp,s));
}

//========================================================================
//                  class BAGGMixedStrategyProfileRep
//========================================================================

template <class T> class BAGGMixedStrategyProfileRep
  : public MixedStrategyProfileRep<T> {

public:
  explicit BAGGMixedStrategyProfileRep(const StrategySupportProfile &p_support)
    : MixedStrategyProfileRep<T>(p_support)
  { }
  ~BAGGMixedStrategyProfileRep() override = default;

  MixedStrategyProfileRep<T> *Copy() const override {
    return new BAGGMixedStrategyProfileRep(*this);
  }
  T GetPayoff(int pl) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &) const override;
  T GetPayoffDeriv(int pl, const GameStrategy &, const GameStrategy &) const override;
};

template <class T>
T BAGGMixedStrategyProfileRep<T>::GetPayoff(int pl) const
{
  auto &g = dynamic_cast<GameBAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s (g.MixedProfileLength());
  Array<int> ns=g.NumStrategies();
  int bplayer=-1,btype=-1;
  for (int i=0,offs=0;i<g.baggPtr->getNumPlayers();++i)
    for (int tp=0;tp<g.baggPtr->getNumTypes(i);++tp) {
      if (pl == g.baggPtr->typeOffset[i]+tp+1){
        bplayer=i;
        btype=tp;
      }
      for (int j=0;j<ns[g.baggPtr->typeOffset[i]+tp+1];++j,++offs){
        GameStrategy strategy = this->m_support.GetGame()->GetPlayer(g.baggPtr->typeOffset[i]+tp+1)->GetStrategy(j+1);
        const int &ind=this->m_support.m_profileIndex[strategy->GetId()];
        s.at(offs)= (ind==-1)?(T)0:this->m_probs[ind];
      }
    }
  return (T) g.baggPtr->getMixedPayoff(bplayer,btype, s);
}

template <class T>
T BAGGMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &ps) const
{
  auto &g = dynamic_cast<GameBAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s (g.MixedProfileLength());
  int bplayer=-1,btype=-1;
  for (int i=0;i<g.baggPtr->getNumPlayers();++i){
    for(int tp=0; tp<g.baggPtr->getNumTypes(i);++tp){
      if(pl == g.baggPtr->typeOffset[i]+tp+1){
        bplayer=i;
        btype=tp;
      }
      if(g.baggPtr->typeOffset[i]+tp+1 == ps->GetPlayer()->GetNumber()){
        for (unsigned int j=0;j<g.baggPtr->typeActionSets.at(i).at(tp).size();++j){
          s.at(g.baggPtr->firstAction(i,tp)+j) = 0;
        }
        s.at(g.baggPtr->firstAction(i,tp)+ ps->GetNumber()-1) = 1;
      }
      else {
        for (int j=0;j<g.baggPtr->getNumActions(i,tp);++j){
          GameStrategy strategy = this->m_support.GetGame()->GetPlayer(g.baggPtr->typeOffset[i]+tp+1)->GetStrategy(j+1);
          const int &ind=this->m_support.m_profileIndex[strategy->GetId()];
          s.at(g.baggPtr->firstAction(i,tp)+j)= (ind==-1)?Rational(0):this->m_probs[ind];
        }
      }
    }
  }
  return (T) g.baggPtr->getMixedPayoff(bplayer,btype, s);
}

template <class T>
T BAGGMixedStrategyProfileRep<T>::GetPayoffDeriv(int pl, const GameStrategy &ps1, const GameStrategy &ps2) const
{
  GamePlayerRep *player1 = ps1->GetPlayer();
  GamePlayerRep *player2 = ps2->GetPlayer();
  if (player1 == player2) return (T) 0;

  auto &g = dynamic_cast<GameBAGGRep &>(*(this->m_support.GetGame()));
  std::vector<double> s (g.MixedProfileLength());
  int bplayer=-1,btype=-1;
  for (int i=0;i<g.baggPtr->getNumPlayers();++i){
    for(int tp=0;tp<g.baggPtr->getNumTypes(i);++tp){
      if(pl == g.baggPtr->typeOffset[i]+tp+1){
        bplayer=i;
        btype=tp;
      }

      if(g.baggPtr->typeOffset[i]+tp+1 == player1->GetNumber()){
        for (unsigned int j=0;j<g.baggPtr->typeActionSets.at(i).at(tp).size();++j){
          s.at(g.baggPtr->firstAction(i,tp)+j) = 0;
        }
        s.at(g.baggPtr->firstAction(i,tp)+ ps1->GetNumber()-1) = 1;
      }
      else if(g.baggPtr->typeOffset[i]+tp+1 == player2->GetNumber()){
        for (int j=0;j<g.baggPtr->getNumActions(i,tp);++j){
          s.at(g.baggPtr->firstAction(i,tp)+j) = 0;
        }
        s.at(g.baggPtr->firstAction(i,tp)+ ps2->GetNumber()-1) = 1;
      }
      else {
        for (unsigned int j=0;j<g.baggPtr->typeActionSets.at(i).at(tp).size();++j){
          GameStrategy strategy = this->m_support.GetGame()->GetPlayer(g.baggPtr->typeOffset[i]+tp+1)->GetStrategy(j+1);
          const int &ind=this->m_support.m_profileIndex[strategy->GetId()];
          s.at(g.baggPtr->firstAction(i,tp)+j)= static_cast<T>((ind==-1)?T(0):this->m_probs[ind]);
        }
      }
    }
  }
  return (T) g.baggPtr->getMixedPayoff(bplayer,btype, s);
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
    for (int j = 0; j < baggPtr->getNumTypes(pl-1); j++,k++) {
      m_players.push_back(new GamePlayerRep(this, k,
                                            baggPtr->getNumActions(pl-1, j)));
      m_players[k]->m_label = lexical_cast<std::string>(k);
      agent2baggPlayer[k] = pl;
      for (int st = 1; st <= m_players[k]->NumStrategies(); st++) {
	m_players[k]->m_strategies[st]->SetLabel(lexical_cast<std::string>(st));
      }
    }
  }
  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length();
	 m_players[pl]->m_strategies[st++]->m_id = id++);
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

Array<int> GameBAGGRep::NumStrategies() const
{
  Array<int> ns;
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    ns.push_back(m_players[pl]->m_strategies.Length());
  }
  return ns;
}

int GameBAGGRep::MixedProfileLength() const
{
  int res = 0;
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    res += m_players[pl]->m_strategies.Length();
  }
  return res;
}


PureStrategyProfile GameBAGGRep::NewPureStrategyProfile() const
{
  return PureStrategyProfile(new BAGGPureStrategyProfileRep(const_cast<GameBAGGRep *>(this)));
}

MixedStrategyProfile<double> GameBAGGRep::NewMixedStrategyProfile(double) const
{
  return MixedStrategyProfile<double>(
    new BAGGMixedStrategyProfileRep<double>(StrategySupportProfile(const_cast<GameBAGGRep *>(this)))
  );
}

MixedStrategyProfile<Rational> GameBAGGRep::NewMixedStrategyProfile(const Rational &) const
{
  return MixedStrategyProfile<Rational>(
    new BAGGMixedStrategyProfileRep<Rational>(StrategySupportProfile(const_cast<GameBAGGRep *>(this)))
  );
}
MixedStrategyProfile<double> GameBAGGRep::NewMixedStrategyProfile(double, const StrategySupportProfile& spt) const
{
  return MixedStrategyProfile<double>(new BAGGMixedStrategyProfileRep<double>(spt));
}

MixedStrategyProfile<Rational> GameBAGGRep::NewMixedStrategyProfile(const Rational &, const StrategySupportProfile& spt) const
{
  return MixedStrategyProfile<Rational>(new BAGGMixedStrategyProfileRep<Rational>(spt));
}

//------------------------------------------------------------------------
//                   GameBAGGRep: Writing data files
//------------------------------------------------------------------------

void GameBAGGRep::Write(std::ostream &p_stream,
                        const std::string &p_format /*="native"*/) const
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

void GameBAGGRep::WriteBaggFile(std::ostream &s) const
{
  s << (*baggPtr);
}

Game GameBAGGRep::ReadBaggFile(std::istream &in)
{
  return new GameBAGGRep(agg::BAGG::makeBAGG(in));
}

}  // end namespace Gambit

