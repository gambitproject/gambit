//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to LCP-based equilibrium computation algorithms
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "guishare/valinteger.h"
#include "guishare/wxstatus.h"
#include "alglcp.h"
#include "nash/seqform.h"  // for extensive-form algorithm
#include "nash/lemkesub.h"  // for efg via nfg
#include "nash/lemke.h"  // for normal-form algorithm


//========================================================================
//                dialogLcp: Parameter-setting dialog
//========================================================================

const int idSTOPAFTER = 9000;

class dialogLcp : public wxDialog {
private:
  wxRadioBox *m_stopAfter, *m_precision;
  wxStaticText *m_stopAfterText;
  wxTextCtrl *m_stopAfterCount;
  wxString m_stopAfterValue;

  // Event handlers
  void OnStopAfter(wxCommandEvent &);

public:
  dialogLcp(wxWindow *p_parent);

  // Data access
  bool SelectSolutions(void) const { return true; }
  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }

  // Redefined virtual functions
  bool Validate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dialogLcp, wxDialog)
  EVT_RADIOBOX(idSTOPAFTER, dialogLcp::OnStopAfter)
END_EVENT_TABLE()

dialogLcp::dialogLcp(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "LcpSolve Parameters"),
    m_stopAfterValue("1")
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
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

void dialogLcp::OnStopAfter(wxCommandEvent &)
{
  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

int dialogLcp::StopAfter(void) const
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

bool dialogLcp::Validate(void)
{
  return (m_stopAfter->GetSelection() != 2 || wxWindow::Validate());
}

//========================================================================
//                         Algorithm classes
//========================================================================

class guiLcpEfg : public efgLcpSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiLcpEfg(wxWindow *p_parent, const SeqFormParams &p_params)
    : efgLcpSolve(p_params), m_parent(p_parent) { }
  virtual ~guiLcpEfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiLcpEfg::SelectSolutions(int, const FullEfg &p_efg,
				gList<BehavSolution> &p_solutions)
{ }

class guiLcpNfg : public efgLcpNfgSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiLcpNfg(wxWindow *p_parent, const LemkeParams &p_params)
    : efgLcpNfgSolve(p_params), m_parent(p_parent) { }
  virtual ~guiLcpNfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiLcpNfg::SelectSolutions(int, const FullEfg &p_efg,
				gList<BehavSolution> &p_solutions)
{ }

//========================================================================
//                         Top-level functions
//========================================================================

bool LcpEfg(wxWindow *p_parent,
	    const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogLcp dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    SeqFormParams params;
    params.stopAfter = dialog.StopAfter();
    params.precision = dialog.Precision();

    guiLcpEfg algorithm(p_parent, params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "LcpSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool LcpNfg(wxWindow *p_parent,
	    const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogLcp dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    LemkeParams params;
    params.stopAfter = dialog.StopAfter();
    params.precision = dialog.Precision();

    guiLcpNfg algorithm(p_parent, params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "LcpSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool LcpNfg(wxWindow *p_parent,
		  const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogLcp dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    LemkeParams params;
    params.stopAfter = dialog.StopAfter();
    params.precision = dialog.Precision();

    wxStatus status(p_parent, "LcpSolve Progress");
    try {
      int npivots;
      double time;
      Lemke(p_support, params, p_solutions, status, npivots, time);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

