// File: outcomed.h -- declaration of the EFG outcome editing dialog
// $Id$
#ifndef OUTCOMED_H
#define OUTCOMED_H
class BaseOutcomeDialogC;
class BaseOutcomeDialog
{
protected:
	BaseOutcomeDialogC *d;
  BaseTreeWindow *tw;
public:
	BaseOutcomeDialog(BaseTreeWindow *tw);
	~BaseOutcomeDialog();
	void SetOutcome(const gString &outc_name);
	void OnOk(void);
};

template <class T>
class OutcomeDialog: public BaseOutcomeDialog
{
public:
	OutcomeDialog(Efg<T> &ef,BaseTreeWindow *tw);
};

#define SHORT_ENTRY_OUTCOMES	0
#define LONG_ENTRY_OUTCOMES		1
#endif
