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
  wxText *m_strategyNameItem;
	
  static void CallbackPlayer(wxListBox &, wxCommandEvent &);
  static void CallbackStrategy(wxListBox &, wxCommandEvent &);

  const char *HelpString(void) const { return "Edit Menu (nfg)"; }

  void OnPlayer(int);
  void OnStrategy(int);

  void OnOk(void);

public:
  dialogStrategies(Nfg &, wxFrame * = 0);
  virtual ~dialogStrategies() { }

  bool GameChanged(void) const { return m_gameChanged; } 
};

#endif   // DLSTRATEGIES_H
