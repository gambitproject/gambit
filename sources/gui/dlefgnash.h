//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for selecting algorithms to compute Nash equilibria
//

#ifndef DLEFGNASH_H
#define DLEFGNASH_H

#include "wx/treectrl.h"
#include "base/gmap.h"
#include "nash/efgalgorithm.h"

class panelEfgNashAlgorithm;

class dialogEfgNash : public wxDialog {
private:
  wxTreeCtrl *m_algorithmTree;
  wxPanel *m_fooPanel, *m_currentPanel;
  gOrdMap<wxTreeItemId, panelEfgNashAlgorithm *> m_algorithms;

  // Private auxiliary functions
  void LoadAlgorithms(const efgGame &);

  // Event handlers
  void OnSelectionChanged(wxTreeEvent &);

public:
  // Lifecycle
  dialogEfgNash(wxWindow *, const EFSupport &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  efgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

#endif   // DLEFGNASH_H
