//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for editing properties of mixed profiles
//

#ifndef DLEDITMIXED_H
#define DLEDITMIXED_H

#include "wx/grid.h"

class dialogEditMixed : public wxDialog {
private:
  mutable MixedSolution m_profile;
  wxTextCtrl *m_profileName;
  wxListBox *m_playerList;
  wxGrid *m_probGrid;
  int m_selection;

  // Event handlers
  void OnSelChanged(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  dialogEditMixed(wxWindow *p_parent, const MixedSolution &p_profile);
  virtual ~dialogEditMixed();

  const MixedSolution &GetProfile(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // MIXEDEDIT_H
