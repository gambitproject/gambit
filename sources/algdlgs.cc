//*************************************************************************************
// Classes that are often used for the algorithm params dialogs.
//*************************************************************************************
// @(#)algdlgs.cc	1.4 7/5/95
//

#include "gambitio.h"
#include "wxio.h"
#include "algdlgs.h"

//*******************************	 OUTPUT PARAMS ************************************

// Make Output Fields
void OutputParamsDialog::MakeOutputFields(unsigned int fields)
{
Form()->Add(wxMakeFormNewLine());
if (fields&OUTPUT_FIELD)
{
	outname=new char[250];strcpy(outname,"wout");
	wxGetResource(PARAMS_SECTION,"Trace-Out",&outname,defaults_file);
	Add(wxMakeFormString("traceFile",&outname,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
}
if (fields&ERROR_FIELD)
{
	errname=new char[250];strcpy(errname,"wout");
	wxGetResource(PARAMS_SECTION,"Trace-Err",&errname,defaults_file);
	Add(wxMakeFormString("ErrFile",&errname,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
}
if (fields&ERROR_FIELD && fields&OUTPUT_FIELD) Add(wxMakeFormNewLine());

trace_list=wxStringListInts(4);trace_str=new char[10];strcpy(trace_str,"0");
wxGetResource(PARAMS_SECTION,"Trace-Level",&trace_str,defaults_file);
Add(wxMakeFormString("Trace Level",&trace_str,wxFORM_CHOICE,
				new wxList(wxMakeConstraintStrings(trace_list), 0),0,wxVERTICAL));
Add(wxMakeFormNewLine());
def=TRUE;
Add(wxMakeFormBool("Save as default",&def));
}
// Constructor
OutputParamsDialog::OutputParamsDialog(const char *label,wxWindow *parent,const char *help_str) :
	MyDialogBox(parent,(char *)label,help_str),outname(0),errname(0),outfile(0),
	errfile(0),trace(0),trace_str(0)
{defaults_file="gambit.ini";}

// Out File
gOutput *OutputParamsDialog::OutFile(void)
{
if (trace && outname)
{
	if (strcmp(outname,gWXOUT)==0 && wout==0) wout=new gWxOutput(gWXOUT);
	if (strcmp(outname,gWXOUT)==0)
		return wout;
	else
		return (new gFileOutput(outname));
}
else
	return &gnull;
}

// Err File
gOutput *OutputParamsDialog::ErrFile(void)
{
if (trace && errname)
{
	if (strcmp(errname,gWXOUT)==0 && wout==0) wout=new gWxOutput(gWXOUT);
	if (strcmp(errname,gWXOUT)==0)
		return wout;
	else
		return (new gFileOutput(outname));
}
else
	return &gnull;
}

// Trace Level
int OutputParamsDialog::TraceLevel(void)
{
if (trace_str) trace=wxListFindString(trace_list,trace_str);
return trace;
}

// Default
bool OutputParamsDialog::Default(void)
{return ((def) ? true : false);}

// Save Defaults
void OutputParamsDialog::SaveDefaults(void)
{
if (!Default()) return;
if (outname) wxWriteResource(PARAMS_SECTION,"Trace-Out",outname,defaults_file);
if (errname) wxWriteResource(PARAMS_SECTION,"Trace-Err",errname,defaults_file);
if (trace_str) wxWriteResource(PARAMS_SECTION,"Trace-Level",trace_str,defaults_file);
}

// Output Params Dialog Destructor
OutputParamsDialog::~OutputParamsDialog(void)
{
SaveDefaults();
if (outname) delete [] outname;
if (errname) delete [] errname;
if (trace_str) delete [] trace_str;
}


//****************************	PXI PARAMS **************************************

// Constructor
PxiParamsDialog::PxiParamsDialog(const char *alg,const char *label,wxWindow *parent,const char *help_str)
		:OutputParamsDialog(label,parent,help_str),pxi_command(0),pxiname(0),algname((char *)alg),pxifile(0),run_pxi(FALSE),type(0),type_str(0)
{ }
// Make Pxi Fields
void PxiParamsDialog::MakePxiFields(void)
{
Form()->Add(wxMakeFormNewLine());

type_list=new wxStringList;type_str=new char[20];
type_list->Add("Lin");type_list->Add("Log");strcpy(type_str,"Log");
char tmp_str[100];sprintf(tmp_str,"%s-Plot-Type",algname);
wxGetResource(PARAMS_SECTION,tmp_str,&type_str,defaults_file);
Form()->Add(wxMakeFormString("Plot Type",&type_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(type_list), 0)));

pxiname=new char[250];if (!algname) algname="pxi";pxiname=wxOutputFile(algname);
Form()->Add(wxMakeFormString("PxiFile",&pxiname));
Form()->Add(wxMakeFormNewLine());

run_pxi=FALSE;
Form()->Add(wxMakeFormBool("Run PXI",&run_pxi));

pxi_command=new char[250];strcpy(pxi_command,"pxi");
wxGetResource(PARAMS_SECTION,"Pxi-Command",&pxi_command,defaults_file);
Form()->Add(wxMakeFormString("PXI Command",&pxi_command));
}

// Pxi File
gOutput *PxiParamsDialog::PxiFile(void)
{
if (pxiname)
{
	if (strcmp(pxiname,gWXOUT)==0 && wout==0) wout=new gWxOutput(gWXOUT);
	if (strcmp(pxiname,gWXOUT)==0)
		pxifile=wout;
	else
		pxifile=new gFileOutput(pxiname);
}
return pxifile;
}

// Pxi Type
int PxiParamsDialog::PxiType(void)
{
if (type_str)
	type=wxListFindString(type_list,type_str);
return type;
}

// Run Pxi
int PxiParamsDialog::RunPxi(void)
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
void PxiParamsDialog::SaveDefaults(void)
{
if (pxi_command) wxWriteResource(PARAMS_SECTION,"Pxi-Command",pxi_command,defaults_file);
if (type_str)
{
	char tmp_str[100];sprintf(tmp_str,"%s-Plot-Type",algname);
	wxWriteResource(PARAMS_SECTION,tmp_str,type_str,defaults_file);
}
}

// Pxi Params Dialog Destructor
PxiParamsDialog::~PxiParamsDialog(void)
{
SaveDefaults();
if (pxiname) delete [] pxiname;
if (type_str) delete [] type_str;
if (pxi_command) delete [] pxi_command;
if (type_list) delete type_list;
if (pxifile) delete pxifile;
};


