//
// FILE: nfginfopanel.h -- Normal form information panel window
//
// $Id$
//

#ifndef NFGINFOPANEL_H
#define NFGINFOPANEL_H

class guiNfgView;

class guiNfgInfoPanel : public wxPanel {
private:
  Nfg &m_nfg;
  guiNfgView *m_parent;
  wxChoice *m_rowPlayer, *m_colPlayer, *m_outcome, *m_support;
  gArray<wxChoice *> m_strategy;
  gArray<wxTextCtrl *> m_playerNames, m_payoffs;

  void OnRowPlayer(wxCommandEvent &);
  void OnColPlayer(wxCommandEvent &);
  void OnStrategy(wxCommandEvent &);
  void OnOutcome(wxCommandEvent &);
  void OnSupport(wxCommandEvent &);
  
public:
  guiNfgInfoPanel(guiNfgView *p_parent, wxWindow *p_window, Nfg &p_nfg);
  virtual ~guiNfgInfoPanel() { }

  void SetProfile(const gArray<int> &p_profile);
  void SetOutcome(NFOutcome *);
  void SetLabels(void);
  void SetSupport(NFSupport *);
  void UpdateSupports(const gList<NFSupport *> &);

  gText GetPlayerName(int pl) const;
  gNumber GetPayoff(int pl) const;
  
  DECLARE_EVENT_TABLE()
};

#endif // NFGINFOPANEL_H
