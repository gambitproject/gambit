//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to simplicial subdivision equilibrium computation algorithms
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "guishare/valinteger.h"
#include "guishare/wxstatus.h"
#include "algsimpdiv.h"
#include "nash/simpsub.h"  // for efg via nfg
#include "nash/simpdiv.h"  // for normal-form algorithm


//========================================================================
//                dialogSimpdiv: Parameter-setting dialog
//========================================================================

const int idSTOPAFTER = 9000;

class dialogSimpdiv : public wxDialog {
private:
  wxRadioBox *m_stopAfter, *m_precision;
  wxStaticText *m_stopAfterText;
  wxTextCtrl *m_stopAfterCount, *m_nRestarts, *m_leashLength;
  wxString m_stopAfterValue, m_nRestartsValue, m_leashLengthValue;

  // Event handlers
  void OnStopAfter(wxCommandEvent &);

public:
  dialogSimpdiv(wxWindow *p_parent);

  // Data access
  bool SelectSolutions(void) const { return true; }
  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
  int NumRestarts(void) const { return atoi(m_nRestarts->GetValue()); }
  int LeashLength(void) const { return atoi(m_leashLength->GetValue()); }

  // Redefined virtual functions
  bool Validate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dialogSimpdiv, wxDialog)
  EVT_RADIOBOX(idSTOPAFTER, dialogSimpdiv::OnStopAfter)
END_EVENT_TABLE()

dialogSimpdiv::dialogSimpdiv(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "SimpdivSolve Parameters"),
    m_stopAfterValue("1"), m_nRestartsValue("10"), m_leashLengthValue("10")
{
  SetAutoLayout(true);

  wxBoxSizer *paramSizer = new wxBoxSizer(wxHORIZONTAL);

  wxString stopAfterChoices[] = { "Find one", "Find two",
				  "Find n", "Find all" };
  m_stopAfter = new wxRadioBox(this, idSTOPAFTER, "Number of equilibria",
			       wxDefaultPosition, wxDefaultSize,
			       4, stopAfterChoices, 1, wxRA_SPECIFY_COLS);
  m_stopAfter->SetSelection(0);
  paramSizer->Add(m_stopAfter, 0, wxALL, 5);

  wxBoxSizer *stopAfterSizer = new wxBoxSizer(wxVERTICAL);
  m_stopAfterText = new wxStaticText(this, wxID_STATIC,
				     "Number of equilibria to find");
  stopAfterSizer->Add(m_stopAfterText, 0, wxALL, 5);
  m_stopAfterCount = new wxTextCtrl(this, -1, "",
				    wxDefaultPosition, wxDefaultSize, 0,
				    gIntegerValidator(&m_stopAfterValue, 1),
				    "Stop After");
  m_stopAfterCount->SetValue(m_stopAfterValue);
  stopAfterSizer->Add(m_stopAfterCount, 0, wxALL | wxCENTER, 5);
  paramSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 0);
  
  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);

  wxBoxSizer *restartSizer = new wxBoxSizer(wxHORIZONTAL);
  restartSizer->Add(new wxStaticText(this, wxID_STATIC, "Number of restarts"),
		    0, wxALL, 5);
  m_nRestarts = new wxTextCtrl(this, -1, "",
			       wxDefaultPosition, wxDefaultSize, 0,
			       gIntegerValidator(&m_nRestartsValue, 0),
			       "Number of restarts");
  restartSizer->Add(m_nRestarts, 0, wxALL, 5);

  wxBoxSizer *leashSizer = new wxBoxSizer(wxHORIZONTAL);
  leashSizer->Add(new wxStaticText(this, wxID_STATIC, "Leash length"),
		  0, wxALL, 5);
  m_leashLength = new wxTextCtrl(this, -1, "",
				 wxDefaultPosition, wxDefaultSize, 0,
				 gIntegerValidator(&m_leashLengthValue, 0),
				 "Leash length");
  leashSizer->Add(m_leashLength, 0, wxALL, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton *helpButton = new wxButton(this, wxID_HELP, "Help");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);
  buttonSizer->Add(helpButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(paramSizer, 0, wxALL, 5);
  topSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);
  topSizer->Add(restartSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(leashSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

void dialogSimpdiv::OnStopAfter(wxCommandEvent &)
{
  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

int dialogSimpdiv::StopAfter(void) const
{
  switch (m_stopAfter->GetSelection()) {
  case 0:
    return 1;
  case 1:
    return 2;
  case 2:
    return atoi(m_stopAfterCount->GetValue());
  case 3:
  default:
    return 0;
  }
}

bool dialogSimpdiv::Validate(void)
{
  return (m_stopAfter->GetSelection() != 2 || wxWindow::Validate());
}

//========================================================================
//                         Algorithm classes
//========================================================================

class guiSimpdivNfg : public efgSimpDivNfgSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiSimpdivNfg(wxWindow *p_parent, const SimpdivParams &p_params)
    : efgSimpDivNfgSolve(p_params), m_parent(p_parent) { }
  virtual ~guiSimpdivNfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiSimpdivNfg::SelectSolutions(int, const FullEfg &p_efg,
				    gList<BehavSolution> &p_solutions)
{ }

//========================================================================
//                         Top-level functions
//========================================================================

bool SimpdivNfg(wxWindow *p_parent,
		const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogSimpdiv dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    SimpdivParams params;
    params.stopAfter = dialog.StopAfter();
    params.precision = dialog.Precision();
    params.nRestarts = dialog.NumRestarts();
    params.leashLength = dialog.LeashLength();

    guiSimpdivNfg algorithm(p_parent, params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "SimpdivSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool SimpdivNfg(wxWindow *p_parent,
		const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogSimpdiv dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    SimpdivParams params;
    params.stopAfter = dialog.StopAfter();
    params.precision = dialog.Precision();
    params.nRestarts = dialog.NumRestarts();
    params.leashLength = dialog.LeashLength();

    wxStatus status(p_parent, "SimpdivSolve Progress");
    try {
      int nevals, niters;
      double time;
      Simpdiv(p_support, params, p_solutions, status, nevals, niters, time);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

