//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for computing quantal response equilibria
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

#ifndef DLNFGQRE_H
#define DLNFGQRE_H

#include "game/nfstrat.h"

class dialogNfgQre : public wxDialog {
private:
  wxRadioBox *m_solveUsing;
  wxTextCtrl *m_startLambda, *m_stopLambda, *m_stepLambda;
  wxTextCtrl *m_del1, *m_tol1, *m_del2, *m_tol2;

  // Event handlers
  void OnSolveUsing(wxCommandEvent &);

public:
  // Lifecycle
  dialogNfgQre(wxWindow *, const gbtNfgSupport &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  bool UseGridSearch(void) const
    { return (m_solveUsing->GetSelection() == 1); }

  double StartLambda(void) const
    { return ToNumber(m_startLambda->GetValue().c_str()); }
  double StopLambda(void) const
    { return ToNumber(m_stopLambda->GetValue().c_str()); }
  double StepLambda(void) const
    { return ToNumber(m_stepLambda->GetValue().c_str()); }

  double Del1(void) const
    { return ToNumber(m_del1->GetValue().c_str()); }
  double Tol1(void) const
    { return ToNumber(m_tol1->GetValue().c_str()); }
  double Del2(void) const
    { return ToNumber(m_del2->GetValue().c_str()); }
  double Tol2(void) const
    { return ToNumber(m_tol2->GetValue().c_str()); }
  

  DECLARE_EVENT_TABLE()
};


#endif  // DLNFGQRE_H
