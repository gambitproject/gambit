//
// FILE: dialoginfosets.h -- Information set editing dialog
//
// $Id$
//

#ifndef DIALOGINFOSETS_H
#define DIALOGINFOSETS_H

#include "dialogauto.h"

class dialogInfosets : public guiAutoDialog {
private:
  FullEfg &m_efg;
  bool m_gameChanged;
  wxListBox *m_playerItem, *m_infosetItem, *m_actionItem;
  wxTextCtrl *m_playerName, *m_infosetName, *m_actionName;
  wxButton *m_removeButton;
	
  void OnSelectPlayer(wxCommandEvent &);
  void OnEditPlayer(wxCommandEvent &);
  void OnNewPlayer(wxCommandEvent &);
  void OnSelectInfoset(wxCommandEvent &);
  void OnEditInfoset(wxCommandEvent &);
  void OnEdit(wxCommandEvent &);

  void NewInfoset(void);
  void RemoveInfoset(void);

  const char *HelpString(void) const { return "Tree Menu"; }

public:
  dialogInfosets(wxWindow *, FullEfg &);
  virtual ~dialogInfosets() { }

  bool GameChanged(void) const { return m_gameChanged; } 

  DECLARE_EVENT_TABLE()
};

#endif   // DIALOGINFOSETS_H
