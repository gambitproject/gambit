// $Id$
#ifndef LEMKEPRM_H
#define LEMKEPRM_H

#include "algdlgs.h"

class LemkeParamsSettings : public OutputParamsSettings
{
protected:
	int stopAfter,maxDepth;
	void SaveDefaults(void);
public:
	LemkeParamsSettings(void);
	~LemkeParamsSettings() {SaveDefaults();}
	void GetParams(LemkeParams &P);
};

class LemkeSolveParamsDialog : public OutputParamsDialog,public LemkeParamsSettings
{
public:
	LemkeSolveParamsDialog(wxWindow *parent=0);
};

#ifdef LEMKE_PRM_INST  // instantiate only once

LemkeParamsSettings::LemkeParamsSettings(void)
{
wxGetResource(PARAMS_SECTION,"Lemke-stopAfter",&stopAfter,defaults_file);
wxGetResource(PARAMS_SECTION,"Lemke-maxDepth",&maxDepth,defaults_file);
}

void LemkeParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Lemke-stopAfter",stopAfter,defaults_file);
wxWriteResource(PARAMS_SECTION,"Lemke-maxDepth",maxDepth,defaults_file);
}

void LemkeParamsSettings::GetParams(LemkeParams &P)
{
P.stopAfter=stopAfter;P.maxDepth=maxDepth;
// Output stuff
P.trace=TraceLevel();P.output=OutFile();
}

LemkeSolveParamsDialog::LemkeSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("Lemke Params",parent,LCP_HELP)

{
Add(wxMakeFormShort("# Equ",&stopAfter));
Add(wxMakeFormShort("Max depth",&maxDepth));
// Now add the basic stuff
MakeOutputFields();
Go();
}

#endif

#endif
