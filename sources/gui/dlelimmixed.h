//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for parameters to eliminate dominated strategies
//

#ifndef DLELIMMIXED_H
#define DLELIMMIXED_H

class dialogElimMixed : public wxDialog {
private:
  int m_numPlayers;
  wxCheckBox **m_players;
  wxRadioBox *m_domTypeBox, *m_domMethodBox, *m_domPrecisionBox;
  wxRadioBox *m_domDepthBox;

  // Event handlers
  void OnMethod(wxCommandEvent &);
  
public:
  dialogElimMixed(wxWindow *, const gArray<gText> &p_players);
  virtual ~dialogElimMixed();

  gArray<int> Players(void) const;

  bool Iterative(void) const { return (m_domDepthBox->GetSelection() == 1); }
  bool DomStrong(void) const { return (m_domTypeBox->GetSelection() == 1); }
  bool DomMixed(void) const { return (m_domMethodBox->GetSelection() == 1); }
  gPrecision Precision(void) const
    { return (m_domPrecisionBox->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }

  DECLARE_EVENT_TABLE()
};

#endif   // DLELIMMIXED_H

