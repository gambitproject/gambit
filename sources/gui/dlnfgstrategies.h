//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to inspect and edit normal form strategies
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

#ifndef DLNFGSTRATEGIES_H
#define DLNFGSTRATEGIES_H

class dialogStrategies : public wxDialog {
private:
  const gbtNfgGame &m_nfg;
  wxChoice *m_player;
  wxListBox *m_strategyList;
  wxTextCtrl *m_strategyName;
  gBlock<gArray<gText> > m_strategyNames;
  int m_lastPlayer, m_lastStrategy;

  // Event handlers
  void OnPlayerChanged(wxCommandEvent &);
  void OnStrategyChanged(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogStrategies(wxWindow *, const gbtNfgGame &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  const gText &GetStrategyName(int pl, int st) const
    { return m_strategyNames[pl][st]; }

  DECLARE_EVENT_TABLE()
};

#endif   // DLNFGSTRATEGIES_H
