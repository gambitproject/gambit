//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to inspect and edit normal form strategies
//

#ifndef DLNFGSTRATEGIES_H
#define DLNFGSTRATEGIES_H

class dialogStrategies : public wxDialog {
private:
  const Nfg &m_nfg;
  wxChoice *m_player;
  wxListBox *m_strategyList;
  wxTextCtrl *m_strategyName;
  gBlock<gArray<gText> > m_strategyNames;
  int m_lastStrategy;

  // Event handlers
  void OnPlayerChanged(wxCommandEvent &);
  void OnStrategyChanged(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogStrategies(wxWindow *, const Nfg &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  const gText &GetStrategyName(int pl, int st) const
    { return m_strategyNames[pl][st]; }

  DECLARE_EVENT_TABLE()
};

#endif   // DLNFGSTRATEGIES_H
