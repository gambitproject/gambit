// $Id$
#ifndef GOBITPRM_H
#define GOBITPRM_H

#include "algdlgs.h"

#define DATA_TYPE_LOG 1
class EFGobitParams;
class NFGobitParams;
class GobitParamsSettings: public virtual PxiParamsSettings
{
protected:
	float minLam, maxLam, delLam, tol1, tolN;
	int maxits1,maxitsN;
	void SaveDefaults(void);
public:
	GobitParamsSettings(const char *fn);
	~GobitParamsSettings();
	void GetParams(EFGobitParams *P);
	void GetParams(NFGobitParams *P);
};

class GobitSolveParamsDialog : public PxiParamsDialog,public GobitParamsSettings
{
public:
	GobitSolveParamsDialog(wxWindow *parent,const gString filename);
//	~GobitSolveParamsDialog(void);
};

#ifdef GOBIT_PRM_INST    // instantiate only once
// Constructor
GobitParamsSettings::GobitParamsSettings(const char *fn)
												:PxiParamsSettings("gobit",fn)
{
wxGetResource(PARAMS_SECTION,"Gobit-minLam",&minLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Gobit-maxLam",&maxLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Gobit-delLam",&delLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolN",&tolN,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tol1",&tol1,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsN",&maxitsN,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxits1",&maxits1,defaults_file);
}

// Save Defaults
void GobitParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Gobit-minLam",minLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Gobit-maxLam",maxLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Gobit-delLam",delLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolN",tolN,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tol1",tol1,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsN",maxitsN,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxits1",maxits1,defaults_file);
}

GobitParamsSettings::~GobitParamsSettings(void)
{SaveDefaults();}

void GobitParamsSettings::GetParams(EFGobitParams *P)
{
// Gobit stuff
P->minLam=minLam;P->maxLam=maxLam;P->delLam=delLam;
P->tol1=tol1;P->tolN=tolN;P->maxits1=maxits1;P->maxitsN=maxitsN;
// Pxi stuff
P->powLam=PxiType();P->pxifile=PxiFile();
// Output stuff
P->trace=TraceLevel();P->tracefile=OutFile();
}

void GobitParamsSettings::GetParams(NFGobitParams *P)
{
// Gobit stuff
P->minLam=minLam;P->maxLam=maxLam;P->delLam=delLam;
P->tol1=tol1;P->tolN=tolN;P->maxits1=maxits1;P->maxitsN=maxitsN;
// Pxi stuff
P->powLam=PxiType();P->pxifile=PxiFile();
// Output stuff
P->trace=TraceLevel();P->tracefile=OutFile();
}
//******************************** Constructor ************************
GobitSolveParamsDialog::GobitSolveParamsDialog(wxWindow *parent,const gString filename)
												:PxiParamsDialog("gobit","Gobit Params",filename,parent,GOBIT_HELP),
													GobitParamsSettings(filename),PxiParamsSettings("gobit",filename)
{
Form()->Add(wxMakeFormFloat("minLam",&minLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("maxLam",&maxLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("delLam",&delLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("Tolerance n-D",&tolN,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("Tolerance 1-D",&tol1,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("Iterations n-D",&maxitsN,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormShort("Iterations 1-D",&maxits1,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
// Now add the basic stuff
MakePxiFields();
MakeOutputFields();
Go();
}

#endif

#endif
