//
// FILE: peprm.h -- Parameter selection for PolEnumSolve
//
// 
//

#ifndef PEPRM_H
#define PEPRM_H

#include "algdlgs.h"

class PolEnumParams;

class guiPolEnumParamsSettings : public virtual OutputParamsSettings {
protected:
  void SaveDefaults(void);

public:
  guiPolEnumParamsSettings(void);
  ~guiPolEnumParamsSettings() { SaveDefaults(); }
  void GetParams(PolEnumParams &);
};

class guiPolEnumParamsDialog : public OutputParamsDialog,
			       public guiPolEnumParamsSettings {
public:
  guiPolEnumParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false);
};

#endif  // PEPRM_H

