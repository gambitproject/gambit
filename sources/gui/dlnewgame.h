//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to create a new game
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
