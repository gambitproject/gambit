// File: purenprm.h -- parameter file for PureNash GUI.
// $Id$
#ifndef PURENPRM_H
#define PURENPRM_H

#include "algdlgs.h"

class PureNashParamsSettings : public virtual OutputParamsSettings
{
protected:
	void SaveDefaults(void) { };
public:
	PureNashParamsSettings(void) { };
	~PureNashParamsSettings() {SaveDefaults();}
};

class PureNashSolveParamsDialog : public OutputParamsDialog,public PureNashParamsSettings
{
public:
	PureNashSolveParamsDialog(wxWindow *parent=0,bool subgames=false);
};

#ifdef PUREN_PRM_INST		// instantiate only once
PureNashSolveParamsDialog::PureNashSolveParamsDialog(wxWindow *parent,bool subgames)
														:OutputParamsDialog("PureNash Params",parent)

{

if (!subgames)
	Add(wxMakeFormMessage("No parameters for PureNash"));
else
	MakeOutputFields(((subgames) ? SPS_FIELD : 0));
Go();
}
#endif

#endif
