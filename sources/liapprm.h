// File: liapprm.h -- definition of the parameter dialog for the liap
// algorithm
// @(#)liapprm.h	1.8 8/7/95
#ifndef LIAPPRM_H
#define LIAPPRM_H
#include "algdlgs.h"

template <class T>
class LiapSolveParamsDialog : public OutputParamsDialog
{
private:
	float tolOpt, tolBrent;
	int maxitsOpt,maxitsBrent,stopAfter,nTries;
	void SaveDefaults(void);
public:
	LiapSolveParamsDialog(wxWindow *parent);
	~LiapSolveParamsDialog(void);
	void GetParams(LiapParams<T> &P);
};


//******************************** Constructor/main ************************
template <class T>
LiapSolveParamsDialog<T>::LiapSolveParamsDialog(wxWindow *parent)
												:OutputParamsDialog("Liap Params",parent)
{
tolOpt=Funct_tolN;tolBrent=Funct_tolBrent;maxitsBrent=Funct_maxitsBrent;maxitsOpt=Funct_maxitsN;
stopAfter=1;nTries=10;

wxGetResource(PARAMS_SECTION,"Liap-Ntries",&nTries,defaults_file);
wxGetResource(PARAMS_SECTION,"Liap-stopAfter",&stopAfter,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolN",&tolOpt,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-tolBrent",&tolBrent,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsBrent",&maxitsBrent,defaults_file);
wxGetResource(PARAMS_SECTION,"Func-maxitsOpt",&maxitsOpt,defaults_file);

Form()->Add(wxMakeFormShort("# Equilibria",&stopAfter,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
Form()->Add(wxMakeFormShort("# Tries",&nTries,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("Tolerance n-D",&tolOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("Tolerance 1-D",&tolBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("Iterations n-D",&maxitsOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormShort("Iterations 1-D",&maxitsBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));

// Now add the basic stuff
MakeOutputFields();
Go();
}

template <class T>
void LiapSolveParamsDialog<T>::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Liap-Ntries",nTries,defaults_file);
wxWriteResource(PARAMS_SECTION,"Liap-stopAfter",stopAfter,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolN",tolOpt,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-tolBrent",tolBrent,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsBrent",maxitsBrent,defaults_file);
wxWriteResource(PARAMS_SECTION,"Func-maxitsOpt",maxitsOpt,defaults_file);
}
template <class T>
LiapSolveParamsDialog<T>::~LiapSolveParamsDialog(void)
{SaveDefaults();}

template <class T>
void LiapSolveParamsDialog<T>::GetParams(LiapParams<T> &P)
{
Funct_tolBrent=tolBrent;Funct_maxitsBrent=maxitsBrent;
P.stopAfter=stopAfter;P.nTries=nTries;
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}
#ifdef LIAP_INST 			// need this so we only create this once
	#ifdef __GNUG__
		#define TEMPLATE template
	#elif defined __BORLANDC__
		#pragma option -Jgd
		#define TEMPLATE
	#endif   // __GNUG__, __BORLANDC__
	TEMPLATE class LiapSolveParamsDialog<double> ;
	#ifdef __BORLANDC__
		#pragma option -Jgx
	#endif
#endif

#endif
