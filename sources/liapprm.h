//
// FILE: liapprm.h -- definition of the parameter dialog for LiapSolve
//
// $Id$
//

#ifndef LIAPPRM_H
#define LIAPPRM_H
#include "algdlgs.h"

class NFLiapParams;
class EFLiapParams;

class LiapParamsSettings : public virtual OutputParamsSettings {
protected:
  float tol1, tolN;
  int maxits1,maxitsN,nTries;
  bool subgames;
  int  start_option; // 0-default,1-saved,2-query
  void SaveDefaults(void);

public:
  LiapParamsSettings(void);
  ~LiapParamsSettings();
  void GetParams(EFLiapParams &);
  void GetParams(NFLiapParams &);
  int StartOption(void) const { return start_option; }
};

class LiapSolveParamsDialog : public OutputParamsDialog,
			      public LiapParamsSettings {
public:
  LiapSolveParamsDialog(wxWindow *p_parent, bool p_subgames = false);
  //	~LiapSolveParamsDialog(void);
};


#endif
