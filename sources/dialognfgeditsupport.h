//
// FILE: dialognfgeditsupport.h -- Dialog for editing supports
//
// $Id$
//

#ifndef DIALOGNFGEDITSUPPORT_H
#define DIALOGNFGEDITSUPPORT_H

class dialogNfgEditSupport : public guiAutoDialog {
private:
  const Nfg &m_nfg;
  NFSupport m_support;
  wxTextCtrl *m_nameItem;
  wxListBox *m_playerItem, *m_strategyItem;

  const char *HelpString(void) const { return ""; }

  void OnPlayer(wxCommandEvent &);
  void OnStrategy(wxCommandEvent &);

public:
  dialogNfgEditSupport(wxWindow *, const NFSupport &);
  virtual ~dialogNfgEditSupport() { }

  const NFSupport &Support(void) const { return m_support; }
  gText Name(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DIALOGNFGEDITSUPPORT_H
