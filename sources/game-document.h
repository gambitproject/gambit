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

  // Properties
  gbtBlock<wxColour> m_playerColors;
  bool m_modified;
  wxString m_filename;
  double m_treeZoom;

  // Management of view list
  gbtList<gbtGameView *> m_views;
  void AddView(gbtGameView *);
  void RemoveView(gbtGameView *);
  void UpdateViews(void);

public:
  gbtGameDocument(const gbtGame &);

  const gbtGame &GetGame(void) const { return m_game; }

  // non-const member access to control updating of views
  gbtGameOutcome NewOutcome(void);
  void SetPayoff(gbtGameOutcome p_outcome,
		 const gbtGamePlayer &p_player, const gbtRational &p_value);

  // Various properties of the document
  wxColour GetPlayerColor(int p_index) const;
  void SetPlayerColor(int p_index, const wxColour &);

  double GetTreeZoom(void) const { return m_treeZoom; }
  void SetTreeZoom(double p_zoom) { m_treeZoom = p_zoom; UpdateViews(); }

  bool IsModified(void) const { return m_modified; }
  void SetModified(bool p_modified) 
    { m_modified = p_modified; UpdateViews(); }

  const wxString &GetFilename(void) const { return m_filename; }
  void SetFilename(const wxString &p_filename) 
    { m_filename = p_filename; UpdateViews(); }

  void Load(const wxString &);
  void Save(const wxString &) const;
};


class gbtGameView {
protected:
  gbtGameDocument *m_doc;

public:
  gbtGameView(gbtGameDocument *);
  virtual ~gbtGameView();

  virtual void OnUpdate(void) = 0;

  gbtGameDocument *GetDocument(void) const { return m_doc; }
};

#endif   // GAME_DOCUMENT_H
