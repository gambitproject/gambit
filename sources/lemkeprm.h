#ifndef LEMKEPRM_H
#define LEMKEPRM_H

#include "algdlgs.h"

class LemkeSolveParamsDialog : public OutputParamsDialog
{
private:
	int stopAfter,maxDepth;
	void SaveDefaults(void);
public:
	LemkeSolveParamsDialog(wxWindow *parent=0);
	~LemkeSolveParamsDialog(void);
	void GetParams(LemkeParams &P);
};


LemkeSolveParamsDialog::LemkeSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("Lemke Params",parent,LCP_HELP)

{
stopAfter=0;maxDepth=0;
wxGetResource(PARAMS_SECTION,"Lemke-stopAfter",&stopAfter,defaults_file);
wxGetResource(PARAMS_SECTION,"Lemke-maxDepth",&maxDepth,defaults_file);

Form()->Add(wxMakeFormShort("# Equ",&stopAfter));
Form()->Add(wxMakeFormShort("Max depth",&maxDepth));
// Now add the basic stuff
MakeOutputFields();
Go();
}

void LemkeSolveParamsDialog::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Lemke-stopAfter",stopAfter,defaults_file);
wxWriteResource(PARAMS_SECTION,"Lemke-maxDepth",maxDepth,defaults_file);
}

LemkeSolveParamsDialog::~LemkeSolveParamsDialog(void)
{SaveDefaults();}

void LemkeSolveParamsDialog::GetParams(LemkeParams &P)
{
P.stopAfter=stopAfter;P.maxDepth=maxDepth;
// Output stuff
P.trace=TraceLevel();P.output=OutFile();

}
#endif
