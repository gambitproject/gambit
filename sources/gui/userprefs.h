//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class for global GUI user preferences.
// (Formerly gambdraw.h, plus extensions and additions for 0.97)
//

#ifndef USERPREFS_H
#define USERPREFS_H

#include "base/base.h"

class UserPreferences {
protected:
  wxColour m_terminalColor, m_chanceColor;
  wxColour m_playerColors[8];

  bool m_warnSolveImperfect, m_warnSupportDelete, m_warnProfileInvalid;

public:
  UserPreferences(void);
  virtual ~UserPreferences();

  void SetChanceColor(const wxColour &p_color) { m_chanceColor = p_color; }
  const wxColour &GetChanceColor(void) const { return m_chanceColor; }

  void SetTerminalColor(const wxColour &p_color) { m_terminalColor = p_color; }
  const wxColour &GetTerminalColor(void) const { return m_terminalColor; }

  void SetPlayerColor(int pl, const wxColour &p_color);
  const wxColour &GetPlayerColor(int pl) const;

  void SetWarnOnSolveImperfectRecall(bool p_warn)
    { m_warnSolveImperfect = p_warn; }
  bool WarnOnSolveImperfectRecall(void) const { return m_warnSolveImperfect; }

  void SetWarnOnSupportDelete(bool p_warn) { m_warnSupportDelete = p_warn; }
  bool WarnOnSupportDelete(void) const { return m_warnSupportDelete; }

  void SetWarnOnProfileInvalid(bool p_warn) { m_warnProfileInvalid = p_warn; }
  bool WarnOnProfileInvalid(void) const { return m_warnProfileInvalid; }

  void LoadOptions(void);
  void SaveOptions(void) const;

  void EditOptions(wxWindow *p_parent);
};

#endif  // USERPREFS_H

