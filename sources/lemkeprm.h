//
// FILE: lemkeprm.h -- Parameter selection for LcpSolve[NFG]
//
// $Id$
//

#ifndef LEMKEPRM_H
#define LEMKEPRM_H

#include "algdlgs.h"

class LemkeParamsSettings : public virtual OutputParamsSettings {
protected:
  void SaveDefaults(void);

public:
  LemkeParamsSettings(void);
  ~LemkeParamsSettings() { SaveDefaults(); }
  void GetParams(LemkeParams &);
};

class LemkeSolveParamsDialog : public OutputParamsDialog,
			       public LemkeParamsSettings {
public:
  LemkeSolveParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false,
			 bool p_vianfg = false);
};

#endif  // LEMKEPRM_H
