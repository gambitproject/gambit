//
// FILE: dlefgeditsupport.h -- Dialog for editing
//
// $id$
//

#ifndef DLEFGEDITSUPPORT_H
#define DLEFGEDITSUPPORT_H

class dialogEfgEditSupport : public guiAutoDialog {
private:
  const Efg &m_efg;
  EFSupport m_support;
  wxText *m_nameItem;
  wxListBox *m_playerItem, *m_infosetItem, *m_actionItem;

  static void CallbackPlayer(wxListBox &, wxCommandEvent &);
  static void CallbackInfoset(wxListBox &, wxCommandEvent &);
  static void CallbackAction(wxListBox &, wxCommandEvent &);

  const char *HelpString(void) const { return ""; }

  void OnPlayer(int);
  void OnInfoset(int);
  void OnAction(int);

public:
  dialogEfgEditSupport(const EFSupport &, wxWindow *);
  virtual ~dialogEfgEditSupport() { }

  const EFSupport &Support(void) const { return m_support; }
  gText Name(void) const { return m_nameItem->GetValue(); }
};

#endif  // DLNFGEDITSUPPORT_H
