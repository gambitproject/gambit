//
// FILE: dlefgplayer.h -- (Extensive form) player selection dialog
//
// $Id$
//

#ifndef DLEFGPLAYER_H
#define DLEFGPLAYER_H

class dialogEfgSelectPlayer : public guiAutoDialog {
private:
  const Efg &m_efg;
  bool m_chance;
  wxListBox *m_playerNameList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogEfgSelectPlayer *) p_object.GetClientData())->OnOk(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogEfgSelectPlayer *) p_object.GetClientData())->OnCancel(); }
  static void CallbackHelp(wxButton &, wxEvent &)
    { wxHelpContents(EFG_TREE_HELP); }

public:
  dialogEfgSelectPlayer(const Efg &, bool p_chance = true, wxWindow * = 0);
  virtual ~dialogEfgSelectPlayer();

  EFPlayer *GetPlayer(void);
};

#endif  // DLEFGPLAYER_H
