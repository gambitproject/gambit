//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to navigation window for normal forms
//

#ifndef NFGNAVIGATE_H
#define NFGNAVIGATE_H

#include "nfgshow.h"

class NfgNavigateWindow : public wxPanel {
private:
  NfgShow *m_parent;
  wxStaticText **m_playerNames;
  wxChoice *m_rowChoice, *m_colChoice, **m_stratProfile;

  int m_rowPlayer, m_colPlayer;
  NFSupport m_support;
  
  void OnStrategyChange(wxCommandEvent &);
  void OnRowPlayerChange(wxCommandEvent &);
  void OnColPlayerChange(wxCommandEvent &);

public:
  NfgNavigateWindow(NfgShow *p_nfgShow, wxWindow *p_parent);
  virtual ~NfgNavigateWindow();
  
  void SetProfile(const gArray<int> &profile);
  gArray<int> GetProfile(void) const;

  void SetStrategy(int p_player, int p_strategy);
  void SetPlayers(int p_rowPlayer, int p_colPlayer);

  int GetRowStrategy(void) const;
  int GetColStrategy(void) const;

  void SetSupport(const NFSupport &);
  const NFSupport &GetSupport(void) const { return m_support; }

  int GetRowPlayer(void) const { return m_rowPlayer; }
  int GetColPlayer(void) const { return m_colPlayer; }

  void UpdateLabels(void);

  DECLARE_EVENT_TABLE()
};


#endif  // NFGNAVIGATE_H
