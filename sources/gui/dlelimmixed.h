//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for parameters to eliminate dominated strategies
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

#ifndef DLELIMMIXED_H
#define DLELIMMIXED_H

class dialogElimMixed : public wxDialog {
private:
  int m_numPlayers;
  wxCheckBox **m_players;
  wxRadioBox *m_domTypeBox, *m_domMethodBox, *m_domPrecisionBox;
  wxRadioBox *m_domDepthBox;

  // Event handlers
  void OnMethod(wxCommandEvent &);
  
public:
  dialogElimMixed(wxWindow *, const gArray<gText> &p_players);
  virtual ~dialogElimMixed();

  gArray<int> Players(void) const;

  bool Iterative(void) const { return (m_domDepthBox->GetSelection() == 1); }
  bool DomStrong(void) const { return (m_domTypeBox->GetSelection() == 1); }
  bool DomMixed(void) const { return (m_domMethodBox->GetSelection() == 1); }
  gPrecision Precision(void) const
    { return (m_domPrecisionBox->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }

  DECLARE_EVENT_TABLE()
};

#endif   // DLELIMMIXED_H

