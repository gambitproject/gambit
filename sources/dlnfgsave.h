//
// FILE dlnfgsave.h -- Dialog for saving .nfg files
//
// $Id$
//

#ifndef DLNFGSAVE_H
#define DLNFGSAVE_H

class dialogNfgSave : public guiAutoDialog {
private:
  wxText *m_fileName, *m_treeLabel;
  wxSlider *m_numDecimals;
  
  static void CallbackBrowse(wxButton &p_object, wxEvent &)
    { ((dialogNfgSave *) p_object.GetClientData())->OnBrowse(); }

  void OnBrowse(void);

public:
  dialogNfgSave(const gText &, const gText &, int, wxWindow *);
  virtual ~dialogNfgSave() { }

  gText Filename(void) const { return m_fileName->GetValue(); }
  gText Label(void) const { return m_treeLabel->GetValue(); }
  int NumDecimals(void) const { return m_numDecimals->GetValue(); }
};

#endif   // DLNFGSAVE_H
