//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of panel showing list of computed mixed strategy profiles
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

#ifndef PANEL_MIXED_LIST_H
#define PANEL_MIXED_LIST_H

#include "game-document.h"

BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_EVENT_TYPE(GBT_MIXED_PROFILE_SELECTED, 7779)
END_DECLARE_EVENT_TYPES()

#define EVT_MIXED_PROFILE_SELECTED(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        GBT_MIXED_PROFILE_SELECTED, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

class gbtMixedProfileCtrl : public wxSheet, public gbtGameView {
private:
  gbtList<gbtMixedProfile<double> > &m_eqa;

  // Overriding wxSheet members for data access
  wxString GetCellValue(const wxSheetCoords &);
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const;

  // Event handlers
  void OnRowSelected(wxSheetRangeSelectEvent &);

public:
  gbtMixedProfileCtrl(wxWindow *p_parent, gbtGameDocument *p_doc,
		      gbtList<gbtMixedProfile<double> > &p_eqa);

  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};

#endif  // PANEL_MIXED_LIST_H
