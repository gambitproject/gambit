//
// FILE: dlefgplayer.h -- (Extensive form) player selection dialog
//
// $Id$
//

#ifndef DLEFGPLAYER_H
#define DLEFGPLAYER_H

class dialogEfgSelectPlayer : public wxDialogBox {
private:
  const Efg &m_efg;
  bool m_chance;
  int m_completed;
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
  dialogEfgSelectPlayer(const Efg &, bool p_chance = true, wxWindow * = 0);
  virtual ~dialogEfgSelectPlayer();

  int Completed(void) const { return m_completed; }
  EFPlayer *GetPlayer(void);
};

#endif  // DLEFGPLAYER_H
