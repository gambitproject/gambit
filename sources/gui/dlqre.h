//
// FILE: dlqre.h -- Parameter selection for QreSolve
//
// $Id$
//

#ifndef DLQRE_H
#define DLQRE_H

#include "algdlgs.h"

class dialogQre : public wxDialog {
private:
  wxRadioBox *m_finiteLambda;
  wxTextCtrl *m_maxLambda, *m_stepSize;
  wxCheckBox *m_pxiFile;
  wxString m_maxLambdaValue, m_stepSizeValue;

  // Event handlers
  void OnStoppingLambda(wxCommandEvent &);

public:
  dialogQre(wxWindow *p_parent);

  // Returning data from dialog; only valid if ShowModal() returns OK.
  bool FiniteLambda(void) const;
  bool GeneratePXIFile(void) const;
  
  double MaxLambda(void) const;
  double StepSize(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DLQRE_H
