//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of game document/view classes
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

#ifndef GAMEDOC_H
#define GAMEDOC_H

#include "game/efg.h"
#include "game/nfg.h"
#include "nash/behavsol.h"
#include "nash/mixedsol.h"

//
// This class encapsulates all the relevant information about a game
// currently being displayed.  In the longer run, it should coordinate
// all manipulation of the game objects.
//
// It is intended that this class will move towards compatibility
// with wxWindows' wxDocument class, but may or may not eventually become
// derived from it.
//

class gbtGameView;
class gbtGameCommand;

class gbtGameDocument {
public:
  efgGame *m_efg;
  Nfg *m_nfg;

  int m_currentProfile;
  gList<BehavSolution> m_behavProfiles;
  gList<MixedSolution> m_mixedProfiles;
  
  EFSupport *m_currentEfgSupport;
  gList<EFSupport *> m_efgSupports;
  gbtNfgSupport *m_currentNfgSupport;
  gList<gbtNfgSupport *> m_nfgSupports;

  Node *m_cursor, *m_cutNode, *m_copyNode;

  gText m_filename;
  int m_numDecimals;

  gList<gbtGameView *> m_views;

  gbtGameDocument(efgGame *p_efg);
  gbtGameDocument(Nfg *p_nfg);
  ~gbtGameDocument();
  
  void AddView(gbtGameView *);
  void RemoveView(gbtGameView *);
  void UpdateAllViews(gbtGameView *p_sender = 0);

  // Implementation of commands
  void Submit(gbtGameCommand *);

  gText UniqueEfgSupportName(void) const;
  void FlushEfgSupports(void);
  EFSupport *GetEfgSupport(void) const { return m_currentEfgSupport; }

  gText UniqueEfgOutcomeName(void) const;

  const BehavSolution &CurrentBehav(void) const
    { return m_behavProfiles[m_currentProfile]; }

  void SelectNode(Node *);
  Node *Cursor(void) const { return m_cursor; }
  Node *CopyNode(void) const { return m_copyNode; }
  Node *CutNode(void) const { return m_cutNode; }

  // Determine various label quantities based on current settings
  // This may not really belong here either; it's always been sort
  // of orphan functionality.
  gText GetRealizProb(const Node *) const;
  gText GetBeliefProb(const Node *) const;
  gText GetNodeValue(const Node *) const;
  gText GetInfosetProb(const Node *) const;
  gText GetInfosetValue(const Node *) const;
  gText GetActionValue(const Node *, int act) const;
  gText GetActionProb(const Node *, int act) const;
  gNumber ActionProb(const Node *n, int br) const;
};

class gbtGameView {
protected:
  gbtGameDocument *m_game;

public:
  gbtGameView(gbtGameDocument *p_game);
  virtual ~gbtGameView();

  virtual void OnUpdate(gbtGameView *p_sender);
};

class gbtGameCommand {
protected:
  bool m_canUndo;
  gText m_name;

public:
  gbtGameCommand(bool p_canUndo, const gText &p_name)
    : m_canUndo(p_canUndo), m_name(p_name) { }
  virtual ~gbtGameCommand() { }

  virtual bool Do(void) = 0;
  virtual bool Undo(void) = 0;
  
  gText GetName(void) const { return m_name; }
  bool CanUndo(void) const { return m_canUndo; }
};

#endif  // GAMEDOC_H

