//
// FILE dlnfgsave.h -- Dialog for saving .nfg files
//
//
//

#ifndef DLNFGSAVE_H
#define DLNFGSAVE_H

class dialogNfgSave : public wxDialogBox {
private:
  int m_completed;

  wxText *m_fileName, *m_treeLabel;
  wxSlider *m_numDecimals;
  
  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogNfgSave *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogNfgSave *) p_object.GetClientData())->OnCancel(); }
  static void CallbackBrowse(wxButton &p_object, wxEvent &)
    { ((dialogNfgSave *) p_object.GetClientData())->OnBrowse(); }

  void OnOK(void);
  void OnCancel(void);
  void OnBrowse(void);
  Bool OnClose(void);

public:
  dialogNfgSave(const gText &, const gText &, int, wxWindow *);
  virtual ~dialogNfgSave() { }

  int Completed(void) const { return m_completed; }
  gText Filename(void) const { return m_fileName->GetValue(); }
  gText Label(void) const { return m_treeLabel->GetValue(); }
  int NumDecimals(void) const { return m_numDecimals->GetValue(); }
};

#endif   // DLNFGSAVE_H
