//
// FILE: dlqre.h -- Parameter selection for QreSolve
//
// $Id$
//

#ifndef DLQRE_H
#define DLQRE_H

#include "algdlgs.h"

class dialogQre : public dialogPxi {
private:
  wxTextCtrl *m_minLam, *m_maxLam, *m_delLam, *m_accuracy;
  wxRadioBox *m_startOption;

  wxString m_minLamValue, m_maxLamValue, m_delLamValue, m_accuracyValue;

  const char *HelpString(void) const  { return "QRE"; }
  
  void AlgorithmFields(void);

public:
  dialogQre(wxWindow *parent, const gText &filename, bool p_vianfg = false);
  virtual ~dialogQre();

  double MinLam(void) const
    { return ToNumber(m_minLam->GetValue().c_str()); }
  double MaxLam(void) const
    { return ToNumber(m_maxLam->GetValue().c_str()); }
  double DelLam(void) const
    { return ToNumber(m_delLam->GetValue().c_str()); }
  int Accuracy(void) const
    { return ToNumber(m_accuracy->GetValue().c_str()); }

  int StartOption(void) const
    { return m_startOption->GetSelection(); }
};

class dialogQreHomotopy : public wxDialog {
private:
  wxRadioBox *m_finiteLambda;
  wxTextCtrl *m_maxLambda, *m_stepSize;
  wxCheckBox *m_pxiFile;
  wxString m_maxLambdaValue, m_stepSizeValue;

  // Event handlers
  void OnStoppingLambda(wxCommandEvent &);

public:
  dialogQreHomotopy(wxWindow *p_parent);

  // Returning data from dialog; only valid if ShowModal() returns OK.
  bool FiniteLambda(void) const;
  bool GeneratePXIFile(void) const;
  
  double MaxLambda(void) const;
  double StepSize(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DLQRE_H
