#ifndef LEMKEPRM_H
#define LEMKEPRM_H

#include "algdlgs.h"

class LemkeSolveParamsDialog : public OutputParamsDialog
{
private:
	int plev,nequilib,maxdepth,dup_strat;
public:
	LemkeSolveParamsDialog(wxWindow *parent=0);
	~LemkeSolveParamsDialog(void);
	void GetParams(LemkeParams &P);
};


LemkeSolveParamsDialog::LemkeSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("Lemke Params",parent)

{
dup_strat=0;nequilib=0;maxdepth=0;nequilib=1;
Form()->Add(wxMakeFormBool("All Solutions",&dup_strat));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("# Equ",&nequilib));
Form()->Add(wxMakeFormShort("Max depth",&maxdepth));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD);
Go();
}

LemkeSolveParamsDialog::~LemkeSolveParamsDialog(void)
{}

void LemkeSolveParamsDialog::GetParams(LemkeParams &P)
{
P.dup_strat=!dup_strat;P.nequilib=nequilib;P.maxdepth=maxdepth;
// Output stuff
P.plev=TraceLevel();P.output=OutFile();

}
#endif
