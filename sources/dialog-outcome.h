//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing outcome payoffs
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

#ifndef DIALOG_OUTCOME_H
#define DIALOG_OUTCOME_H

#include <wx/grid.h>

class gbtOutcomeDialog : public wxDialog {
private:
  gbtGame m_game;
  gbtGameOutcome m_outcome;
  wxGrid *m_grid;

public:
  // Lifecycle
  gbtOutcomeDialog(wxWindow *p_parent, gbtGameDocument *p_doc,
		   const gbtGame &p_game,
		   const gbtGameOutcome &p_outcome);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  gbtRational GetPayoff(int pl) const;
};

#endif   // DIALOG_OUTCOME_H
