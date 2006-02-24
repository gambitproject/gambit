//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Toolbar for interacting with players
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

#ifndef PLAYERS_H
#define PLAYERS_H

#include <wx/sheet/sheet.h>

class gbtPlayerToolbar : public wxSheet, public gbtGameView {
private:
  bool m_isHorizontal;

  // @name Overriding wxSheet members
  //@{
  /// Gets the value in a cell
  wxString GetCellValue(const wxSheetCoords &);
  /// Sets the value in a cell: update the game
  void SetCellValue(const wxSheetCoords &, const wxString &);
  /// Disables the drawing of the grid cell cursor
  void DrawCursorHighlight(wxDC &, const wxSheetSelection &) { }
  //@}

  //! @name Overriding wxSheet members to disable selection behavior
  //@{
  bool SelectRow(int, bool = false, bool = false)
    { return false; }
  bool SelectRows(int, int, bool = false, bool = false)
    { return false; }
  bool SelectCol(int, bool = false, bool = false)
    { return false; }
  bool SelectCols(int, int, bool = false, bool = false)
    { return false; }
  bool SelectCell(const wxSheetCoords&, bool = false, bool = false)
    { return false; }
  bool SelectBlock(const wxSheetBlock&, bool = false, bool = false)
    { return false; }
  bool SelectAll(bool = false) { return false; }

  bool HasSelection(bool = true) const { return false; }
  bool IsCellSelected(const wxSheetCoords &) const { return false; }
  bool IsRowSelected(int) const { return false; }
  bool IsColSelected(int) const { return false; }
  bool DeselectBlock(const wxSheetBlock &, bool = false) { return false; }
  bool ClearSelection(bool = false) { return false; }
  //@}

  // @name Event handlers
  //@{
  /// User dragging: veto selection, start drag and drop
  void OnMouseEvents(wxMouseEvent &);
  /// User right-clicks a cell; edit color
  void OnCellRightClick(wxSheetEvent &);
  /// Window is resized
  void OnSize(wxSizeEvent &);
  //@}

  // @name Implementation of gbtGameView members
  //@{
  void OnUpdate(void);
  void PostPendingChanges(void);
  //@}

public:
  gbtPlayerToolbar(wxWindow *p_parent, gbtGameDocument *p_doc);
};


#endif // PLAYERS_H
