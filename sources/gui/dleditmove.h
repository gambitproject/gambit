//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a move
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

#ifndef DLEDITMOVE_H
#define DLEDITMOVE_H

class dialogEditMove : public wxDialog {
private:
  gbtEfgInfoset m_infoset;
  wxChoice *m_player;
  wxTextCtrl *m_infosetName, *m_actionName, *m_actionProb;
  wxString m_actionProbValue;
  wxListBox *m_actionList;
  wxButton *m_addBeforeButton, *m_addAfterButton, *m_deleteButton;
  gBlock<gText> m_actionNames;
  gBlock<gNumber> m_actionProbs;
  gBlock<gbtEfgAction> m_actions;
  int m_lastSelection;

  // Event handlers
  void OnActionChanged(wxCommandEvent &);
  void OnAddActionBefore(wxCommandEvent &);
  void OnAddActionAfter(wxCommandEvent &);
  void OnDeleteAction(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogEditMove(wxWindow *p_parent, gbtEfgInfoset p_infoset);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetInfosetName(void) const { return m_infosetName->GetValue(); }
  int GetPlayer(void) const { return (m_player->GetSelection() + 1); }

  int NumActions(void) const { return m_actions.Length(); }
  const gBlock<gbtEfgAction> &GetActions(void) const { return m_actions; }
  gText GetActionName(int p_act) const { return m_actionNames[p_act]; }
  gNumber GetActionProb(int p_act) const { return m_actionProbs[p_act]; }

  DECLARE_EVENT_TABLE()
};

#endif   // DLEDITMOVE_H
