//
// FILE: nfgtable.h -- Declaration of normal form table class
//
// $Id$
//

#ifndef NFGTABLE_H
#define NFGTABLE_H

#include "wx/grid.h"

class NfgTable : public wxPanel {
private:
  NfgShow *m_parent;
  wxGrid *m_grid;
  wxChoice *m_rowChoice, *m_colChoice, **m_stratProfile;

  int m_showProb, m_showDom, m_showValue;

  // Event handlers
  void OnLeftClick(wxGridEvent &);
  void OnLeftDoubleClick(wxGridEvent &);

  void OnStrategyChange(wxCommandEvent &);
  void OnRowPlayerChange(wxCommandEvent &);
  void OnColPlayerChange(wxCommandEvent &);

public:
  NfgTable(NfgShow *);
  virtual ~NfgTable() { }

  void SetProfile(const gArray<int> &profile);
  gArray<int> GetProfile(void) const;

  void SetStrategy(int p_player, int p_strategy);
  void SetPlayers(int p_rowPlayer, int p_colPlayer);

  int GetRowStrategy(void) const { return m_grid->GetCursorRow() + 1; }
  int GetColStrategy(void) const { return m_grid->GetCursorColumn() + 1; }

  void ToggleProbs(void);
  int ShowProbs(void) const { return m_showProb; }
  void ToggleDominance(void);
  int ShowDominance(void) const { return m_showDom; }
  void ToggleValues(void);
  int ShowValues(void) const { return m_showValue; }
  
  int GetDecimals(void) const { return m_parent->GetDecimals(); }

  void OnChangeValues(void);
  void OnChangeLabels(void);

  void SetSupport(const NFSupport &);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGTABLE_H






