//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of base class for representing games
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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


#ifndef LIBGAMBIT_GAME_H
#define LIBGAMBIT_GAME_H

namespace Gambit {

/// This is a base class for all game-related objects.  Primary among
/// its responsibility is maintaining a reference count.  Calling code
/// which maintains pointers to objects representing parts of a game
/// (e.g., nodes) which may be deleted should increment the reference
/// count for that object.  The library guarantees that any object
/// with a positive reference count will not have its memory deleted,
/// but will instead be marked as deleted.  Calling code should always
/// be careful to check the deleted status of the object before any
/// operations on it.
class GameObject {
protected:
  int m_refCount;
  bool m_valid;

public:
  /// @name Lifecycle
  //@{
  /// Constructor; initializes reference count
  GameObject(void) : m_refCount(0), m_valid(true) { }
  //@}

  /// @name Validation
  //@{
  /// Is the object still valid?
  bool IsValid(void) const { return m_valid; }
  /// Invalidate the object; delete if not referenced elsewhere
  void Invalidate(void)
  { if (!m_refCount) delete this; else m_valid = false; }
  //@}

  /// @name Reference counting
  //@{
  /// Increment the reference count
  void IncRef(void) { m_refCount++; }
  /// Decrement the reference count; delete if reference count is zero.
  void DecRef(void) { if (!--m_refCount && !m_valid) delete this; }
  //@}
};


//
// This is a handle class that is used by all calling code to refer to
// member objects of games.  It takes care of all the reference-counting
// considerations.
//
template <class T> class GameObjectPtr {
private:
  T *rep;

public:
  GameObjectPtr(T *r = 0) : rep(r)
    { if (rep) rep->IncRef(); }
  GameObjectPtr(const GameObjectPtr<T> &r) : rep(r.rep)
    { if (rep) rep->IncRef(); }
  ~GameObjectPtr() { if (rep) rep->DecRef(); }

  GameObjectPtr<T> &operator=(const GameObjectPtr<T> &r)
    { if (&r != this) {
	if (rep) rep->DecRef();
	rep = r.rep;
	if (rep) rep->IncRef();
      }
      return *this;
    }

  T *operator->(void) const { return rep; }

  bool operator==(const GameObjectPtr<T> &r) const
  { return (rep == r.rep); }
  bool operator==(T *r) const { return (rep == r); }
  bool operator!=(const GameObjectPtr<T> &r) const 
  { return (rep != r.rep); }
  bool operator!=(T *r) const { return (rep != r); }

  operator T *(void) const { return rep; }

  bool operator!(void) const { return !rep; }
};

//
// This is a base class for representing an arbitrary finite game.
//
class GameRep : public GameObject {
public:
  /// Get the text label associated with the game
  virtual const std::string &GetTitle(void) const = 0;
  /// Set the text label associated with the game
  virtual void SetTitle(const std::string &s) = 0;

  /// Get the text comment associated with the game
  virtual const std::string &GetComment(void) const = 0;
  /// Set the text comment associated with the game
  virtual void SetComment(const std::string &) = 0;

  /// Returns true if the game is constant-sum
  virtual bool IsConstSum(void) const = 0; 
  /// Returns the smallest payoff in any outcome of the game
  virtual gbtRational GetMinPayoff(int pl = 0) const = 0;
  /// Returns the largest payoff in any outcome of the game
  virtual gbtRational GetMaxPayoff(int pl = 0) const = 0;

  /// Renumber all game objects in a canonical way
  virtual void Canonicalize(void) = 0;  

  /// Returns the number of players in the game
  virtual int NumPlayers(void) const = 0; 
};

typedef GameObjectPtr<GameRep> Game;


/// This class represents an outcome in a game.  An outcome
/// specifies a vector of payoffs to players.  Payoffs are specified
/// using text strings, in either decimal or rational format.  All
/// payoffs are treated as exact (that is, no conversion to floating
/// point is done).
class GameOutcomeRep : public GameObject  {
  friend class GameTableRep;
  friend class GameTreeRep;

private:
  GameRep *m_game;
  int m_number;
  std::string m_label;
  gbtArray<std::string> m_textPayoffs;
  gbtArray<gbtRational> m_ratPayoffs;

  /// @name Lifecycle
  //@{
  /// Creates a new outcome object, with payoffs set to zero
  GameOutcomeRep(GameRep *p_game, int p_number)
    : m_game(p_game), m_number(p_number),
      m_textPayoffs(m_game->NumPlayers()),
      m_ratPayoffs(m_game->NumPlayers())
    {
      for (int pl = 1; pl <= m_textPayoffs.Length(); pl++) {
	m_textPayoffs[pl] = "0";
      }
    }

  virtual ~GameOutcomeRep() { }
  //@}

public:
  /// @name Data access
  //@{
  /// Returns the strategic game on which the outcome is defined.
  Gambit::Game GetGame(void) const { return m_game; }
  /// Returns the index number of the outcome
  int GetNumber(void) const { return m_number; }

  /// Returns the text label associated with the outcome
  const std::string &GetLabel(void) const { return m_label; }
  /// Sets the text label associated with the outcome 
  void SetLabel(const std::string &p_label) { m_label = p_label; }

  /// Gets the payoff associated with the outcome to player 'pl'
  const gbtRational &GetPayoff(int pl) const { return m_ratPayoffs[pl]; }
  /// Gets the text representation of the payoff to player 'pl'
  const std::string &GetPayoffText(int pl) const { return m_textPayoffs[pl]; }
  /// Sets the payoff to player 'pl'
  void SetPayoff(int pl, const std::string &p_value)
  {
    m_textPayoffs[pl] = p_value;
    m_ratPayoffs[pl] = ToRational(p_value);
    //  m_efg->ClearComputedValues();
  }
  //@}

};

typedef GameObjectPtr<GameOutcomeRep> GameOutcome;

} // end namespace gambit

#endif   // LIBGAMBIT_GAME_H
