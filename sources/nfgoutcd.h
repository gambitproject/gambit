// File: nfgoutcd.h -- outcomes dialog for the NFG
//  $Id$

#ifndef NFGOUTCD_H
#define NFGOUTCD_H
class NfgOutcomeDialogC;
class NfgOutcomeDialog
{
protected:
	NfgOutcomeDialogC *d;
	NfgShow *ns;
public:
	NfgOutcomeDialog(Nfg &nf,NfgShow *ns);
	~NfgOutcomeDialog();
	void SetOutcome(const gString &outc_name);
   void UpdateVals(void);
	void OnOk(void);
};

#define SHORT_ENTRY_OUTCOMES	0
#define LONG_ENTRY_OUTCOMES		1
#endif

