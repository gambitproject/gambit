//
// FILE: dlsimpdiv.h -- Parameter selection for SimpdivSolve
//
// $Id$
//

#ifndef DLSIMPDIV_H
#define DLSIMPDIV_H

#include "algdlgs.h"

class dialogSimpdiv : public dialogAlgorithm {
private:
  wxTextCtrl *m_nRestarts, *m_leashLength;
  wxString m_nRestartsValue, m_leashLengthValue;

  const char *HelpString(void) const { return "SimpDiv"; }

  void AlgorithmFields(void);

public:
  dialogSimpdiv(wxWindow *p_parent = 0, bool p_subgames = false);
  virtual ~dialogSimpdiv();

  int NumRestarts(void) const 
    { return ToNumber(m_nRestarts->GetValue().c_str()); }
  int LeashLength(void) const
    { return ToNumber(m_leashLength->GetValue().c_str()); }
  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DLSIMPDIV_H
