#ifndef SIMPPRM_H
#define SIMPPRM_H

#include "algdlgs.h"

class SimpdivSolveParamsDialog : public OutputParamsDialog
{
private:
	int number,ndivs,leash;
	void SaveDefaults(void);
public:
	SimpdivSolveParamsDialog(wxWindow *parent=0);
	~SimpdivSolveParamsDialog(void);
	void GetParams(SimpdivParams &P);
};


SimpdivSolveParamsDialog::SimpdivSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("SimpDiv Params",parent)

{
number=1;ndivs=10;leash=0;
wxGetResource(PARAMS_SECTION,"Simpdiv-number",&number,defaults_file);
wxGetResource(PARAMS_SECTION,"Simpdiv-ndivs",&ndivs,defaults_file);
wxGetResource(PARAMS_SECTION,"Simpdiv-leash",&leash,defaults_file);

Form()->Add(wxMakeFormShort("# Equilibria",&number));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("# Restarts",&ndivs));
Form()->Add(wxMakeFormShort("Leash",&leash));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD);
Go();
}

void SimpdivSolveParamsDialog::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Simpdiv-number",number,defaults_file);
wxWriteResource(PARAMS_SECTION,"Simpdiv-ndivs",ndivs,defaults_file);
wxWriteResource(PARAMS_SECTION,"Simpdiv-leash",leash,defaults_file);
}
SimpdivSolveParamsDialog::~SimpdivSolveParamsDialog(void)
{SaveDefaults();}

void SimpdivSolveParamsDialog::GetParams(SimpdivParams &P)
{
P.ndivs=ndivs;P.leash=leash;P.number=number;
// Output stuff
P.plev=TraceLevel();P.output=OutFile();
}

#endif
