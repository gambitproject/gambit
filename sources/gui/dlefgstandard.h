//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to select from standard algorithms for extensive form games
//

#ifndef DLEFGSTANDARD_H
#define DLEFGSTANDARD_H

class dialogEfgSolveStandard : public wxDialog {
private:
  wxRadioBox *m_equilibriumType, *m_precision;
  wxTextCtrl *m_algorithm, *m_details;
  const Efg::Game &m_efg;

  void UpdateFields(void);

  // Event handlers
  void OnChanged(wxCommandEvent &);

public:
  dialogEfgSolveStandard(wxWindow *p_parent, const Efg::Game &p_efg);
  virtual ~dialogEfgSolveStandard();

  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }

  DECLARE_EVENT_TABLE()
};

#endif   // DLEFGSTANDARD_H
