//
// FILE: dlinfosets.h -- Declaration of a class to inspect information sets
//
// $Id$
//

#ifndef DLINFOSETS_H
#define DLINFOSETS_H

class dialogInfosets : public guiAutoDialog {
private:
  FullEfg &m_efg;
  bool m_gameChanged;
  Infoset *m_prevInfoset;

  wxListBox *m_playerItem, *m_infosetItem;
  wxTextCtrl *m_infosetNameItem, *m_actionsItem, *m_membersItem;
  wxButton *m_removeButton, *m_editButton;
	
  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);
  void OnEdit(wxCommandEvent &);
  void OnNew(wxCommandEvent &);
  void OnRemove(wxCommandEvent &);

  const char *HelpString(void) const { return "Edit Menu (efg)"; }

public:
  dialogInfosets(FullEfg &, wxWindow *);
  virtual ~dialogInfosets() { }

  bool GameChanged(void) const { return m_gameChanged; } 

  DECLARE_EVENT_TABLE()
};

#endif   // DLINFOSETS_H
