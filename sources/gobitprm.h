// $Id$
#ifndef GOBITPRM_H
#define GOBITPRM_H

#include "algdlgs.h"

#define DATA_TYPE_LOG 1
template <class T>
class GobitParamsSettings: public virtual PxiParamsSettings
{
protected:
	float minLam, maxLam, delLam, tolOpt, tolBrent;
	int maxitsOpt,maxitsBrent;
	void SaveDefaults(void);
public:
	GobitParamsSettings(const char *fn);
	~GobitParamsSettings();
	void GetParams(GobitParams<T> &P);
};

template <class T>
class GobitSolveParamsDialog : public PxiParamsDialog,public GobitParamsSettings<T>
{
public:
	GobitSolveParamsDialog(wxWindow *parent,const gString filename);
//	~GobitSolveParamsDialog(void);
};

#ifdef GOBIT_PRM_INST    // instantiate only once
// Constructor
template <class T>
GobitParamsSettings<T>::GobitParamsSettings(const char *fn)
												:PxiParamsSettings("gobit",fn)
{
wxGetResource(PARAMS_SECTION,"Gobit-minLam",&minLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Gobit-maxLam",&maxLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Gobit-delLam",&delLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolN",&tolOpt,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolBrent",&tolBrent,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsBrent",&maxitsBrent,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsOpt",&maxitsOpt,defaults_file);
}

// Save Defaults
template <class T>
void GobitParamsSettings<T>::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Gobit-minLam",minLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Gobit-maxLam",maxLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Gobit-delLam",delLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolN",tolOpt,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolBrent",tolBrent,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsBrent",maxitsBrent,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsOpt",maxitsOpt,defaults_file);
}

template <class T>
GobitParamsSettings<T>::~GobitParamsSettings(void)
{SaveDefaults();}

template <class T>
void GobitParamsSettings<T>::GetParams(GobitParams<T> &P)
{
// Gobit stuff
P.minLam=minLam;P.maxLam=maxLam;P.delLam=delLam;
Funct_tolBrent=tolBrent;Funct_maxitsBrent=maxitsBrent;
Funct_tolN=tolOpt;Funct_maxitsN=maxitsOpt;
// Pxi stuff
P.powLam=PxiType();P.pxifile=PxiFile();
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

//******************************** Constructor ************************
template <class T>
GobitSolveParamsDialog<T>::GobitSolveParamsDialog(wxWindow *parent,const gString filename)
												:PxiParamsDialog("gobit","Gobit Params",filename,parent,GOBIT_HELP),
													GobitParamsSettings<T>(filename),PxiParamsSettings("gobit",filename)
{
Form()->Add(wxMakeFormFloat("minLam",&minLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("maxLam",&maxLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("delLam",&delLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("Tolerance n-D",&tolOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("Tolerance 1-D",&tolBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("Iterations n-D",&maxitsOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormShort("Iterations 1-D",&maxitsBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
// Now add the basic stuff
MakePxiFields();
MakeOutputFields();
Go();
}

#ifdef __GNUG__
	#define TEMPLATE template
#elif defined __BORLANDC__
	#pragma option -Jgd
	#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class GobitSolveParamsDialog<double> ;
TEMPLATE class GobitParamsSettings<double> ;
#ifdef __BORLANDC__
	#pragma option -Jgx
#endif

#endif

#endif
