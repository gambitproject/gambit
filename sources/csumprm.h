// $Id$
#ifndef ZSUMPRM_H
#define ZSUMPRM_H

#include "algdlgs.h"

class LPParamsSettings: public OutputParamsSettings
{
protected:
	int stopAfter;
	void SaveDefaults(void);
public:
	LPParamsSettings(void);
	~LPParamsSettings() {SaveDefaults();}
	void GetParams(ZSumParams &P);
};
class LPSolveParamsDialog : public OutputParamsDialog, public LPParamsSettings
{
public:
	LPSolveParamsDialog(wxWindow *parent=0);
};

#ifdef CSUM_PRM_INST   // instantiate only once
LPParamsSettings::LPParamsSettings(void)
{
wxGetResource(PARAMS_SECTION,"LP-StopAfter",&stopAfter,defaults_file);
}

void LPParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"LP-StopAfter",stopAfter,defaults_file);
}

void LPParamsSettings::GetParams(ZSumParams &P)
{
P.stopAfter=stopAfter;
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

LPSolveParamsDialog::LPSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("LP Params",parent,LP_HELP)

{
Add(wxMakeFormShort("# Equilibria",&stopAfter));
Add(wxMakeFormNewLine());

// Now add the basic stuff
MakeOutputFields();
Go();
}

#endif

#endif
