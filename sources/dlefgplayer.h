//
// FILE: dlefgplayer.h -- (Extensive form) player selection dialog
//
//
//

#ifndef DLEFGPLAYER_H
#define DLEFGPLAYER_H

class dialogEfgSelectPlayer : public wxDialogBox {
private:
  Efg &m_efg;
  int m_playerSelected, m_completed;
  wxListBox *m_playerNameList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogEfgSelectPlayer *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogEfgSelectPlayer *) p_object.GetClientData())->OnCancel(); }
  static void CallbackHelp(wxButton &, wxEvent &)
    { wxHelpContents(EFG_TREE_HELP); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  dialogEfgSelectPlayer(Efg &, wxWindow * = 0);
  virtual ~dialogEfgSelectPlayer();

  int Completed(void) const { return m_completed; }
  EFPlayer *GetPlayer(void);
};

#endif  // DLEFGPLAYER_H
