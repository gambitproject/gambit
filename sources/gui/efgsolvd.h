//
// FILE: efgsolvd.h -- Selecting algorithm dialogs
//
// $Id$
//

#ifndef EFGSOLVD_H
#define EFGSOLVD_H

typedef enum {
  efgSTANDARD_NASH = 0, efgSTANDARD_PERFECT = 1, efgSTANDARD_SEQUENTIAL = 2
} efgStandardType;

typedef enum {
  efgSTANDARD_ONE = 0, efgSTANDARD_TWO = 1, efgSTANDARD_ALL = 2
} efgStandardNum;

class dialogEfgSolveStandard : public wxDialog {
private:
  wxRadioBox *m_equilibriumType, *m_precision;
  wxTextCtrl *m_algorithm, *m_details;
  const Efg::Game &m_efg;

  void UpdateFields(void);

  // Button event handler
  void OnChanged(wxCommandEvent &);

public:
  dialogEfgSolveStandard(wxWindow *p_parent, const Efg::Game &p_efg);
  virtual ~dialogEfgSolveStandard();

  efgStandardType Type(void) const;
  efgStandardNum Number(void) const;
  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }

  DECLARE_EVENT_TABLE()
};

#endif   // EFGSOLVD_H
