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
//            dialogAlgorithm: Member function definitions
//========================================================================

dialogAlgorithm::dialogAlgorithm(const gText &label,
                                       wxWindow *parent, 
                                       const char */*help_str*/)
  : wxDialogBox(parent, label, TRUE), m_depthChoice(0), m_typeChoice(0),
    m_methodChoice(0), m_markSubgames(0)
{ }

dialogAlgorithm::~dialogAlgorithm(void)
{ }

void dialogAlgorithm::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogAlgorithm::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogAlgorithm::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void dialogAlgorithm::DominanceFields(bool p_mixed)
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

void dialogAlgorithm::SubgameFields(void)
{
  (void) new wxMessage(this, "Subgames:");
  NewLine();

  m_markSubgames = new wxCheckBox(this, 0, "Mark subgames before solving");
  NewLine();
  m_selectSolutions = new wxCheckBox(this, 0,
				     "Interactively select subgame solutions");
  NewLine();
}

void dialogAlgorithm::MakeCommonFields(bool p_dominance, bool p_subgames,
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

//=======================================================================
//                class PxiParamsDialog: Member functions
//=======================================================================

// Constructor
PxiParamsDialog::PxiParamsDialog(const char *alg, const char *label, 
                                 const char *fn, wxWindow *parent,
                                 const char *help_str)
  : dialogAlgorithm(label, parent, help_str)
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



//=======================================================================
//                   dialogEnumPure: Member functions
//=======================================================================

#include "dlenumpure.h"

dialogEnumPure::dialogEnumPure(wxWindow *p_parent, bool p_subgames,
			       bool p_vianfg)
  : dialogAlgorithm("EnumPureSolve Parameters", p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

dialogEnumPure::~dialogEnumPure()
{ }

void dialogEnumPure::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters:");
  m_stopAfter = new wxIntegerItem(this, "Stop after");
  NewLine();
}

int dialogEnumPure::StopAfter(void) const
{ return (int) ToDouble(m_stopAfter->GetValue()); }
