// $Id$
#ifndef SIMPPRM_H
#define SIMPPRM_H

#include "algdlgs.h"

class SimpdivParamsSettings:public OutputParamsSettings
{
protected:
	int nRestarts, leashLength;
	void SaveDefaults(void);
public:
	SimpdivParamsSettings(void);
	~SimpdivParamsSettings() {SaveDefaults();}
	void GetParams(SimpdivParams &P);
};

class SimpdivSolveParamsDialog : public OutputParamsDialog,public SimpdivParamsSettings
{
public:
	SimpdivSolveParamsDialog(wxWindow *parent=0,bool subgames=false);
};

#ifdef SIMP_PRM_INST // instantiate only once
SimpdivParamsSettings::SimpdivParamsSettings(void)
{
wxGetResource(PARAMS_SECTION,"Simpdiv-nRestarts",&nRestarts,defaults_file);
wxGetResource(PARAMS_SECTION,"Simpdiv-leashLength",&leashLength,defaults_file);
}

void SimpdivParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Simpdiv-nRestarts",nRestarts,defaults_file);
wxWriteResource(PARAMS_SECTION,"Simpdiv-leashLength",leashLength,defaults_file);
}

void SimpdivParamsSettings::GetParams(SimpdivParams &P)
{
P.nRestarts=nRestarts;P.leashLength=leashLength;P.stopAfter=StopAfter();
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

SimpdivSolveParamsDialog::SimpdivSolveParamsDialog(wxWindow *parent,bool subgames)
														:OutputParamsDialog("SimpDiv Params",parent,SIMPDIV_HELP)

{
Add(wxMakeFormNewLine());
Add(wxMakeFormShort("# Restarts",&nRestarts));
Add(wxMakeFormShort("Leash",&leashLength));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD|MAXSOLN_FIELD| ((subgames) ? SPS_FIELD : 0));
Go();
}

#endif
#endif
