//
// FILE: nfgdraw.h -- top level display options for the normal form
//
// $Id$
//

#ifndef NFGDRAW_H
#define NFGDRAW_H

#include "gambdraw.h"

class NormalDrawSettings : public GambitDrawSettings {
private:
  int m_decimals, m_outcomeValues;
  wxFont m_dataFont;

public:
  NormalDrawSettings(void);
  virtual ~NormalDrawSettings();

  void SetDecimals(int p_decimals) { m_decimals = p_decimals; }
  int GetDecimals(void) const { return m_decimals; }
  void SetOutcomeValues(int p_outcomeValues) 
    { m_outcomeValues = p_outcomeValues; }
  int OutcomeValues(void) const { return m_outcomeValues; }

  void SetDataFont(const wxFont &p_font) { m_dataFont = p_font; }
  const wxFont &GetDataFont(void) const { return m_dataFont; }

  void LoadSettings(void);
  void SaveSettings(void) const;
};

#endif  // NFGDRAW_H
