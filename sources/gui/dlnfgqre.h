//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for computing quantal response equilibria
//

#ifndef DLNFGQRE_H
#define DLNFGQRE_H

class dialogNfgQre : public wxDialog {
private:
  wxRadioBox *m_solveUsing;
  wxTextCtrl *m_startLambda, *m_stopLambda, *m_stepLambda;
  wxTextCtrl *m_del1, *m_tol1, *m_del2, *m_tol2;

  // Event handlers
  void OnSolveUsing(wxCommandEvent &);

public:
  // Lifecycle
  dialogNfgQre(wxWindow *, const NFSupport &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  bool UseGridSearch(void) const
    { return (m_solveUsing->GetSelection() == 1); }

  double StartLambda(void) const
    { return ToNumber(m_startLambda->GetValue().c_str()); }
  double StopLambda(void) const
    { return ToNumber(m_stopLambda->GetValue().c_str()); }
  double StepLambda(void) const
    { return ToNumber(m_stepLambda->GetValue().c_str()); }

  double Del1(void) const
    { return ToNumber(m_del1->GetValue().c_str()); }
  double Tol1(void) const
    { return ToNumber(m_tol1->GetValue().c_str()); }
  double Del2(void) const
    { return ToNumber(m_del2->GetValue().c_str()); }
  double Tol2(void) const
    { return ToNumber(m_tol2->GetValue().c_str()); }
  

  DECLARE_EVENT_TABLE()
};


#endif  // DLNFGQRE_H
