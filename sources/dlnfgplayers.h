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

  void OnEdit(wxCommandEvent &);

  const char *HelpString(void) const { return "Edit Menu (nfg)"; }

public:
  dialogNfgPlayers(Nfg &p_nfg, wxWindow *p_parent = 0);
  virtual ~dialogNfgPlayers()  { }

  DECLARE_EVENT_TABLE()
};

#endif  // DLNFGPLAYERS_H
