// $Id$
#ifndef ENUMPRM_H
#define ENUMPRM_H

#include "algdlgs.h"

class EnumParamsSettings:public OutputParamsSettings
{
protected:
	void SaveDefaults(void);
public:
	EnumParamsSettings(void);
	~EnumParamsSettings() {SaveDefaults();}
	void GetParams(EnumParams &P);
};

class EnumSolveParamsDialog : public OutputParamsDialog,public EnumParamsSettings
{
public:
	EnumSolveParamsDialog(wxWindow *parent=0,bool subgames=false);
};


class wxEnumStatus: public wxStatus // Enum needs a custom progress indicator
{
private:
	int pass;
public:
	wxEnumStatus(wxFrame *parent);
	void SetProgress(double p);
};

#ifdef ENUM_PRM_INST  // instantiate only once
EnumParamsSettings::EnumParamsSettings(void)
{}
void EnumParamsSettings::SaveDefaults(void)
{}

void EnumParamsSettings::GetParams(EnumParams &P)
{
P.stopAfter=StopAfter();
// Output stuff
P.trace=TraceLevel();P.tracefile=OutFile();
}


EnumSolveParamsDialog::EnumSolveParamsDialog(wxWindow *parent,bool subgames)
														:OutputParamsDialog("Enum Params",parent,ENUMMIXED_HELP)

{
Add(wxMakeFormNewLine());

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD|MAXSOLN_FIELD| ((subgames) ? SPS_FIELD : 0));
Go();
}


wxEnumStatus::wxEnumStatus(wxFrame *parent)
					: wxStatus(parent,"EnumMixed Algorithm"),pass(0)
{ };

void wxEnumStatus::SetProgress(double p)
{
if (p>-.5)	// actually, as long as its not -1.0, but floating point ==
	gauge->SetValue((int)((p+pass)/3.0*100.0));
else
	pass++;
wxYield();
}



#endif
#endif
