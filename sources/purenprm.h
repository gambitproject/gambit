//
// FILE: purenprm.h -- parameter file for PureNash GUI.
//
// $Id$
//

#ifndef PURENPRM_H
#define PURENPRM_H

#include "algdlgs.h"

class PureNashParamsSettings : public virtual OutputParamsSettings {
protected:
  void SaveDefaults(void) { }
public:
  PureNashParamsSettings(void) { };
  ~PureNashParamsSettings() { SaveDefaults(); } 
  void GetParams(int &p_stopAfter);
};

class PureNashSolveParamsDialog : public OutputParamsDialog,
				  public PureNashParamsSettings {
public:
  PureNashSolveParamsDialog(wxWindow *parent=0,bool subgames=false);
};

#ifdef PUREN_PRM_INST		// instantiate only once

void PureNashParamsSettings::GetParams(int &p_stopAfter)
{
  p_stopAfter = StopAfter();
}

PureNashSolveParamsDialog::PureNashSolveParamsDialog(wxWindow *parent,
						     bool subgames)
  : OutputParamsDialog("PureNash Params", parent)

{
  MakeOutputFields(OUTPUT_FIELD | MAXSOLN_FIELD |
		   ((subgames) ? SPS_FIELD : 0));
  Go();
}

void


#endif

#endif
