// File: seqfprm.h -- definition of params dialog for the sequence form
// algorithm
// $Id$
#ifndef SEQFPRM_H
#define SEQFPRM_H

#include "algdlgs.h"

class SeqFormParamsDialog : public OutputParamsDialog
{
private:
	int plev,nequilib,maxdepth,dup_strat;
	void SaveDefaults(void);
public:
	SeqFormParamsDialog(wxWindow *parent=0);
	~SeqFormParamsDialog(void);
	void GetParams(SeqFormParams &P);
	int	DupStrat(void);
};

#ifdef SEQF_PRM_INST // instantiate only once
SeqFormParamsDialog::SeqFormParamsDialog(wxWindow *parent)
														:OutputParamsDialog("Lemke Params",parent)

{
nequilib=0;maxdepth=0;dup_strat=0;
wxGetResource(PARAMS_SECTION,"SeqForm-dup_strat",&dup_strat,defaults_file);
wxGetResource(PARAMS_SECTION,"SeqForm-Nequilib",&nequilib,defaults_file);
wxGetResource(PARAMS_SECTION,"SeqForm-maxdepth",&maxdepth,defaults_file);

Form()->Add(wxMakeFormBool("All Solutions",&dup_strat));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("# Equ",&nequilib));
Form()->Add(wxMakeFormShort("Max depth",&maxdepth));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD);
Go();
}

void SeqFormParamsDialog::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"SeqForm-dup_strat",dup_strat,defaults_file);
wxWriteResource(PARAMS_SECTION,"SeqForm-Nequilib",nequilib,defaults_file);
wxWriteResource(PARAMS_SECTION,"SeqForm-maxdepth",maxdepth,defaults_file);
}

SeqFormParamsDialog::~SeqFormParamsDialog(void)
{SaveDefaults();}

int SeqFormParamsDialog::DupStrat(void) {return dup_strat;}

void SeqFormParamsDialog::GetParams(SeqFormParams &P)
{
P.nequilib=nequilib;P.maxdepth=maxdepth;
// Output stuff
P.plev=TraceLevel();P.output=OutFile();
}
#endif

#endif
