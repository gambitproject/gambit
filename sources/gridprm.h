// $Id$

#ifndef GRIDPRM_H
#define GRIDPRM_H
#include "algdlgs.h"

class GridParamsSettings:public virtual PxiParamsSettings
{
protected:
	float minLam, maxLam, delLam, delp, tol;
	void SaveDefaults(void);
public:
	GridParamsSettings(const char *fn);
	~GridParamsSettings();
	void GetParams(GridParams &P);
};


class GridSolveParamsDialog : public PxiParamsDialog,public GridParamsSettings
{
public:
	GridSolveParamsDialog(wxWindow *parent,const gString filename);
//	~GridSolveParamsDialog(void);
};
#ifdef GRID_PRM_INST    // instantiate only once
//******************************** Constructor ************************
GridParamsSettings::GridParamsSettings(const char *fn)
											:PxiParamsSettings("grid",fn)
{
wxGetResource(PARAMS_SECTION,"Grid-minLam",&minLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-maxLam",&maxLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-delLam",&delLam,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-delp",&delp,defaults_file);
wxGetResource(PARAMS_SECTION,"Grid-tol",&tol,defaults_file);
}

void GridParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Grid-minLam",minLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-maxLam",maxLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-delLam",delLam,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-delp",delp,defaults_file);
wxWriteResource(PARAMS_SECTION,"Grid-tol",tol,defaults_file);
}


GridParamsSettings::~GridParamsSettings(void)
{SaveDefaults();}


void GridParamsSettings::GetParams(GridParams &P)
{
P.minLam=minLam;P.maxLam=maxLam;P.delLam=delLam;P.tol=tol;P.delp=delp;
// Pxi stuff
P.powLam=PxiType();P.pxifile=PxiFile();
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}


GridSolveParamsDialog::GridSolveParamsDialog(wxWindow *parent,const gString filename)
				:PxiParamsDialog("grid","Grid Params",filename,parent,GOBIT_HELP),
				 GridParamsSettings(filename),PxiParamsSettings("grid",filename)
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

#endif // GRID_PRM_INST

#endif
