//
// FILE: simpprm.h -- Parameter selection for SimpdivSolve
//
// $Id$
//

#ifndef SIMPPRM_H
#define SIMPPRM_H

#include "algdlgs.h"

class SimpdivParamsSettings : public virtual OutputParamsSettings {
protected:
  int nRestarts, leashLength;
  void SaveDefaults(void);

public:
  SimpdivParamsSettings(void);
  ~SimpdivParamsSettings() { SaveDefaults(); }
  void GetParams(SimpdivParams &);
};

class SimpdivSolveParamsDialog : public OutputParamsDialog,
				 public SimpdivParamsSettings {
public:
  SimpdivSolveParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false);
};

#endif  // SIMPPRM_H
