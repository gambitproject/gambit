// File: liapprm.h -- definition of the parameter dialog for the liap
// algorithm
// $Id$
#ifndef LIAPPRM_H
#define LIAPPRM_H
#include "algdlgs.h"

class NFLiapParams;
class EFLiapParams;
class LiapParamsSettings: public virtual OutputParamsSettings
{
protected:
	float tol1, tolN;
	int maxits1,maxitsN,nTries;
	bool subgames;
	void SaveDefaults(void);
public:
	LiapParamsSettings(void);
	~LiapParamsSettings();
	void GetParams(EFLiapParams *P);
	void GetParams(NFLiapParams *P);
};

class LiapSolveParamsDialog : public OutputParamsDialog,public LiapParamsSettings
{
public:
	LiapSolveParamsDialog(wxWindow *parent,bool subgames=false);
//	~LiapSolveParamsDialog(void);
};

#ifdef LIAP_PRM_INST 			// instantiate only once

//******************************** Constructor/main ************************
LiapParamsSettings::LiapParamsSettings(void)
										:OutputParamsSettings()
{
wxGetResource(PARAMS_SECTION,"Liap-Ntries",&nTries,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolN",&tolN,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tol1",&tol1,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsN",&maxitsN,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxits1",&maxits1,defaults_file);
}

void LiapParamsSettings::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Liap-Ntries",nTries,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolN",tolN,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tol1",tol1,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsN",maxitsN,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxits1",maxits1,defaults_file);
}

LiapParamsSettings::~LiapParamsSettings(void)
{SaveDefaults();}

void LiapParamsSettings::GetParams(EFLiapParams *P)
{
P->tol1=tol1;P->tolN=tolN;P->maxits1=maxits1;P->maxitsN=maxitsN;
//P->stopAfter=StopAfter();P->nTries=nTries;
// Output stuff
P->trace=TraceLevel();P->tracefile=OutFile();
}
void LiapParamsSettings::GetParams(NFLiapParams *P)
{
P->tol1=tol1;P->tolN=tolN;P->maxits1=maxits1;P->maxitsN=maxitsN;
//P->stopAfter=StopAfter();P->nTries=nTries;
// Output stuff
P->trace=TraceLevel();P->tracefile=OutFile();
}


LiapSolveParamsDialog::LiapSolveParamsDialog(wxWindow *parent,bool subgames)
												:OutputParamsDialog("Liap Params",parent),LiapParamsSettings()
{
Form()->Add(wxMakeFormShort("Max # Tries",&nTries,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("Tolerance n-D",&tolN,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("Tolerance 1-D",&tol1,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("Iterations n-D",&maxitsN,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormShort("Iterations 1-D",&maxits1,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD|MAXSOLN_FIELD| ((subgames) ? SPS_FIELD : 0));
Go();
}

#endif

#endif
