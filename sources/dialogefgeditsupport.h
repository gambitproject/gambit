//
// FILE: dialogefgeditsupport.h -- Dialog for editing supports
//
// $Id$
//

#ifndef DIALOGEFGEDITSUPPORT_H
#define DIALOGEFGEDITSUPPORT_H

class dialogEfgEditSupport : public guiAutoDialog {
private:
  const Efg &m_efg;
  EFSupport m_support;
  wxTextCtrl *m_nameItem;
  wxListBox *m_playerItem, *m_infosetItem, *m_actionItem;

  const char *HelpString(void) const { return ""; }

  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);
  void OnAction(wxCommandEvent &);

public:
  dialogEfgEditSupport(wxWindow *, const EFSupport &);
  virtual ~dialogEfgEditSupport() { }

  const EFSupport &Support(void) const { return m_support; }
  gText Name(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DIALOGEFGEDITSUPPORT_H
