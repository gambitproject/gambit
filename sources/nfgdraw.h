//
// FILE: nfgdraw.h -- top level display options for the normal form
//
// $Id$
//

#ifndef NFGDRAW_H
#define NFGDRAW_H

#include "gambdraw.h"

#define OUTCOME_NAMES		1
#define	OUTCOME_VALUES	0

class NormalDrawSettings : public GambitDrawSettings {
private:
  int output_precision;
  int outcome_disp;

public:
  NormalDrawSettings(void);
  virtual ~NormalDrawSettings();

  void SetOutputPrecision(int p) {output_precision=p;}
  int  OutputPrecision(void) const {return output_precision;}
  void SetOutcomeDisp(int d) {outcome_disp=d;}
  int	 OutcomeDisp(void) const {return outcome_disp;}

  void SaveSettings(void) const;
};

#endif  // NFGDRAW_H
