//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/labelcell.h
// Declaration of wxSheet editor for Gambit labels
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

#ifndef GAMBIT_GUI_LABELCELLEDITOR_H
#define GAMBIT_GUI_LABELCELLEDITOR_H

#include "wx/sheet/sheet.h"

#include "editlabel.h"
#include "renratio.h" // for DECLARE_GAMBIT_SHEETOBJREFDATA_COPY_CLASS

namespace Gambit::GUI {

class LabelEditorRefData final : public wxSheetCellTextEditorRefData {
  LabelCharacterPolicy m_policy{LabelCharacterPolicy::AsciiOnly};

public:
  explicit LabelEditorRefData(LabelCharacterPolicy p_policy = LabelCharacterPolicy::AsciiOnly);

  void CreateEditor(wxWindow *, wxWindowID, wxEvtHandler *, wxSheet *) override;

  /// Override basic text editor behavior to normalize label editing.
  bool IsAcceptedKey(wxKeyEvent &) override;
  void StartingKey(wxKeyEvent &) override;
  bool EndEdit(const wxSheetCoords &, wxSheet *) override;

  bool Copy(const LabelEditorRefData &p_other);

  // NOLINTNEXTLINE(modernize-use-auto)
  DECLARE_GAMBIT_SHEETOBJREFDATA_COPY_CLASS(LabelEditorRefData, wxSheetCellTextEditorRefData)
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_LABELCELLEDITOR_H
