//
// FILE: dlnfgnewsupport.h -- Dialog for creating NFSupports
//
//
//

#ifndef DLNFGNEWSUPPORT_H
#define DLNFGNEWSUPPORT_H

class dialogNfgNewSupport : public wxDialogBox {
private:
  const Nfg &m_nfg;
  int m_completed;
  wxListBox **m_strategyLists;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogNfgNewSupport *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogNfgNewSupport *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  dialogNfgNewSupport(const Nfg &, wxWindow *);
  virtual ~dialogNfgNewSupport();

  int Completed(void) const { return m_completed; }
  NFSupport *CreateSupport(void) const;
};

#endif  // DLNFGNEWSUPPORT_H
