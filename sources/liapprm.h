// File: liapprm.h -- definition of the parameter dialog for the liap
// algorithm
// $Id$
#ifndef LIAPPRM_H
#define LIAPPRM_H
#include "algdlgs.h"

class LiapParamsSettings: public OutputParamsSettings
{
protected:
	float tolOpt, tolBrent;
	int maxitsOpt,maxitsBrent,stopAfter,nTries;
	void SaveDefaults(void);
public:
	LiapParamsSettings(void);
	~LiapParamsSettings();
	void GetParams(LiapParams &P);
};

class LiapSolveParamsDialog : public OutputParamsDialog,public LiapParamsSettings
{
public:
	LiapSolveParamsDialog(wxWindow *parent);
//	~LiapSolveParamsDialog(void);
};

#ifdef LIAP_PRM_INST 			// instantiate only once

//******************************** Constructor/main ************************
LiapParamsSettings::LiapParamsSettings(void)
										:OutputParamsSettings()
{
wxGetResource(PARAMS_SECTION,"Liap-Ntries",&nTries,defaults_file);
wxGetResource(PARAMS_SECTION,"Liap-stopAfter",&stopAfter,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolN",&tolOpt,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolBrent",&tolBrent,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsBrent",&maxitsBrent,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsOpt",&maxitsOpt,defaults_file);
}

void LiapParamsSettings::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Liap-Ntries",nTries,defaults_file);
wxWriteResource(PARAMS_SECTION,"Liap-stopAfter",stopAfter,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolN",tolOpt,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolBrent",tolBrent,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsBrent",maxitsBrent,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsOpt",maxitsOpt,defaults_file);
}

LiapParamsSettings::~LiapParamsSettings(void)
{SaveDefaults();}

void LiapParamsSettings::GetParams(LiapParams &P)
{
Funct_tolBrent=tolBrent;Funct_maxitsBrent=maxitsBrent;
P.stopAfter=stopAfter;P.nTries=nTries;
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}


LiapSolveParamsDialog::LiapSolveParamsDialog(wxWindow *parent)
												:OutputParamsDialog("Liap Params",parent),LiapParamsSettings()
{
Form()->Add(wxMakeFormShort("# Equilibria",&stopAfter,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormShort("# Tries",&nTries,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("Tolerance n-D",&tolOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("Tolerance 1-D",&tolBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("Iterations n-D",&maxitsOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormShort("Iterations 1-D",&maxitsBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD|SUBGAME_FIELD);
Go();
}

#endif

#endif
