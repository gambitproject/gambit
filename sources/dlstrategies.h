//
// FILE: dlstrategies.h -- Declaration of a class to inspect strategies
//
// $Id$
//

#ifndef DLSTRATEGIES_H
#define DLSTRATEGIES_H

class dialogStrategies : public wxDialogBox {
private:
  Nfg &m_nfg;
  bool m_gameChanged;
  Strategy *m_prevStrategy;

  wxListBox *m_playerItem, *m_strategyItem;
  wxText *m_strategyNameItem;
	
  static void CallbackPlayer(wxListBox &, wxCommandEvent &);
  static void CallbackStrategy(wxListBox &, wxCommandEvent &);
  static void CallbackOk(wxButton &, wxCommandEvent &);
  static void CallbackHelp(wxButton &, wxCommandEvent &);

  void OnOk(void);
  Bool OnClose(void);
  void OnHelp(void);

  void OnPlayer(int);
  void OnStrategy(int);

public:
  dialogStrategies(Nfg &, wxFrame * = 0);
  virtual ~dialogStrategies() { }

  bool GameChanged(void) const { return m_gameChanged; } 
};

#endif   // DLSTRATEGIES_H
