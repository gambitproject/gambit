//
// FILE: nfgdraw.h -- top level display options for the normal form
//
// $Id$
//

#ifndef NFGDRAW_H
#define NFGDRAW_H

#include "gambdraw.h"

#define OUTCOME_NAMES		1
#define	OUTCOME_VALUES	0

class NormalDrawSettings : public GambitDrawSettings {
private:
  int m_decimals;
  int outcome_disp;

  wxFont *m_dataFont;

public:
  NormalDrawSettings(void);
  virtual ~NormalDrawSettings();

  void SetDecimals(int p_decimals) { m_decimals = p_decimals; }
  int GetDecimals(void) const { return m_decimals; }
  void SetOutcomeDisp(int d) {outcome_disp=d;}
  int	 OutcomeDisp(void) const {return outcome_disp;}

  void SetDataFont(wxFont *p_font) { m_dataFont = p_font; }
  wxFont *GetDataFont(void) const { return m_dataFont; }

  void SaveSettings(void) const;
};

#endif  // NFGDRAW_H
