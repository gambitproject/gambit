#ifndef LEMKEPRM_H
#define LEMKEPRM_H

#include "algdlgs.h"

class LemkeSolveParamsDialog : public OutputParamsDialog
{
private:
	int plev,nequilib,maxdepth,dup_strat;
	void SaveDefaults(void);
public:
	LemkeSolveParamsDialog(wxWindow *parent=0);
	~LemkeSolveParamsDialog(void);
	void GetParams(LemkeParams &P);
};


LemkeSolveParamsDialog::LemkeSolveParamsDialog(wxWindow *parent)
														:OutputParamsDialog("Lemke Params",parent)

{
dup_strat=0;nequilib=0;maxdepth=0;
wxGetResource(PARAMS_SECTION,"Lemke-dup_strat",&dup_strat,defaults_file);
wxGetResource(PARAMS_SECTION,"Lemke-Nequilib",&nequilib,defaults_file);
wxGetResource(PARAMS_SECTION,"Lemke-maxdepth",&maxdepth,defaults_file);

Form()->Add(wxMakeFormBool("All Solutions",&dup_strat));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("# Equ",&nequilib));
Form()->Add(wxMakeFormShort("Max depth",&maxdepth));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD);
Go();
}

void LemkeSolveParamsDialog::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Lemke-dup_strat",dup_strat,defaults_file);
wxWriteResource(PARAMS_SECTION,"Lemke-Nequilib",nequilib,defaults_file);
wxWriteResource(PARAMS_SECTION,"Lemke-maxdepth",maxdepth,defaults_file);
}

LemkeSolveParamsDialog::~LemkeSolveParamsDialog(void)
{SaveDefaults();}

void LemkeSolveParamsDialog::GetParams(LemkeParams &P)
{
P.dup_strat=!dup_strat;P.nequilib=nequilib;P.maxdepth=maxdepth;
// Output stuff
P.plev=TraceLevel();P.output=OutFile();

}
#endif
