//
// FILE: algdlgs.cc -- Classes that are often used for the algorithm
//                     parameter dialogs
//
// $Id$
//

#include "gstream.h"
#include "wxio.h"
#include "algdlgs.h"

// Need this function since the default wxGetResource takes a char **value,
// and replaces it with a newly created string (while deleting the old one).
// This is NOT what we want.

Bool wxGetResourceStr(char *section, char *entry, char *value, char *file)
{
  char *tmp_str = 0;
  Bool ok = wxGetResource(section, entry, &tmp_str, file);

  if (ok) {
    strcpy(value, tmp_str);
    delete [] tmp_str;
  }
  
  return ok;
}

//========================================================================
//            OutputParamsSettings: Member function definitions
//========================================================================

#define SOLN_SECT           "Soln-Defaults"

OutputParamsSettings::OutputParamsSettings(void)
{
  defaults_file = "gambit.ini";
  
  // read in the defaults
  outname = new char[250];
  wxGetResourceStr(PARAMS_SECTION, "Trace-Out", outname, defaults_file);
  errname = new char[250];
  wxGetResourceStr(PARAMS_SECTION, "Trace-Err", errname, defaults_file);
  trace_str = new char[10];
  wxGetResourceStr(PARAMS_SECTION, "Trace-Level", trace_str, defaults_file);
  wxGetResource(PARAMS_SECTION, "Stop-After", &m_stopAfter, defaults_file);
  wxGetResource(PARAMS_SECTION, "Max-Solns", &m_maxSolns, defaults_file);
  m_precisionStr = new char[10];
  wxGetResourceStr(PARAMS_SECTION, "Precision", m_precisionStr,
           defaults_file);
  trace_list = wxStringListInts(4, NULL, 0);
  m_precisionList = new wxStringList;
  m_precisionList->Add("Float");
  m_precisionList->Add("Rational");
  outfile = 0;
  errfile = 0;

  m_domDepthStr = new char[20];
  wxGetResourceStr(SOLN_SECT,"Nfg-ElimDom-Depth", m_domDepthStr,
           defaults_file);
  wxGetResource(SOLN_SECT,"Nfg-ElimDom-Type",&dom_type,defaults_file);
  wxGetResource(SOLN_SECT,"Nfg-ElimDom-Method",&dom_method,defaults_file);

  wxGetResource(SOLN_SECT, "Efg-Mark-Subgames", &markSubgames, defaults_file);
}

OutputParamsSettings::~OutputParamsSettings(void)
{
  SaveDefaults();
  delete [] outname;
  delete [] errname;
  delete [] trace_str;
  delete [] m_domDepthStr;
  delete trace_list;

  delete [] m_precisionStr;
  delete m_precisionList;

  if (outfile && outfile != wout && outfile != &gnull) 
    delete outfile;

  if (errfile && errfile != wout && errfile != &gnull) 
    delete errfile;
}

void OutputParamsSettings::SaveDefaults(void)
{
  wxWriteResource(PARAMS_SECTION, "Trace-Out", outname, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Trace-Err", errname, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Trace-Level", trace_str, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Stop-After", m_stopAfter, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Max-Solns", m_maxSolns, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Precision", m_precisionStr, defaults_file);

  wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Depth",m_domDepthStr,defaults_file);
  wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Type",dom_type,defaults_file);
  wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Method",dom_method,defaults_file);

  wxWriteResource(SOLN_SECT, "Efg-Mark-Subgames", markSubgames,
          defaults_file);
}

//
// Make output file.  If a real file was created, it is saved in outp,
// if the default output window is used outp is not modified.
//
gOutput *OutputParamsSettings::MakeOutputFile(const char *s,
                          gOutput *&outp) const
{
  if (!s) 
    return &gnull;

  if (strcmp(s, "gnull") == 0) 
    return &gnull;

  if (strcmp(s, gWXOUT) == 0) {
    if (!wout) 
      wout = new gWxOutput(gWXOUT);
    
    return wout;
  }
  else {
    outp = new gFileOutput(s);
    return outp;
  }
}

gOutput *OutputParamsSettings::OutFile(void) const
{
  if (strcmp(trace_str, "0") != 0) 
    return MakeOutputFile(outname, outfile);
  else 
    return 0;
}

gOutput *OutputParamsSettings::ErrFile(void) const
{
  if (strcmp(trace_str, "0") != 0) 
    return MakeOutputFile(errname, errfile);
  else 
    return 0;
}

int OutputParamsSettings::TraceLevel(void) const
{
  return wxListFindString(trace_list, trace_str);
}

gPrecision OutputParamsSettings::Precision(void) const
{
  return ((wxListFindString(m_precisionList, m_precisionStr) == 1) ?
      precRATIONAL : precDOUBLE);
}


//========================================================================
//            OutputParamsDialog: Member function definitions
//========================================================================

OutputParamsDialog::OutputParamsDialog(const char *label, 
                                       wxWindow *parent, 
                                       const char *help_str) 
  : MyDialogBox(parent, (char *)label, help_str)
{ }

OutputParamsDialog::~OutputParamsDialog(void)
{ }

void OutputParamsDialog::MakeCommonFields(bool p_dominance, bool p_subgames)
{
  if (p_dominance) {
    Add(wxMakeFormMessage("Dominance elimination:"));
    Add(wxMakeFormNewLine());

    m_domDepthList = new wxStringList("None", "Once", "Iterative", 0);
    Add(wxMakeFormString("Depth", &m_domDepthStr, wxFORM_RADIOBOX,
             new wxList(wxMakeConstraintStrings(m_domDepthList),
                    0), 0, wxVERTICAL));
    Add(wxMakeFormNewLine());

    dom_type_list = new wxStringList("Weak", "Strong", 0);
    dom_type_str = new char[20];
    strcpy(dom_type_str, (char *) dom_type_list->Nth(dom_type)->Data());
    Add(wxMakeFormString("Type", &dom_type_str, wxFORM_RADIOBOX,
             new wxList(wxMakeConstraintStrings(dom_type_list), 0),
             0, wxVERTICAL));
  
    dom_method_list = new wxStringList("Pure", "Mixed", 0);
    dom_method_str = new char[20];
    strcpy(dom_method_str, (char *) dom_method_list->Nth(dom_method)->Data());
    Add(wxMakeFormString("Method", &dom_method_str, wxFORM_RADIOBOX,
             new wxList(wxMakeConstraintStrings(dom_method_list),
                    0), 0, wxVERTICAL));
  
    Add(wxMakeFormNewLine());
  }
  
  if (p_subgames) {
    Add(wxMakeFormMessage("Subgames:"));
    Add(wxMakeFormNewLine());
    Add(wxMakeFormBool("Mark subgames before solving", &markSubgames));
    Add(wxMakeFormNewLine());
  }

  Add(wxMakeFormMessage("Algorithm behavior:"));
  Add(wxMakeFormNewLine());
}  

void OutputParamsDialog::MakeOutputFields(unsigned int fields)
{
  Add(wxMakeFormNewLine());
  if (fields & PRECISION_FIELD) {
    Add(wxMakeFormString("Precision", &m_precisionStr, wxFORM_RADIOBOX,
             new wxList(wxMakeConstraintStrings(m_precisionList),
                    0), 
             0, wxVERTICAL));
    Add(wxMakeFormNewLine());
  }

  if ((fields & MAXSOLN_FIELD) && !(fields & SPS_FIELD)) {
    Add(wxMakeFormShort("Max Ttl Solns", &m_stopAfter, wxFORM_DEFAULT, 
            NULL, NULL, wxVERTICAL, 100));
    Add(wxMakeFormNewLine());
  }

  if ((fields & MAXSOLN_FIELD) && (fields & SPS_FIELD)) {
    Add(wxMakeFormShort("Solns/Subgame", &m_stopAfter, wxFORM_DEFAULT, 
            NULL, NULL, wxVERTICAL, 100));
    Add(wxMakeFormShort("Max Ttl Solns", &m_maxSolns, wxFORM_DEFAULT, 
            NULL, NULL, wxVERTICAL, 100));
    Add(wxMakeFormNewLine());
  }

  if (fields & OUTPUT_FIELD) {
    Add(wxMakeFormString("TraceFile", &outname, wxFORM_DEFAULT, 
             NULL, NULL, wxVERTICAL));
  }

  if (fields & ERROR_FIELD) {
    Add(wxMakeFormString("ErrFile", &errname, wxFORM_DEFAULT, 
              NULL, NULL, wxVERTICAL));
  }

  if ((fields & ERROR_FIELD) && (fields & OUTPUT_FIELD)) { 
    Add(wxMakeFormNewLine());
  }

  Add(wxMakeFormString("Trace Level", &trace_str, wxFORM_CHOICE,
               new wxList(wxMakeConstraintStrings(trace_list), 0), 
               0, wxVERTICAL));
  Add(wxMakeFormNewLine());
}


//****************************  PXI PARAMS **************************************
#define DEFAULT_NAME    0
#define SAVED_NAME      1
#define PROMPT_NAME     2

int PxiParamsSettings::naming_option = 0;

PxiParamsSettings::PxiParamsSettings(const char *alg, const char *fn)
{
  algname = new char[20];
  strcpy(algname, ((alg) ? alg : "Pxi"));
  filename = new char[250];
  strcpy(filename, fn);
  type_str = new char[20];
  char tmp_str[100];
  sprintf(tmp_str, "%s-Plot-Type", algname);
  wxGetResourceStr(PARAMS_SECTION, tmp_str, type_str, defaults_file);
  pxiname = new char[250];
  
  if (naming_option == SAVED_NAME)
    wxGetResourceStr(PARAMS_SECTION, "Pxi-Saved-Name", pxiname, defaults_file);
  
  if (naming_option == DEFAULT_NAME)
    strcpy(pxiname, filename);

  if (naming_option == PROMPT_NAME) {
    if (FromDialog()) { // will be set later.  this is just the dialog settings
      strcpy(pxiname, "PROMPT");
    }
    else {
      char *s = wxGetTextFromUser("Pxi data output file", 
				  "Enter File Name", filename);
      
      if (!s) 
	s = "pxi.out";

      strcpy(pxiname, s);
    }
  }

  name_option_str = new char[20];
  pxi_command = new char[250];
  wxGetResourceStr(PARAMS_SECTION, "Pxi-Command", pxi_command, defaults_file);
  wxGetResource(PARAMS_SECTION, "Run-Pxi", &run_pxi, defaults_file);
  
  type_list = new wxStringList("Lin", "Log", 0);
  name_option_list = new wxStringList("Default", "Saved", "Prompt", 0);
  strcpy(name_option_str, (char *)name_option_list->Nth(naming_option)->Data());
  
  pxifile = 0;
}


// Pxi File
gOutput *PxiParamsSettings::PxiFile(void)
{
    return MakeOutputFile(pxiname, pxifile);
}


// Pxi Type
int PxiParamsSettings::PxiType(void)
{
    return wxListFindString(type_list, type_str);
}


// Run Pxi
int PxiParamsSettings::RunPxi(void)
{
    if (pxifile && run_pxi && pxi_command && pxiname)
    {
        // first of all, close the pxifile
        delete pxifile;
        pxifile = 0;

        gText pxi_run(pxi_command);
        pxi_run += " ";
        // if the filename includes path, use it, if not, append current dir
        char *fname = copystring(FileNameFromPath(pxiname));
        gText pxifilename;

        if (strcmp(fname, pxiname) == 0)
        {
            pxifilename = wxGetWorkingDirectory();
#ifdef wx_x
            pxifilename += '/';
#else
            pxifilename += '\\';
#endif
            pxifilename += pxiname;
        }
        else
        {
            pxifilename = pxiname;
        }

        pxi_run += pxifilename;
        delete [] fname;
        Bool ok = wxExecute((char *)pxi_run);

        if (!ok) wxMessageBox("PXI could not run!\nIt was either not found or\n"
                              " there was insufficient memory/resources.\n"
                              "Please check the path and free memory.", 
                              "PXI Error");
        return ok;
    }
    else
    {
        return 0;
    }
}


// Save Defaults
void PxiParamsSettings::SaveDefaults(void)
{
    wxWriteResource(PARAMS_SECTION, "Pxi-Command", pxi_command, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Pxi-Saved-Name", pxiname, defaults_file);
    naming_option = wxListFindString(name_option_list, name_option_str);
    char tmp_str[100];
    sprintf(tmp_str, "%s-Plot-Type", algname);
    wxWriteResource(PARAMS_SECTION, tmp_str, type_str, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Run-Pxi", run_pxi, defaults_file);
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

    if (pxifile) 
        delete pxifile;
    // could have been deleted in RunPxi
};



// Constructor
PxiParamsDialog::PxiParamsDialog(const char *alg, const char *label, 
                                 const char *fn, wxWindow *parent,
                                 const char *help_str)
    : OutputParamsDialog(label, parent, help_str), PxiParamsSettings(alg, fn)
{ }

// Make Pxi Fields
void PxiParamsDialog::MakePxiFields(void)
{
    Form()->Add(wxMakeFormNewLine());

    Form()->Add(wxMakeFormString("Plot Type", &type_str, wxFORM_RADIOBOX,
                                 new wxList(wxMakeConstraintStrings(type_list), 0)));

    Form()->Add(wxMakeFormString("PxiFile", &pxiname, 
                                 wxFORM_DEFAULT, NULL, NULL, 0,
                                 300));

    Add(wxMakeFormString("Next", &name_option_str, wxFORM_RADIOBOX,
                         new wxList(wxMakeConstraintStrings(name_option_list), 0)));
    Form()->Add(wxMakeFormNewLine());

    Form()->Add(wxMakeFormBool("Run PXI", &run_pxi));

    Form()->Add(wxMakeFormString("PXI Command", &pxi_command,
                                 wxFORM_DEFAULT, NULL, NULL, 0,
                                 300));
}

// Destructor
PxiParamsDialog::~PxiParamsDialog() { }

