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

  void OnEdit(wxCommandEvent &);
  void OnNew(wxCommandEvent &);

  const char *HelpString(void) const { return "Edit Menu (efg)"; }

public:
  dialogEfgPlayers(FullEfg &p_efg, wxWindow *p_parent = 0);
  virtual ~dialogEfgPlayers()  { }

  DECLARE_EVENT_TABLE()
};

#endif  // DLEFGPLAYERS_H
