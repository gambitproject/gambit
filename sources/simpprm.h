#ifndef SIMPPRM_H
#define SIMPPRM_H

#include "algdlgs.h"

class SimpdivSolveParamsDialog : public OutputParamsDialog
{
private:
	int stopAfter, nRestarts, leashLength;
	void SaveDefaults(void);
public:
	SimpdivSolveParamsDialog(wxWindow *parent=0);
	~SimpdivSolveParamsDialog(void);
	void GetParams(SimpdivParams &P);
};


SimpdivSolveParamsDialog::SimpdivSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("SimpDiv Params",parent,SIMPDIV_HELP)

{
stopAfter=1;nRestarts=10;leashLength=0;
wxGetResource(PARAMS_SECTION,"Simpdiv-stopAfter",&stopAfter,defaults_file);
wxGetResource(PARAMS_SECTION,"Simpdiv-nRestarts",&nRestarts,defaults_file);
wxGetResource(PARAMS_SECTION,"Simpdiv-leashLength",&leashLength,defaults_file);

Form()->Add(wxMakeFormShort("# Equilibria",&stopAfter));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("# Restarts",&nRestarts));
Form()->Add(wxMakeFormShort("Leash",&leashLength));

// Now add the basic stuff
MakeOutputFields();
Go();
}

void SimpdivSolveParamsDialog::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Simpdiv-stopAfter",stopAfter,defaults_file);
wxWriteResource(PARAMS_SECTION,"Simpdiv-nRestarts",nRestarts,defaults_file);
wxWriteResource(PARAMS_SECTION,"Simpdiv-leashLength",leashLength,defaults_file);
}
SimpdivSolveParamsDialog::~SimpdivSolveParamsDialog(void)
{SaveDefaults();}

void SimpdivSolveParamsDialog::GetParams(SimpdivParams &P)
{
P.nRestarts=nRestarts;P.leashLength=leashLength;P.stopAfter=stopAfter;
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

#endif
