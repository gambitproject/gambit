//
// FILE: dlefgoutcome.h -- Outcome selection dialog
//
//
//

#ifndef DLEFGOUTCOME_H
#define DLEFGOUTCOME_H

class dialogEfgOutcomeSelect : public wxDialogBox {
private:
  Efg &m_efg;
  int m_outcomeSelected, m_completed;
  wxListBox *m_outcomeList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogEfgOutcomeSelect *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogEfgOutcomeSelect *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  dialogEfgOutcomeSelect(Efg &, wxWindow * = 0);
  virtual ~dialogEfgOutcomeSelect() { }

  int Completed(void) const { return m_completed; }
  EFOutcome *GetOutcome(void); 
};

#endif  // DLEFGOUTCOME_H
