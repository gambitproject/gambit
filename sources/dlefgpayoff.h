//
// FILE: dlefgpayoff.h -- Dialog for editing outcome payoffs
//
//
//

#ifndef DLEFGPAYOFF_H
#define DLEFGPAYOFF_H

class dialogEfgPayoffs : public wxDialogBox {
private:
  EFOutcome *m_outcome;
  const Efg &m_efg;
  gArray<gNumber> m_payoffs;
  gText m_name;
  int m_completed;
  wxText *m_outcomeName;
  wxText **m_outcomePayoffs;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogEfgPayoffs *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogEfgPayoffs *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  dialogEfgPayoffs(const Efg &, EFOutcome *, bool, wxWindow *parent);
  virtual ~dialogEfgPayoffs() { }

  int Completed(void) const { return m_completed; }
  const gArray<gNumber> &Payoffs(void) const { return m_payoffs; }
  gText Name(void) const { return m_name; }
};

#endif  // DLEFGPAYOFF_H
