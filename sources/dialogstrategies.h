//
// FILE: dialogstrategies.h -- Declaration of a class to inspect strategies
//
// $Id$
//

#ifndef DIALOGSTRATEGIES_H
#define DIALOGSTRATEGIES_H

class dialogStrategies : public guiAutoDialog {
private:
  Nfg &m_nfg;
  wxListBox *m_players, *m_strategies;
  wxTextCtrl *m_playerName, *m_strategyName;
	
  const char *HelpString(void) const { return "Edit Menu"; }

  void OnSelectPlayer(void);
  void OnEditPlayer(void);
  void OnSelectStrategy(void);
  void OnEditStrategy(void);
 
public:
  dialogStrategies(wxWindow *p_parent, Nfg &p_nfg);
  virtual ~dialogStrategies() { }

  DECLARE_EVENT_TABLE()
};

#endif   // DIALOGSTRATEGIES_H
