// $Id$
#ifndef ENUMPRM_H
#define ENUMPRM_H

#include "algdlgs.h"

class EnumParamsSettings:public OutputParamsSettings
{
protected:
	int stopAfter;
	void SaveDefaults(void);
public:
	EnumParamsSettings(void);
	~EnumParamsSettings() {SaveDefaults();}
	void GetParams(EnumParams &P);
};

class EnumSolveParamsDialog : public OutputParamsDialog,public EnumParamsSettings
{
public:
	EnumSolveParamsDialog(wxWindow *parent=0);
};

#ifdef ENUM_PRM_INST  // instantiate only once
EnumParamsSettings::EnumParamsSettings(void)
{
wxGetResource(PARAMS_SECTION,"Enum-StopAfter",&stopAfter,defaults_file);
}
void EnumParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Enum-StopAfter",stopAfter,defaults_file);
}

void EnumParamsSettings::GetParams(EnumParams &P)
{
P.stopAfter=stopAfter;
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}


EnumSolveParamsDialog::EnumSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("Enum Params",parent,ENUMMIXED_HELP)

{
Add(wxMakeFormShort("# Equilibria",&stopAfter));
Add(wxMakeFormNewLine());

// Now add the basic stuff
MakeOutputFields();
Go();
}

#endif
#endif
