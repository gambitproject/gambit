//
// FILE: dlinfosets.h -- Declaration of a class to inspect information sets
//
// $Id$
//

#ifndef DLINFOSETS_H
#define DLINFOSETS_H

class dialogInfosets : public wxDialogBox {
private:
  Efg &m_efg;
  int m_completed;
  bool m_gameChanged;
  Infoset *m_prevInfoset;

  wxListBox *m_playerItem, *m_infosetItem;
  wxText *m_infosetNameItem, *m_actionsItem, *m_membersItem;
	
  static void CallbackPlayer(wxListBox &, wxCommandEvent &);
  static void CallbackInfoset(wxListBox &, wxCommandEvent &);
  static void CallbackNew(wxButton &, wxCommandEvent &);
  static void CallbackRemove(wxButton &, wxCommandEvent &);
  static void CallbackOk(wxButton &, wxCommandEvent &);
  static void CallbackCancel(wxButton &, wxCommandEvent &);
  static void CallbackHelp(wxButton &, wxCommandEvent &);

  void OnOk(void);
  void OnCancel(void);
  Bool OnClose(void);
  void OnHelp(void);

  void OnPlayer(int);
  void OnInfoset(int);

  void NewInfoset(void);
  void RemoveInfoset(void);

public:
  dialogInfosets(Efg &, wxFrame * = 0);
  virtual ~dialogInfosets() { }

  int Completed(void) const { return m_completed; }
  bool GameChanged(void) const { return m_gameChanged; } 
};

#endif   // DLINFOSETS_H
