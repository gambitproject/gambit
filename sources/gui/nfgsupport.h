//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Declaration of normal form support palette window
//

#ifndef NFGSUPPORT_H
#define NFGSUPPORT_H

#include "wx/treectrl.h"
#include "nfgshow.h"
#include "base/gmap.h"

class NfgSupportWindow : public wxPanel {
private:
  NfgShow *m_parent;
  wxChoice *m_supportList;
  wxButton *m_prevButton, *m_nextButton;
  wxTreeCtrl *m_strategyTree;
  gOrdMap<wxTreeItemId, Strategy *> m_map;

  void OnSupportList(wxCommandEvent &);
  void OnSupportPrev(wxCommandEvent &);
  void OnSupportNext(wxCommandEvent &);

  void OnTreeItemCollapse(wxTreeEvent &);

public:
  NfgSupportWindow(NfgShow *p_nfgShow, wxWindow *p_parent);
  virtual ~NfgSupportWindow() { }

  void UpdateValues(void);
  int GetSupport(void) const { return m_supportList->GetSelection(); }
  void ToggleItem(wxTreeItemId);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGSUPPORT_H
