//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to show QRE correspondence and optionally write PXI file
//

#ifndef DLQREFILE_H
#define DLQREFILE_H

#include "wx/listctrl.h"

//
// Keeping both types of solution here is more than a bit kludgy;
// need a much better way of doing this.
// Really, this should evolve into a flexible dialog for listing
// and selecting profiles, and the .pxifile exporting stuff should
// be moved to a set of qretools
//
class dialogQreFile : public wxDialog {
private:
  wxListCtrl *m_qreList;
  gList<MixedSolution> m_mixedProfiles;
  gList<BehavSolution> m_behavProfiles;

  // Event handlers
  void OnPxiFile(wxCommandEvent &);

public:
  dialogQreFile(wxWindow *, const gList<MixedSolution> &);
  dialogQreFile(wxWindow *, const gList<BehavSolution> &);

  DECLARE_EVENT_TABLE()
};

#endif   // DLQREFILE_H



