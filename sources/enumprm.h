//
// FILE: enumprm.h -- Dialog boxes for parameters to EnumSolve
//
// $Id$
//

#ifndef ENUMPRM_H
#define ENUMPRM_H

#include "algdlgs.h"
#include "enum.h"

class EnumParamsSettings : public virtual OutputParamsSettings {
protected:
  void SaveDefaults(void);

public:
  EnumParamsSettings(void);
  ~EnumParamsSettings() { SaveDefaults(); }
  void GetParams(EnumParams &);
};

class EnumSolveParamsDialog : public OutputParamsDialog,
			      public EnumParamsSettings {
public:
  EnumSolveParamsDialog(wxWindow *parent = 0, bool subgames = false,
			bool vianfg = false);
};

//
// Customized progress indicator for wxEnumStatus
//
class wxEnumStatus : public wxStatus {
private:
  int pass;

public:
  wxEnumStatus(wxFrame *p_parent);
  void SetProgress(double p_value);
};

#endif  // ENUMPRM_H

