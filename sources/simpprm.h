#ifndef SIMPPRM_H
#define SIMPPRM_H

#include "algdlgs.h"

class SimpdivSolveParamsDialog : public OutputParamsDialog
{
private:
	int number,ndivs,leash;
public:
	SimpdivSolveParamsDialog(wxWindow *parent=0);
	~SimpdivSolveParamsDialog(void);
	void GetParams(SimpdivParams &P);
};


SimpdivSolveParamsDialog::SimpdivSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("SimpDiv Params",parent)

{
number=1;ndivs=10;leash=0;
Form()->Add(wxMakeFormShort("# Equilibria",&number));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("# Restarts",&ndivs));
Form()->Add(wxMakeFormShort("Leash",&leash));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD);
Go();
}

SimpdivSolveParamsDialog::~SimpdivSolveParamsDialog(void)
{}

void SimpdivSolveParamsDialog::GetParams(SimpdivParams &P)
{
P.ndivs=ndivs;P.leash=leash;P.number=number;
// Output stuff
P.plev=TraceLevel();P.output=OutFile();
}

#endif
