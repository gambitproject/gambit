//
// FILE: dlnfgpayoff.h -- Dialog for editing outcome payoffs
//
// $Id$
//

#ifndef DLNFGPAYOFF_H
#define DLNFGPAYOFF_H

class dialogNfgPayoffs : public wxDialogBox {
private:
  NFOutcome *m_outcome;
  const Nfg &m_nfg;
  gText m_name;
  int m_completed, m_pageNumber;
  static int s_payoffsPerDialog;
  gArray<gNumber> m_payoffs;
  wxText *m_outcomeName;
  wxNumberItem **m_outcomePayoffs;
  wxButton *m_backButton, *m_nextButton, *m_okButton, *m_cancelButton;

  static void CallbackNext(wxButton &p_object, wxEvent &)
    { ((dialogNfgPayoffs *) p_object.GetClientData())->OnNext(); }
  static void CallbackBack(wxButton &p_object, wxEvent &)
    { ((dialogNfgPayoffs *) p_object.GetClientData())->OnBack(); }
  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogNfgPayoffs *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogNfgPayoffs *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

  void OnNext(void);
  void OnBack(void);

public:
  dialogNfgPayoffs(const Nfg &, NFOutcome *, bool, wxWindow *parent);
  virtual ~dialogNfgPayoffs() { }

  int Completed(void) const { return m_completed; }
  const gArray<gNumber> &Payoffs(void) const { return m_payoffs; }
  gText Name(void) const { return m_name; }
};

#endif  // DLNFGPAYOFF_H
