#ifndef ALGDLGS_H
#define ALGDLGS_H

#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"

//*************************************************************************************
// Classes that are often used for the algorithm params dialogs.
//*************************************************************************************
#define PARAMS_SECTION	"Algorithm Params"		// section in .ini file
// This class just creates fields for error/output and a trace level.
// Also creates the option to use the current settings as default
#define		ERROR_FIELD 		1
#define		OUTPUT_FIELD		2
class OutputParamsDialog :public MyDialogBox
{
private:
	char *outname,*errname;
	gOutput *outfile,*errfile;
	wxStringList *trace_list;
	int	trace;
	Bool def;
	char *trace_str;
// Save defaults.  Called automatically in the destructor
	void SaveDefaults(void);
protected:
	char *defaults_file;
public:
// Constructor
	OutputParamsDialog(const char *label=0,wxWindow *parent=0);
// Destructor
	~OutputParamsDialog(void);
// Create the fields
	void MakeOutputFields(unsigned int fields=OUTPUT_FIELD);
// Return the results...
	gOutput *OutFile(void);
	gOutput *ErrFile(void);
	int			TraceLevel(void);
	bool		Default(void);
};


// This class is used for PXI type algorithms.   That is those that generate PXI compatible
// output files.  It creates the fields for output name, date type [Log/Lin], and allows
// one to run PXI right after the algorithm is done (call RunPxi).
class PxiParamsDialog: public OutputParamsDialog
{
private:
	char *pxi_command,*pxiname,*algname;
	gOutput *pxifile;
	Bool run_pxi;
	int type;
	wxStringList *type_list;
	char *type_str;
	void SaveDefaults(void);
public:
// Constructor
	PxiParamsDialog(const char *alg=0,const char *label=0,wxWindow *parent=0);
// Destructor
	~PxiParamsDialog(void);
// Create Fields
	void MakePxiFields(void);
// Return the results...
	gOutput *PxiFile(void);
	int	PxiType(void);
// Run pxi if necessary
	int RunPxi(void);
};
#endif
