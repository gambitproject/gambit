//
// FILE: dialogenumpure.h -- EnumPureSolve interface
//
// $Id$
//

#ifndef DIALOGENUMPURE_H
#define DIALOGENUMPURE_H

#include "dialogalgorithm.h"

class dialogEnumPure : public dialogAlgorithm {
private:
  void AlgorithmFields(void);

  gText HelpTopic(void) const { return "EnumPure"; }

public:
  dialogEnumPure(wxWindow *p_parent, bool p_subgames, bool p_vianfg);
  virtual ~dialogEnumPure();

  int StopAfter(void) const;
};

#endif  // DIALOGENUMPURE_H


