//
// FILE dlnfgsave.h -- Dialog for saving .nfg files
//
// $Id$
//

#ifndef DLNFGSAVE_H
#define DLNFGSAVE_H

class dialogNfgSave : public guiAutoDialog {
private:
  wxTextCtrl *m_fileName, *m_treeLabel;
  wxSlider *m_numDecimals;
  
  void OnBrowse(wxCommandEvent &);

public:
  dialogNfgSave(const gText &, const gText &, int, wxWindow *);
  virtual ~dialogNfgSave() { }

  gText Filename(void) const;
  gText Label(void) const;
  int NumDecimals(void) const { return m_numDecimals->GetValue(); }

  DECLARE_EVENT_TABLE()
};

#endif   // DLNFGSAVE_H
