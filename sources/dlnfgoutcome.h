//
// FILE: dlnfgoutcome.h -- Outcome selection dialog
//
//
//

#ifndef DLNFGOUTCOME_H
#define DLNFGOUTCOME_H

class dialogNfgOutcomeSelect : public wxDialogBox {
private:
  Nfg &m_nfg;
  int m_outcomeSelected, m_completed;
  wxListBox *m_outcomeList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogNfgOutcomeSelect *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogNfgOutcomeSelect *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  dialogNfgOutcomeSelect(Nfg &, wxWindow * = 0);
  virtual ~dialogNfgOutcomeSelect() { }

  int Completed(void) const { return m_completed; }
  NFOutcome *GetOutcome(void); 
};

#endif  // DLNFGOUTCOME_H
