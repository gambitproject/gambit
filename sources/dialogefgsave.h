//
// FILE: dlefgsave.h -- Dialog for saving .efg files
//
// $Id$
// 

#ifndef DIALOGEFGSAVE_H
#define DIALOGEFGSAVE_H

class dialogEfgSave : public guiAutoDialog {
private:
  wxTextCtrl *m_fileName, *m_treeLabel;
  wxSlider *m_numDecimals;

  void OnBrowse(wxCommandEvent &);

public:
  dialogEfgSave(wxWindow *, const gText &, const gText &, int);
  virtual ~dialogEfgSave() { }

  gText Filename(void) const;
  gText Label(void) const;
  int NumDecimals(void) const { return m_numDecimals->GetValue(); }

  DECLARE_EVENT_TABLE()
};

#endif  // DIALOGEFGSAVE_H
