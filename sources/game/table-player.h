//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Representation of players for table games
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

#ifndef TABLE_PLAYER_H
#define TABLE_PLAYER_H

class gbtTableStrategyRep : public gbtGameStrategyRep {
public:
  int m_refCount, m_id;
  gbtTableInfosetRep *m_infoset;
  bool m_deleted;
  std::string m_label;
  long m_index;

  /// @name Constructor and destructor
  //@{
  gbtTableStrategyRep(gbtTableInfosetRep *, int p_id);
  virtual ~gbtTableStrategyRep();
  //@}
  
  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  //@}

  /// @name General information about the strategy
  //@{
  int GetId(void) const;
  std::string GetLabel(void) const;
  void SetLabel(const std::string &);
  bool IsDeleted(void) const { return m_deleted; }
  //@}

  /// @name Accessing information about the player
  //@{
  gbtGamePlayer GetPlayer(void) const;

  gbtGameAction GetBehavior(const gbtGameInfoset &) const
    { throw gbtGameUndefinedException(); }
  //@}

  /// @name Dominance properties
  //@{
  bool Dominates(const gbtGameStrategy &, bool) const;
  bool IsDominated(bool) const;
  //@}
};

class gbtTableInfosetRep : public gbtGameInfosetRep {
public:
  int m_refCount, m_id;
  gbtTablePlayerRep *m_player;
  bool m_deleted;
  std::string m_label;
  gbtBlock<gbtTableStrategyRep *> m_actions;

  /// @name Constructor and destructor
  //@{
  gbtTableInfosetRep(gbtTablePlayerRep *, int id, int br);
  virtual ~gbtTableInfosetRep();
  //@}

  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  void Delete(void)
    { if (m_refCount == 0) delete this; else m_deleted = true; }
  //@}

  /// @name General information about the information set
  //@{
  int GetId(void) const { return 1; }
  void SetLabel(const std::string &p_label) { m_label = p_label; }
  std::string GetLabel(void) const { return m_label; }
  bool IsDeleted(void) const { return m_deleted; }
  //@}

  /// @name Accessing information about the player
  //@{
  bool IsChanceInfoset(void) const { return false; }
  gbtGamePlayer GetPlayer(void) const;
  void SetPlayer(const gbtGamePlayer &) { throw gbtGameUndefinedException(); }
  //@}

  /// @name Accessing information about the actions
  //@{
  int NumActions(void) const { throw gbtGameUndefinedException(); }
  gbtGameAction GetAction(int) const { throw gbtGameUndefinedException(); }
  //@}

  /// @name Accessing information about the members
  //@{
  int NumMembers(void) const { throw gbtGameUndefinedException(); }
  gbtGameNode GetMember(int) const { throw gbtGameUndefinedException(); }
  //@}

  /// @name Editing the game tree
  //@{
  bool Precedes(const gbtGameNode &) const 
    { throw gbtGameUndefinedException(); }
  void MergeInfoset(const gbtGameInfoset &) 
    { throw gbtGameUndefinedException(); }
  void DeleteInfoset(void) { throw gbtGameUndefinedException(); }
  gbtGameAction InsertAction(int) { throw gbtGameUndefinedException(); }
  void Reveal(const gbtGamePlayer &) { throw gbtGameUndefinedException(); }
  //@}
};

class gbtTablePlayerRep : public gbtGamePlayerRep {
public:
  int m_refCount, m_id;
  gbtTableGameRep *m_nfg;
  bool m_deleted;
  std::string m_label;
  gbtBlock<gbtTableInfosetRep *> m_infosets;
  
  /// @name Constructor and destructor
  //@{
  gbtTablePlayerRep(gbtTableGameRep *, int, int);
  virtual ~gbtTablePlayerRep() { }
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
  bool IsChance(void) const { return false; }
  void SetLabel(const std::string &);
  std::string GetLabel(void) const;
  bool IsDeleted(void) const;
  //@}

  /// @name Accessing the information sets of the player
  //@{
  int NumInfosets(void) const;
  gbtGameInfoset GetInfoset(int) const { throw gbtGameUndefinedException(); }
  gbtGameInfoset NewInfoset(int) { throw gbtGameUndefinedException(); }
  //@}

  /// @name Accessing the sequences of the player
  //@{
  int NumSequences(void) const;
  gbtGameSequence GetSequence(int) const { throw gbtGameUndefinedException(); }
  //@}

  /// @name Accessing the strategies of the player
  //@{
  int NumStrategies(void) const;
  gbtGameStrategy GetStrategy(int) const;
  //@}
};

#endif  // TABLE_PLAYER_H
