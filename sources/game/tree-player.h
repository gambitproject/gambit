//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Representation of players for explicit game trees
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#ifndef TREE_PLAYER_H
#define TREE_PLAYER_H

class gbtTreeStrategyRep : public gbtGameStrategyRep {
public:
  int m_refCount, m_id;
  gbtTreePlayerRep *m_player;
  gbtArray<int> m_behav;
  std::string m_label;
  bool m_deleted;

  /// @name Constructor and destructor
  //@{
  gbtTreeStrategyRep(gbtTreePlayerRep *p_player, int p_id)
    : m_refCount(0), m_id(p_id), m_player(p_player), m_deleted(false) { }
  virtual ~gbtTreeStrategyRep() { }
  //@}

  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  void Delete(void)
    { if (m_refCount == 0) delete this; else m_deleted = true; }
  //@}

  /// @name General information about the strategy
  //@{
  int GetId(void) const { return m_id; }
  std::string GetLabel(void) const { return m_label; }
  void SetLabel(const std::string &s) { m_label = s; }
  bool IsDeleted(void) const { return m_deleted; }
  //@}

  /// @name Accessing information about the player
  //@{
  gbtGamePlayer GetPlayer(void) const;
  gbtGameAction GetBehavior(const gbtGameInfoset &) const;
  //@}
};

class gbtTreePlayerRep : public gbtGamePlayerRep {
public:
  int m_refCount, m_id;
  gbtTreeGameRep *m_efg;
  bool m_deleted;
  std::string m_label;
  gbtBlock<gbtTreeInfosetRep *> m_infosets;
  gbtBlock<gbtTreeStrategyRep *> m_strategies;

  /// @name Constructor and destructor
  //@{
  gbtTreePlayerRep(gbtTreeGameRep *, int);
  virtual ~gbtTreePlayerRep();
  //@}

  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  void Delete(void)
    { if (m_refCount == 0) delete this; else m_deleted = true; }
  //@}

  /// @name General information about the player
  //@{
  int GetId(void) const;
  bool IsChance(void) const;
  void SetLabel(const std::string &);
  std::string GetLabel(void) const;
  bool IsDeleted(void) const;
  //@}

  /// @name Accessing the information sets of the player
  //@{
  int NumInfosets(void) const;
  gbtGameInfoset GetInfoset(int) const;
  gbtGameInfoset NewInfoset(int);
  //@}

  /// @name Accessing the sequences of the player
  //@{
  int NumSequences(void) const;
  //@}

  /// @name Accessing the strategies of the player
  //@{
  int NumStrategies(void) const;
  gbtGameStrategy GetStrategy(int) const;
  //@}
};

#endif  // TREE_PLAYER_H
