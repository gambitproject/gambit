// File: seqfprm.h -- definition of params dialog for the sequence form
// algorithm
// $Id$
#ifndef SEQFPRM_H
#define SEQFPRM_H

#include "algdlgs.h"

class SeqFormParamsSettings: public OutputParamsSettings
{
protected:
	int plev,maxdepth,dup_strat;
	void SaveDefaults(void);
public:
	SeqFormParamsSettings(void);
	~SeqFormParamsSettings();
	void GetParams(SeqFormParams &P);
	int	DupStrat(void);
};

class SeqFormParamsDialog : public OutputParamsDialog,public SeqFormParamsSettings
{
public:
	SeqFormParamsDialog(wxWindow *parent=0,bool subgames=false);
//	~SeqFormParamsDialog(void);
};

#ifdef SEQF_PRM_INST // instantiate only once
SeqFormParamsSettings::SeqFormParamsSettings(void)
										:OutputParamsSettings()
{
wxGetResource(PARAMS_SECTION,"SeqForm-dup_strat",&dup_strat,defaults_file);
wxGetResource(PARAMS_SECTION,"SeqForm-maxdepth",&maxdepth,defaults_file);
}

void SeqFormParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"SeqForm-dup_strat",dup_strat,defaults_file);
wxWriteResource(PARAMS_SECTION,"SeqForm-maxdepth",maxdepth,defaults_file);
}

SeqFormParamsSettings::~SeqFormParamsSettings(void)
{SaveDefaults();}

int SeqFormParamsSettings::DupStrat(void) {return dup_strat;}

void SeqFormParamsSettings::GetParams(SeqFormParams &P)
{
P.stopAfter=StopAfter();P.maxdepth=maxdepth;
// Output stuff
P.plev=TraceLevel();P.output=OutFile();
}


SeqFormParamsDialog::SeqFormParamsDialog(wxWindow *parent,bool subgames)
										:OutputParamsDialog("LCP Params",parent),SeqFormParamsSettings()

{
Form()->Add(wxMakeFormBool("All Solutions",&dup_strat));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("Max depth",&maxdepth));

// Now add the basic stuff
MakeOutputFields(OUTPUT_FIELD|MAXSOLN_FIELD| ((subgames) ? SPS_FIELD : 0));
Go();
}

#endif

#endif
