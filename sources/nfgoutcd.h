// File: nfgoutcd.h -- outcomes dialog for the NFG
// $Id$

#ifndef NFGOUTCD_H
#define NFGOUTCD_H
class BaseNFOutcomeDialogC;
class BaseNFOutcomeDialog
{
protected:
	BaseNFOutcomeDialogC *d;
	BaseNormShow *bns;
public:
	BaseNFOutcomeDialog(BaseNormShow *bns);
	~BaseNFOutcomeDialog();
	void SetOutcome(const gString &outc_name);
	void OnOk(void);
};

template <class T>
class NFOutcomeDialog: public BaseNFOutcomeDialog
{
public:
	NFOutcomeDialog(Nfg<T> &nf,BaseNormShow *bns);
};

#define SHORT_ENTRY_OUTCOMES	0
#define LONG_ENTRY_OUTCOMES		1
#endif

