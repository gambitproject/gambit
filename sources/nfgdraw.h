// File: nfgdraw.h -- top level display options for the NF
// $Id$

#include "gambdraw.h"
#define OUTCOME_NAMES		1
#define	OUTCOME_VALUES	0
class NormalDrawSettings:public GambitDrawSettings
{
private:
	int output_precision;
	int outcome_disp;
public:
	void SetOutputPrecision(int p) {output_precision=p;}
	int  OutputPrecision(void) const {return output_precision;}
  void OutcomeOptions(void);
	void SetOutcomeDisp(int d) {outcome_disp=d;}
	int	 OutcomeDisp(void) const {return outcome_disp;}
	NormalDrawSettings(void);
};
