//
// FILE: dialogauto.h -- Generic autolayout dialog
//
// $Id$
//

#ifndef DIALOGAUTO_H
#define DIALOGAUTO_H

//
// A generic standard dialog box featuring automatic layout, frame control
// handling, and standard button placement
//
class guiAutoDialog : public wxDialog {
private:

protected:
  wxButton *m_okButton, *m_cancelButton, *m_helpButton;

  void AutoSize(void);

public:
  guiAutoDialog(wxWindow *p_parent, char *p_title);
  virtual ~guiAutoDialog() { }

};

#endif  // DIALOGAUTO_H
