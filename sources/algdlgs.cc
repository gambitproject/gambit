//****************************************************************************
// Classes that are often used for the algorithm params dialogs.
//****************************************************************************
// $Id$
//

#include "gambitio.h"
#include "wxio.h"
#include "algdlgs.h"

//******************	 OUTPUT PARAMS SETTINGS ******************************
OutputParamsSettings::OutputParamsSettings(void)
{
defaults_file="gambit.ini";
// read in the defaults
outname=new char[250];
wxGetResource(PARAMS_SECTION,"Trace-Out",&outname,defaults_file);
errname=new char[250];
wxGetResource(PARAMS_SECTION,"Trace-Err",&errname,defaults_file);
trace_str=new char[10];
wxGetResource(PARAMS_SECTION,"Trace-Level",&trace_str,defaults_file);
wxGetResource(PARAMS_SECTION,"Stop-After",&stopAfter,defaults_file);
wxGetResource(PARAMS_SECTION,"Max-Solns",&max_solns,defaults_file);
trace_list=wxStringListInts(4);
outfile=0;errfile=0;
}

OutputParamsSettings::~OutputParamsSettings(void)
{
SaveDefaults();
delete [] outname;
delete [] errname;
delete [] trace_str;
delete trace_list;
}

void OutputParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Trace-Out",outname,defaults_file);
wxWriteResource(PARAMS_SECTION,"Trace-Err",errname,defaults_file);
wxWriteResource(PARAMS_SECTION,"Trace-Level",trace_str,defaults_file);
wxWriteResource(PARAMS_SECTION,"Stop-After",stopAfter,defaults_file);
wxWriteResource(PARAMS_SECTION,"Max-Solns",max_solns,defaults_file);
}


// Make Output file
gOutput *OutputParamsSettings::MakeOutputFile(const char *s)
{
if (!s) return &gnull;
if (strcmp(s,gWXOUT)==0)
{
	if (!wout) wout=new gWxOutput(gWXOUT);
	return wout;
}
else
	return (new gFileOutput(outname));
}

// Out File
gOutput *OutputParamsSettings::OutFile(void)
{if (strcmp(trace_str,"0")!=0) return MakeOutputFile(outname); else return 0;}

// Err File
gOutput *OutputParamsSettings::ErrFile(void)
{if (strcmp(trace_str,"0")!=0) return MakeOutputFile(errname); else return 0;}

// Trace Level
int OutputParamsSettings::TraceLevel(void)
{return wxListFindString(trace_list,trace_str);}
// Subgame stuff
int OutputParamsSettings::StopAfter(void)
{return stopAfter;}
int OutputParamsSettings::MaxSolns(void)
{return max_solns;}

//******************	 OUTPUT PARAMS DIALOG ********************************
// Constructor
OutputParamsDialog::OutputParamsDialog(const char *label,wxWindow *parent,const char *help_str) :
	OutputParamsSettings(),MyDialogBox(parent,(char *)label,help_str)
{ }

//  Destructor
OutputParamsDialog::~OutputParamsDialog(void)
{ }

// Make Output Fields
// The following fields are available:
// Max Solns --
void OutputParamsDialog::MakeOutputFields(unsigned int fields)
{
Add(wxMakeFormNewLine());
if (fields&MAXSOLN_FIELD && !fields&SPS_FIELD)
{
	Add(wxMakeFormShort("Max Ttl Solns",&stopAfter,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
	Add(wxMakeFormNewLine());
}

if (fields&MAXSOLN_FIELD && fields&SPS_FIELD)
{
	Add(wxMakeFormShort("Solns/Subgame",&stopAfter,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
	Add(wxMakeFormShort("Max Ttl Solns",&max_solns,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
	Add(wxMakeFormNewLine());
}

if (fields&OUTPUT_FIELD)
	Add(wxMakeFormString("TraceFile",&outname,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
if (fields&ERROR_FIELD)
	Add(wxMakeFormString("ErrFile",&errname,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
if (fields&ERROR_FIELD && fields&OUTPUT_FIELD) Add(wxMakeFormNewLine());

Add(wxMakeFormString("Trace Level",&trace_str,wxFORM_CHOICE,
				new wxList(wxMakeConstraintStrings(trace_list), 0),0,wxVERTICAL));
Add(wxMakeFormNewLine());
}


//****************************	PXI PARAMS **************************************
#define	DEFAULT_NAME	0
#define	SAVED_NAME		1
#define	PROMPT_NAME		2
int PxiParamsSettings::naming_option=0;

PxiParamsSettings::PxiParamsSettings(const char *alg,const char *fn):
										OutputParamsSettings()
{
algname=new char[20];
strcpy(algname,((alg) ? alg : "Pxi"));
filename=new char[250];
strcpy(filename,fn);
type_str=new char[20];
char tmp_str[100];sprintf(tmp_str,"%s-Plot-Type",algname);
wxGetResource(PARAMS_SECTION,tmp_str,&type_str,defaults_file);
pxiname=new char[250];
if (naming_option==SAVED_NAME)
	wxGetResource(PARAMS_SECTION,"Pxi-Saved-Name",&pxiname,defaults_file);
if (naming_option==DEFAULT_NAME)
	strcpy(pxiname,wxOutputFile(filename));
if (naming_option==PROMPT_NAME)
	if (FromDialog()) // will be set later.  this is just the dialog settings
		strcpy(pxiname,"PROMPT");
	else
		{
			char *s=wxGetTextFromUser("Pxi data output file","Enter File Name",wxOutputFile(filename));
			if (!s) s="pxi.out";
			strcpy(pxiname,s);
		}
name_option_str=new char[20];
pxi_command=new char[250];
wxGetResource(PARAMS_SECTION,"Pxi-Command",&pxi_command,defaults_file);
wxGetResource(PARAMS_SECTION,"Run-Pxi",&run_pxi,defaults_file);

type_list=new wxStringList("Lin","Log",0);
name_option_list=new wxStringList("Default","Saved","Prompt",0);
strcpy(name_option_str,(char *)name_option_list->Nth(naming_option)->Data());

pxifile=0;
}

// Pxi File
gOutput *PxiParamsSettings::PxiFile(void)
{return MakeOutputFile(pxiname);}

// Pxi Type
int PxiParamsSettings::PxiType(void)
{return	wxListFindString(type_list,type_str);}

// Run Pxi
int PxiParamsSettings::RunPxi(void)
{
if (pxifile && run_pxi && pxi_command && pxiname)
{
// first of all, close the pxifile
	delete pxifile;pxifile=0;

	gString pxi_run(pxi_command);
	pxi_run+=" ";
// if the filename includes path, use it, if not, append current dir
	char *fname=copystring(FileNameFromPath(pxiname));
	gString pxifilename;
	if (strcmp(fname,pxiname)==0)
	{
		pxifilename=wxGetWorkingDirectory();
		#ifdef wx_x
		pxifilename+='/';
		#else
		pxifilename+='\\';
		#endif
		pxifilename+=pxiname;
	}
	else
		pxifilename=pxiname;
	pxi_run+=pxifilename;
	delete [] fname;
	Bool ok=wxExecute((char *)pxi_run);
	if (!ok) wxMessageBox("PXI could not run!\nIt was either not found or\n there was insufficient memory/resources.\nPlease check the path and free memory.","PXI Error");
	return ok;
}
else
	return 0;
}


// Save Defaults
void PxiParamsSettings::SaveDefaults(void)
{
wxWriteResource(PARAMS_SECTION,"Pxi-Command",pxi_command,defaults_file);
wxWriteResource(PARAMS_SECTION,"Pxi-Saved-Name",pxiname,defaults_file);
naming_option=wxListFindString(name_option_list,name_option_str);
char tmp_str[100];sprintf(tmp_str,"%s-Plot-Type",algname);
wxWriteResource(PARAMS_SECTION,tmp_str,type_str,defaults_file);
}

// Pxi Params Settings Destructor
PxiParamsSettings::~PxiParamsSettings(void)
{
SaveDefaults();
delete [] algname;
delete [] filename;
delete [] pxiname;
delete [] type_str;
delete [] pxi_command;
delete [] name_option_str;
delete type_list;
delete name_option_list;
if (pxifile) delete pxifile; // could have been deleted in RunPxi
};



// Constructor
PxiParamsDialog::PxiParamsDialog(const char *alg,const char *label,const char *fn,wxWindow *parent,const char *help_str)
		:OutputParamsDialog(label,parent,help_str),PxiParamsSettings(alg,fn)

{ }
// Make Pxi Fields
void PxiParamsDialog::MakePxiFields(void)
{
Form()->Add(wxMakeFormNewLine());

Form()->Add(wxMakeFormString("Plot Type",&type_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(type_list), 0)));

Form()->Add(wxMakeFormString("PxiFile",&pxiname));

Add(wxMakeFormString("Next",&name_option_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(name_option_list), 0)));
Form()->Add(wxMakeFormNewLine());

Form()->Add(wxMakeFormBool("Run PXI",&run_pxi));

Form()->Add(wxMakeFormString("PXI Command",&pxi_command));
}

// Destructor
PxiParamsDialog::~PxiParamsDialog() { }
