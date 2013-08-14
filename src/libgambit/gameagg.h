//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gameagg.h
// Declaration of GameAggRep, the action-graph game representation
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

#ifndef GAMEAGG_H
#define GAMEAGG_H

#include "libagg/agg.h"

//forward declaration
class aggame;

namespace Gambit {

class GameAggRep : public GameRep {
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class AggMixedStrategyProfileRep;
  friend class AggPureStrategyProfileRep;
  friend class ::aggame;

private:
  agg *aggPtr;
  Array<GamePlayerRep *> m_players;

public:
  static GameAggRep* ReadAggFile(istream& in);

  /// @name Lifecycle
  //@{
  /// Constructor
  GameAggRep(agg* _aggPtr)
  :aggPtr(_aggPtr)
  {
	  for (int pl=1; pl <= aggPtr->getNumPlayers(); pl++){
		  m_players.Append(new GamePlayerRep(this,pl,aggPtr->getNumActions(pl-1)));
		  m_players[pl]->m_label = lexical_cast<std::string>(pl);
		  for (int st = 1; st <= m_players[pl]->NumStrategies(); st++) {
		      m_players[pl]->m_strategies[st]->SetLabel(lexical_cast<std::string>(st));
		  }
	  }
	  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
	    for (int st = 1; st <= m_players[pl]->m_strategies.Length();
		 m_players[pl]->m_strategies[st++]->m_id = id++);
	  }
  }
  /// Destructor
  virtual ~GameAggRep() { }

  /// Create a copy of the game, as a new game
  virtual Game Copy(void) const;
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  virtual PVector<int> NumActions(void) const{
	  throw UndefinedException();
  }
  /// The number of members in each information set
  virtual PVector<int> NumMembers(void) const
  { throw UndefinedException(); }
  /// The number of strategies for each player
  virtual Array<int> NumStrategies(void) const{
	  Array<int> ns;
	  for (int pl=1;pl<=aggPtr->getNumPlayers();pl++){
		  ns.Append(m_players[pl]->NumStrategies());
	  }
	  return ns;
  }
  /// Gets the i'th strategy in the game, numbered globally
  virtual GameStrategy GetStrategy(int p_index) const{
	  for (int pl=1;pl<=aggPtr->getNumPlayers();pl++){
		  if (m_players[pl]->NumStrategies()>=p_index){
			  return m_players[pl]->GetStrategy(p_index);
		  }
		  else{
			  p_index -= m_players[pl]->NumStrategies();
		  }
	  }
	  throw IndexException();
  }
  /// Returns the total number of actions in the game
  virtual int BehavProfileLength(void) const
  { throw UndefinedException(); }
  /// Returns the total number of strategies in the game
  virtual int MixedProfileLength(void) const {
	  return aggPtr->getNumActions();
  }
  virtual int NumStrategyContingencies(void) const
  { throw UndefinedException(); }
  //@}

  virtual PureStrategyProfile NewPureStrategyProfile(void) const;
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double) const;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const;
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double, const StrategySupport&) const;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &, const StrategySupport&) const;

  /// @name Players
  //@{
  /// Returns the number of players in the game
  virtual int NumPlayers(void) const {
	  return aggPtr->getNumPlayers();
  }
  /// Returns the pl'th player in the game
  virtual GamePlayer GetPlayer(int pl) const{
	  return m_players[pl];
  }
  /// Returns an iterator over the players
  virtual GamePlayerIterator Players(void) const{
	  return m_players;
  }
  /// Returns the chance (nature) player
  virtual GamePlayer GetChance(void) const
  { throw UndefinedException(); }
  /// Creates a new player in the game, with no moves
  virtual GamePlayer NewPlayer(void)
  { throw UndefinedException(); }
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  virtual GameInfoset GetInfoset(int iset) const
  { throw UndefinedException(); }
  /// Returns an array with the number of information sets per personal player
  virtual Array<int> NumInfosets(void) const
  { throw UndefinedException(); }
  /// Returns the act'th action in the game (numbered globally)
  virtual GameAction GetAction(int act) const
  { throw UndefinedException(); }
  //@}


  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  virtual int NumOutcomes(void) const
  { throw UndefinedException(); }
  /// Returns the index'th outcome defined in the game
  virtual GameOutcome GetOutcome(int index) const
  { throw UndefinedException(); }
  /// Creates a new outcome in the game
  virtual GameOutcome NewOutcome(void)
  { throw UndefinedException(); }
  /// Deletes the specified outcome from the game
  virtual void DeleteOutcome(const GameOutcome &)
  { throw UndefinedException(); }
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  virtual GameNode GetRoot(void) const
  { throw UndefinedException(); }
  /// Returns the number of nodes in the game
  virtual int NumNodes(void) const
  { throw UndefinedException(); }
  //@}

  /// @name General data access
  //@{
  virtual bool IsTree(void) const { return false; }
  virtual bool IsAgg(void) const { return true; }
  virtual bool IsPerfectRecall(GameInfoset &, GameInfoset &) const { return true; }
  virtual bool IsConstSum(void) const;
  /// Returns the smallest payoff in any outcome of the game
  virtual Rational GetMinPayoff(int) const {
	  return aggPtr->getMinPayoff();
  }
  /// Returns the largest payoff in any outcome of the game
  virtual Rational GetMaxPayoff(int) const {
	  return aggPtr->getMaxPayoff();
  }

  //@}

  /// @name Writing data files
  //@{
  /// Write the game to a savefile in the specified format.
  virtual void Write(std::ostream &p_stream,
		     const std::string &p_format="native") const;
  virtual void WriteNfgFile(std::ostream &) const;
  virtual void WriteAggFile(std::ostream &) const;
  //@}
};

}
#endif /* GAMEAGG_H */
