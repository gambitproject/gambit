//
// FILE: mixededit.h -- Dialog for editing mixed profiles
//
// $Id$
//

#ifndef MIXEDEDIT_H
#define MIXEDEDIT_H

#include "wx/grid.h"

class dialogMixedEditor : public wxDialog {
private:
  MixedSolution m_profile;
  wxListBox *m_playerList;
  wxGrid *m_probGrid;
  int m_selection;

  // Event handlers
  void OnSelChanged(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  dialogMixedEditor(wxWindow *p_parent, const MixedSolution &p_profile);
  virtual ~dialogMixedEditor();

  const MixedSolution &GetProfile(void) const { return m_profile; }

  DECLARE_EVENT_TABLE()
};

#endif  // MIXEDEDIT_H
