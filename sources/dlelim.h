//
// FILE: dlelim.h -- Parameters for dominance elimination
//
// $Id$
//

#ifndef DLELIM_H
#define DLELIM_H

class dialogElimMixed : public guiAutoDialog {
private:
  wxListBox *m_playerBox;
  wxCheckBox *m_allBox;
  wxRadioBox *m_domTypeBox, *m_domMethodBox, *m_domPrecisionBox;

  const char *HelpString(void) const { return "Normal form Supports"; }

  // Event handlers
  void OnOK(wxCommandEvent &);
  void OnMethod(wxCommandEvent &);
  
public:
  dialogElimMixed(wxWindow *, const gArray<gText> &p_players);
  virtual ~dialogElimMixed();

  gArray<int> Players(void) const;

  bool Iterative(void) const { return m_allBox->GetValue(); }
  bool DomStrong(void) const { return (m_domTypeBox->GetSelection() == 1); }
  bool DomMixed(void) const { return (m_domMethodBox->GetSelection() == 1); }
  gPrecision Precision(void) const
    { return (m_domPrecisionBox->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }

  DECLARE_EVENT_TABLE()
};

class dialogElimBehav : public guiAutoDialog {
private:
  wxListBox *m_playerBox;
  wxCheckBox *m_allBox;
  wxRadioBox *m_domTypeBox, *m_domConditionalBox;

  const char *HelpString(void) const { return "Extensive form Supports"; }
  
  // Event handlers
  void OnOK(wxCommandEvent &);

public:
  dialogElimBehav(wxWindow *, const gArray<gText> &p_players);
  virtual ~dialogElimBehav();

  gArray<int> Players(void) const;

  bool Iterative(void) const { return m_allBox->GetValue(); }
  bool DomStrong(void) const { return (m_domTypeBox->GetSelection() == 1); }
  bool DomConditional(void) const 
    { return (m_domConditionalBox->GetSelection() == 0); }

  DECLARE_EVENT_TABLE()
};

#endif   // DLELIM_H

