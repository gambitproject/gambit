//
// FILE: algdlgs.cc -- Classes that are often used for the algorithm
//                     parameter dialogs
//
// $Id$
//

#include "gstream.h"
#include "system.h"
#include "wxio.h"
#include "gambit.h"
#include "algdlgs.h"
#include "wxmisc.h"

#include "nfgconst.h"

// Need this function since the default wxGeResource takes a char **value,
// and replaces it with a newly created string (while deleting the old one).
// This is NOT what we want.

Bool wxGetResourceStr(char *section, char *entry, gText &value, char *file)
{
  char *tmp_str = 0;
  Bool ok = wxGetResource(section, entry, &tmp_str, file);

  if (ok) {
    value = tmp_str;
    delete [] tmp_str;
  }

  return ok;
}

//========================================================================
//            dialogAlgorithm: Member function definitions
//========================================================================

dialogAlgorithm::dialogAlgorithm(const gText &p_label, bool p_usesNfg,
				 wxWindow *p_parent,
				 const char */*help_str*/)
  : wxDialogBox(p_parent, p_label, TRUE),
    m_usesNfg(p_usesNfg),
    m_dominanceGroup(0), m_subgamesGroup(0), m_algorithmGroup(0),
    m_traceGroup(0), m_depthChoice(0), m_typeChoice(0),
    m_methodChoice(0), m_markSubgames(0),
    m_stopAfter(0), m_findAll(0), m_precision(0)
{ }

dialogAlgorithm::~dialogAlgorithm(void)
{
  if (m_completed == wxOK) {
    if (m_usesNfg) {
      wxWriteResource("Soln-Defaults", "Nfg-ElimDom-Depth",
		      m_depthChoice->GetSelection(), gambitApp.ResourceFile());
      if (m_depthChoice->GetSelection() != 0) {
	wxWriteResource("Soln-Defaults", "Nfg-ElimDom-Type",
			m_typeChoice->GetSelection(), gambitApp.ResourceFile());
	wxWriteResource("Soln-Defaults", "Nfg-ElimDom-Method",
			m_methodChoice->GetSelection(), gambitApp.ResourceFile());
      }
    }
    else {
      wxWriteResource("Soln-Defaults", "Efg-ElimDom-Depth",
		      m_depthChoice->GetSelection(), gambitApp.ResourceFile());
      if (m_depthChoice->GetSelection() != 0)
	wxWriteResource("Soln-Defaults", "Efg-ElimDom-Type",
			m_typeChoice->GetSelection(), gambitApp.ResourceFile());
    }

    if (m_subgames) {
      wxWriteResource("Soln-Defaults", "Efg-Mark-Subgames",
		      m_markSubgames->GetValue(), gambitApp.ResourceFile());
      wxWriteResource("Soln-Defaults", "Efg-Interactive-Solns",
		      m_selectSolutions->GetValue(), gambitApp.ResourceFile());
    }

    if (m_stopAfter) {
      if (m_findAll->GetValue())
	wxWriteResource("Algorithm Params", "StopAfter", 0, gambitApp.ResourceFile());
      else
	wxWriteResource("Algorithm Params", "StopAfter",
			m_stopAfter->GetInteger(), gambitApp.ResourceFile());
    }

    if (m_precision)
      wxWriteResource("Algorithm Params", "Precision",
		      m_precision->GetSelection(), gambitApp.ResourceFile());
  }
}

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

void dialogAlgorithm::OnDepth(void)
{
  m_typeChoice->Enable(m_depthChoice->GetSelection() > 0);
  if (m_methodChoice && m_usesNfg)
    m_methodChoice->Enable(m_depthChoice->GetSelection() > 0);
}

void dialogAlgorithm::OnAll(void)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

void dialogAlgorithm::OnTrace(void)
{
  m_traceFile->Enable(m_traceDest->GetSelection() == 2);
  m_traceLevel->Enable(m_traceDest->GetSelection() != 0);
}

void dialogAlgorithm::DominanceFields(bool p_usesNfg)
{
  int depth = 0, type = 0, method = 0;

  SetAutoLayout(TRUE);

  if (p_usesNfg) {
    m_dominanceGroup = new wxGroupBox(this, "Eliminate dominated mixed strategies");
    wxGetResource("Soln-Defaults", "Nfg-ElimDom-Depth", &depth, gambitApp.ResourceFile());
    wxGetResource("Soln-Defaults", "Nfg-ElimDom-Type", &type, gambitApp.ResourceFile());
    wxGetResource("Soln-Defaults", "Nfg-ElimDom-Method", &method,
		  gambitApp.ResourceFile());
  }
  else {
    m_dominanceGroup = new wxGroupBox(this, "Eliminate dominated behavior strategies");
    wxGetResource("Soln-Defaults", "Efg-ElimDom-Depth", &depth, gambitApp.ResourceFile());
    wxGetResource("Soln-Defaults", "Efg-ElimDom-Type", &type, gambitApp.ResourceFile());
  }
  NewLine();

  char *depthChoices[] = { "None", "Once", "Iterative" };
  m_depthChoice = new wxRadioBox(this, (wxFunction) CallbackDepth, "Depth",
				 -1, -1, -1, -1, 3, depthChoices, 0, wxVERTICAL);
  m_depthChoice->SetClientData((char *) this);
  if (depth >= 0 && depth <= 2)
    m_depthChoice->SetSelection(depth);

  char *typeChoices[] = { "Weak", "Strong" };
  m_typeChoice = new wxRadioBox(this, 0, "Type", -1, -1, -1, -1,
				2, typeChoices, 0, wxVERTICAL);
  if (m_depthChoice->GetSelection() == 0)
    m_typeChoice->Enable(FALSE);
  else if (type == 0 || type == 1)
    m_typeChoice->SetSelection(type);

  char *methodChoices[] = { "Pure", "Mixed" };
  m_methodChoice = new wxRadioBox(this, 0, "Method", -1, -1, -1, -1,
 	    2, methodChoices, 0, wxVERTICAL);
  if (m_depthChoice->GetSelection() == 0 || !p_usesNfg)
    m_methodChoice->Enable(FALSE);
  else if (method == 0 || method == 1)
    m_methodChoice->SetSelection(method);

  wxLayoutConstraints *constraints;

  constraints = new wxLayoutConstraints;
  constraints->left.SameAs(m_dominanceGroup, wxLeft, 10);
  constraints->top.SameAs(m_dominanceGroup, wxTop, 20);
  constraints->height.AsIs();
  constraints->width.AsIs();
  m_depthChoice->SetConstraints(constraints);

  constraints = new wxLayoutConstraints;
  constraints->left.SameAs(m_depthChoice, wxRight, 10);
  constraints->top.SameAs(m_depthChoice, wxTop);
  constraints->height.AsIs();
  constraints->width.AsIs();
  m_typeChoice->SetConstraints(constraints);

  constraints = new wxLayoutConstraints;
  constraints->top.SameAs(m_depthChoice, wxTop);
  constraints->left.SameAs(m_typeChoice, wxRight, 10);
  constraints->height.AsIs();
  constraints->width.AsIs();
  m_methodChoice->SetConstraints(constraints);

  constraints = new wxLayoutConstraints;
  constraints->top.SameAs(this, wxTop, 5);
  constraints->left.SameAs(this, wxLeft, 5);
  constraints->bottom.SameAs(m_depthChoice, wxBottom, -10);
  constraints->right.SameAs(m_methodChoice, wxRight, -10);
  m_dominanceGroup->SetConstraints(constraints);

  NewLine();
}

void dialogAlgorithm::SubgameFields(void)
{
  Bool mark = false, select = false;
  m_subgamesGroup = new wxGroupBox(this, "Subgames");
  NewLine();

  wxGetResource("Soln-Defaults", "Efg-Mark-Subgames", &mark, gambitApp.ResourceFile());
  wxGetResource("Soln-Defaults", "Efg-Interactive-Solns", &select,
		gambitApp.ResourceFile());

  m_markSubgames = new wxCheckBox(this, 0, "Mark subgames before solving");
  m_markSubgames->SetValue(mark);
  NewLine();
  m_selectSolutions = new wxCheckBox(this, 0,
				     "Interactively select subgame solutions");
  m_selectSolutions->SetValue(select);
  NewLine();

  wxLayoutConstraints *constraints;

  constraints = new wxLayoutConstraints;
  constraints->left.SameAs(m_subgamesGroup, wxLeft, 10);
  constraints->top.SameAs(m_subgamesGroup, wxTop, 20);
  constraints->height.AsIs();
  constraints->width.AsIs();
  m_markSubgames->SetConstraints(constraints);

  constraints = new wxLayoutConstraints;
  constraints->left.SameAs(m_markSubgames, wxLeft);
  constraints->top.SameAs(m_markSubgames, wxBottom, 10);
  constraints->height.AsIs();
  constraints->width.AsIs();
  m_selectSolutions->SetConstraints(constraints);

  constraints = new wxLayoutConstraints;
  constraints->top.SameAs(m_dominanceGroup, wxBottom, 15);
  constraints->left.SameAs(m_dominanceGroup, wxLeft);
  constraints->bottom.SameAs(m_selectSolutions, wxBottom, -10);
  constraints->right.SameAs(m_selectSolutions, wxRight, -10);
  m_subgamesGroup->SetConstraints(constraints);

}

void dialogAlgorithm::TraceFields(void)
{
  m_traceGroup = new wxGroupBox(this, "Trace options");
  NewLine();
  char *traceChoices[] = { "None", "Window", "File" };
  NewLine();
  m_traceDest = new wxRadioBox(this, (wxFunction) CallbackTrace,
			       "Destination", -1, -1, -1, -1,
			       3, traceChoices);
  m_traceDest->SetClientData((char *) this);
  NewLine();
  m_traceFile = new wxText(this, 0, "Filename");
  m_traceFile->Enable(FALSE);
  m_traceLevel = new wxIntegerItem(this, "Level", 0, -1, -1, 100, -1);
  m_traceLevel->Enable(FALSE);
  NewLine();

  wxLayoutConstraints *constraints;

  constraints = new wxLayoutConstraints;
  constraints->left.SameAs(m_traceGroup, wxLeft, 10);
  constraints->top.SameAs(m_traceGroup, wxTop, 20);
  constraints->height.AsIs();
  constraints->width.AsIs();
  m_traceDest->SetConstraints(constraints);

  constraints = new wxLayoutConstraints;
  constraints->left.SameAs(m_traceDest, wxLeft);
  constraints->top.SameAs(m_traceDest, wxBottom, 10);
  constraints->height.AsIs();
  constraints->right.SameAs(m_traceGroup, wxRight, 10);
  m_traceFile->SetConstraints(constraints);

  constraints = new wxLayoutConstraints;
  constraints->left.SameAs(m_traceFile, wxLeft);
  constraints->top.SameAs(m_traceFile, wxBottom, 10);
  constraints->height.AsIs();
  constraints->right.SameAs(m_traceGroup, wxRight, 10);
  m_traceLevel->SetConstraints(constraints);

  constraints = new wxLayoutConstraints;
  if (m_algorithmGroup) {
    constraints->top.SameAs(m_algorithmGroup, wxBottom, 15);
  }
  else {
    constraints->top.AsIs();
  }
  constraints->left.SameAs(m_dominanceGroup, wxLeft);
  constraints->bottom.SameAs(m_traceLevel, wxBottom, -10);
  constraints->right.SameAs(m_traceDest, wxRight, -10);
  m_traceGroup->SetConstraints(constraints);

}

void dialogAlgorithm::MakeCommonFields(bool p_dominance, bool p_subgames,
				       bool p_usesNfg)
{
  m_subgames = p_subgames;
  if (p_dominance)   DominanceFields(p_usesNfg);
  if (p_subgames)    SubgameFields();
  AlgorithmFields();
  TraceFields();

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  okButton->SetConstraints(new wxLayoutConstraints);
  okButton->GetConstraints()->left.SameAs(m_traceGroup, wxLeft);
  okButton->GetConstraints()->top.SameAs(m_traceGroup, wxBottom, 10);
  okButton->GetConstraints()->height.AsIs();
  okButton->GetConstraints()->width.AsIs();

  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);
  cancelButton->SetConstraints(new wxLayoutConstraints);
  cancelButton->GetConstraints()->left.SameAs(okButton, wxRight, 10);
  cancelButton->GetConstraints()->top.SameAs(okButton, wxTop);
  cancelButton->GetConstraints()->height.AsIs();
  cancelButton->GetConstraints()->width.AsIs();

  Layout();
  Fit();

}

void dialogAlgorithm::StopAfterField(void)
{
  int stopAfter = 0;
  wxGetResource("Algorithm Params", "StopAfter", &stopAfter, gambitApp.ResourceFile());

  m_findAll = new wxCheckBox(this, (wxFunction) CallbackAll, "Find all");
  m_findAll->SetClientData((char *) this);

  m_stopAfter = new wxIntegerItem(this, "Stop after",
				  (stopAfter > 0) ? stopAfter : 1,
				  -1, -1, -1, -1);

  if (stopAfter == 0) {
    m_findAll->SetValue(true);
    m_stopAfter->Enable(FALSE);
  }
}

void dialogAlgorithm::PrecisionField(void)
{
  int precision = 0;
  wxGetResource("Algorithm Params", "Precision", &precision, gambitApp.ResourceFile());

  char *precisionChoices[] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, 0, "Precision", -1, -1, -1, -1,
			       2, precisionChoices);
  if (precision == 0 || precision == 1)
    m_precision->SetSelection(precision);;
}

gOutput *dialogAlgorithm::TraceFile(void) const
{
  switch (m_traceDest->GetSelection()) {
  case 0:
    return &gnull;
  case 1:
    return new gWxOutput;
  case 2:
    try {
      return new gFileOutput(m_traceFile->GetValue());
    }
    catch (gFileOutput::OpenFailed &E) {
      return &gnull;
    }
  default:
    return &gnull;
  }
}

int dialogAlgorithm::TraceLevel(void) const
{
  if (m_traceDest->GetSelection() == 0)
    return 0;
  else
    return m_traceLevel->GetInteger();
}

//=======================================================================
//                   class dialogPxi: Member functions
//=======================================================================

static char *wxOutputFile(const char *name)
{
  static char t_outfile[250];
  static char slash[2];
  slash[0] = System::Slash();
  slash[1] = '\0';

  if (strstr(slash, name)) {
    strcpy(t_outfile, FileNameFromPath((char *)name)); // strip the path
  }
  else
    strcpy(t_outfile, name);

  char *period = strchr(t_outfile, '.'); // strip the extension
  
  if (period) t_outfile[period-t_outfile] = '\0';
  strcat(t_outfile, ".pxi"); // add a ".pxi" extension
  
  return t_outfile;
}

dialogPxi::dialogPxi(const char *p_label, const char *p_filename,
		     wxWindow *p_parent, const char *p_helpStr)
  : dialogAlgorithm(p_label, false, p_parent, p_helpStr)
{ 
  m_defaultPxiFile = wxOutputFile(p_filename);
}

dialogPxi::~dialogPxi()
{
  if (m_completed == wxOK) {
    wxWriteResource("Algorithm Params", "Pxi-Plot-Type",
		    m_plotType->GetSelection(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Run-Pxi",
		    m_runPxi->GetValue(), gambitApp.ResourceFile());
    if (m_runPxi->GetValue()) 
      wxWriteResource("Algorithm Params", "Pxi-Command",
		      m_pxiCommand->GetValue(), gambitApp.ResourceFile());
  }
}

void dialogPxi::PxiFields(void)
{
  int plotType = 0;
  wxGetResource("Algorithm Params", "Pxi-Plot-Type", &plotType, gambitApp.ResourceFile());
  char *plotTypeChoices[] = { "Log", "Linear" };
  m_plotType = new wxRadioBox(this, 0, "Plot type", -1, -1, -1, -1,
			      2, plotTypeChoices);
  if (plotType == 0 || plotType == 1)
    m_plotType->SetSelection(plotType);
  NewLine();

  m_pxiFile = new wxText(this, 0, "PXI file");
  m_pxiFile->SetValue(m_defaultPxiFile);
  NewLine();

  Bool runPxi = false;
  wxGetResource("Algorithm Params", "Run-Pxi", &runPxi, gambitApp.ResourceFile());
  m_runPxi = new wxCheckBox(this, (wxFunction) CallbackRun, "Run PXI");
  m_runPxi->SetClientData((char *) this);
  m_runPxi->SetValue(runPxi);

  gText pxiCommand;
  wxGetResourceStr("Algorithm Params", "Pxi-Command", pxiCommand, 
		   gambitApp.ResourceFile());
  m_pxiCommand = new wxText(this, 0, "PXI command");
  m_pxiCommand->SetValue(pxiCommand);
  m_pxiCommand->Enable(m_runPxi->GetValue());
  NewLine();
}

void dialogPxi::OnRun(void)
{
  m_pxiCommand->Enable(m_runPxi->GetValue());
}

gOutput *dialogPxi::PxiFile(void) const
{
  try {
    return new gFileOutput(m_pxiFile->GetValue());
  }
  catch (gFileOutput::OpenFailed &E) {
    return 0;
  }
}

//=======================================================================
//                   dialogEnumPure: Member functions
//=======================================================================

#include "dlenumpure.h"

dialogEnumPure::dialogEnumPure(wxWindow *p_parent, bool p_subgames,
			       bool p_vianfg)
  : dialogAlgorithm("EnumPureSolve Parameters", p_vianfg, p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

dialogEnumPure::~dialogEnumPure()
{ }

void dialogEnumPure::AlgorithmFields(void)
{
  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters");
  StopAfterField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_subgamesGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_stopAfter, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_stopAfter, wxBottom, -10);

  m_findAll->SetConstraints(new wxLayoutConstraints);
  m_findAll->GetConstraints()->centreY.SameAs(m_stopAfter, wxCentreY);
  m_findAll->GetConstraints()->left.SameAs(m_algorithmGroup, wxLeft, 10);
  m_findAll->GetConstraints()->width.AsIs();
  m_findAll->GetConstraints()->height.AsIs();
  
  m_stopAfter->SetConstraints(new wxLayoutConstraints);
  m_stopAfter->GetConstraints()->top.SameAs(m_algorithmGroup, wxTop, 20);
  m_stopAfter->GetConstraints()->left.SameAs(m_findAll, wxRight, 10);
  m_stopAfter->GetConstraints()->width.AsIs();
  m_stopAfter->GetConstraints()->height.AsIs();
}

int dialogEnumPure::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return m_stopAfter->GetInteger(); 
}

//=======================================================================
//                   dialogEnumMixed: Member functions
//=======================================================================

#include "dlenummixed.h"

dialogEnumMixed::dialogEnumMixed(wxWindow *p_parent, bool p_subgames)
  : dialogAlgorithm("EnumMixedSolve Parameters", true, p_parent,
		    ENUMMIXED_HELP)
{
  MakeCommonFields(true, p_subgames, true);
  Go();
}

dialogEnumMixed::~dialogEnumMixed()
{ }

void dialogEnumMixed::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters:");
  NewLine();
  StopAfterField();
  NewLine();
  PrecisionField();
  NewLine();
}

int dialogEnumMixed::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return m_stopAfter->GetInteger(); 
}

wxEnumStatus::wxEnumStatus(wxFrame *p_parent)
  : wxStatus(p_parent, "EnumMixedSolve"), pass(0)
{ }

void wxEnumStatus::SetProgress(double p_value)
{
  if (p_value > -.5)  {
    // actually, as long as its not -1.0, but floating point ==
    gauge->SetValue((int) ((p_value + pass) / 3.0 *100.0));
  }
  else {
    pass++;
  }
  wxYield();
}


//=======================================================================
//                        dialogLp: Member functions
//=======================================================================

#include "dllp.h"

dialogLp::dialogLp(wxWindow *p_parent, bool p_subgames, bool p_vianfg)
  : dialogAlgorithm("LpSolve Parameters", p_vianfg, p_parent, LP_HELP)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

dialogLp::~dialogLp()
{ }

void dialogLp::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm Parameters:");
  NewLine();
  PrecisionField();
  NewLine();
}

//=======================================================================
//                       dialogLcp: Member functions
//=======================================================================

#include "dllcp.h"

dialogLcp::dialogLcp(wxWindow *p_parent, bool p_subgames, bool p_vianfg)
  : dialogAlgorithm("LcpSolve Parameters", p_vianfg, p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

dialogLcp::~dialogLcp()
{ }

void dialogLcp::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters");
  NewLine();
  StopAfterField();
  NewLine();
  PrecisionField();
  NewLine();
}

int dialogLcp::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return m_stopAfter->GetInteger(); 
}

//=======================================================================
//                       dialogLiap: Member functions
//=======================================================================

#include "dlliap.h"

dialogLiap::dialogLiap(wxWindow *p_parent, bool p_subgames, bool p_vianfg)
  : dialogAlgorithm("LiapSolve Parameters", p_vianfg, p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

dialogLiap::~dialogLiap()
{
  if (m_completed == wxOK) {
    wxWriteResource("Algorithm Params", "Liap-nTries", NumTries(), 
		    gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Func-tolND", TolND(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Func-tol1D", Tol1D(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Func-maxitsND",
		    MaxitsND(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Func-maxits1D",
		    Maxits1D(), gambitApp.ResourceFile());
  }
}

void dialogLiap::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters:");
  NewLine();

  StopAfterField();
  NewLine();

  int nTries = 0;
  wxGetResource("Algorithm Params", "Liap-nTries", &nTries, gambitApp.ResourceFile());
  m_nTries = new wxIntegerItem(this, "nTries", nTries, -1, -1, 100, -1);
  NewLine();

  int tolND = 8;
  wxGetResource("Algorithm Params", "Func-tolND", &tolND, gambitApp.ResourceFile());
  m_tolND = new wxIntegerItem(this, "Tol n-D: 1.0 e -", tolND, -1, -1, 150, -1);
  int tol1D = 8;
  wxGetResource("Algorithm Params", "Func-tol1D", &tol1D, gambitApp.ResourceFile());
  m_tol1D = new wxIntegerItem(this, "Tol 1-D: 1.0 e -", tol1D, -1, -1, 150, -1);
  NewLine();

  int maxitsND = 0;
  wxGetResource("Algorithm Params", "Func-maxitsND", &maxitsND, gambitApp.ResourceFile());
  m_maxitsND = new wxIntegerItem(this, "Iterations n-D", maxitsND,
				 -1, -1, 150, -1);
  int maxits1D = 0;
  wxGetResource("Algorithm Params", "Func-maxits1D", &maxits1D, gambitApp.ResourceFile());
  m_maxits1D = new wxIntegerItem(this, "Iterations 1-D", maxits1D,
				 -1, -1, 150, -1);
  NewLine();

  int startOption = 0;
  wxGetResource("Algorithm Params", "Start-Option", &startOption,
		gambitApp.ResourceFile());
  char *startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, 0, "Start", -1, -1, -1, -1,
				 3, startOptions);
  if (startOption >= 0 && startOption <= 2)
    m_startOption->SetSelection(startOption);
  NewLine();
}

int dialogLiap::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return m_stopAfter->GetInteger(); 
}

//=======================================================================
//                    dialogSimpdiv: Member functions
//=======================================================================

#include "dlsimpdiv.h"

dialogSimpdiv::dialogSimpdiv(wxWindow *p_parent, bool p_subgames)
  : dialogAlgorithm("SimpdivSolve Parameters", true, p_parent, SIMPDIV_HELP)
{
  MakeCommonFields(true, p_subgames, true);
  Go();
}
dialogSimpdiv::~dialogSimpdiv()
{
  if (m_completed == wxOK) {
    wxWriteResource("Algorithm Params", "Simpdiv-nRestarts", NumRestarts(),
		    gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Simpdiv-leashLength", LeashLength(),
		    gambitApp.ResourceFile());
  }
}

void dialogSimpdiv::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters:");
  NewLine();
  StopAfterField();
  NewLine();
  PrecisionField();
  NewLine();

  int nRestarts = 0;
  wxGetResource("Algorithm Params", "Simpdiv-nRestarts", &nRestarts,
		gambitApp.ResourceFile());
  m_nRestarts = new wxIntegerItem(this, "# restarts", nRestarts,
				  -1, -1, 100, -1);

  int leashLength = 0;
  wxGetResource("Algorithm Params", "Simpdiv-leashLength", &leashLength,
		gambitApp.ResourceFile());
  m_leashLength = new wxIntegerItem(this, "Leash length", leashLength,
				    -1, -1, 100, -1);
  NewLine();
}  

int dialogSimpdiv::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return m_stopAfter->GetInteger(); 
}

//=======================================================================
//                    dialogPolEnum: Member functions
//=======================================================================

#include "dlpolenum.h"

dialogPolEnum::dialogPolEnum(wxWindow *p_parent,
			     bool p_subgames, bool p_vianfg)
  : dialogAlgorithm("PolEnumSolve Parameters", p_vianfg, p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

dialogPolEnum::~dialogPolEnum()
{ }

void dialogPolEnum::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters");
  NewLine();
  StopAfterField();
  NewLine();
}

int dialogPolEnum::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return m_stopAfter->GetInteger(); 
}

//=======================================================================
//                      dialogQre: Member functions
//=======================================================================

#include "dlqre.h"

dialogQre::dialogQre(wxWindow *p_parent, const gText &p_filename,
		     bool p_vianfg)
  : dialogPxi("QreSolve Params", p_filename, p_parent, QRE_HELP)
{
  MakeCommonFields(true, false, p_vianfg);
  Go();
}

dialogQre::~dialogQre()
{
  if (m_completed == wxOK) {
    wxWriteResource("Algorithm Params", "Qre-minLam",
		    m_minLam->GetValue(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Qre-maxLam",
		    m_maxLam->GetValue(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Qre-delLam",
		    m_delLam->GetValue(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Func-tolND",
		    m_tolN->GetInteger(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Func-tol1D",
		    m_tol1->GetInteger(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Func-maxitsND",
		    m_maxitsN->GetInteger(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Func-maxits1D",
		    m_maxits1->GetInteger(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Qre-startOption",
		    m_startOption->GetSelection(), gambitApp.ResourceFile());
  }
}

void dialogQre::AlgorithmFields(void)
{
  (void) new wxMessage(this, "Algorithm parameters");
  NewLine();

  gText minLam, maxLam, delLam;
  int tolN, tol1, maxitsN, maxits1;
  wxGetResourceStr("Algorithm Params", "Qre-minLam", minLam, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "Qre-maxLam", maxLam, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "Qre-delLam", delLam, gambitApp.ResourceFile());
  wxGetResource("Algorithm Params", "Func-tolND", &tolN, gambitApp.ResourceFile());
  wxGetResource("Algorithm Params", "Func-tol1D", &tol1, gambitApp.ResourceFile());
  wxGetResource("Algorithm Params", "Func-maxitsND", &maxitsN, gambitApp.ResourceFile());
  wxGetResource("Algorithm Params", "Func-maxits1D", &maxits1, gambitApp.ResourceFile());

  m_minLam = new wxNumberItem(this, "minLam", minLam);
  m_maxLam = new wxNumberItem(this, "maxLam", maxLam);
  m_delLam = new wxNumberItem(this, "delLam", delLam);
  NewLine();

  m_tolN = new wxIntegerItem(this, "Tol n-D: 1.0 e -", tolN);
  m_tol1 = new wxIntegerItem(this, "Tol 1-D: 1.0 e -", tol1);
  NewLine();

  m_maxitsN = new wxIntegerItem(this, "Iterations n-D", maxitsN);
  m_maxits1 = new wxIntegerItem(this, "Iterations 1-D", maxits1);
  NewLine();

  int startOption;
  wxGetResource("Algorithm Params", "Qre-startOption",
		&startOption, gambitApp.ResourceFile());
  char *startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, 0, "Start", -1, -1, -1, -1,
				 3, startOptions);
  if (startOption >= 0 && startOption <= 2)
    m_startOption->SetSelection(startOption);
  NewLine();

  PxiFields();
}  

//=======================================================================
//                    dialogQreGrid: Member functions
//=======================================================================

#include "dlqregrid.h"

dialogQreGrid::dialogQreGrid(wxWindow *p_parent,
			     const gText &p_filename)
  : dialogPxi("QreGridSolve Params", p_filename, p_parent, QRE_HELP)
{
  MakeCommonFields(true, false, true);
  Go();
}

dialogQreGrid::~dialogQreGrid()
{
  if (m_completed == wxOK) {
    wxWriteResource("Algorithm Params", "QreGrid-minLam",
		    (float) m_minLam->GetNumber(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "QreGrid-maxLam",
		    (float) m_maxLam->GetNumber(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "QreGrid-delLam",
		    (float) m_delLam->GetNumber(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Parmas", "QreGrid-delp1",
		    (float) m_delp1->GetNumber(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "QreGrid-delp2",
		    (float) m_delp2->GetNumber(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "QreGrid-tol1",
		    (float) m_tol1->GetNumber(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "QreGrid-tol2",
		    (float) m_tol2->GetNumber(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "QreGrid-startOption",
		    m_startOption->GetSelection(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "QreGrid-multiGrid",
		    m_multiGrid->GetValue(), gambitApp.ResourceFile());
  }
}

void dialogQreGrid::AlgorithmFields(void)
{
  gText minLam, maxLam, delLam, delp1, delp2, tol1, tol2;
  wxGetResourceStr("Algorithm Params", "QreGrid-minLam", minLam, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "QreGrid-maxLam", maxLam, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "QreGrid-delLam", delLam, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Parmas", "QreGrid-delp1", delp1, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "QreGrid-delp2", delp2, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "QreGrid-tol1", tol1, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "QreGrid-tol2", tol2, gambitApp.ResourceFile());

  m_minLam = new wxNumberItem(this, "minLam", minLam);
  m_maxLam = new wxNumberItem(this, "maxLam", maxLam);
  m_delLam = new wxNumberItem(this, "delLam", delLam);
  NewLine();

  m_delp1 = new wxNumberItem(this, "Grid 1 Del", delp1);
  m_tol1 = new wxNumberItem(this, "Grid 1 Tol", tol1);
  NewLine();
  m_delp2 = new wxNumberItem(this, "Grid 2 Del", delp2);
  m_tol2 = new wxNumberItem(this, "Grid 2 Tol", tol2);
  NewLine();

  Bool multiGrid;
  wxGetResource("Algorithm Params", "QreGrid-multiGrid",
		&multiGrid, gambitApp.ResourceFile());
  m_multiGrid = new wxCheckBox(this, 0, "Use MultiGrid");
  m_multiGrid->SetValue(multiGrid);
  NewLine();

  int startOption;
  wxGetResource("Algorithm Params", "Qre-GridstartOption",
		&startOption, gambitApp.ResourceFile());
  char *startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, 0, "Start", -1, -1, -1, -1,
				 3, startOptions);
  if (startOption >= 0 && startOption <= 2)
    m_startOption->SetSelection(startOption);
  NewLine();

  PxiFields();
}

