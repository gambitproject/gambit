//
// FILE: behavedit.h -- Dialog for editing behavior profiles
//
// $Id$
//

#ifndef BEHAVEDIT_H
#define BEHAVEDIT_H

#include "wx/treectrl.h"
#include "wx/grid.h"

#include "gmap.h"

class dialogBehavEditor : public wxDialog {
private:
  BehavSolution m_profile;
  wxTreeCtrl *m_infosetTree;
  wxGrid *m_probGrid;
  gOrdMap<wxTreeItemId, Infoset *> m_map;

  // Event handlers
  void OnItemCollapsing(wxTreeEvent &);
  void OnSelChanging(wxTreeEvent &);
  void OnSelChanged(wxTreeEvent &);
  void OnOK(wxCommandEvent &);

public:
  dialogBehavEditor(wxWindow *p_parent, const BehavSolution &p_profile);
  virtual ~dialogBehavEditor();

  const BehavSolution &GetProfile(void) const { return m_profile; }

  DECLARE_EVENT_TABLE()
};


#endif  // BEHAVEDIT_H
