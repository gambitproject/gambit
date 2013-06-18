//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/nfgpanel.h
// Panel to display normal form games in tabular format
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

#ifndef NFGPANEL_H
#define NFGPANEL_H

#include "wx/sheet/sheet.h"        // for wxSheet

class wxPrintout;
class gbtTableWidget;
class gbtTablePlayerToolbar;

class gbtNfgPanel : public wxPanel, public gbtGameView {
private:
  wxWindow *m_dominanceToolbar;
  gbtTablePlayerToolbar *m_playerToolbar;
  gbtTableWidget *m_tableWidget;

  void OnToolsDominance(wxCommandEvent &);

public:
  gbtNfgPanel(wxWindow *p_parent, gbtGameDocument *p_doc);
  virtual ~gbtNfgPanel() { }

  bool IsDominanceShown(void) const { return m_dominanceToolbar->IsShown(); }

  /// @name Exporting/printing graphics
  //@{
  /// Creates a printout object of the game as currently displayed
  wxPrintout *GetPrintout(void);
  /// Creates a bitmap of the game as currently displayed
  bool GetBitmap(wxBitmap &, int marginX, int marginY);
  /// Outputs the game as currently displayed to a SVG file
  void GetSVG(const wxString &p_filename, int marginX, int marginY);
  /// Prints the game as currently displayed, centered on the DC
  void RenderGame(wxDC &p_dc, int marginX, int marginY);
  //@}

  // Overriding gbtGameView members
  void OnUpdate(void);
  void PostPendingChanges(void);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGPANEL_H






