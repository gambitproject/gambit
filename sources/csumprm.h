// $Id$
#ifndef ZSUMPRM_H
#define ZSUMPRM_H

#include "algdlgs.h"

class LPSolveParamsDialog : public OutputParamsDialog
{
private:
	int stopAfter;
	void SaveDefaults(void);
public:
	LPSolveParamsDialog(wxWindow *parent=0);
	~LPSolveParamsDialog(void);
	void GetParams(ZSumParams &P);
};

#ifdef CSUM_PRM_INST   // instantiate only once
LPSolveParamsDialog::LPSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("LP Params",parent,LP_HELP)

{
stopAfter=0;
wxGetResource(PARAMS_SECTION,"LP-StopAfter",&stopAfter,defaults_file);

Form()->Add(wxMakeFormShort("# Equilibria",&stopAfter));
Form()->Add(wxMakeFormNewLine());

// Now add the basic stuff
MakeOutputFields();
Go();
}

void LPSolveParamsDialog::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"LP-StopAfter",stopAfter,defaults_file);
}

LPSolveParamsDialog::~LPSolveParamsDialog(void)
{SaveDefaults();}

void LPSolveParamsDialog::GetParams(ZSumParams &P)
{
P.stopAfter=stopAfter;
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}
#endif

#endif
