// $Id$
#ifndef ENUMPRM_H
#define ENUMPRM_H

#include "algdlgs.h"

class EnumSolveParamsDialog : public OutputParamsDialog
{
private:
	int stopAfter;
	void SaveDefaults(void);
public:
	EnumSolveParamsDialog(wxWindow *parent=0);
	~EnumSolveParamsDialog(void);
	void GetParams(EnumParams &P);
};


EnumSolveParamsDialog::EnumSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("Enum Params",parent)

{
stopAfter=0;
wxGetResource(PARAMS_SECTION,"Enum-StopAfter",&stopAfter,defaults_file);

Form()->Add(wxMakeFormShort("# Equilibria",&stopAfter));
Form()->Add(wxMakeFormNewLine());

// Now add the basic stuff
MakeOutputFields();
Go();
}

void EnumSolveParamsDialog::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Enum-StopAfter",stopAfter,defaults_file);
}

EnumSolveParamsDialog::~EnumSolveParamsDialog(void)
{SaveDefaults();}

void EnumSolveParamsDialog::GetParams(EnumParams &P)
{
P.stopAfter=stopAfter;
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

#endif
