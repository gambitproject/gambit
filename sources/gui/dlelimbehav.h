//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for parameters to eliminate dominated actions
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

#ifndef DLELIMBEHAV_H
#define DLELIMBEHAV_H

class dialogElimBehav : public wxDialog {
private:
  int m_numPlayers;
  wxCheckBox **m_players;
  wxRadioBox *m_domTypeBox, *m_domConditionalBox, *m_domDepthBox;

public:
  dialogElimBehav(wxWindow *, const gArray<gText> &p_players);
  virtual ~dialogElimBehav();

  gArray<int> Players(void) const;

  bool Iterative(void) const { return (m_domDepthBox->GetSelection() == 1); }
  bool DomStrong(void) const { return (m_domTypeBox->GetSelection() == 1); }
  bool DomConditional(void) const 
    { return (m_domConditionalBox->GetSelection() == 0); }
};

#endif  // DLELIMBEHAV_H
