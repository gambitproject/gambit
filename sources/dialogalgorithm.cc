//
// FILE: dialogalgorithm.cc -- Base class for algorithm parameter selection
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "gstream.h"
#include "gtext.h"
#include "gnumber.h"

#include "dialogauto.h"
#include "dialogalgorithm.h"

//========================================================================
//                  dialogTrace auxiliary dialog class
//========================================================================

class dialogTrace : public guiAutoDialog {
private:
  wxRadioBox *m_traceDest;
  wxTextCtrl *m_traceFile, *m_traceLevel;
  
  void OnTrace(void);

public:
  dialogTrace(wxWindow *p_parent, int p_traceDest, const gText &p_traceFile,
              int p_TraceLevel);
  virtual ~dialogTrace();

  int TraceDest(void) const  { return m_traceDest->GetSelection(); }
  gText TraceFile(void) const;
  int TraceLevel(void) const { return atoi(m_traceLevel->GetValue().c_str()); }

  DECLARE_EVENT_TABLE()
};

const int idTRACEDEST = 100;

BEGIN_EVENT_TABLE(dialogTrace, wxDialog)
  EVT_RADIOBOX(idTRACEDEST, OnTrace)
END_EVENT_TABLE()

dialogTrace::dialogTrace(wxWindow *p_parent,
                         int p_traceDest, const gText &p_traceFile,
                         int p_traceLevel)
  : guiAutoDialog(p_parent, "Tracing Parameters")
{
  wxString traceChoices[] = { "None", "Window", "File" };
  m_traceDest = new wxRadioBox(this, idTRACEDEST, "Destination",
			       wxDefaultPosition, wxDefaultSize,
			       3, traceChoices);
  m_traceDest->SetSelection(p_traceDest);

  wxStaticText *traceFileLabel = new wxStaticText(this, -1, "Trace file");
  m_traceFile = new wxTextCtrl(this, -1);
  m_traceFile->SetValue((char *) p_traceFile);
  m_traceFile->Enable(p_traceDest == 2);

  wxStaticText *traceLevelLabel = new wxStaticText(this, -1, "Trace level");
  m_traceLevel = new wxTextCtrl(this, -1, "");
  m_traceLevel->SetValue((char *) ToText(p_traceLevel));
  m_traceLevel->Enable(p_traceDest > 0);

  m_traceDest->SetConstraints(new wxLayoutConstraints);
  m_traceDest->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_traceDest->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_traceDest->GetConstraints()->width.AsIs();
  m_traceDest->GetConstraints()->height.AsIs();

  traceFileLabel->SetConstraints(new wxLayoutConstraints);
  traceFileLabel->GetConstraints()->top.SameAs(m_traceDest, wxBottom, 10);
  traceFileLabel->GetConstraints()->left.SameAs(m_traceDest, wxLeft);
  traceFileLabel->GetConstraints()->width.AsIs();
  traceFileLabel->GetConstraints()->height.AsIs();

  m_traceFile->SetConstraints(new wxLayoutConstraints);
  m_traceFile->GetConstraints()->centreY.SameAs(traceFileLabel, wxCentreY);
  m_traceFile->GetConstraints()->left.SameAs(traceFileLabel, wxRight, 5);
  m_traceFile->GetConstraints()->width.AsIs();
  m_traceFile->GetConstraints()->height.AsIs();

  traceLevelLabel->SetConstraints(new wxLayoutConstraints);
  traceLevelLabel->GetConstraints()->top.SameAs(traceFileLabel, wxBottom, 10);
  traceLevelLabel->GetConstraints()->left.SameAs(m_traceDest, wxLeft);
  traceLevelLabel->GetConstraints()->width.AsIs();
  traceLevelLabel->GetConstraints()->height.AsIs();

  m_traceLevel->SetConstraints(new wxLayoutConstraints);
  m_traceLevel->GetConstraints()->centreY.SameAs(traceLevelLabel, wxCentreY);
  m_traceLevel->GetConstraints()->left.SameAs(traceLevelLabel, wxRight, 5);
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

  AutoSize();
}

dialogTrace::~dialogTrace()
{ }

void dialogTrace::OnTrace(void)
{
  m_traceFile->Enable(m_traceDest->GetSelection() == 2);
  m_traceLevel->Enable(m_traceDest->GetSelection() != 0);
}

gText dialogTrace::TraceFile(void) const  
{
  return m_traceFile->GetValue().c_str(); 
}

const int idALGORITHM_DEPTH = 1000;
const int idALGORITHM_TYPE = 1001;
const int idALGORITHM_DOMINANCE = 1002;
const int idALGORITHM_FINDALL = 1003;
const int idALGORITHM_TRACE = 1004;

BEGIN_EVENT_TABLE(dialogAlgorithm, wxDialog)
  EVT_CHECKBOX(idALGORITHM_FINDALL, OnAll)
  EVT_RADIOBOX(idALGORITHM_DEPTH, OnDepth)
  EVT_BUTTON(idALGORITHM_TRACE, OnTrace)
END_EVENT_TABLE()

dialogAlgorithm::dialogAlgorithm(wxWindow *p_parent,
				 const gText &p_label, bool p_usesNfg)
  : guiAutoDialog(p_parent, p_label),
    m_usesNfg(p_usesNfg), m_traceDest(0), m_traceLevel(0), m_traceFile(""),
    m_dominanceGroup(0), m_subgamesGroup(0), m_algorithmGroup(0),
    m_depthChoice(0), m_typeChoice(0),
    m_methodChoice(0), m_markSubgames(0),
    m_stopAfter(0), m_findAll(0), m_precision(0)
{ }

dialogAlgorithm::~dialogAlgorithm(void)
{
  /*
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
  */
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

  if (dialog.ShowModal() == wxID_OK)  {
    m_traceLevel = dialog.TraceLevel();
    m_traceFile = dialog.TraceFile();
    m_traceDest = dialog.TraceDest();
  }
}

void dialogAlgorithm::DominanceFields(bool p_usesNfg)
{
  int depth = 0, type = 0, method = 0;

  if (p_usesNfg) {
    m_dominanceGroup = new wxStaticBox(this, -1,
				       "Eliminate dominated mixed strategies");
    /*
    wxGetResource("Soln-Defaults", "Nfg-ElimDom-Depth", &depth, gambitApp.ResourceFile());
    wxGetResource("Soln-Defaults", "Nfg-ElimDom-Type", &type, gambitApp.ResourceFile());
    wxGetResource("Soln-Defaults", "Nfg-ElimDom-Method", &method,
		  gambitApp.ResourceFile());
    */
  }
  else {
    m_dominanceGroup = new wxStaticBox(this, -1, 
				       "Eliminate dominated behavior strategies");
    /*
    wxGetResource("Soln-Defaults", "Efg-ElimDom-Depth", &depth, gambitApp.ResourceFile());
    wxGetResource("Soln-Defaults", "Efg-ElimDom-Type", &type, gambitApp.ResourceFile());
    */
  }

  wxString depthChoices[] = { "None", "Once", "Iterative" };
  m_depthChoice = new wxRadioBox(this, idALGORITHM_DEPTH, "Depth",
				 wxDefaultPosition, wxDefaultSize,
				 3, depthChoices, 0, wxRA_SPECIFY_ROWS);
  if (depth >= 0 && depth <= 2)
    m_depthChoice->SetSelection(depth);

  wxString typeChoices[] = { "Weak", "Strong" };
  m_typeChoice = new wxRadioBox(this, -1, "Type",
				wxDefaultPosition, wxDefaultSize,
				2, typeChoices, 0, wxRA_SPECIFY_ROWS);
  if (m_depthChoice->GetSelection() == 0)
    m_typeChoice->Enable(FALSE);
  else if (type == 0 || type == 1)
    m_typeChoice->SetSelection(type);

  wxString methodChoices[] = { "Pure", "Mixed" };
  m_methodChoice = new wxRadioBox(this, -1, "Method",
				  wxDefaultPosition, wxDefaultSize,
				  2, methodChoices, 0, wxRA_SPECIFY_ROWS);
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
}

void dialogAlgorithm::SubgameFields(void)
{
  bool mark = false, select = false;
  m_subgamesGroup = new wxStaticBox(this, -1, "Subgames");
  /*
  wxGetResource("Soln-Defaults", "Efg-Mark-Subgames", &mark, gambitApp.ResourceFile());
  wxGetResource("Soln-Defaults", "Efg-Interactive-Solns", &select,
		gambitApp.ResourceFile());
  */
  m_markSubgames = new wxCheckBox(this, -1, "Mark subgames before solving");
  m_markSubgames->SetValue(mark);
  m_selectSolutions = new wxCheckBox(this, -1,
				     "Interactively select subgame solutions");
  m_selectSolutions->SetValue(select);

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

void dialogAlgorithm::MakeCommonFields(bool p_dominance, bool p_subgames,
				       bool p_usesNfg)
{
  m_subgames = p_subgames;
  if (p_dominance)   DominanceFields(p_usesNfg);
  if (p_subgames)    SubgameFields();
  AlgorithmFields();

  wxButton *traceButton = new wxButton(this, idALGORITHM_TRACE, "Trace...");

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
  //  wxGetResource("Algorithm Params", "StopAfter", &stopAfter, gambitApp.ResourceFile());

  m_findAll = new wxCheckBox(this, idALGORITHM_FINDALL, "Find all");
  m_stopAfter = new wxTextCtrl(this, -1, "Stop after");
  m_stopAfter->SetValue((char *) ToText((stopAfter > 0) ? stopAfter : 1));

  if (stopAfter == 0) {
    m_findAll->SetValue(true);
    m_stopAfter->Enable(FALSE);
  }
}

void dialogAlgorithm::PrecisionField(void)
{
  int precision = 0;
  //  wxGetResource("Algorithm Params", "Precision", &precision, gambitApp.ResourceFile());

  wxString precisionChoices[] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
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
    /*
    return new gWxOutput;
    */
    return &gnull;
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


