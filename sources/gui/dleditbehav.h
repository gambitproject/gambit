//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for editing properties of behavior profiles
//

#ifndef DLEDITBEHAV_H
#define DLEDITBEHAV_H

#include "wx/treectrl.h"
#include "wx/grid.h"

#include "base/base.h"

class dialogEditBehav : public wxDialog {
private:
  mutable BehavSolution m_profile;
  Infoset *m_lastInfoset;
  wxTextCtrl *m_profileName;
  wxTreeCtrl *m_infosetTree;
  wxGrid *m_probGrid;
  gOrdMap<wxTreeItemId, Infoset *> m_map;

  // Event handlers
  void OnItemCollapsing(wxTreeEvent &);
  void OnSelChanging(wxTreeEvent &);
  void OnSelChanged(wxTreeEvent &);
  void OnOK(wxCommandEvent &);

public:
  dialogEditBehav(wxWindow *p_parent, const BehavSolution &p_profile);
  virtual ~dialogEditBehav();

  const BehavSolution &GetProfile(void) const;

  DECLARE_EVENT_TABLE()
};


#endif  // BEHAVEDIT_H
