//
// FILE: dlpolenum.h -- Parameter selection for PolEnumSolve
//
// $Id$
//

#ifndef DLPOLENUM_H
#define DLPOLENUM_H

#include "algdlgs.h"

class dialogPolEnum : public dialogAlgorithm {
private:
  const char *HelpString(void) const { return "PolEnum"; }

  void AlgorithmFields(void);

public:
  dialogPolEnum(wxWindow *p_parent = 0, bool p_subgames = false,
		bool p_vianfg = false);
  virtual ~dialogPolEnum();

  int StopAfter(void) const;
};

#include "wxstatus.h"

class wxPolEnumStatus : public wxStatus {
private:
  int pass;

public:
  wxPolEnumStatus(wxFrame *p_parent);
  void SetProgress(double p_value);
};

#endif  // DLPOLENUM_H

