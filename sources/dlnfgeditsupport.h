//
// FILE: dlnfgeditsupport.h -- Dialog for editing
//
// $Id$
//

#ifndef DLNFGEDITSUPPORT_H
#define DLNFGEDITSUPPORT_H

class dialogNfgEditSupport : public guiAutoDialog {
private:
  const Nfg &m_nfg;
  NFSupport m_support;
  wxListBox *m_playerItem, *m_strategyItem;

  static void CallbackPlayer(wxListBox &, wxCommandEvent &);
  static void CallbackStrategy(wxListBox &, wxCommandEvent &);

  const char *HelpString(void) const { return ""; }

  void OnPlayer(int);
  void OnStrategy(int);

public:
  dialogNfgEditSupport(const NFSupport &, wxWindow *);
  virtual ~dialogNfgEditSupport() { }

  const NFSupport &Support(void) const { return m_support; }
};

#endif  // DLNFGEDITSUPPORT_H
