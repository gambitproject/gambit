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
  wxTextCtrl *m_nameItem;
  wxListBox *m_playerItem, *m_strategyItem;

  const char *HelpString(void) const
    { return "Creating and Editing Supports"; }

  void OnPlayer(wxCommandEvent &);
  void OnStrategy(wxCommandEvent &);

public:
  dialogNfgEditSupport(const NFSupport &, wxWindow *);
  virtual ~dialogNfgEditSupport() { }

  const NFSupport &Support(void) const { return m_support; }
  gText Name(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DLNFGEDITSUPPORT_H
