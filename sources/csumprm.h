// $Id$
#ifndef ZSUMPRM_H
#define ZSUMPRM_H

#include "algdlgs.h"

class LPParamsSettings: public virtual OutputParamsSettings
{
protected:
	void SaveDefaults(void);
public:
	LPParamsSettings(void);
	~LPParamsSettings() {SaveDefaults();}
	void GetParams(ZSumParams &P);
};
class LPSolveParamsDialog : public OutputParamsDialog, public LPParamsSettings
{
public:
	LPSolveParamsDialog(wxWindow *parent=0,bool subgames=false);
};

#ifdef CSUM_PRM_INST   // instantiate only once
LPParamsSettings::LPParamsSettings(void)
{}

void LPParamsSettings::SaveDefaults(void)
{}

void LPParamsSettings::GetParams(ZSumParams &P)
{
P.stopAfter=StopAfter();
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

LPSolveParamsDialog::LPSolveParamsDialog(wxWindow *parent,bool subgames)
														:OutputParamsDialog("LP Params",parent,LP_HELP)

{
Add(wxMakeFormNewLine());
// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD|MAXSOLN_FIELD| ((subgames) ? SPS_FIELD : 0));
Go();
}

#endif

#endif
