//
// FILE: dlelim.h -- Parameters for dominance elimination
//
// $Id$
//

#ifndef DLELIM_H
#define DLELIM_H

typedef enum {
  elimWEAK = 0, elimSTRONG = 1
} elimStrengthType;

typedef enum {
  elimPURE = 0, elimMIXED = 1
} elimMethodType;

class dialogElim : public guiAutoDialog {
private:
  wxListBox *m_playerBox;
  wxCheckBox *m_allBox, *m_compressBox;
  wxRadioBox *m_domTypeBox, *m_domMethodBox, *m_domPrecisionBox;
  bool m_mixed;

  const char *HelpString(void) const { return ""; }
  
public:
  dialogElim(const gArray<gText> &p_players, bool p_mixed,
	     wxWindow *p_parent = NULL);
  virtual ~dialogElim();

  gArray<int> Players(void) const;
  bool Compress(void) const { return m_compressBox->GetValue(); }

  bool FindAll(void) const { return m_allBox->GetValue(); }
  bool DomStrong(void) const { return (m_domTypeBox->GetSelection() == 0); }
  bool DomMixed(void) const { return (m_domMethodBox->GetSelection() == 1); }
  gPrecision Precision(void) const
    { return (m_domPrecisionBox->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif   // DLELIM_H

