// @(#)enumprm.h	1.2 6/23/95
#ifndef ENUMPRM_H
#define ENUMPRM_H

#include "algdlgs.h"

class EnumSolveParamsDialog : public OutputParamsDialog
{
private:
	int nequilib;
public:
	EnumSolveParamsDialog(wxWindow *parent=0);
	~EnumSolveParamsDialog(void);
	void GetParams(EnumParams &P);
};


EnumSolveParamsDialog::EnumSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("Enum Params",parent)

{
nequilib=0;
Form()->Add(wxMakeFormShort("# Equilibria",&nequilib));
Form()->Add(wxMakeFormNewLine());

// Now add the basic stuff
MakeOutputFields();
Go();
}

EnumSolveParamsDialog::~EnumSolveParamsDialog(void)
{}

void EnumSolveParamsDialog::GetParams(EnumParams &P)
{
P.nequilib=nequilib;
// Output stuff
P.plev=TraceLevel();P.outfile=OutFile();P.errfile=ErrFile();
}

#endif
