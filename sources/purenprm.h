//
// FILE: purenprm.h -- Parameter selection for EnumPureSolve
//
// $Id$
//

#ifndef PURENPRM_H
#define PURENPRM_H

#include "algdlgs.h"

class PureNashParamsSettings : public virtual OutputParamsSettings {
protected:
  void SaveDefaults(void) { }
public:
  PureNashParamsSettings(void) { };
  ~PureNashParamsSettings() { SaveDefaults(); } 
  void GetParams(int &p_stopAfter) { p_stopAfter = StopAfter(); }
};

class PureNashSolveParamsDialog : public OutputParamsDialog,
				  public PureNashParamsSettings {
public:
  PureNashSolveParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false);
};

#endif  // PURENPRM_H
