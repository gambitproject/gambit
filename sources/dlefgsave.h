//
// FILE: dlefgsave.h -- Dialog for saving .efg files
//
// $Id$
// 

#ifndef DLEFGSAVE_H
#define DLEFGSAVE_H

class dialogEfgSave : public guiAutoDialog {
private:
  wxText *m_fileName, *m_treeLabel;
  wxSlider *m_numDecimals;

  static void CallbackBrowse(wxButton &p_object, wxEvent &)
    { ((dialogEfgSave *) p_object.GetClientData())->OnBrowse(); }

  void OnBrowse(void);

public:
  dialogEfgSave(const gText &, const gText &, int, wxWindow *);
  virtual ~dialogEfgSave() { }

  gText Filename(void) const { return m_fileName->GetValue(); }
  gText Label(void) const { return m_treeLabel->GetValue(); }
  int NumDecimals(void) const { return m_numDecimals->GetValue(); }
};

#endif  // DLEFGSAVE_H
