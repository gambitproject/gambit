//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to select from standard algorithms for normal form games
//

#ifndef DLNFGSTANDARD_H
#define DLNFGSTANDARD_H

class dialogNfgSolveStandard : public wxDialog {
private:
  wxRadioBox *m_equilibriumType, *m_precision;
  wxTextCtrl *m_algorithm, *m_details;
  const Nfg &m_nfg;

  void UpdateFields(void);

  // Event handler
  void OnChanged(wxCommandEvent &);

public:
  dialogNfgSolveStandard(wxWindow *p_parent, const Nfg &p_nfg);
  virtual ~dialogNfgSolveStandard();

  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }

  DECLARE_EVENT_TABLE()
};

#endif // DLNFGSTANDARD_H
