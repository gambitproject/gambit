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

//========================================================================
//                  dialogTrace auxiliary dialog class
//========================================================================

class dialogTrace : public guiAutoDialog {
private:
  wxRadioBox *m_traceDest;
  wxText *m_traceFile;
  wxIntegerItem *m_traceLevel;

  static void CallbackTrace(wxRadioBox &p_object, wxEvent &)
    { ((dialogTrace *) p_object.GetClientData())->OnTrace(); }

  void OnTrace(void);

public:
  dialogTrace(wxWindow *p_parent, int p_traceDest, const gText &p_traceFile,
              int p_TraceLevel);
  virtual ~dialogTrace();

  int TraceDest(void) const  { return m_traceDest->GetSelection(); }
  gText TraceFile(void) const  { return m_traceFile->GetValue(); }
  int TraceLevel(void) const { return m_traceLevel->GetInteger(); }
};

dialogTrace::dialogTrace(wxWindow *p_parent,
                         int p_traceDest, const gText &p_traceFile,
                         int p_traceLevel)
  : guiAutoDialog(p_parent, "Tracing Parameters")
{
  char *traceChoices[] = { "None", "Window", "File" };
  m_traceDest = new wxRadioBox(this, (wxFunction) CallbackTrace,
			       "Destination", 1, 1, -1, -1,
			       3, traceChoices);
  m_traceDest->SetSelection(p_traceDest);
  m_traceDest->SetClientData((char *) this);

  m_traceFile = new wxText(this, 0, "Filename", "", 1, 1);
  m_traceFile->SetValue(p_traceFile);
  m_traceFile->Enable(p_traceDest == 2);

  m_traceLevel = new wxIntegerItem(this, "Level", 0, 1, 1, 100, -1);
  m_traceLevel->SetInteger(p_traceLevel);
  m_traceLevel->Enable(p_traceDest > 0);

  m_traceDest->SetConstraints(new wxLayoutConstraints);
  m_traceDest->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_traceDest->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_traceDest->GetConstraints()->width.AsIs();
  m_traceDest->GetConstraints()->height.AsIs();

  m_traceFile->SetConstraints(new wxLayoutConstraints);
  m_traceFile->GetConstraints()->top.SameAs(m_traceDest, wxBottom, 10);
  m_traceFile->GetConstraints()->left.SameAs(m_traceDest, wxLeft);
  m_traceFile->GetConstraints()->width.AsIs();
  m_traceFile->GetConstraints()->height.AsIs();

  m_traceLevel->SetConstraints(new wxLayoutConstraints);
  m_traceLevel->GetConstraints()->top.SameAs(m_traceFile, wxTop);
  m_traceLevel->GetConstraints()->left.SameAs(m_traceFile, wxRight, 10);
  m_traceLevel->GetConstraints()->width.AsIs();
  m_traceLevel->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_okButton->GetConstraints()->top.SameAs(m_traceLevel, wxBottom, 10);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  m_cancelButton->GetConstraints()->left.SameAs(this, wxCentreX, 5);
  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.AsIs();

  m_helpButton->Show(FALSE);

  Go();
}

dialogTrace::~dialogTrace()
{ }

void dialogTrace::OnTrace(void)
{
  m_traceFile->Enable(m_traceDest->GetSelection() == 2);
  m_traceLevel->Enable(m_traceDest->GetSelection() != 0);
}

//========================================================================
//            dialogAlgorithm: Member function definitions
//========================================================================

dialogAlgorithm::dialogAlgorithm(const gText &p_label, bool p_usesNfg,
				 wxWindow *p_parent)
  : guiAutoDialog(p_parent, p_label),
    m_usesNfg(p_usesNfg), m_traceDest(0), m_traceLevel(0), m_traceFile(""),
    m_dominanceGroup(0), m_subgamesGroup(0), m_algorithmGroup(0),
    m_depthChoice(0), m_typeChoice(0),
    m_methodChoice(0), m_markSubgames(0),
    m_stopAfter(0), m_findAll(0), m_precision(0)
{ }

dialogAlgorithm::~dialogAlgorithm(void)
{
  if (Completed() == wxOK) {
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
  dialogTrace dialog(this, m_traceDest, m_traceFile, m_traceLevel);

  if (dialog.Completed() == wxOK)  {
    m_traceLevel = dialog.TraceLevel();
    m_traceFile = dialog.TraceFile();
    m_traceDest = dialog.TraceDest();
  }
}

void dialogAlgorithm::DominanceFields(bool p_usesNfg)
{
  int depth = 0, type = 0, method = 0;

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

  m_depthChoice->SetConstraints(new wxLayoutConstraints);
  m_depthChoice->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft, 10);
  m_depthChoice->GetConstraints()->top.SameAs(m_dominanceGroup, wxTop, 20);
  m_depthChoice->GetConstraints()->height.AsIs();
  m_depthChoice->GetConstraints()->width.AsIs();

  m_typeChoice->SetConstraints(new wxLayoutConstraints);
  m_typeChoice->GetConstraints()->left.SameAs(m_depthChoice, wxRight, 10);
  m_typeChoice->GetConstraints()->top.SameAs(m_depthChoice, wxTop);
  m_typeChoice->GetConstraints()->height.AsIs();
  m_typeChoice->GetConstraints()->width.AsIs();

  m_methodChoice->SetConstraints(new wxLayoutConstraints);
  m_methodChoice->GetConstraints()->top.SameAs(m_depthChoice, wxTop);
  m_methodChoice->GetConstraints()->left.SameAs(m_typeChoice, wxRight, 10);
  m_methodChoice->GetConstraints()->height.AsIs();
  m_methodChoice->GetConstraints()->width.AsIs();

  m_dominanceGroup->SetConstraints(new wxLayoutConstraints);    
  m_dominanceGroup->GetConstraints()->top.SameAs(this, wxTop, 5);
  m_dominanceGroup->GetConstraints()->left.SameAs(this, wxLeft, 5);
  m_dominanceGroup->GetConstraints()->bottom.SameAs(m_depthChoice, wxBottom, -10);
  m_dominanceGroup->GetConstraints()->right.SameAs(m_methodChoice, wxRight, -10);

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

  m_markSubgames->SetConstraints(new wxLayoutConstraints);
  m_markSubgames->GetConstraints()->left.SameAs(m_subgamesGroup, wxLeft, 10);
  m_markSubgames->GetConstraints()->top.SameAs(m_subgamesGroup, wxTop, 20);
  m_markSubgames->GetConstraints()->height.AsIs();
  m_markSubgames->GetConstraints()->width.AsIs();

  m_selectSolutions->SetConstraints(new wxLayoutConstraints);
  m_selectSolutions->GetConstraints()->left.SameAs(m_markSubgames, wxLeft);
  m_selectSolutions->GetConstraints()->top.SameAs(m_markSubgames, wxBottom, 10);
  m_selectSolutions->GetConstraints()->height.AsIs();
  m_selectSolutions->GetConstraints()->width.AsIs();

  m_subgamesGroup->SetConstraints(new wxLayoutConstraints);
  m_subgamesGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  m_subgamesGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_subgamesGroup->GetConstraints()->bottom.SameAs(m_selectSolutions, wxBottom, -10);
  m_subgamesGroup->GetConstraints()->right.SameAs(m_selectSolutions, wxRight, -10);
}

void dialogAlgorithm::MakeCommonFields(bool p_dominance, bool p_subgames,
				       bool p_usesNfg)
{
  m_subgames = p_subgames;
  if (p_dominance)   DominanceFields(p_usesNfg);
  if (p_subgames)    SubgameFields();
  AlgorithmFields();

  wxButton *traceButton = new wxButton(this, (wxFunction) CallbackTrace,
					"Trace...");
  traceButton->SetClientData((char *) this);

  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->top.SameAs(m_algorithmGroup, wxBottom, 10);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(traceButton, wxWidth);

  m_cancelButton->GetConstraints()->right.SameAs(this, wxCentreX, 5);
  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.SameAs(traceButton, wxWidth);

  traceButton->SetConstraints(new wxLayoutConstraints);
  traceButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  traceButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  traceButton->GetConstraints()->height.AsIs();
  traceButton->GetConstraints()->width.AsIs();

  m_helpButton->GetConstraints()->left.SameAs(traceButton, wxRight, 10);
  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->GetConstraints()->width.SameAs(traceButton, wxWidth);
}

void dialogAlgorithm::StopAfterField(void)
{
  int stopAfter = 0;
  wxGetResource("Algorithm Params", "StopAfter", &stopAfter, gambitApp.ResourceFile());

  m_findAll = new wxCheckBox(this, (wxFunction) CallbackAll, "Find all", 1, 1);
  m_findAll->SetClientData((char *) this);

  m_stopAfter = new wxIntegerItem(this, "Stop after",
				  (stopAfter > 0) ? stopAfter : 1,
				  1, 1, -1, -1);

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
  switch (m_traceDest) {
  case 0:
    return &gnull;
  case 1:
    return new gWxOutput;
  case 2:
    try {
      return new gFileOutput(m_traceFile);
    }
    catch (gFileOutput::OpenFailed &E) {
      return &gnull;
    }
  default:
    return &gnull;
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
  NewLine();
  StopAfterField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
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
  : dialogAlgorithm("EnumMixedSolve Parameters", true, p_parent)
{
  MakeCommonFields(true, p_subgames, true);
  Go();
}

dialogEnumMixed::~dialogEnumMixed()
{ }

void dialogEnumMixed::AlgorithmFields(void)
{
  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters");
  NewLine();
  StopAfterField();
  NewLine();
  PrecisionField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_stopAfter, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_precision, wxBottom, -10);

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

  m_precision->SetConstraints(new wxLayoutConstraints);
  m_precision->GetConstraints()->top.SameAs(m_stopAfter, wxBottom, 10);
  m_precision->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_precision->GetConstraints()->width.AsIs();
  m_precision->GetConstraints()->height.AsIs();

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

#include "dlpolenum.h"

wxPolEnumStatus::wxPolEnumStatus(wxFrame *p_parent)
  : wxStatus(p_parent, "PolEnumSolve"), pass(0)
{ }

void wxPolEnumStatus::SetProgress(double p_value)
{
  if (p_value > -.5)  {
    // actually, as long as its not -1.0
    if(pass==0)  // allocate a quarter of the time to enumerating supports
      gauge->SetValue((int)(p_value/4.0  * 100.0));
    else  // and the rest to computing the solutions
      gauge->SetValue((int) ((3.0*p_value + pass) / 4.0 * 100.0));
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
  : dialogAlgorithm("LpSolve Parameters", p_vianfg, p_parent)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  Go();
}

dialogLp::~dialogLp()
{ }

void dialogLp::AlgorithmFields(void)
{
  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters");
  NewLine();
  PrecisionField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_precision, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_precision, wxBottom, -10);

  m_precision->SetConstraints(new wxLayoutConstraints);
  m_precision->GetConstraints()->top.SameAs(m_algorithmGroup, wxTop, 20);
  m_precision->GetConstraints()->left.SameAs(m_algorithmGroup, wxLeft, 10);
  m_precision->GetConstraints()->width.AsIs();
  m_precision->GetConstraints()->height.AsIs();

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
  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters");
  NewLine();
  StopAfterField();
  NewLine();
  PrecisionField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_stopAfter, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_precision, wxBottom, -10);

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

  m_precision->SetConstraints(new wxLayoutConstraints);
  m_precision->GetConstraints()->top.SameAs(m_stopAfter, wxBottom, 10);
  m_precision->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_precision->GetConstraints()->width.AsIs();
  m_precision->GetConstraints()->height.AsIs();

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
  if (Completed() == wxOK) {
    wxWriteResource("Algorithm Params", "Liap-nTries", NumTries(), 
		    gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Liap-accuracy", Accuracy(),
		    gambitApp.ResourceFile());
  }
}

void dialogLiap::AlgorithmFields(void)
{
  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters", 1, 1);
  StopAfterField();

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

  int nTries = 0;
  wxGetResource("Algorithm Params", "Liap-nTries", &nTries, gambitApp.ResourceFile());
  m_nTries = new wxIntegerItem(this, "nTries", nTries, 1, 1);
  m_nTries->SetConstraints(new wxLayoutConstraints);
  m_nTries->GetConstraints()->top.SameAs(m_stopAfter, wxBottom, 10);
  m_nTries->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_nTries->GetConstraints()->width.AsIs();
  m_nTries->GetConstraints()->height.AsIs();
  NewLine();

  int accuracy = 4;
  wxGetResource("Algorithm Params", "Liap-accuracy", 
		&accuracy, gambitApp.ResourceFile());
  m_accuracy = new wxIntegerItem(this, "Accuracy: 1.0 e -", accuracy,
				 1, 1, 150, -1);
  m_accuracy->SetConstraints(new wxLayoutConstraints);
  m_accuracy->GetConstraints()->top.SameAs(m_nTries, wxBottom, 10);
  m_accuracy->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_accuracy->GetConstraints()->width.AsIs();
  m_accuracy->GetConstraints()->height.AsIs();
  NewLine();

  int startOption = 0;
  wxGetResource("Algorithm Params", "Start-Option", &startOption,
		gambitApp.ResourceFile());
  char *startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, 0, "Start", 1, 1, -1, -1,
				 3, startOptions);
  if (startOption >= 0 && startOption <= 2)
    m_startOption->SetSelection(startOption);
  m_startOption->SetConstraints(new wxLayoutConstraints);
  m_startOption->GetConstraints()->top.SameAs(m_accuracy, wxBottom, 10);
  m_startOption->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_startOption->GetConstraints()->width.AsIs();
  m_startOption->GetConstraints()->height.AsIs();
  NewLine();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_stopAfter, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_startOption, wxBottom, -10);
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
  : dialogAlgorithm("SimpdivSolve Parameters", true, p_parent)
{
  MakeCommonFields(true, p_subgames, true);
  Go();
}

dialogSimpdiv::~dialogSimpdiv()
{
  if (Completed() == wxOK) {
    wxWriteResource("Algorithm Params", "Simpdiv-nRestarts", NumRestarts(),
		    gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Simpdiv-leashLength", LeashLength(),
		    gambitApp.ResourceFile());
  }
}

void dialogSimpdiv::AlgorithmFields(void)
{
  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters");
  NewLine();
  StopAfterField();
  NewLine();
  PrecisionField();

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

  m_precision->SetConstraints(new wxLayoutConstraints);
  m_precision->GetConstraints()->top.SameAs(m_stopAfter, wxBottom, 10);
  m_precision->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_precision->GetConstraints()->width.AsIs();
  m_precision->GetConstraints()->height.AsIs();

  NewLine();

  int nRestarts = 0;
  wxGetResource("Algorithm Params", "Simpdiv-nRestarts", &nRestarts,
		gambitApp.ResourceFile());
  m_nRestarts = new wxIntegerItem(this, "# restarts", nRestarts,
				  -1, -1, 150, -1);
  m_nRestarts->SetConstraints(new wxLayoutConstraints);
  m_nRestarts->GetConstraints()->top.SameAs(m_precision, wxBottom, 10);
  m_nRestarts->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_nRestarts->GetConstraints()->width.AsIs();
  m_nRestarts->GetConstraints()->height.AsIs();

  int leashLength = 0;
  wxGetResource("Algorithm Params", "Simpdiv-leashLength", &leashLength,
		gambitApp.ResourceFile());
  m_leashLength = new wxIntegerItem(this, "Leash length", leashLength,
				    -1, -1, 150, -1);
  m_leashLength->SetConstraints(new wxLayoutConstraints);
  m_leashLength->GetConstraints()->top.SameAs(m_nRestarts, wxTop);
  m_leashLength->GetConstraints()->left.SameAs(m_nRestarts, wxRight, 10);
  m_leashLength->GetConstraints()->width.AsIs();
  m_leashLength->GetConstraints()->height.AsIs();
  NewLine();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_leashLength, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_leashLength, wxBottom, -10);
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
  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters");
  NewLine();
  StopAfterField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
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

int dialogPolEnum::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return m_stopAfter->GetInteger(); 
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
    strcpy(t_outfile, gFileNameFromPath((char *)name)); // strip the path
  }
  else
    strcpy(t_outfile, name);

  char *period = strchr(t_outfile, '.'); // strip the extension
  
  if (period) t_outfile[period-t_outfile] = '\0';
  strcat(t_outfile, ".pxi"); // add a ".pxi" extension
  
  return t_outfile;
}

dialogPxi::dialogPxi(const char *p_label, const char *p_filename,
		     wxWindow *p_parent)
  : dialogAlgorithm(p_label, false, p_parent)
{ 
  m_defaultPxiFile = wxOutputFile(p_filename);
}

dialogPxi::~dialogPxi()
{
  if (Completed() == wxOK) {
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
  m_pxiGroup = new wxGroupBox(this, "PXI options");

  m_pxiFile = new wxText(this, 0, "PXI file");
  m_pxiFile->SetValue(m_defaultPxiFile);
  NewLine();

  Bool runPxi = false;
  wxGetResource("Algorithm Params", "Run-Pxi", &runPxi, gambitApp.ResourceFile());
  m_runPxi = new wxCheckBox(this, (wxFunction) CallbackRun, "Run PXI");
  m_runPxi->SetClientData((char *) this);
  m_runPxi->SetValue(runPxi);
  NewLine();

  gText pxiCommand;
  wxGetResourceStr("Algorithm Params", "Pxi-Command", pxiCommand, 
		   gambitApp.ResourceFile());
  m_pxiCommand = new wxText(this, 0, "PXI command");
  m_pxiCommand->SetValue(pxiCommand);
  m_pxiCommand->Enable(m_runPxi->GetValue());
  NewLine();

  m_pxiGroup->SetConstraints(new wxLayoutConstraints);
  m_pxiGroup->GetConstraints()->top.SameAs(m_algorithmGroup, wxBottom, 10);
  m_pxiGroup->GetConstraints()->bottom.SameAs(m_pxiCommand, wxBottom, -10);
  m_pxiGroup->GetConstraints()->left.SameAs(m_algorithmGroup, wxLeft);
  m_pxiGroup->GetConstraints()->right.SameAs(m_algorithmGroup, wxRight);

  m_pxiFile->SetConstraints(new wxLayoutConstraints);
  m_pxiFile->GetConstraints()->top.SameAs(m_pxiGroup, wxTop, 20);
  m_pxiFile->GetConstraints()->left.SameAs(m_pxiGroup, wxLeft, 10);
  m_pxiFile->GetConstraints()->right.SameAs(m_pxiGroup, wxRight, 10);
  m_pxiFile->GetConstraints()->height.AsIs();

  m_runPxi->SetConstraints(new wxLayoutConstraints);
  m_runPxi->GetConstraints()->top.SameAs(m_pxiFile, wxBottom, 10);
  m_runPxi->GetConstraints()->left.SameAs(m_pxiFile, wxLeft);
  m_runPxi->GetConstraints()->width.AsIs();
  m_runPxi->GetConstraints()->height.AsIs();

  m_pxiCommand->SetConstraints(new wxLayoutConstraints);
  m_pxiCommand->GetConstraints()->centreY.SameAs(m_runPxi, wxCentreY);
  m_pxiCommand->GetConstraints()->left.SameAs(m_runPxi, wxRight, 10);
  m_pxiCommand->GetConstraints()->right.SameAs(m_pxiGroup, wxRight, 10);
  m_pxiCommand->GetConstraints()->height.AsIs();
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
//                      dialogQre: Member functions
//=======================================================================

#include "dlqre.h"

dialogQre::dialogQre(wxWindow *p_parent, const gText &p_filename,
		     bool p_vianfg)
  : dialogPxi("QreSolve Parameters", p_filename, p_parent)
{
  MakeCommonFields(true, false, p_vianfg);
  m_okButton->GetConstraints()->top.SameAs(m_pxiGroup, wxBottom, 10);
  Go();
}

dialogQre::~dialogQre()
{
  if (Completed() == wxOK) {
    wxWriteResource("Algorithm Params", "Qre-minLam",
		    m_minLam->GetValue(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Qre-maxLam",
		    m_maxLam->GetValue(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Qre-delLam",
		    m_delLam->GetValue(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Qre-accuracy",
		    m_accuracy->GetInteger(), gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Qre-startOption",
		    m_startOption->GetSelection(), gambitApp.ResourceFile());
  }
}

void dialogQre::AlgorithmFields(void)
{
  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters");
  NewLine();

  gText minLam, maxLam, delLam;
  int accuracy = 4;
  wxGetResourceStr("Algorithm Params", "Qre-minLam", minLam, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "Qre-maxLam", maxLam, gambitApp.ResourceFile());
  wxGetResourceStr("Algorithm Params", "Qre-delLam", delLam, gambitApp.ResourceFile());
  wxGetResource("Algorithm Params", "Qre-accuracy", &accuracy, gambitApp.ResourceFile());

  m_minLam = new wxNumberItem(this, "minLam", minLam);
  m_minLam->SetConstraints(new wxLayoutConstraints);
  m_minLam->GetConstraints()->top.SameAs(m_algorithmGroup, wxTop, 20);
  m_minLam->GetConstraints()->left.SameAs(m_algorithmGroup, wxLeft, 10);
  m_minLam->GetConstraints()->width.AsIs();
  m_minLam->GetConstraints()->height.AsIs();
  NewLine();

  m_maxLam = new wxNumberItem(this, "maxLam", maxLam);
  m_maxLam->SetConstraints(new wxLayoutConstraints);
  m_maxLam->GetConstraints()->top.SameAs(m_minLam, wxBottom, 10);
  m_maxLam->GetConstraints()->left.SameAs(m_minLam, wxLeft);
  m_maxLam->GetConstraints()->width.AsIs();
  m_maxLam->GetConstraints()->height.AsIs();
  
  m_delLam = new wxNumberItem(this, "delLam", delLam);
  m_delLam->SetConstraints(new wxLayoutConstraints);
  m_delLam->GetConstraints()->top.SameAs(m_maxLam, wxBottom, 10);
  m_delLam->GetConstraints()->left.SameAs(m_minLam, wxLeft);
  m_delLam->GetConstraints()->width.AsIs();
  m_delLam->GetConstraints()->height.AsIs();
  NewLine();

  m_accuracy = new wxIntegerItem(this, "Accuracy: 1.0 e -", accuracy, -1, -1, -1, -1);
  m_accuracy->SetConstraints(new wxLayoutConstraints);
  m_accuracy->GetConstraints()->top.SameAs(m_minLam, wxTop);
  m_accuracy->GetConstraints()->left.SameAs(m_minLam, wxRight, 10);
  m_accuracy->GetConstraints()->width.AsIs();
  m_accuracy->GetConstraints()->height.AsIs();

  NewLine();

  int startOption;
  wxGetResource("Algorithm Params", "Qre-startOption",
		&startOption, gambitApp.ResourceFile());
  char *startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, 0, "Starting Point", -1, -1, -1, -1,
				 3, startOptions);
  if (startOption >= 0 && startOption <= 2)
    m_startOption->SetSelection(startOption);

  m_startOption->SetConstraints(new wxLayoutConstraints);
  m_startOption->GetConstraints()->top.SameAs(m_delLam, wxBottom, 10);
  m_startOption->GetConstraints()->left.SameAs(m_minLam, wxLeft);
  m_startOption->GetConstraints()->width.AsIs();
  m_startOption->GetConstraints()->height.AsIs();
  NewLine();

  int plotType = 0;
  wxGetResource("Algorithm Params", "Pxi-Plot-Type", &plotType, gambitApp.ResourceFile());
  char *plotTypeChoices[] = { "Log", "Linear" };
  m_plotType = new wxRadioBox(this, 0, "Plot type", -1, -1, -1, -1,
			      2, plotTypeChoices);
  if (plotType == 0 || plotType == 1)
    m_plotType->SetSelection(plotType);
#ifdef wx_motif
  NewLine();
#endif  // wx_motif

  m_plotType->SetConstraints(new wxLayoutConstraints);
#ifdef wx_motif
  m_plotType->GetConstraints()->top.SameAs(m_startOption, wxBottom, 10);
  m_plotType->GetConstraints()->left.SameAs(m_startOption, wxLeft);
#else
  m_plotType->GetConstraints()->top.SameAs(m_startOption, wxTop);
  m_plotType->GetConstraints()->left.SameAs(m_startOption, wxRight, 10);
#endif  // wx_motif
  m_plotType->GetConstraints()->width.AsIs();
  m_plotType->GetConstraints()->height.AsIs();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_accuracy, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_plotType, wxBottom, -10);

  PxiFields();
}

//=======================================================================
//                    dialogQreGrid: Member functions
//=======================================================================

#include "dlqregrid.h"

dialogQreGrid::dialogQreGrid(wxWindow *p_parent,
			     const gText &p_filename)
  : dialogPxi("QreGridSolve Parameters", p_filename, p_parent)
{
  MakeCommonFields(true, false, true);
  m_okButton->GetConstraints()->top.SameAs(m_pxiGroup, wxBottom, 10);
  Go();
}

dialogQreGrid::~dialogQreGrid()
{
  if (Completed() == wxOK) {
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

  m_algorithmGroup = new wxGroupBox(this, "Algorithm parameters");

  m_minLam = new wxNumberItem(this, "minLam", minLam);
  m_minLam->SetConstraints(new wxLayoutConstraints);
  m_minLam->GetConstraints()->top.SameAs(m_algorithmGroup, wxTop, 20);
  m_minLam->GetConstraints()->left.SameAs(m_algorithmGroup, wxLeft, 10);
  m_minLam->GetConstraints()->width.AsIs();
  m_minLam->GetConstraints()->height.AsIs();
  NewLine();

  m_maxLam = new wxNumberItem(this, "maxLam", maxLam);
  m_maxLam->SetConstraints(new wxLayoutConstraints);
  m_maxLam->GetConstraints()->top.SameAs(m_minLam, wxBottom, 10);
  m_maxLam->GetConstraints()->left.SameAs(m_minLam, wxLeft);
  m_maxLam->GetConstraints()->width.AsIs();
  m_maxLam->GetConstraints()->height.AsIs();
  NewLine();

  m_delLam = new wxNumberItem(this, "delLam", delLam);
  m_delLam->SetConstraints(new wxLayoutConstraints);
  m_delLam->GetConstraints()->top.SameAs(m_maxLam, wxBottom, 10);
  m_delLam->GetConstraints()->left.SameAs(m_minLam, wxLeft);
  m_delLam->GetConstraints()->width.AsIs();
  m_delLam->GetConstraints()->height.AsIs();
  NewLine();

  m_delp1 = new wxNumberItem(this, "Grid 1 Del", delp1);
  m_delp1->SetConstraints(new wxLayoutConstraints);
  m_delp1->GetConstraints()->top.SameAs(m_minLam, wxTop);
  m_delp1->GetConstraints()->left.SameAs(m_minLam, wxRight, 10);
  m_delp1->GetConstraints()->width.AsIs();
  m_delp1->GetConstraints()->height.AsIs();

  m_tol1 = new wxNumberItem(this, "Grid 1 Tol", tol1);
  m_tol1->SetConstraints(new wxLayoutConstraints);
  m_tol1->GetConstraints()->top.SameAs(m_delp1, wxBottom, 10);
  m_tol1->GetConstraints()->left.SameAs(m_delp1, wxLeft);
  m_tol1->GetConstraints()->width.AsIs();
  m_tol1->GetConstraints()->height.AsIs();

  NewLine();
  m_delp2 = new wxNumberItem(this, "Grid 2 Del", delp2);
  m_delp2->SetConstraints(new wxLayoutConstraints);
  m_delp2->GetConstraints()->top.SameAs(m_tol1, wxBottom, 10);
  m_delp2->GetConstraints()->left.SameAs(m_delp1, wxLeft);
  m_delp2->GetConstraints()->width.AsIs();
  m_delp2->GetConstraints()->height.AsIs();

  NewLine();
  m_tol2 = new wxNumberItem(this, "Grid 2 Tol", tol2);
  m_tol2->SetConstraints(new wxLayoutConstraints);
  m_tol2->GetConstraints()->top.SameAs(m_delp2, wxBottom, 10);
  m_tol2->GetConstraints()->left.SameAs(m_delp1, wxLeft);
  m_tol2->GetConstraints()->width.AsIs();
  m_tol2->GetConstraints()->height.AsIs();

  Bool multiGrid = true;
  wxGetResource("Algorithm Params", "QreGrid-multiGrid",
		&multiGrid, gambitApp.ResourceFile());
  m_multiGrid = new wxCheckBox(this, 0, "Use MultiGrid");
  m_multiGrid->SetValue(multiGrid);
  m_multiGrid->SetConstraints(new wxLayoutConstraints);
  m_multiGrid->GetConstraints()->centreY.SameAs(m_tol2, wxCentreY);
  m_multiGrid->GetConstraints()->left.SameAs(m_minLam, wxLeft);
  m_multiGrid->GetConstraints()->width.AsIs();
  m_multiGrid->GetConstraints()->height.AsIs();

  int plotType = 0;
  wxGetResource("Algorithm Params", "Pxi-Plot-Type", &plotType, gambitApp.ResourceFile());
  char *plotTypeChoices[] = { "Log", "Linear" };
  m_plotType = new wxRadioBox(this, 0, "Plot type", -1, -1, -1, -1,
			      2, plotTypeChoices);
  if (plotType == 0 || plotType == 1)
    m_plotType->SetSelection(plotType);
#ifdef wx_motif
  NewLine();
#endif  // wx_motif

  m_plotType->SetConstraints(new wxLayoutConstraints);
#ifdef wx_motif
  m_plotType->GetConstraints()->top.SameAs(m_tol2, wxBottom, 10);
  m_plotType->GetConstraints()->left.SameAs(m_minLam, wxLeft);
#else
  m_plotType->GetConstraints()->top.SameAs(m_tol2, wxTop);
  m_plotType->GetConstraints()->left.SameAs(m_minLam, wxRight, 10);
#endif  // wx_motif
  m_plotType->GetConstraints()->width.AsIs();
  m_plotType->GetConstraints()->height.AsIs();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_delp1, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_plotType, wxBottom, -10);

  NewLine();

  PxiFields();
  NewLine();
}



