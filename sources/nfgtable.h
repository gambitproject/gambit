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

  struct ns_features_struct {
    int prob, dom, val; /* these are actually int, not bool 0 or 1 */
    bool verbose;
    ns_features_struct(void) :prob(0), dom(0), val(0), verbose(TRUE) { }
    ns_features_struct(const ns_features_struct &s): prob(s.prob), dom(s.dom),
      val(s.val), verbose(s.verbose) { }
  } features;

  // Event handlers
  void OnLeftDoubleClick(wxGridEvent &);

  // Overriding wxGrid member functions
  void OnSelectCell(int p_row, int p_col);

public:
  NfgTable(NfgShow *);
  virtual ~NfgTable() { }

  void SetProfile(const gArray<int> &profile);
  void SetStrategy(int p_player, int p_strategy);

  void SetPlayers(int p_rowPlayer, int p_colPlayer);

  // Functions to create an extra row&col to display probs, dominators, values
  void MakeProbDisp(void);
  void RemoveProbDisp(void);
  int  HaveProbs(void) const { return features.prob; }
  void MakeDomDisp(void);
  void RemoveDomDisp(void);
  int  HaveDom(void) const { return features.dom; }
  void MakeValDisp(void);
  void RemoveValDisp(void);
  int  HaveVal(void) const { return features.val; }
  
  int GetDecimals(void) const { return m_parent->GetDecimals(); }

  void OnChangeValues(void);
  virtual void OnChangeLabels(void);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGTABLE_H
