//
// FILE: dialogpaged.h -- Base class for "paged" dialog
//
// %W% %G%
//

#ifndef DIALOGPAGED_H
#define DIALOGPAGED_H

#include "dialogauto.h"
#include "garray.h"
#include "gtext.h"

/**
 * A "paged" dialog class (i.e., with "back" and "next" buttons)
 */
class guiPagedDialog : public guiAutoDialog {
private:
  void OnNext(wxCommandEvent &);
  void OnBack(wxCommandEvent &);

protected:
  int m_pageNumber, m_numFields;
  static int s_itemsPerPage;
  wxTextCtrl **m_dataFields;
  wxButton *m_backButton, *m_nextButton;
  gArray<gText> m_dataValues;

public:
  guiPagedDialog(wxWindow *p_parent, char *p_title, int p_numItems);
  virtual ~guiPagedDialog();

  const gText &GetValue(int p_index) const { return m_dataValues[p_index]; }
  void SetValue(int p_index, const gText &p_value);

  DECLARE_EVENT_TABLE()
};

#endif  // DIALOGPAGED_H
