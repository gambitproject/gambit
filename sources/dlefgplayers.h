//
// FILE: dlefgplayers.h -- Defines a dialog to inspect/create players
//
// $Id$
//

#ifndef DLEFGPLAYERS_H
#define DLEFGPLAYERS_H

class dialogEfgPlayers : public guiAutoDialog {
private:
  int m_lastSelection;
  FullEfg &m_efg;

  wxListBox *m_playerNameList;

  static void CallbackEdit(wxButton &p_object, wxCommandEvent &)
    { ((dialogEfgPlayers *) p_object.GetClientData())->OnEdit(); }
  static void CallbackNew(wxButton &p_object, wxCommandEvent &)
    { ((dialogEfgPlayers *) p_object.GetClientData())->OnNew(); }

  void OnEdit(void);
  void OnNew(void);

  const char *HelpString(void) const { return "Tree Menu"; }

public:
  dialogEfgPlayers(FullEfg &p_efg, wxWindow *p_parent = 0);
  virtual ~dialogEfgPlayers()  { }
};

#endif  // DLEFGPLAYERS_H
