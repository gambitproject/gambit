//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/renratio.h
// Declaration of wxSheet renderer for rational numbers
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

#ifndef RENRATIO_H
#define RENRATIO_H

#include "wx/sheet/sheet.h" // the wxSheet widget

//
// The below is taken from the implementation of
// DECLARE_SHEETOBJREFDATA_COPY_CLASS in sheetdef.h.
// We need to customise the Clone() declaration to have an 'override';
// not every class which uses this DECLARE_ is overriding a Clone().
// This inconsistency is a function of wxSheet being ~20 years old.
#define DECLARE_GAMBIT_SHEETOBJREFDATA_COPY_CLASS(classname, basename)                            \
  DECLARE_DYNAMIC_CLASS(classname)                                                                \
public:                                                                                           \
  basename *Clone() const override                                                                \
  {                                                                                               \
    classname *aclass = new classname();                                                          \
    aclass->Copy(*((classname *)this));                                                           \
    return (basename *)aclass;                                                                    \
  }

//
// This class is based on the wxSheetCellStringRendererRefData implementation
//
class gbtRationalRendererRefData : public wxSheetCellRendererRefData {
public:
  gbtRationalRendererRefData() = default;

  // draw the string
  void Draw(wxSheet &grid, const wxSheetCellAttr &attr, wxDC &dc, const wxRect &rect,
            const wxSheetCoords &coords, bool isSelected) override;

  // return the string extent
  wxSize GetBestSize(wxSheet &grid, const wxSheetCellAttr &attr, wxDC &dc,
                     const wxSheetCoords &coords) override;

  void DoDraw(wxSheet &grid, const wxSheetCellAttr &attr, wxDC &dc, const wxRect &rect,
              const wxSheetCoords &coords, bool isSelected);

  // set the text colours before drawing
  void SetTextColoursAndFont(wxSheet &grid, const wxSheetCellAttr &attr, wxDC &dc,
                             bool isSelected);

  // calc the string extent for given string/font
  wxSize DoGetBestSize(wxSheet &grid, const wxSheetCellAttr &attr, wxDC &dc, const wxString &text);

  bool Copy(const gbtRationalRendererRefData &other)
  {
    return wxSheetCellRendererRefData::Copy(other);
  }

  // NOLINTNEXTLINE(modernize-use-auto)
  DECLARE_GAMBIT_SHEETOBJREFDATA_COPY_CLASS(gbtRationalRendererRefData, wxSheetCellRendererRefData)
};

class gbtRationalEditorRefData : public wxSheetCellTextEditorRefData {
public:
  gbtRationalEditorRefData() = default;

  void CreateEditor(wxWindow *, wxWindowID, wxEvtHandler *, wxSheet *) override;

  /// Override basic text editor behavior to restrict input
  bool IsAcceptedKey(wxKeyEvent &) override;
  void StartingKey(wxKeyEvent &) override;

  bool Copy(const gbtRationalEditorRefData &other);

  // NOLINTNEXTLINE(modernize-use-auto)
  DECLARE_GAMBIT_SHEETOBJREFDATA_COPY_CLASS(gbtRationalEditorRefData, wxSheetCellTextEditorRefData)
};

#endif // RENRATIO_H
