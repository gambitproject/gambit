//
// FILE: dlstrategies.h -- Declaration of a class to inspect strategies
//
// $Id$
//

#ifndef DLSTRATEGIES_H
#define DLSTRATEGIES_H

class dialogStrategies : public guiAutoDialog {
private:
  Nfg &m_nfg;
  bool m_gameChanged;
  Strategy *m_prevStrategy;

  wxListBox *m_playerItem, *m_strategyItem;
  wxTextCtrl *m_strategyNameItem;
	
  const char *HelpString(void) const { return "Edit Menu (nfg)"; }

  void OnPlayer(wxCommandEvent &i);
  void OnStrategy(wxCommandEvent &i);

  void OnOk(void);

public:
  dialogStrategies(Nfg &, wxFrame * = 0);
  virtual ~dialogStrategies() { }

  bool GameChanged(void) const { return m_gameChanged; } 

  DECLARE_EVENT_TABLE()
};

#endif   // DLSTRATEGIES_H
