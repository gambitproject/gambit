//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for parameters to eliminate dominated actions
//

#ifndef DLELIMBEHAV_H
#define DLELIMBEHAV_H

class dialogElimBehav : public wxDialog {
private:
  int m_numPlayers;
  wxCheckBox **m_players;
  wxRadioBox *m_domTypeBox, *m_domConditionalBox, *m_domDepthBox;

public:
  dialogElimBehav(wxWindow *, const gArray<gText> &p_players);
  virtual ~dialogElimBehav();

  gArray<int> Players(void) const;

  bool Iterative(void) const { return (m_domDepthBox->GetSelection() == 1); }
  bool DomStrong(void) const { return (m_domTypeBox->GetSelection() == 1); }
  bool DomConditional(void) const 
    { return (m_domConditionalBox->GetSelection() == 0); }
};

#endif  // DLELIMBEHAV_H
