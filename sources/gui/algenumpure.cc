//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to pure-strategy equilibrium enumeration algorithms
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "guishare/valinteger.h"
#include "guishare/wxstatus.h"
#include "algenumpure.h"
#include "nash/efgpure.h"  // for extensive-form algorithm
#include "nash/psnesub.h"  // for efg via nfg
#include "nash/nfgpure.h"  // for normal-form algorithm


//========================================================================
//                dialogEnumPure: Parameter-setting dialog
//========================================================================

const int idSTOPAFTER = 9000;

class dialogEnumPure : public wxDialog {
private:
  wxRadioBox *m_stopAfter;
  wxStaticText *m_stopAfterText;
  wxTextCtrl *m_stopAfterCount;
  wxString m_stopAfterValue;

  // Event handlers
  void OnStopAfter(wxCommandEvent &);

public:
  dialogEnumPure(wxWindow *p_parent);

  // Data access
  bool SelectSolutions(void) const { return true; }
  int StopAfter(void) const;

  // Redefined virtual functions
  bool Validate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dialogEnumPure, wxDialog)
  EVT_RADIOBOX(idSTOPAFTER, dialogEnumPure::OnStopAfter)
END_EVENT_TABLE()

dialogEnumPure::dialogEnumPure(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "EnumPureSolve Parameters"),
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
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

void dialogEnumPure::OnStopAfter(wxCommandEvent &)
{
  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

int dialogEnumPure::StopAfter(void) const
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

bool dialogEnumPure::Validate(void)
{
  return (m_stopAfter->GetSelection() != 2 || wxWindow::Validate());
}

//========================================================================
//                         Algorithm classes
//========================================================================

class guiEnumPureEfg : public efgEnumPure {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiEnumPureEfg(wxWindow *p_parent, int p_stopAfter)
    : efgEnumPure(p_stopAfter), m_parent(p_parent) { }
  virtual ~guiEnumPureEfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiEnumPureEfg::SelectSolutions(int, const FullEfg &p_efg,
				     gList<BehavSolution> &p_solutions)
{ }

class guiEnumPureNfg : public efgEnumPureNfgSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiEnumPureNfg(wxWindow *p_parent, int p_stopAfter)
    : efgEnumPureNfgSolve(p_stopAfter), m_parent(p_parent) { }
  virtual ~guiEnumPureNfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiEnumPureNfg::SelectSolutions(int, const FullEfg &p_efg,
				     gList<BehavSolution> &p_solutions)
{ }

//========================================================================
//                         Top-level functions
//========================================================================

bool EnumPureEfg(wxWindow *p_parent,
		 const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogEnumPure dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    guiEnumPureEfg algorithm(p_parent, dialog.StopAfter());
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "EnumPureSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool EnumPureNfg(wxWindow *p_parent,
		 const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogEnumPure dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    guiEnumPureNfg algorithm(p_parent, dialog.StopAfter());
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "EnumPureSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool EnumPureNfg(wxWindow *p_parent,
		 const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogEnumPure dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    nfgEnumPure algorithm;
    algorithm.SetStopAfter(dialog.StopAfter());
    wxStatus status(p_parent, "EnumPureSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

