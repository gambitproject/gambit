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
  wxText *m_infosetNameItem, *m_actionsItem, *m_membersItem;
  wxButton *m_removeButton, *m_editButton;
	
  static void CallbackPlayer(wxListBox &, wxCommandEvent &);
  static void CallbackInfoset(wxListBox &, wxCommandEvent &);
  static void CallbackEdit(wxButton &, wxCommandEvent &);
  static void CallbackNew(wxButton &, wxCommandEvent &);
  static void CallbackRemove(wxButton &, wxCommandEvent &);

  void OnPlayer(int);
  void OnInfoset(void);
  void OnEdit(void);

  void NewInfoset(void);
  void RemoveInfoset(void);

  const char *HelpString(void) const { return "Edit Menu (efg)"; }

public:
  dialogInfosets(FullEfg &, wxFrame * = 0);
  virtual ~dialogInfosets() { }

  bool GameChanged(void) const { return m_gameChanged; } 
};

#endif   // DLINFOSETS_H
