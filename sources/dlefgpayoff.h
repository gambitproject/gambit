//
// FILE: dlefgpayoff.h -- Dialog for editing outcome payoffs
//
// $Id$
//

#ifndef DLEFGPAYOFF_H
#define DLEFGPAYOFF_H

class dialogEfgPayoffs : public wxDialogBox {
private:
  EFOutcome *m_outcome;
  const Efg &m_efg;
  gText m_name;
  int m_completed, m_pageNumber;
  static int s_payoffsPerDialog;
  gArray<gNumber> m_payoffs;
  wxText *m_outcomeName;
  wxNumberItem **m_outcomePayoffs;
  wxButton *m_backButton, *m_nextButton, *m_okButton, *m_cancelButton;

  static void CallbackNext(wxButton &p_object, wxEvent &)
    { ((dialogEfgPayoffs *) p_object.GetClientData())->OnNext(); }
  static void CallbackBack(wxButton &p_object, wxEvent &)
    { ((dialogEfgPayoffs *) p_object.GetClientData())->OnBack(); }
  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogEfgPayoffs *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogEfgPayoffs *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

  void OnNext(void);
  void OnBack(void);

public:
  dialogEfgPayoffs(const Efg &, EFOutcome *, bool, wxWindow *parent);
  virtual ~dialogEfgPayoffs() { }

  int Completed(void) const { return m_completed; }
  const gArray<gNumber> &Payoffs(void) const { return m_payoffs; }
  gText Name(void) const { return m_name; }
};

#endif  // DLEFGPAYOFF_H
