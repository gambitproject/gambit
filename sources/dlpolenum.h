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
  gText HelpTopic(void) const { return "PolEnum"; }
  void AlgorithmFields(void);

public:
  dialogPolEnum(wxWindow *p_parent = 0, bool p_subgames = false,
		bool p_vianfg = false);
  virtual ~dialogPolEnum();

  int StopAfter(void) const;
};

#endif  // DLPOLENUM_H

