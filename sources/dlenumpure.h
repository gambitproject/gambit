//
// FILE: dlenumpure.h -- EnumPureSolve interface
//
// $Id$
//

#ifndef DLENUMPURE_H
#define DLENUMPURE_H

#include "algdlgs.h"

class dialogEnumPure : public dialogAlgorithm {
private:
  wxIntegerItem *m_stopAfter;

  void AlgorithmFields(void);

public:
  dialogEnumPure(wxWindow *p_parent = 0, bool p_subgames = false,
		 bool p_vianfg = false);
  virtual ~dialogEnumPure();

  int StopAfter(void) const;
};

#endif  // DLENUMPURE_H


