//
// FILE: csumprm.h -- Parameter selection for LpSolve
//
// $Id$
//

#ifndef CSUMPRM_H
#define CSUMPRM_H

#include "algdlgs.h"

class CSSeqFormParams;
class ZSumParams;

class LPParamsSettings : public virtual OutputParamsSettings {
protected:
  void SaveDefaults(void);

public:
  LPParamsSettings(void);
  ~LPParamsSettings() { SaveDefaults(); }
  void GetParams(ZSumParams &);
  void GetParams(CSSeqFormParams &);
};

class LPSolveParamsDialog : public OutputParamsDialog,
			    public LPParamsSettings {
public:
  LPSolveParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false,
		      bool p_vianfg = false);
};

#endif  // CSUMPRM_H

