//
// FILE: nfgtable.h -- Declaration of normal form table class
//
// $Id$
//

#ifndef NFGTABLE_H
#define NFGTABLE_H

#include "wx/grid.h"

class NfgTable : public wxGrid {
private:
  NfgShow *m_parent;
  int m_showProb, m_showDom, m_showValue;

  // Event handlers
  void OnLeftClick(wxGridEvent &);
  void OnLeftDoubleClick(wxGridEvent &);

public:
  NfgTable(NfgShow *);
  virtual ~NfgTable() { }

  void SetProfile(const gArray<int> &profile);
  void SetStrategy(int p_player, int p_strategy);

  void SetPlayers(int p_rowPlayer, int p_colPlayer);

  void ToggleProbs(void);
  int ShowProbs(void) const { return m_showProb; }
  void ToggleDominance(void);
  int ShowDominance(void) const { return m_showDom; }
  void ToggleValues(void);
  int ShowValues(void) const { return m_showValue; }
  
  int GetDecimals(void) const { return m_parent->GetDecimals(); }

  void OnChangeValues(void);
  void OnChangeLabels(void);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGTABLE_H






