//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of document class
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#ifndef GAME_DOCUMENT_H
#define GAME_DOCUMENT_H

#include <gambit/game/game.h>

class gbtGameView;

class gbtGameDocument {
  friend class gbtGameView;
private:
  gbtGame m_game;

  // Management of view list
  gbtList<gbtGameView *> m_views;
  void AddView(gbtGameView *);
  void RemoveView(gbtGameView *);
  void UpdateViews(void);

public:
  gbtGameDocument(const gbtGame &);

  const gbtGame &GetGame(void) const { return m_game; }

  // non-const member access to control updating of views;
  gbtGameOutcome NewOutcome(void) 
    { gbtGameOutcome r = m_game->NewOutcome(); UpdateViews(); return r; }
  void SetPayoff(gbtGameOutcome p_outcome,
		 const gbtGamePlayer &p_player, const gbtRational &p_value)
    { p_outcome->SetPayoff(p_player, p_value); UpdateViews(); }
};


class gbtGameView {
protected:
  gbtGameDocument *m_doc;

public:
  gbtGameView(gbtGameDocument *);
  virtual ~gbtGameView();

  virtual void OnUpdate(void) = 0;
};

#endif   // GAME_DOCUMENT_H
