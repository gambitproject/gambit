//
// FILE: algdlgs.cc -- Classes that are often used for the algorithm
//                     parameter dialogs
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "base/gstream.h"
#include "system.h"
#include "wxio.h"
#include "gambit.h"
#include "algdlgs.h"

#include "guishare/wxmisc.h"
#include "guishare/valnumber.h"
#include "guishare/valinteger.h"

#include "nfgconst.h"

#include "dlqregrid.h"

//========================================================================
//                  dialogTrace auxiliary dialog class
//========================================================================

class dialogTrace : public guiAutoDialog {
private:
  wxRadioBox *m_traceDest;
  wxTextCtrl *m_traceFile;
  wxSlider *m_traceLevel;

  void OnTrace(wxCommandEvent &);

public:
  dialogTrace(wxWindow *p_parent, int p_traceDest, const gText &p_traceFile,
              int p_TraceLevel);
  virtual ~dialogTrace();

  int TraceDest(void) const  { return m_traceDest->GetSelection(); }
  gText TraceFile(void) const;
  int TraceLevel(void) const { return m_traceLevel->GetValue(); }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dialogTrace, guiAutoDialog)
  EVT_RADIOBOX(idTRACE_CHOICE, dialogTrace::OnTrace)
END_EVENT_TABLE()

dialogTrace::dialogTrace(wxWindow *p_parent,
                         int p_traceDest, const gText &p_traceFile,
                         int p_traceLevel)
  : guiAutoDialog(p_parent, "Tracing Parameters")
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxString traceChoices[] = { "None", "Window", "File" };
  m_traceDest = new wxRadioBox(this, idTRACE_CHOICE, "Destination",
			       wxDefaultPosition, wxDefaultSize,
			       3, traceChoices, 0, wxRA_SPECIFY_ROWS);
  m_traceDest->SetSelection(p_traceDest);
  sizer->Add(m_traceDest, 0, wxALL, 5);

  wxBoxSizer *fileSizer = new wxBoxSizer(wxHORIZONTAL);
  fileSizer->Add(new wxStaticText(this, -1, "Filename"),
		 0, wxALL | wxCENTER, 5);
  m_traceFile = new wxTextCtrl(this, -1);
  m_traceFile->SetValue((char *) p_traceFile);
  m_traceFile->Enable(p_traceDest == 2);
  fileSizer->Add(m_traceFile, 1, wxEXPAND | wxALL, 5);
  sizer->Add(fileSizer, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *levelSizer = new wxBoxSizer(wxHORIZONTAL);
  levelSizer->Add(new wxStaticText(this, -1, "Trace level"),
		  0, wxALL | wxCENTER, 5);
  m_traceLevel = new wxSlider(this, -1, p_traceLevel, 0, 10, 
			      wxDefaultPosition, wxDefaultSize,
			      wxSL_HORIZONTAL | wxSL_LABELS);
  m_traceLevel->Enable(p_traceDest > 0);
  levelSizer->Add(m_traceLevel, 1, wxEXPAND | wxALL, 5);
  sizer->Add(levelSizer, 0, wxEXPAND | wxALL, 5);

  sizer->Add(m_buttonSizer, 0, wxALL | wxCENTER, 5);
  m_helpButton->Enable(false);

  SetSizer(sizer); 
  sizer->Fit(this);
  sizer->SetSizeHints(this); 
  Layout();
}

dialogTrace::~dialogTrace()
{ }

void dialogTrace::OnTrace(wxCommandEvent &)
{
  m_traceFile->Enable(m_traceDest->GetSelection() == 2);
  m_traceLevel->Enable(m_traceDest->GetSelection() != 0);
}

gText dialogTrace::TraceFile(void) const
{ return m_traceFile->GetValue().c_str(); }

//========================================================================
//            dialogAlgorithm: Member function definitions
//========================================================================

BEGIN_EVENT_TABLE(dialogAlgorithm, guiAutoDialog)
  EVT_CHECKBOX(idALL_CHECKBOX, dialogAlgorithm::OnAll)
  EVT_BUTTON(idTRACE_BUTTON, dialogAlgorithm::OnTrace)
  EVT_RADIOBOX(idDEPTH_CHOICE, dialogAlgorithm::OnDepth)
END_EVENT_TABLE()

dialogAlgorithm::dialogAlgorithm(const gText &p_label, bool p_usesNfg,
				 wxWindow *p_parent)
  : guiAutoDialog(p_parent, p_label),
    m_usesNfg(p_usesNfg), m_subgames(false),
    m_traceDest(0), m_traceLevel(0), m_traceFile(""),
    m_dominanceBox(0), m_subgamesBox(0), m_algorithmBox(0),
    m_depthChoice(0), m_typeChoice(0),
    m_methodChoice(0), m_markSubgames(0), m_selectSolutions(0),
    m_stopAfter(0), m_findAll(0), m_precision(0)
{
  m_topSizer = new wxBoxSizer(wxVERTICAL);
}

dialogAlgorithm::~dialogAlgorithm()
{ 
  if (GetReturnCode() == wxID_OK) {
    wxConfig config("Gambit");

    if (m_usesNfg) {
      config.Write("Solutions/Nfg-ElimDom-Depth",
		   (long) m_depthChoice->GetSelection());
      if (m_depthChoice->GetSelection() != 0) {
	config.Write("Solutions/Nfg-ElimDom-Type",
		     (long) m_typeChoice->GetSelection());
	config.Write("Solutions/Nfg-ElimDom-Method",
		     (long) m_methodChoice->GetSelection());
      }
    }
    else {
      config.Write("Solutions/Efg-ElimDom-Depth",
		   (long) m_depthChoice->GetSelection());
      if (m_depthChoice->GetSelection() != 0) {
	config.Write("Solutions/Efg-ElimDom-Type", 
		     (long) m_typeChoice->GetSelection());
      }
    }

    if (m_subgames) {
      config.Write("Solutions/Efg-Mark-Subgames",
		   (long) m_markSubgames->GetValue());
      config.Write("Solutions/Efg-Interactive-Solutions",
		   (long) m_selectSolutions->GetValue());
    }

    if (m_stopAfter) {
      if (m_findAll->GetValue()) {
	config.Write("Solutions/StopAfter", 0l);
      }
      else {
	config.Write("Solutions/StopAfter", 
		     (long) ToNumber(m_stopAfter->GetValue().c_str()));
      }
    }

    if (m_precision) {
      config.Write("Solutions/Precision", (long) m_precision->GetSelection());
    }
  }
}

void dialogAlgorithm::OnDepth(wxCommandEvent &)
{
  m_typeChoice->Enable(m_depthChoice->GetSelection() > 0);
  if (m_methodChoice && m_usesNfg)
    m_methodChoice->Enable(m_depthChoice->GetSelection() > 0);
}

void dialogAlgorithm::OnAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

void dialogAlgorithm::OnTrace(wxCommandEvent &)
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
  long depth = 0, type = 0, method = 0;

  if (p_usesNfg) {
    m_dominanceBox = new wxStaticBoxSizer
      (new wxStaticBox(this, -1, "Eliminate dominated mixed strategies"),
       wxHORIZONTAL);

    wxConfig config("Gambit");
    config.Read("Solutions/Nfg-ElimDom-Depth", &depth);
    config.Read("Solutions/Nfg-ElimDom-Type", &type);
    config.Read("Solutions/Nfg-ElimDom-Method", &method);
  }
  else {
    m_dominanceBox = new wxStaticBoxSizer
      (new wxStaticBox(this, -1, "Eliminate dominated behavior strategies"),
       wxHORIZONTAL);

    wxConfig config("Gambit");
    config.Read("Solutions/Efg-ElimDom-Depth", &depth);
    config.Read("Solutions/Efg-ElimDom-Type", &type);
  }
  m_topSizer->Add(m_dominanceBox, 0, wxALL, 5);

  wxString depthChoices[] = { "None", "Once", "Iterative" };
  m_depthChoice = new wxRadioBox(this, idDEPTH_CHOICE, "Depth",
				 wxDefaultPosition, wxDefaultSize,
				 3, depthChoices, 0, wxRA_SPECIFY_COLS);
  if (depth >= 0 && depth <= 2)
    m_depthChoice->SetSelection(depth);
  m_dominanceBox->Add(m_depthChoice, 0, wxALL, 5);

  wxString typeChoices[] = { "Weak", "Strong" };
  m_typeChoice = new wxRadioBox(this, -1, "Type",
				wxDefaultPosition, wxDefaultSize,
				2, typeChoices, 0, wxRA_SPECIFY_COLS);
  if (m_depthChoice->GetSelection() == 0)
    m_typeChoice->Enable(false);
  else if (type == 0 || type == 1)
    m_typeChoice->SetSelection(type);
  m_dominanceBox->Add(m_typeChoice, 0, wxALL, 5);

  wxString methodChoices[] = { "Pure", "Mixed" };
  m_methodChoice = new wxRadioBox(this, -1, "Method",
				  wxDefaultPosition, wxDefaultSize,
				  2, methodChoices, 0, wxRA_SPECIFY_COLS);
  if (m_depthChoice->GetSelection() == 0 || !p_usesNfg)
    m_methodChoice->Enable(false);
  else if (method == 0 || method == 1)
    m_methodChoice->SetSelection(method);
  m_dominanceBox->Add(m_methodChoice, 0, wxALL, 5);
}

void dialogAlgorithm::SubgameFields(void)
{
  long mark = 0, select = 0;
  m_subgamesBox = new wxStaticBoxSizer(new wxStaticBox(this, -1, "Subgames"), 
				       wxVERTICAL);
  m_topSizer->Add(m_subgamesBox, 0, wxALL, 5);

  wxConfig config("Gambit");
  config.Read("Solutions/Efg-Mark-Subgames", &mark);
  config.Read("Solutions/Efg-Interactive-Solutions", &select);

  m_markSubgames = new wxCheckBox(this, -1, "Mark subgames before solving");
  m_markSubgames->SetValue(mark);
  m_subgamesBox->Add(m_markSubgames, 0, wxALL, 5);
  m_selectSolutions = new wxCheckBox(this, -1,
				     "Interactively select subgame solutions");
  m_selectSolutions->SetValue(select);
  m_subgamesBox->Add(m_selectSolutions, 0, wxALL, 5);
}

void dialogAlgorithm::MakeCommonFields(bool p_dominance, bool p_subgames,
				       bool p_usesNfg)
{
  m_subgames = p_subgames;
  if (p_dominance)   DominanceFields(p_usesNfg);
  if (p_subgames)    SubgameFields();
  AlgorithmFields();

  m_buttonSizer->Add(new wxButton(this, idTRACE_BUTTON, "Trace..."),
		     0, wxALL, 5);
  
  m_topSizer->Add(m_buttonSizer, 0, wxALL | wxCENTER, 5);
  SetSizer(m_topSizer); 
  m_topSizer->Fit(this);
  m_topSizer->SetSizeHints(this); 
  Layout();
}

void dialogAlgorithm::StopAfterField(void)
{
  long stopAfter = 0;
  wxConfig config("Gambit");
  config.Read("Solutions/StopAfter", &stopAfter);
  m_stopAfterValue = (char *) ToText(stopAfter);

  m_findAll = new wxCheckBox(this, idALL_CHECKBOX, "Find all");
  m_stopAfter = new wxTextCtrl(this, -1, "",
			       wxDefaultPosition, wxDefaultSize, 0,
			       gIntegerValidator(&m_stopAfterValue, 1),
			       "Stop After");

  if (stopAfter == 0) {
    m_findAll->SetValue(true);
    m_stopAfter->Enable(FALSE);
  }

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(m_findAll, 0, wxALL, 5);
  sizer->Add(m_stopAfter, 0, wxALL, 5);
  m_algorithmBox->Add(sizer, 0, wxALL, 5);
}

void dialogAlgorithm::PrecisionField(void)
{
  long precision = 0;
  wxConfig config("Gambit");
  config.Read("Solutions/Precision", &precision);

  wxString precisionChoices[] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 0, wxRA_SPECIFY_ROWS);
  if (precision == 0 || precision == 1) {
    m_precision->SetSelection(precision);
  }
  m_algorithmBox->Add(m_precision, 0, wxALL, 5);
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
//                   class dialogPxi: Member functions
//=======================================================================

BEGIN_EVENT_TABLE(dialogPxi, dialogAlgorithm)
  EVT_CHECKBOX(idRUNPXI_BOX, dialogPxi::OnRun)
END_EVENT_TABLE()

static char *wxOutputFile(const char *name)
{
  static char t_outfile[250];
  static char slash[2];
  slash[0] = System::Slash();
  slash[1] = '\0';

  if (strstr(slash, name)) {
    strcpy(t_outfile, wxFileNameFromPath((char *)name)); // strip the path
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
  if (GetReturnCode() == wxID_OK) {
    wxConfig config("Gambit");
    config.Write("Solutions/Pxi-Plot-Type", (long) m_plotType->GetSelection());
    config.Write("Solutions/Run-Pxi", m_runPxi->GetValue());
    config.Write("Solutions/Pxi-Command", m_pxiCommand->GetValue());
  }
}

void dialogPxi::PxiFields(void)
{
  m_pxiBox = new wxStaticBoxSizer(new wxStaticBox(this, -1, "PXI options"),
				  wxVERTICAL);
  m_topSizer->Add(m_pxiBox, 0, wxALL, 5);

  wxBoxSizer *fileSizer = new wxBoxSizer(wxHORIZONTAL);
  fileSizer->Add(new wxStaticText(this, -1, "File"), 0, wxALL | wxCENTER, 5);
  m_pxiFile = new wxTextCtrl(this, -1);
  m_pxiFile->SetValue((char *) m_defaultPxiFile);
  fileSizer->Add(m_pxiFile, 0, wxALL, 5);
  m_pxiBox->Add(fileSizer, 0, wxALL, 5);

  wxBoxSizer *commandSizer = new wxBoxSizer(wxHORIZONTAL);
  long runPxi = 0;
  wxConfig config("Gambit");
  config.Read("Solutions/Run-Pxi", &runPxi);
  m_runPxi = new wxCheckBox(this, idRUNPXI_BOX, "Run PXI");
  m_runPxi->SetValue(runPxi);
  commandSizer->Add(m_runPxi, 0, wxALL, 5);

  commandSizer->Add(new wxStaticText(this, -1, "PXI Command"),
		    0, wxALL | wxCENTER, 5);
  wxString pxiCommand;
  config.Read("Solutions/Pxi-Command", &pxiCommand);
  m_pxiCommand = new wxTextCtrl(this, -1);
  m_pxiCommand->SetValue(pxiCommand);
  m_pxiCommand->Enable(m_runPxi->GetValue());
  commandSizer->Add(m_pxiCommand, 0, wxALL, 5);
  m_pxiBox->Add(commandSizer, 0, wxALL, 5);
}

void dialogPxi::OnRun(wxCommandEvent &)
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

gText dialogPxi::PxiFilename(void) const
{ return m_pxiFile->GetValue().c_str(); }

gText dialogPxi::PxiCommand(void) const
{ return m_pxiCommand->GetValue().c_str(); }

//=======================================================================
//                    dialogQreGrid: Member functions
//=======================================================================

dialogQreGrid::dialogQreGrid(wxWindow *p_parent,
			     const gText &p_filename)
  : dialogPxi("QreGridSolve Parameters", p_filename, p_parent)
{
  MakeCommonFields(true, false, true);
}

dialogQreGrid::~dialogQreGrid()
{
  if (GetReturnCode() == wxID_OK) {
    wxConfig config("Gambit");
    config.Write("Solutions/QreGrid-minLam", m_minLam->GetValue());
    config.Write("Solutions/QreGrid-maxLam", m_maxLam->GetValue());
    config.Write("Solutions/QreGrid-delLam", m_delLam->GetValue());
    config.Write("Solutions/QreGrid-delp1", m_delp1->GetValue());
    config.Write("Solutions/QreGrid-delp2", m_delp2->GetValue());
    config.Write("Solutions/QreGrid-tol1", m_tol1->GetValue());
    config.Write("Solutions/QreGrid-tol2", m_tol2->GetValue());
    config.Write("Solutions/QreGrid-multiGrid", m_multiGrid->GetValue());
    config.Write("Solutions/QreGrid-startOption", 
		 (long) m_startOption->GetSelection());
  }
}

void dialogQreGrid::AlgorithmFields(void)
{
  m_minLamValue = ".01";
  m_maxLamValue = "30";
  m_delLamValue = ".01";
  m_delp1Value = ".01";
  m_delp2Value = ".01";
  m_tol1Value = ".01";
  m_tol2Value = ".01";
  
  wxConfig config("Gambit");
  config.Read("Solutions/QreGrid-minLam", &m_minLamValue);
  config.Read("Solutions/QreGrid-maxLam", &m_maxLamValue);
  config.Read("Solutions/QreGrid-delLam", &m_delLamValue);
  config.Read("Solutions/QreGrid-delp1", &m_delp1Value);
  config.Read("Solutions/QreGrid-delp2", &m_delp2Value);
  config.Read("Solutions/QreGrid-tol1", &m_tol1Value);
  config.Read("Solutions/QreGrid-tol2", &m_tol2Value);

  m_algorithmBox = new wxStaticBoxSizer
    (new wxStaticBox(this, -1, "Algorithm parameters"), wxVERTICAL);
  m_topSizer->Add(m_algorithmBox, 0, wxALL, 5);

  wxBoxSizer *lambdaSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *minLamSizer = new wxBoxSizer(wxHORIZONTAL);
  minLamSizer->Add(new wxStaticText(this, -1, "minLam"),
		   0, wxALL | wxCENTER, 5);
  m_minLam = new wxTextCtrl(this, -1, "",
			    wxDefaultPosition, wxDefaultSize, 0,
			    gNumberValidator(&m_minLamValue, 0), "minLam");
  minLamSizer->Add(m_minLam, 0, wxALL, 5);
  lambdaSizer->Add(minLamSizer, 0, wxALL, 5);

  wxBoxSizer *maxLamSizer = new wxBoxSizer(wxHORIZONTAL);
  maxLamSizer->Add(new wxStaticText(this, -1, "maxLam"),
		   0, wxALL | wxCENTER, 5);
  m_maxLam = new wxTextCtrl(this, -1, "",
			    wxDefaultPosition, wxDefaultSize, 0,
			    gNumberValidator(&m_maxLamValue, 0), "maxLam");
  maxLamSizer->Add(m_maxLam, 0, wxALL, 5);
  lambdaSizer->Add(maxLamSizer, 0, wxALL, 5);

  wxBoxSizer *delLamSizer = new wxBoxSizer(wxHORIZONTAL);
  delLamSizer->Add(new wxStaticText(this, -1, "delLam"),
		   0, wxALL | wxCENTER, 5);
  m_delLam = new wxTextCtrl(this, -1, "",
			    wxDefaultPosition, wxDefaultSize, 0,
			    gNumberValidator(&m_delLamValue, 0), "delLam");
  delLamSizer->Add(m_delLam, 0, wxALL, 5);
  lambdaSizer->Add(delLamSizer, 0, wxALL, 5);

  wxBoxSizer *tolSizer = new wxBoxSizer(wxVERTICAL);
  
  wxBoxSizer *delp1Sizer = new wxBoxSizer(wxHORIZONTAL);
  delp1Sizer->Add(new wxStaticText(this, -1, "Grid 1 Del"),
		  0, wxALL | wxCENTER, 5);
  m_delp1 = new wxTextCtrl(this, -1, "",
			   wxDefaultPosition, wxDefaultSize, 0,
			   gNumberValidator(&m_delp1Value, 0), "Grid 1 Del");
  delp1Sizer->Add(m_delp1, 0, wxALL, 5);
  tolSizer->Add(delp1Sizer, 0, wxALL, 5);

  wxBoxSizer *tol1Sizer = new wxBoxSizer(wxHORIZONTAL);
  tol1Sizer->Add(new wxStaticText(this, -1, "Grid 1 Tol"),
		 0, wxALL | wxCENTER, 5);
  m_tol1 = new wxTextCtrl(this, -1, "",
			  wxDefaultPosition, wxDefaultSize, 0,
			  gNumberValidator(&m_tol1Value, 0), "Grid 1 Tol");
  tol1Sizer->Add(m_tol1, 0, wxALL, 5);
  tolSizer->Add(tol1Sizer, 0, wxALL, 5);

  wxBoxSizer *delp2Sizer = new wxBoxSizer(wxHORIZONTAL);
  delp2Sizer->Add(new wxStaticText(this, -1, "Grid 2 Del"),
		  0, wxALL | wxCENTER, 5);
  m_delp2 = new wxTextCtrl(this, -1, "",
			   wxDefaultPosition, wxDefaultSize, 0,
			   gNumberValidator(&m_delp2Value, 0), "Grid 2 Del");
  delp2Sizer->Add(m_delp2, 0, wxALL, 5);
  tolSizer->Add(delp2Sizer, 0, wxALL, 5);

  wxBoxSizer *tol2Sizer = new wxBoxSizer(wxHORIZONTAL);
  tol2Sizer->Add(new wxStaticText(this, -1, "Grid 2 Tol"),
		 0, wxALL | wxCENTER, 5);
  m_tol2 = new wxTextCtrl(this, -1, "",
			  wxDefaultPosition, wxDefaultSize, 0,
			  gNumberValidator(&m_tol2Value, 0), "Grid 2 Tol");
  tol2Sizer->Add(m_tol2, 0, wxALL, 5);
  tolSizer->Add(tol2Sizer, 0, wxALL, 5);

  wxBoxSizer *parallelSizer = new wxBoxSizer(wxHORIZONTAL);
  parallelSizer->Add(lambdaSizer, 0, wxALL, 5);
  parallelSizer->Add(tolSizer, 0, wxALL, 5);
  m_algorithmBox->Add(parallelSizer, 0, wxALL, 5);

  long multiGrid = 1;
  config.Read("Solutions/QreGrid-multiGrid", &multiGrid);
  m_multiGrid = new wxCheckBox(this, -1, "Use MultiGrid");
  m_multiGrid->SetValue(multiGrid);
  m_algorithmBox->Add(m_multiGrid);

  long startOption = 0;
  config.Read("Solutions/QreGrid-startOption", &startOption);
  wxString startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, -1, "Starting Point",
				 wxDefaultPosition, wxDefaultSize,
				 3, startOptions, 0, wxRA_SPECIFY_ROWS);
  if (startOption >= 0 && startOption <= 2) {
    m_startOption->SetSelection(startOption);
  }
  m_algorithmBox->Add(m_startOption, 0, wxALL, 5);

  long plotType = 0;
  config.Read("Solutions/Pxi-Plot-Type", &plotType);
  wxString plotTypeChoices[] = { "Log", "Linear" };
  m_plotType = new wxRadioBox(this, -1, "Plot type",
			      wxDefaultPosition, wxDefaultSize,
			      2, plotTypeChoices, 0, wxRA_SPECIFY_ROWS);
  if (plotType == 0 || plotType == 1) {
    m_plotType->SetSelection(plotType);
  }
  m_algorithmBox->Add(m_plotType, 0, wxALL, 5);

  PxiFields();
}



