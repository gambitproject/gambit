//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of view of normal form in matrix format
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

#ifndef TABLE_MATRIX_H
#define TABLE_MATRIX_H

#include "game-document.h"

class gbtMatrixSheet;
class gbtPlayerLabelCtrl;
class gbtTableChoiceCtrl;

class gbtTableMatrix : public wxPanel, public gbtGameView {
private:
  gbtMatrixSheet *m_sheet;
  gbtPlayerLabelCtrl *m_rowPlayerLabel, *m_colPlayerLabel;
  gbtTableChoiceCtrl *m_tableChoiceCtrl;

  int m_rowPlayer, m_colPlayer;
  gbtArray<int> m_contingency;

  // Implementation of gbtGameView members
  void OnUpdate(void);

  void RefreshLayout(void);

  // Event handlers
  void OnSize(wxSizeEvent &);

public:
  gbtTableMatrix(wxWindow *p_parent, gbtGameDocument *p_doc);

  int GetRowPlayer(void) const { return m_rowPlayer; }
  int GetColPlayer(void) const { return m_colPlayer; }
  int GetTablePlayer(int index) const;
  int GetStrategy(int pl) const { return m_contingency[pl]; }

  void SetPlayers(int row, int col);
  void SetStrategy(int pl, int st);

  DECLARE_EVENT_TABLE()
};

#endif  // TABLE_MATRIX_H


