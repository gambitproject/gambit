//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of view of normal form with Schelling-style payoffs
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

#ifndef TABLE_SCHELLING_H
#define TABLE_SCHELLING_H

#include "game-document.h"
#include "control-player.h"    // for gbtPlayerLabelCtrl

class gbtSchellingMatrix;

class gbtTableSchelling : public wxPanel, public gbtGameView {
private:
  gbtSchellingMatrix *m_sheet;
  gbtPlayerLabelCtrl *m_rowPlayer, *m_colPlayer;

  // Implementation of gbtGameView members
  void OnUpdate(void);

  // Event handlers
  void OnSize(wxSizeEvent &);
  void OnSetRowPlayer(wxCommandEvent &);
  void OnSetColPlayer(wxCommandEvent &);

  void RefreshLayout(void);

public:
  gbtTableSchelling(wxWindow *p_parent, gbtGameDocument *p_doc);

  int GetRowPlayer(void) const { return m_rowPlayer->GetPlayer(); }
  int GetColPlayer(void) const { return m_colPlayer->GetPlayer(); }

  DECLARE_EVENT_TABLE()
};

#endif  // TABLE_SCHELLING_H


