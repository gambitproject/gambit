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

#define SOLN_SECT           "Soln-Defaults"

//========================================================================
//            OutputParamsDialog: Member function definitions
//========================================================================

OutputParamsDialog::OutputParamsDialog(const gText &label,
                                       wxWindow *parent, 
                                       const char */*help_str*/)
  : wxDialogBox(parent, label, TRUE), m_depthChoice(0), m_typeChoice(0),
    m_methodChoice(0), m_markSubgames(0)
{ }

OutputParamsDialog::~OutputParamsDialog(void)
{ }

void OutputParamsDialog::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void OutputParamsDialog::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool OutputParamsDialog::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void OutputParamsDialog::DominanceFields(bool p_mixed)
{
  (void) new wxMessage(this, "Dominance elimination:");
  NewLine();
  
  char *depthChoices[] = { "None", "Once", "Iterative" };
  m_depthChoice = new wxRadioBox(this, 0, "Depth", -1, -1, -1, -1,
				 3, depthChoices);
  NewLine();

  char *typeChoices[] = { "Weak", "Strong" };
  m_typeChoice = new wxRadioBox(this, 0, "Type", -1, -1, -1, -1,
				2, typeChoices);
  NewLine();

  if (p_mixed) {
    char *methodChoices[] = { "Pure", "Mixed" };
    m_methodChoice = new wxRadioBox(this, 0, "Method", -1, -1, -1, -1,
				    2, methodChoices);
    NewLine();
  }
}

void OutputParamsDialog::SubgameFields(void)
{
  (void) new wxMessage(this, "Subgames:");
  NewLine();

  m_markSubgames = new wxCheckBox(this, 0, "Mark subgames before solving");
  NewLine();
  m_selectSolutions = new wxCheckBox(this, 0,
				     "Interactively select subgame solutions");
  NewLine();
}

void OutputParamsDialog::MakeCommonFields(bool p_dominance, bool p_subgames,
					  bool/* p_vianfg*/)
{
  if (p_dominance)   DominanceFields(false);
  if (p_subgames)    SubgameFields();
  AlgorithmFields();

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);
}

void OutputParamsDialog::MakeOutputFields(unsigned int fields)
{
  /*
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
    Add(wxMakeFormBool("Select Solutions", &m_select, wxFORM_DEFAULT,
		       NULL, NULL, wxVERTICAL, 100));
    Add(wxMakeFormNewLine());
  }

  if ((fields & MAXSOLN_FIELD) && (fields & SPS_FIELD)) {
    Add(wxMakeFormShort("Solns/Subgame", &m_stopAfter, wxFORM_DEFAULT, 
            NULL, NULL, wxVERTICAL, 100));
    Add(wxMakeFormShort("Max Ttl Solns", &m_maxSolns, wxFORM_DEFAULT, 
            NULL, NULL, wxVERTICAL, 100));
    Add(wxMakeFormNewLine());
    Add(wxMakeFormBool("Select Solutions", &m_select, wxFORM_DEFAULT,
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
  */
}

//=======================================================================
//                class PxiParamsDialog: Member functions
//=======================================================================

// Constructor
PxiParamsDialog::PxiParamsDialog(const char *alg, const char *label, 
                                 const char *fn, wxWindow *parent,
                                 const char *help_str)
  : OutputParamsDialog(label, parent, help_str)
{ }

// Make Pxi Fields
void PxiParamsDialog::MakePxiFields(void)
{
  /*
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
  */
}

// Destructor
PxiParamsDialog::~PxiParamsDialog() { }

