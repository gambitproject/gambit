//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Miscellaneous functions extending wxWindows
//

#ifndef WXMISC_H
#define WXMISC_H
#ifndef	wxRUNNING
#define	wxRUNNING 12345
#endif
#include <stdio.h>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP


//
// A generic standard dialog box featuring automatic layout, frame control
// handling, and standard button placement
//
class guiAutoDialog : public wxDialog {
protected:
  wxButton *m_okButton, *m_cancelButton, *m_helpButton;
  wxBoxSizer *m_buttonSizer;

  virtual const char *HelpString(void) const { return ""; }

  void OnHelp(void);

  void Go(void);

public:
  guiAutoDialog(wxWindow *p_parent, char *p_title);
  virtual ~guiAutoDialog();
};

class wxNumberItem : public wxTextCtrl {
private:
  wxString m_data;
  double m_value; 

public:
  wxNumberItem(wxPanel *p_parent, char *p_label, const wxString &p_default,
	       const wxPoint &pos = wxDefaultPosition, 
	       const wxSize &size = wxDefaultSize);
  virtual ~wxNumberItem();

  void SetNumber(const double &p_value);
  double GetNumber(void);
};

#endif // WXMISC_H



