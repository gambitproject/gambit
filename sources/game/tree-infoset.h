//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Representation of information sets for explicit game trees
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

#ifndef TREE_INFOSET_H
#define TREE_INFOSET_H

class gbtTreeActionRep : public gbtGameActionRep {
public:
  int m_id;
  gbtTreeInfosetRep *m_infoset;
  bool m_deleted;
  std::string m_label;

  /// @name Constructor and destructor
  //@{
  gbtTreeActionRep(gbtTreeInfosetRep *, int);
  virtual ~gbtTreeActionRep() { }
  //@}

  /// @name General information about the action
  //@{
  int GetId(void) const;
  void SetLabel(const std::string &);
  std::string GetLabel(void) const;
  //@}

  /// @name Information about the game tree
  //@{
  gbtGameInfoset GetInfoset(void) const;
  bool Precedes(const gbtGameNode &) const;
  //@}

  /// @name Manipulating chance actions
  //@{
  void SetChanceProb(const gbtRational &);
  gbtRational GetChanceProb(void) const;
  //@}

  /// @name Editing the game tree
  //@{
  void DeleteAction(void);
  //@}
};

class gbtTreeInfosetRep : public gbtGameInfosetRep {
public:
  int m_id;
  gbtTreePlayerRep *m_player;
  bool m_deleted;
  std::string m_label;
  gbtBlock<gbtTreeActionRep *> m_actions;
  gbtBlock<gbtRational> m_chanceProbs;
  gbtBlock<gbtTreeNodeRep *> m_members;
  int m_flag, m_whichbranch;

  /// @name Constructor and destructor
  //@{
  gbtTreeInfosetRep(gbtTreePlayerRep *, int id, int br);
  virtual ~gbtTreeInfosetRep();
  //@}

  /// @name General information about the information set
  //@{
  int GetId(void) const;
  void SetLabel(const std::string &);
  std::string GetLabel(void) const;
  //@}

  /// @name Accessing information about the player
  //@{
  bool IsChanceInfoset(void) const;
  gbtGamePlayer GetPlayer(void) const;
  void SetPlayer(const gbtGamePlayer &);
  //@}

  /// @name Accessing information about the actions
  //@{
  int NumActions(void) const;
  gbtGameAction GetAction(int) const;
  //@}
  
  /// @name Accessing information about the members
  //@{
  int NumMembers(void) const;
  gbtGameNode GetMember(int) const;
  //@}

  /// @name Editing the game tree
  //@{
  bool Precedes(const gbtGameNode &) const;
  void MergeInfoset(const gbtGameInfoset &);
  void DeleteInfoset(void);
  gbtGameAction InsertAction(int);
  void Reveal(const gbtGamePlayer &);
  //@}
};

#endif // TREE_INFOSET_H
