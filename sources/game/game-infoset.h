//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to objects representing information sets
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

#ifndef GAME_INFOSET_H
#define GAME_INFOSET_H

#include "math/gvector.h"
#include "game.h"

class gbtGameNodeRep;
typedef gbtGameObjectHandle<gbtGameNodeRep> gbtGameNode;

class gbtGamePlayerRep;
typedef gbtGameObjectHandle<gbtGamePlayerRep> gbtGamePlayer;

class gbtGameInfosetRep;
typedef gbtGameObjectHandle<gbtGameInfosetRep> gbtGameInfoset;

class gbtGameActionRep : public gbtGameObject {
  friend class gbtGameObjectHandle<gbtGameActionRep>;

public:
  //!
  //! @name General information about the action
  //!
  //@{
  /// Returns the ID number of the action (unique within the infoset)
  virtual int GetId(void) const = 0;
  /// Set the text label associated with the action
  virtual void SetLabel(const std::string &) = 0;
  /// Get the text label associated with the action
  virtual std::string GetLabel(void) const = 0;
  //@}

  //!
  //! @name Information about the game tree
  //!
  //@{
  /// Returns the information set to which the action belongs
  virtual gbtGameInfoset GetInfoset(void) const = 0;
  /// Returns true if the action precedes the node in the tree
  virtual bool Precedes(const gbtGameNode &) const = 0;
  //@}
  
  //!
  //! @name Manipulating chance actions
  //!
  //@{
  /// Sets the chance probability of the action (if a chance move)
  virtual void SetChanceProb(const gbtRational &) = 0;
  /// Gets the chance probability of the action (if a chance move)
  virtual gbtRational GetChanceProb(void) const = 0;
  //@}

  //!
  //! @name Editing the game tree
  //!
  //@{
  /// Deletes the action from the information set
  virtual void DeleteAction(void) = 0;
  //@}
};

std::ostream &operator<<(std::ostream &, const gbtGameActionRep &);

typedef gbtGameObjectHandle<gbtGameActionRep> gbtGameAction;
inline std::ostream &operator<<(std::ostream &p_stream, const gbtGameAction &)
{ return p_stream; }


class gbtGameInfosetRep : public gbtGameObject {
  friend class gbtGameObjectHandle<gbtGameInfosetRep>;
  friend struct gbt_efg_game_rep;

public:
  //!
  //! @name General information about the information set
  //!
  /// Returns the ID number of the information set (unique for a player)
  virtual int GetId(void) const = 0;
  /// Set the text label associated with the information set
  virtual void SetLabel(const std::string &) = 0;
  /// Get the text label associated with the information set
  virtual std::string GetLabel(void) const = 0;
  //@}

  //!
  //! @name Accessing information about the player
  //!
  //@{
  /// Returns true if the information set is owned by the chance player
  virtual bool IsChanceInfoset(void) const = 0;
  /// Returns the player who has the move at the information set
  virtual gbtGamePlayer GetPlayer(void) const = 0;
  /// Sets the player who has the move at the information set
  virtual void SetPlayer(const gbtGamePlayer &) = 0;
  //@}

  //!
  //! @name Accessing information about the actions
  //!
  //@{
  /// Returns the number of actions at the information set
  virtual int NumActions(void) const = 0;
  /// Gets the ith action in the information set
  virtual gbtGameAction GetAction(int i) const = 0;
  //@}

  //!
  //! @name Accessing information about the members
  //!
  //@{
  /// Returns the number of members in the information set
  virtual int NumMembers(void) const = 0;
  /// Gets the ith member in the information set
  virtual gbtGameNode GetMember(int i) const = 0;
  //@}

  //!
  //! @name Editing the game tree
  //!
  //@{
  /// Returns true if the information set precedes the node
  virtual bool Precedes(const gbtGameNode &p_node) const = 0;
  /// Places all nodes in p_infoset into this information set
  virtual void MergeInfoset(const gbtGameInfoset &p_infoset) = 0;

  virtual void DeleteInfoset(void) = 0;
  /// Insert an action into the information set
  virtual gbtGameAction InsertAction(int where) = 0;

  virtual void Reveal(const gbtGamePlayer &) = 0;
  //@}
};

class gbtGameInfosetIterator {
private:
  int m_index;
  gbtGamePlayer m_player;

public:
  gbtGameInfosetIterator(const gbtGamePlayer &p_player);
  
  gbtGameInfoset operator*(void) const;
  gbtGameInfosetIterator &operator++(int);

  bool Begin(void);
  bool End(void) const;
};

#endif   // GAME_INFOSET_H
