#ifndef GOBITPRM_H
#define GOBITPRM_H

#include "algdlgs.h"

#define DATA_TYPE_LOG 1
template <class T>
class GobitSolveParamsDialog : public PxiParamsDialog
{
private:
	float minLam, maxLam, delLam, tolOpt, tolBrent;
	int maxitsOpt,maxitsBrent;
	void SaveDefaults(void);
public:
	GobitSolveParamsDialog(wxWindow *parent);
	~GobitSolveParamsDialog(void);
	void GetParams(GobitParams<T> &P);
};


//******************************** Constructor/main ************************
template <class T>
GobitSolveParamsDialog<T>::GobitSolveParamsDialog(wxWindow *parent)
												:PxiParamsDialog("gobit","Gobit Params",parent,GOBIT_HELP)
{
minLam=Gobit_default_minLam;maxLam=Gobit_default_maxLam;delLam=Gobit_default_delLam;
tolOpt=Funct_tolN;tolBrent=Funct_tolBrent;maxitsBrent=Funct_maxitsBrent;maxitsOpt=Funct_maxitsN;

wxGetResource(PARAMS_SECTION,"Gobit-minLam",&minLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Gobit-maxLam",&maxLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Gobit-delLam",&delLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolN",&tolOpt,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolBrent",&tolBrent,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsBrent",&maxitsBrent,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsOpt",&maxitsOpt,defaults_file);

Form()->Add(wxMakeFormFloat("L Start",&minLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("L Stop",&maxLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("L Step",&delLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
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
// Save Defaults
template <class T>
void GobitSolveParamsDialog<T>::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Gobit-minLam",minLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Gobit-maxLam",maxLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Gobit-delLam",delLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolN",ToString(tolOpt),defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolBrent",ToString(tolBrent),defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsBrent",maxitsBrent,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsOpt",maxitsOpt,defaults_file);
}

template <class T>
GobitSolveParamsDialog<T>::~GobitSolveParamsDialog(void)
{SaveDefaults();}

template <class T>
void GobitSolveParamsDialog<T>::GetParams(GobitParams<T> &P)
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

#ifdef GOBIT_INST		// need this so we only create this once
	#ifdef __GNUG__
		#define TEMPLATE template
	#elif defined __BORLANDC__
		#pragma option -Jgd
		#define TEMPLATE
	#endif   // __GNUG__, __BORLANDC__
	TEMPLATE class GobitSolveParamsDialog<double> ;
	#ifdef __BORLANDC__
		#pragma option -Jgx
	#endif
#endif

#endif
