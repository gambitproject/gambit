//
// FILE: seqfprm.h -- definition of params dialog for LCPSolve[EFG]
//
// $Id$
//

#ifndef SEQFPRM_H
#define SEQFPRM_H

class efgLcpSolveDialog : public wxDialogBox {
private:
  int m_completed;
  wxRadioBox *m_dominanceDepth, *m_dominanceType, *m_precisionChoice,
             *m_stopAfterChoice;
  wxText *m_maxDepth, *m_maxSolutions;
  wxCheckBox *m_markSubgames, *m_selectSolutions;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((efgLcpSolveDialog *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((efgLcpSolveDialog *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  efgLcpSolveDialog(wxWindow *p_parent = 0, bool p_subgames = false);
  virtual ~efgLcpSolveDialog() { }

  int Completed(void) const { return m_completed; }

  int StopAfter(void) const;
  int MaxDepth(void) const
    { return (int) ToDouble(m_maxDepth->GetValue()); }
  gPrecision Precision(void) const;
};

#endif  // SEQFPRM_H
