//
// FILE: nfgpanel.h -- Declaration of normal form navigation panel
//
// $Id$
//

#ifndef NFGPANEL_H
#define NFGPANEL_H

class NfgPanel : public wxPanel {
private:
  NfgShow *m_parent;
  wxChoice *m_rowChoice, *m_colChoice, **m_stratProfile;

  // Event handlers
  void OnStrategyChange(wxCommandEvent &);
  void OnRowPlayerChange(wxCommandEvent &);
  void OnColPlayerChange(wxCommandEvent &);
  
public:
  NfgPanel(NfgShow *, wxWindow *);
  virtual ~NfgPanel() { }

  void SetProfile(const gArray<int> &p_profile);
  gArray<int> GetProfile(void) const;

  void SetSupport(const NFSupport &);
  void SetStrategy(int p_player, int p_strategy);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGPANEL_H
