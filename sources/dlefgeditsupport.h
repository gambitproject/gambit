//
// FILE: dlefgeditsupport.h -- Dialog for editing
//
// $Id$
//

#ifndef DLEFGEDITSUPPORT_H
#define DLEFGEDITSUPPORT_H

class dialogEfgEditSupport : public guiAutoDialog {
private:
  const Efg::Game &m_efg;
  EFSupport m_support;
  wxTextCtrl *m_nameItem;
  wxListBox *m_playerItem, *m_infosetItem, *m_actionItem;

  const char *HelpString(void) const
    { return "Creating and Editing Supports"; }

  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);
  void OnAction(wxCommandEvent &);

public:
  dialogEfgEditSupport(const EFSupport &, wxWindow *);
  virtual ~dialogEfgEditSupport() { }

  const EFSupport &Support(void) const { return m_support; }
  gText Name(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DLEFGEDITSUPPORT_H
