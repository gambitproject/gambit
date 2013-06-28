//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlnash.h
// Dialog for selecting algorithms to compute Nash equilibria
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

#ifndef DLNASH_H
#define DLNASH_H

#include "gamedoc.h"

class gbtNashChoiceDialog : public wxDialog {
private:
  gbtGameDocument *m_doc;
  wxChoice *m_countChoice, *m_methodChoice, *m_repChoice;

  // Event handlers
  void OnCount(wxCommandEvent &);
  void OnMethod(wxCommandEvent &);

public:
  gbtNashChoiceDialog(wxWindow *, gbtGameDocument *);

  gbtAnalysisOutput *GetCommand(void) const;

  bool UseStrategic(void) const;
};


#endif   // DLNFGNASH_H

