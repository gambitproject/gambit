//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Declaration of extensive form support palette window
//

#ifndef EFGSUPPORT_H
#define EFGSUPPORT_H

#include "efgshow.h"
#include "base/gmap.h"

class EfgSupportWindow : public wxPanel {
private:
  EfgShow *m_parent;
  wxChoice *m_supportList;
  wxButton *m_prevButton, *m_nextButton;
  wxTreeCtrl *m_actionTree;
  gOrdMap<wxTreeItemId, Action *> m_map;

  void OnSupportList(wxCommandEvent &);
  void OnSupportPrev(wxCommandEvent &);
  void OnSupportNext(wxCommandEvent &);

  void OnTreeItemCollapse(wxTreeEvent &);

public:
  EfgSupportWindow(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~EfgSupportWindow() { }

  void UpdateValues(void);

  DECLARE_EVENT_TABLE()
};

#endif  // EFGSUPPORT_H