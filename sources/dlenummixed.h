//
// FILE: dlenummixed.h -- EnumMixedSolve interface
//
// $Id$
//

#ifndef DLENUMMIXED_H
#define DLENUMMIXED_H

#include "algdlgs.h"

class dialogEnumMixed : public dialogAlgorithm {
private:
  wxCheckBox *m_findAll;
  wxIntegerItem *m_stopAfter;
  wxRadioBox *m_precision;

  static void CallbackAll(wxCheckBox &p_object, wxEvent &)
    { ((dialogEnumMixed *) p_object.GetClientData())->OnAll(); } 

  void OnAll(void);
  void AlgorithmFields(void);

public:
  dialogEnumMixed(wxWindow *p_parent = 0, bool p_subgames = false);
  virtual ~dialogEnumMixed();

  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#include "wxstatus.h"

class wxEnumStatus : public wxStatus {
private:
  int pass;

public:
  wxEnumStatus(wxFrame *p_parent);
  void SetProgress(double p_value);
};

#endif  // DLENUMMIXED_H


