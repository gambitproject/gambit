//
// FILE: dlnfgplayers.h -- Defines a dialog to inspect/create players
//
// $Id$
//

#ifndef DLNFGPLAYERS_H
#define DLNFGPLAYERS_H

class dialogNfgPlayers : public guiAutoDialog {
private:
  int m_lastSelection;
  Nfg &m_nfg;

  wxListBox *m_playerNameList;

  static void CallbackEdit(wxButton &p_object, wxCommandEvent &)
    { ((dialogNfgPlayers *) p_object.GetClientData())->OnEdit(); }

  void OnEdit(void);

  const char *HelpString(void) const { return "Edit Menu (nfg)"; }

public:
  dialogNfgPlayers(Nfg &p_nfg, wxWindow *p_parent = 0);
  virtual ~dialogNfgPlayers()  { }
};

#endif  // DLNFGPLAYERS_H
