//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of normal form game representation
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef LIBGAMBIT_TABLEGAME_H
#define LIBGAMBIT_TABLEGAME_H

#include "base.h"
#include "rational.h"
#include "game.h"


class gbtNumber;
class gbtStrategyProfile;
template <class T> class gbtMixedProfile;

namespace Gambit {

//
// Forward declarations of classes defined in this file.
//
class GameStrategyRep;
typedef Gambit::GameObjectPtr<GameStrategyRep> GameStrategy;

class TablePlayerRep;
typedef Gambit::GameObjectPtr<TablePlayerRep> TablePlayer;

class GameTableRep;
typedef Gambit::GameObjectPtr<GameTableRep> GameTable;

/// This class represents a strategy in a strategic game.
/// Internally, this strategy stores an 'index'.  This index has the
/// property that, for a strategy profile, adding the indices of the
/// strategies gives the index into the strategic game's table to
/// find the outcome for that strategy profile, making payoff computation
/// relatively efficient.
class GameStrategyRep : public GameObject  {
  friend class GameTableRep;
  friend class TablePlayerRep;
  friend class ::gbtStrategyProfile;
  friend class gbtMixedProfile<double>;
  friend class gbtMixedProfile<gbtRational>;
  friend class gbtMixedProfile<gbtNumber>;
private:
  int m_number;
  TablePlayerRep *m_player;
  long m_index;
  std::string m_name;

  /// @name Lifecycle
  //@{
  /// Creates a new strategy for the given player.
  GameStrategyRep(TablePlayerRep *p_player)
    : m_number(0), m_player(p_player), m_index(0L) { }
  //@}

public:
  /// @name Data access
  //@{
  /// Returns the text label associated with the strategy
  const std::string &GetName(void) const { return m_name; }
  /// Sets the text label associated with the strategy
  void SetName(const std::string &s) { m_name = s; }
  
  /// Returns the player for whom this is a strategy
  TablePlayer GetPlayer(void) const;
  /// Returns the index of the strategy for its player
  int GetNumber(void) const { return m_number; }

  /// Remove this strategy from the game
  void DeleteStrategy(void);
  //@}
};


/// This class represents a player in a strategic game.
class TablePlayerRep : public Gambit::GameObject {
  friend class GameTableRep;
  friend class GameStrategyRep;
private:
  int number;
  std::string name;
  GameTableRep *m_nfg;
  
  gbtArray<GameStrategyRep *> strategies;

  /// @name Lifecycle
  //@{
  /// Constructs a new player in the specified game, with 'num' strategies
  TablePlayerRep(int n, GameTableRep *no, int num);
  /// Cleans up a player object; responsible for deallocating strategies
  ~TablePlayerRep();
  //@}

public:
  /// @name Data access
  //@{
  /// Returns the text label associated with the player
  const std::string &GetName(void) const { return name; }
  /// Sets the text label associated with the player
  void SetName(const std::string &s) { name = s; }

  /// Returns the game on which the player is defined
  GameTable GetGame(void) const { return m_nfg; }
  /// Returns the index of the player in its game
  int GetNumber(void) const  { return number; }
  //@}

  /// @name Strategies
  //@{
  /// Returns the number of strategies available to the player
  int NumStrats(void) const { return strategies.Length(); }
  /// Returns the st'th strategy for the player
  GameStrategy GetStrategy(int st) { return strategies[st]; }
  /// Creates a new strategy for the player
  GameStrategy NewStrategy(void);
  //@}
};


class gbtStrategyProfile;

/// This class represents a strategic game in a tabular format.
/// The table is a list of outcomes, each of which may appear in
/// multiple locations in the table.  Entries in the table may also
/// be empty (i.e., a null gbtNfgOutcome pointer), in which case
/// the payoff in that contingency is zero to all players.  Upon
/// creation, all entries in the table are set to the null pointer.
/// See gbtStrategyProfile for the facilities for setting entries
/// in the game table.
class GameTableRep : public GameRep {
friend class ::gbtStrategyProfile;
friend class NfgFileReader;
friend class Gambit::GameTreeRep;
friend class TablePlayerRep;
friend class GameStrategyRep;
friend void SetPayoff(GameTable, int, int, const std::string &);
friend void ParseOutcomeBody(class gbtGameParserState &, Gambit::GameTable);
friend class gbtMixedProfile<double>;
friend class gbtMixedProfile<gbtRational>;
friend class gbtMixedProfile<gbtNumber>;
protected:
  std::string m_title, m_comment;
  gbtArray<int> dimensions;

  gbtArray<TablePlayerRep *> players;
  gbtArray<Gambit::GameOutcomeRep *> outcomes;

  gbtArray<Gambit::GameOutcomeRep *> results;

  Gambit::GameTree efg;

  /// @name Private auxiliary functions
  //@{
  void IndexStrategies(void);
  void RebuildTable(void);
  //@}
  
public:
  /// @name Lifecycle
  //@{
  /// Construct a new strategic form game with the specified dimension
  GameTableRep(const gbtArray<int> &dim);
  /// Create a copy of a strategic form game
    //GameTable(const GameTable &b);
  /// Clean up a strategic form game
  ~GameTableRep();
  //@}
    
  /// @name General data access
  //@{
  /// Get the text label associated with the game
  const std::string &GetTitle(void) const { return m_title; }
  /// Set the text label associated with the game
  void SetTitle(const std::string &p_title) { m_title = p_title; }

  /// Get the text comment associated with the game
  const std::string &GetComment(void) const { return m_comment; }
  /// Set the text comment associated with the game
  void SetComment(const std::string &p_comment) { m_comment = p_comment; }

  /// Returns true if the game is constant-sum
  bool IsConstSum(void) const;
  /// Returns the smallest payoff in any outcome of the game
  gbtRational GetMinPayoff(int pl = 0) const;
  /// Returns the largest payoff in any outcome of the game
  gbtRational GetMaxPayoff(int pl = 0) const;

  /// Returns the extensive form associated with this game, if any
  Gambit::GameTree AssociatedEfg(void) const 
  { return efg; }
  //@}

  /// @name Writing data files
  //@{
  /// Write the game in .nfg format to the specified stream
  void WriteNfgFile(std::ostream &) const;
  //@}

  /// @name Players
  //@{
  /// Returns the number of players in the game
  int NumPlayers(void) const { return players.Length(); }
  /// Returns the pl'th player in the game
  TablePlayer GetPlayer(int pl) const { return players[pl]; }
  /// Creates a new player in the game, with one strategy
  TablePlayer NewPlayer(void);
  //@}

  /// @name Strategies
  //@{
  /// Returns the number of strategies available to player pl
  int NumStrats(int pl) const;
  /// Returns the dimension of the game
  const gbtArray<int> &NumStrats(void) const  { return dimensions; }
  /// Returns the total number of strategies in the game
  int ProfileLength(void) const;
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  int NumOutcomes(void) const   { return outcomes.Length(); }
  /// Returns the p_outc'th outcome defined in the game
  Gambit::GameOutcome GetOutcome(int p_outc) const { return outcomes[p_outc]; }

  /// Creates a new outcome in the game
  Gambit::GameOutcome NewOutcome(void);
  /// Deletes the specified outcome from the game
  void DeleteOutcome(Gambit::GameOutcome);
  //@}

  /// Renumber all game objects in a canonical way
  void Canonicalize(void) { }
};

// Exception thrown by ReadNfg if not valid .nfg file
class gbtNfgParserException : public gbtException {
private:
  std::string m_description;

public:
  gbtNfgParserException(void)
    : m_description("Not a valid .nfg file") { }
  gbtNfgParserException(const std::string &p_description) 
    : m_description(p_description) { }
  gbtNfgParserException(int p_line, const std::string &p_description);
  virtual ~gbtNfgParserException() { }

  std::string GetDescription(void) const { return m_description; }
};

/// Reads a strategic game in .nfg format from the input stream
GameTable ReadNfg(std::istream &);

}  // end namespace Gambit

#endif  // LIBGAMBIT_TABLEGAME_H
