//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of dialog to set tree layout parameters
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

#ifndef DLEFGLAYOUT_H
#define DLEFGLAYOUT_H

#include "wx/notebook.h"
#include "prefs.h"

class dialogLayout : public wxDialog {
private:
  wxNotebook *m_notebook;

public:
  // Lifecycle
  dialogLayout(wxWindow *p_parent, const gbtPreferences &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  void GetPreferences(gbtPreferences &);
};

#endif  // DLEFGLAYOUT_H

