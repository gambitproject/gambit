#ifndef GOBITPRM_H
#define GOBITPRM_H

#include "outprm.h"

#define DATA_TYPE_LOG 1
template <class T>
class GobitSolveParamsDialog : public PxiParamsDialog
{
private:
	float minLam, maxLam, delLam, tolOpt, tolBrent;
	int maxitsOpt,maxitsBrent,nequilib;
public:
	GobitSolveParamsDialog(wxWindow *parent);
	~GobitSolveParamsDialog(void);
	void GetParams(GobitParams<T> &P);
};


//******************************** Constructor/main ************************
template <class T>
GobitSolveParamsDialog<T>::GobitSolveParamsDialog(wxWindow *parent)
												:PxiParamsDialog("gobit","Gobit Params",parent)
{
minLam=0.01;maxLam=30.0;delLam=.01;nequilib=1;
tolOpt=.0001;tolBrent=0.0001;maxitsBrent=100;maxitsOpt=200;
Form()->Add(wxMakeFormShort("# Equilibria",&nequilib));
Form()->Add(wxMakeFormNewLine());
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

template <class T>
GobitSolveParamsDialog<T>::~GobitSolveParamsDialog(void)
{ }

template <class T>
void GobitSolveParamsDialog<T>::GetParams(GobitParams<T> &P)
{
// Gobit stuff
P.minLam=minLam;P.maxLam=maxLam;P.delLam=delLam;
P.tolBrent=tolBrent;P.maxitsBrent=maxitsBrent;
P.nequilib=nequilib;
P.tolOpt=tolOpt;P.maxitsOpt=maxitsOpt;
// Pxi stuff
P.type=PxiType();P.pxifile=PxiFile();
// Output stuff
P.plev=TraceLevel();P.outfile=OutFile();P.errfile=ErrFile();
}

#ifdef GOBIT_INST		// need this so we only create this once
	#ifdef __GNUG__
		#define TEMPLATE template
	#elif defined __BORLANDC__
		#pragma option -Jgd
		#define TEMPLATE
	#endif   // __GNUG__, __BORLANDC__
	TEMPLATE class GobitSolveParamsDialog<double> ;
  #include "rational.h"
	TEMPLATE class GobitSolveParamsDialog<gRational> ;
	#ifdef __BORLANDC__
		#pragma option -Jgx
	#endif
#endif

#endif
