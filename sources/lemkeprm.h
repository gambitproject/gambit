// $Id$
#ifndef LEMKEPRM_H
#define LEMKEPRM_H

#include "algdlgs.h"

class LemkeParamsSettings : public virtual OutputParamsSettings
{
protected:
	int maxDepth;
	void SaveDefaults(void);
public:
	LemkeParamsSettings(void);
	~LemkeParamsSettings() {SaveDefaults();}
	void GetParams(LemkeParams &P);
};

class LemkeSolveParamsDialog : public OutputParamsDialog,public LemkeParamsSettings
{
public:
	LemkeSolveParamsDialog(wxWindow *parent=0,bool subgames=false);
};

#ifdef LEMKE_PRM_INST  // instantiate only once

LemkeParamsSettings::LemkeParamsSettings(void)
{
wxGetResource(PARAMS_SECTION,"Lemke-maxDepth",&maxDepth,defaults_file);
}

void LemkeParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Lemke-maxDepth",maxDepth,defaults_file);
}

void LemkeParamsSettings::GetParams(LemkeParams &P)
{
P.stopAfter=StopAfter();P.maxdepth=maxDepth;
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

LemkeSolveParamsDialog::LemkeSolveParamsDialog(wxWindow *parent,bool subgames)
														:OutputParamsDialog("Lemke Params",parent,LCP_HELP)

{
Add(wxMakeFormShort("Max depth",&maxDepth));
// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD|MAXSOLN_FIELD| ((subgames) ? SPS_FIELD : 0));
Go();
}

#endif

#endif
