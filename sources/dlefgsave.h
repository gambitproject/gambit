//
// FILE: dlefgsave.h -- Dialog for saving .efg files
//
// $Id$
// 

#ifndef DLEFGSAVE_H
#define DLEFGSAVE_H

class dialogEfgSave : public guiAutoDialog {
private:
  wxTextCtrl *m_fileName, *m_treeLabel;
  wxSlider *m_numDecimals;

  void OnBrowse(wxCommandEvent &);

public:
  dialogEfgSave(const gText &, const gText &, int, wxWindow *);
  virtual ~dialogEfgSave() { }

  gText Filename(void) const;
  gText Label(void) const;
  int NumDecimals(void) const { return m_numDecimals->GetValue(); }

  DECLARE_EVENT_TABLE()
};

#endif  // DLEFGSAVE_H
