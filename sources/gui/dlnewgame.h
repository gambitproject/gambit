//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to create a new game
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

#ifndef DLNEWGAME_H
#define DLNEWGAME_H

#include "base/gblock.h"
#include "wx/spinctrl.h"

class dialogNewGame : public wxDialog {
private:
  wxRadioBox *m_gameType;
  wxSpinCtrl *m_numPlayers, *m_numStrategies;
  wxListBox *m_strategyList;
  gBlock<int> m_strategies;
  wxCheckBox *m_createOutcomes;

  // Event handlers
  void OnGameType(wxCommandEvent &);
  void OnNumPlayers(wxSpinEvent &);
  void OnStrategy(wxCommandEvent &);
  void OnNumStrategies(wxSpinEvent &);

public:
  // Lifecycle
  dialogNewGame(wxWindow *p_parent);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  bool CreateEfg(void) const { return (m_gameType->GetSelection() == 0); }
  int NumPlayers(void) const { return m_numPlayers->GetValue(); }

  // Only valid if CreateEfg() returns false
  const gArray<int> &NumStrategies(void) const { return m_strategies; }
  bool CreateOutcomes(void) const { return m_createOutcomes->GetValue(); }

  DECLARE_EVENT_TABLE()
};

#endif  // DLNEWGAME_H
