// $Id$

#ifndef GRIDPRM_H
#define GRIDPRM_H
#include "algdlgs.h"

template <class T>
class GridParamsSettings:public virtual PxiParamsSettings
{
protected:
	float minLam, maxLam, delLam, delp, tol;
	void SaveDefaults(void);
public:
	GridParamsSettings(const char *fn);
	~GridParamsSettings();
	void GetParams(GridParams<T> &P);
};


template <class T>
class GridSolveParamsDialog : public PxiParamsDialog,public GridParamsSettings<T>
{
public:
	GridSolveParamsDialog(wxWindow *parent,const gString filename);
//	~GridSolveParamsDialog(void);
};
#ifdef GRID_PRM_INST    // instantiate only once
//******************************** Constructor ************************
template <class T>
GridParamsSettings<T>::GridParamsSettings(const char *fn)
											:PxiParamsSettings("grid",fn)
{
wxGetResource(PARAMS_SECTION,"Grid-minLam",&minLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-maxLam",&maxLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-delLam",&delLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-delp",&delp,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-tol",&tol,defaults_file);
}

template <class T>
void GridParamsSettings<T>::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Grid-minLam",minLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-maxLam",maxLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-delLam",delLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-delp",delp,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-tol",tol,defaults_file);
}

template <class T>
GridParamsSettings<T>::~GridParamsSettings(void)
{SaveDefaults();}

template <class T>
void GridParamsSettings<T>::GetParams(GridParams<T> &P)
{
P.minLam=minLam;P.maxLam=maxLam;P.delLam=delLam;P.tol=tol;P.delp=delp;
// Pxi stuff
P.powLam=PxiType();P.pxifile=PxiFile();
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}

template <class T>
GridSolveParamsDialog<T>::GridSolveParamsDialog(wxWindow *parent,const gString filename)
				:PxiParamsDialog("grid","Grid Params",filename,parent,GOBIT_HELP),
				 GridParamsSettings<T>(filename),PxiParamsSettings("grid",filename)
{
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


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class GridSolveParamsDialog<double> ;
TEMPLATE class GridSolveParamsDialog<gRational> ;
TEMPLATE class GridParamsSettings<double>;
TEMPLATE class GridParamsSettings<gRational>;
#ifdef __BORLANDC__
#pragma -Jgx
#endif

#endif

#endif // GRID_PRM_INST
