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

class dialogQreFile : public wxDialog {
private:
  wxListCtrl *m_qreList;
  const gList<MixedSolution> m_profiles;

  // Event handlers
  void OnPxiFile(wxCommandEvent &);

public:
  dialogQreFile(wxWindow *, const gList<MixedSolution> &);

  DECLARE_EVENT_TABLE()
};



#endif   // DLQREFILE_H



