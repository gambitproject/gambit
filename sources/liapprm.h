#ifndef LIAPPRM_H
#define LIAPPRM_H
#include "outprm.h"

template <class T>
class LiapSolveParamsDialog : public OutputParamsDialog
{
private:
	float tolOpt, tolBrent;
	int maxitsOpt,maxitsBrent,nequilib,ntries;
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
tolOpt=.0001;tolBrent=0.0001;maxitsBrent=100;maxitsOpt=200;
nequilib=1;ntries=10;
Form()->Add(wxMakeFormShort("# Equilibria",&nequilib,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
Form()->Add(wxMakeFormShort("# Tries",&ntries,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
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
LiapSolveParamsDialog<T>::~LiapSolveParamsDialog(void)
{}
template <class T>
void LiapSolveParamsDialog<T>::GetParams(LiapParams<T> &P)
{
P.tolBrent=tolBrent;P.maxitsBrent=maxitsBrent;
P.nequilib=nequilib;P.ntries=ntries;
// Output stuff
P.plev=TraceLevel();P.outfile=OutFile();P.errfile=ErrFile();
}
#ifdef LIAP_INST 			// need this so we only create this once
	#ifdef __GNUG__
		#define TEMPLATE template
	#elif defined __BORLANDC__
		#pragma option -Jgd
		#define TEMPLATE
	#endif   // __GNUG__, __BORLANDC__
	TEMPLATE class LiapSolveParamsDialog<double> ;
	TEMPLATE class LiapSolveParamsDialog<gRational> ;
	#ifdef __BORLANDC__
		#pragma option -Jgx
	#endif
#endif

#endif
