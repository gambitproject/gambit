//
// FILE: dlnfgpayoff.h -- Dialog for editing outcome payoffs
//
//
//

#ifndef DLNFGPAYOFF_H
#define DLNFGPAYOFF_H

class dialogNfgPayoffs : public wxDialogBox {
private:
  NFOutcome *m_outcome;
  const Nfg &m_nfg;
  gArray<gNumber> m_payoffs;
  gText m_name;
  int m_completed;
  wxText *m_outcomeName;
  wxText **m_outcomePayoffs;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogNfgPayoffs *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogNfgPayoffs *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  dialogNfgPayoffs(const Nfg &, NFOutcome *, bool, wxWindow *parent);
  virtual ~dialogNfgPayoffs() { }

  int Completed(void) const { return m_completed; }
  const gArray<gNumber> &Payoffs(void) const { return m_payoffs; }
  gText Name(void) const { return m_name; }
};

#endif  // DLNFGPAYOFF_H
