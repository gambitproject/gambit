//
// FILE: gobitprm.h -- Parameter selection for GobitSolve
//
// $Id$
//

#ifndef GOBITPRM_H
#define GOBITPRM_H

#include "algdlgs.h"

#define DATA_TYPE_LOG 1

class EFGobitParams;
class NFGobitParams;

class GobitParamsSettings : public virtual PxiParamsSettings {
protected:
  float minLam, maxLam, delLam, tol1, tolN;
  int maxits1,maxitsN;
  int  start_option; // 0-default,1-saved,2-query
  void SaveDefaults(void);

public:
  GobitParamsSettings(const char *fn);
  ~GobitParamsSettings();
  void GetParams(EFGobitParams &);
  void GetParams(NFGobitParams &);
  int StartOption(void) const { return start_option; }
};

class GobitSolveParamsDialog : public PxiParamsDialog,
			       public GobitParamsSettings {
public:
  GobitSolveParamsDialog(wxWindow *parent,const gText filename);
  //	~GobitSolveParamsDialog(void);
};

#endif  // GOBITPRM_H
