//
// FILE: playersd.h -- Defines a dialog to inspect/create players
//
// $Id$
//

#ifndef DLEFGPLAYERS_H
#define DLEFGPLAYERS_H

class dialogEfgPlayers : public wxDialogBox {
private:
  int m_lastSelection;
  Efg	&m_efg;

  wxListBox *m_playerNameList;

  static void CallbackEdit(wxButton &p_object, wxCommandEvent &)
    { ((dialogEfgPlayers *) p_object.GetClientData())->OnEdit(); }
  static void CallbackNew(wxButton &p_object, wxCommandEvent &)
    { ((dialogEfgPlayers *) p_object.GetClientData())->OnNew(); }
  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogEfgPlayers *) p_object.GetClientData())->OnOK(); }
  static void CallbackHelp(wxButton &, wxEvent &)
    { wxHelpContents(EFG_TREE_HELP); }

  void OnOK(void);
  Bool OnClose(void);

  void OnEdit(void);
  void OnNew(void);

public:
  dialogEfgPlayers(Efg &p_efg, wxWindow *p_parent = 0);
  virtual ~dialogEfgPlayers()  { }
};

#endif  // DLEFGPLAYERS_H
