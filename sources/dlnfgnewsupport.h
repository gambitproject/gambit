//
// FILE: dlnfgnewsupport.h -- Dialog for creating NFSupports
//
// $Id$
//

#ifndef DLNFGNEWSUPPORT_H
#define DLNFGNEWSUPPORT_H

class dialogNfgNewSupport : public guiAutoDialog {
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
  dialogNfgNewSupport(const Nfg &, wxWindow *);
  virtual ~dialogNfgNewSupport() { }

  NFSupport *CreateSupport(void) const;
};

#endif  // DLNFGNEWSUPPORT_H
