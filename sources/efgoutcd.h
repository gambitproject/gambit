// File: efgoutcd.h -- declaration of the EFG outcome editing dialog
// $Id$
#ifndef EFGOUTCD_H
#define EFGOUTCD_H
class EfgOutcomeDialogC;
#include "paramsd.h"
class EfgOutcomeDialog
{
protected:
	EfgOutcomeDialogC *d;
	EfgShow *es;
public:
	EfgOutcomeDialog(Efg &ef,EfgShow *es);
	~EfgOutcomeDialog();
	void SetOutcome(const gText &outc_name);
   void UpdateVals(void);
	void OnOk(void);
};

#define SHORT_ENTRY_OUTCOMES	0
#define LONG_ENTRY_OUTCOMES	1
#endif
