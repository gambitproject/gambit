// File: efgoutcd.h -- declaration of the EFG outcome editing dialog
// $Id$
#ifndef EFGOUTCD_H
#define EFGOUTCD_H
class EfgOutcomeDialogC;
class EfgOutcomeDialog
{
protected:
	EfgOutcomeDialogC *d;
	TreeWindow *tw;
public:
	EfgOutcomeDialog(Efg &ef,TreeWindow *tw);
	~EfgOutcomeDialog();
	void SetOutcome(const gString &outc_name);
	void OnOk(void);
};

#define SHORT_ENTRY_OUTCOMES	0
#define LONG_ENTRY_OUTCOMES	1
#endif
