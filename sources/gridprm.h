#ifndef GRIDPRM_H
#define GRIDPRM_H
#include "algdlgs.h"

template <class T>
class GridSolveParamsDialog : public PxiParamsDialog
{
private:
	float minLam, maxLam, delLam, delp, tol;
	void SaveDefaults(void);
public:
	GridSolveParamsDialog(wxWindow *parent);
	~GridSolveParamsDialog(void);
	void GetParams(GridParams<T> &P);
};

//******************************** Constructor/main ************************
template <class T>
GridSolveParamsDialog<T>::GridSolveParamsDialog(wxWindow *parent)
				:PxiParamsDialog("grid","Grid Params",parent,GOBIT_HELP)
{
minLam=0.01;maxLam=3.0;delLam=.1;delp=.01;tol=.01;
wxGetResource(PARAMS_SECTION,"Grid-minLam",&minLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-maxLam",&maxLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-delLam",&delLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-delp",&delp,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-tol",&tol,defaults_file);

Form()->Add(wxMakeFormFloat("minLam",&minLam,wxFORM_DEFAULT,0,0,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("maxLam",&maxLam,wxFORM_DEFAULT,0,0,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("delLam",&delLam,wxFORM_DEFAULT,0,0,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("del P",&delp,wxFORM_DEFAULT,0,0,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("Tolerance",&tol,wxFORM_DEFAULT,0,0,wxVERTICAL,100));

// Now add the basic stuff
MakePxiFields();
MakeOutputFields();
Go();
}

template <class T>
void GridSolveParamsDialog<T>::SaveDefaults(void)
{
if (!Default()) return;
wxWriteResource(PARAMS_SECTION,"Grid-minLam",minLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-maxLam",maxLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-delLam",delLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-delp",delp,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-tol",tol,defaults_file);
}

template <class T>
GridSolveParamsDialog<T>::~GridSolveParamsDialog(void)
{SaveDefaults();}

template <class T>
void GridSolveParamsDialog<T>::GetParams(GridParams<T> &P)
{
P.minLam=minLam;P.maxLam=maxLam;P.delLam=delLam;P.tol=tol;P.delp=delp;
// Pxi stuff
P.powLam=PxiType();P.pxifile=PxiFile();
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class GridSolveParamsDialog<double> ;
TEMPLATE class GridSolveParamsDialog<gRational> ;
#ifdef __BORLANDC__
#pragma -Jgx
#endif

#endif
