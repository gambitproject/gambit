//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a move
//

#ifndef DLEDITMOVE_H
#define DLEDITMOVE_H

class dialogEditMove : public wxDialog {
private:
  Infoset *m_infoset;
  wxChoice *m_player;
  wxTextCtrl *m_infosetName, *m_actionName, *m_actionProb;
  wxString m_actionProbValue;
  wxListBox *m_actionList;
  wxButton *m_addBeforeButton, *m_addAfterButton, *m_deleteButton;
  gBlock<gText> m_actionNames;
  gBlock<gNumber> m_actionProbs;
  gBlock<Action *> m_actions;
  int m_lastSelection;

  // Event handlers
  void OnActionChanged(wxCommandEvent &);
  void OnAddActionBefore(wxCommandEvent &);
  void OnAddActionAfter(wxCommandEvent &);
  void OnDeleteAction(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogEditMove(wxWindow *p_parent, Infoset *p_infoset);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetInfosetName(void) const { return m_infosetName->GetValue(); }
  int GetPlayer(void) const { return (m_player->GetSelection() + 1); }

  int NumActions(void) const { return m_actions.Length(); }
  const gBlock<Action *> &GetActions(void) const { return m_actions; }
  gText GetActionName(int p_act) const { return m_actionNames[p_act]; }
  gNumber GetActionProb(int p_act) const { return m_actionProbs[p_act]; }

  DECLARE_EVENT_TABLE()
};

#endif   // DLEDITMOVE_H
