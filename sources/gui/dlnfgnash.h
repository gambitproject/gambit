//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for selecting algorithms to compute Nash equilibria
//

#ifndef DLNFGNASH_H
#define DLNFGNASH_H

#include "wx/treectrl.h"
#include "base/gmap.h"
#include "nash/nfgalgorithm.h"

class panelNfgNashAlgorithm;

class dialogNfgNash : public wxDialog {
private:
  wxTreeCtrl *m_algorithmTree;
  wxPanel *m_fooPanel, *m_currentPanel;
  gOrdMap<wxTreeItemId, panelNfgNashAlgorithm *> m_algorithms;

  // Private auxiliary functions
  void LoadAlgorithms(void);

  // Event handlers
  void OnSelectionChanged(wxTreeEvent &);

public:
  // Lifecycle
  dialogNfgNash(wxWindow *, const NFSupport &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  nfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};



#endif   // DLNFGNASH_H
