//
// FILE: gridprm.h -- Parameter selection for GobitGridSolve
//
// $Id$
//

#ifndef GRIDPRM_H
#define GRIDPRM_H

#include "algdlgs.h"

class GridParamsSettings : public virtual PxiParamsSettings {
protected:
  float minLam, maxLam, delLam, delp1, tol1, delp2, tol2;
  int multi_grid;
  void SaveDefaults(void);

public:
  GridParamsSettings(const char *fn);
  ~GridParamsSettings();
  void GetParams(GridParams &);
};

class GridSolveParamsDialog : public MyDialogBox, public GridParamsSettings {
public:
  GridSolveParamsDialog(wxWindow *p_parent, const gText &p_filename);
  //  ~GridSolveParamsDialog(void);
};

#endif // GRIDPRM_H
