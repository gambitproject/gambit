//*************************************************************************************
// Classes that are often used for the algorithm params dialogs.
//*************************************************************************************
// $Id$
//

#include "zfortify.hpp"
#include "gambitio.h"
#include "wxio.h"
#include "outprm.h"
//*******************************	 OUTPUT PARAMS ************************************

// Make Output Fields
void OutputParamsDialog::MakeOutputFields(unsigned int fields)
{
Form()->Add(wxMakeFormNewLine());
if (fields&OUTPUT_FIELD)
{
	outname=new char[250];strcpy(outname,"wout");
	Add(wxMakeFormString("OutFile",&outname,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
}
if (fields&ERROR_FIELD)
{
	errname=new char[250];strcpy(errname,"wout");
	Add(wxMakeFormString("ErrFile",&errname,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
}
if (fields&ERROR_FIELD && fields&OUTPUT_FIELD) Add(wxMakeFormNewLine());

trace_list=wxStringListInts(3);trace_str=new char[10];strcpy(trace_str,"0");
Add(wxMakeFormString("Trace Level",&trace_str,wxFORM_CHOICE, new wxList(wxMakeConstraintStrings(trace_list), 0)));
}
// Constructor
OutputParamsDialog::OutputParamsDialog(const char *label,wxWindow *parent) :
	MyDialogBox(parent,(char *)label),outname(0),errname(0),outfile(0),errfile(0),trace(0),trace_str(0)
{ }

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
// Output Params Dialog Destructor
OutputParamsDialog::~OutputParamsDialog(void)
{
if (outname) delete [] outname;
if (errname) delete [] errname;
if (trace_str) delete [] trace_str;
}


//****************************	PXI PARAMS **************************************

// Constructor
PxiParamsDialog::PxiParamsDialog(const char *alg,const char *label,wxWindow *parent)
		:OutputParamsDialog(label,parent),algname((char *)alg),pxiname(0),pxifile(0),pxi_command(0),run_pxi(FALSE),type(0),type_str(0)
{ }
// Make Pxi Fields
void PxiParamsDialog::MakePxiFields(void)
{
pxiname=new char[250];pxi_command=new char[250];
if (!algname) algname="pxi";
pxiname=wxOutputFile(algname);
strcpy(pxi_command,"pxi");wxGetResource("Gambit","Pxi-Command",&pxi_command,"gambit.ini");

Form()->Add(wxMakeFormNewLine());
type_list=new wxStringList;type_str=new char[20];
type_list->Add("Lin");type_list->Add("Log");strcpy(type_str,"Log");
Form()->Add(wxMakeFormString("Plot Type",&type_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(type_list), 0)));
Form()->Add(wxMakeFormString("PxiFile",&pxiname));
Form()->Add(wxMakeFormNewLine());
run_pxi=FALSE;
Form()->Add(wxMakeFormBool("Run PXI",&run_pxi));
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
	return wxExecute((char *)pxi_run);
}
else
	return 0;
}

// Pxi Params Dialog Destructor
PxiParamsDialog::~PxiParamsDialog(void)
{
if (pxiname) delete [] pxiname;
if (type_str) delete [] type_str;
if (pxi_command)
{
	wxWriteResource("Gambit","Pxi-Command",pxi_command,"gambit.ini");
	delete [] pxi_command;
}
if (type_list) delete type_list;
if (pxifile) delete pxifile;
};


