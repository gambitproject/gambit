//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for computing cognitive hierarchy correspondence
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

#ifndef DLCH_H
#define DLCH_H

#include "game/nfg.h"

class dialogNfgCH : public wxDialog {
private:
  wxTextCtrl *m_minTau, *m_maxTau, *m_stepTau;

public:
  // Lifecycle
  dialogNfgCH(wxWindow *, const gbtNfgSupport &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  double MinTau(void) const
    { return ToNumber(m_minTau->GetValue().c_str()); }
  double MaxTau(void) const
    { return ToNumber(m_maxTau->GetValue().c_str()); }
  double StepTau(void) const
    { return ToNumber(m_stepTau->GetValue().c_str()); }
};


#endif  // DLCH_H
