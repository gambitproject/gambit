//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to equilibrium enumeration via polynomial solver algorithms
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "guishare/valinteger.h"
#include "guishare/wxstatus.h"
#include "gnullstatus.h"
#include "algpolenum.h"
#include "efgalleq.h"  // for extensive-form algorithm
#include "polensub.h"  // for efg via nfg
#include "nfgalleq.h"  // for normal-form algorithm


//========================================================================
//                dialogPolEnum: Parameter-setting dialog
//========================================================================

const int idSTOPAFTER = 9000;

class dialogPolEnum : public wxDialog {
private:
  wxRadioBox *m_stopAfter;
  wxStaticText *m_stopAfterText;
  wxTextCtrl *m_stopAfterCount;
  wxString m_stopAfterValue;

  // Event handlers
  void OnStopAfter(wxCommandEvent &);

public:
  dialogPolEnum(wxWindow *p_parent);

  // Data access
  bool SelectSolutions(void) const { return true; }
  int StopAfter(void) const;

  // Redefined virtual functions
  bool Validate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dialogPolEnum, wxDialog)
  EVT_RADIOBOX(idSTOPAFTER, dialogPolEnum::OnStopAfter)
END_EVENT_TABLE()

dialogPolEnum::dialogPolEnum(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "PolEnumSolve Parameters"),
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

void dialogPolEnum::OnStopAfter(wxCommandEvent &)
{
  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

int dialogPolEnum::StopAfter(void) const
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

bool dialogPolEnum::Validate(void)
{
  return (m_stopAfter->GetSelection() != 2 || wxWindow::Validate());
}

//========================================================================
//                         Algorithm classes
//========================================================================

class guiPolEnumEfg : public efgPolEnumSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiPolEnumEfg(wxWindow *p_parent, EfgPolEnumParams &p_params)
    : efgPolEnumSolve(p_params), m_parent(p_parent) { }
  virtual ~guiPolEnumEfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiPolEnumEfg::SelectSolutions(int, const FullEfg &p_efg,
				     gList<BehavSolution> &p_solutions)
{
  if (m_selectSolutions) {
    wxMessageBox("Someday, you'll be able to select solutions.\n"
		 "For now, you just get them all!");
  }
}

class guiPolEnumNfg : public efgPolEnumNfgSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiPolEnumNfg(wxWindow *p_parent, const PolEnumParams &p_params)
    : efgPolEnumNfgSolve(p_params), m_parent(p_parent) { }
  virtual ~guiPolEnumNfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiPolEnumNfg::SelectSolutions(int, const FullEfg &p_efg,
				     gList<BehavSolution> &p_solutions)
{
  if (m_selectSolutions) {
    wxMessageBox("Someday, you'll be able to select solutions.\n"
		 "For now, you just get them all!");
  }
}

//========================================================================
//                         Top-level functions
//========================================================================

bool PolEnumEfg(wxWindow *p_parent,
		 const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogPolEnum dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    EfgPolEnumParams params;
    guiPolEnumEfg algorithm(p_parent, params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "PolEnumSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool PolEnumNfg(wxWindow *p_parent,
		const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogPolEnum dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    PolEnumParams params;
    guiPolEnumNfg algorithm(p_parent, params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "PolEnumSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool PolEnumNfg(wxWindow *p_parent,
		 const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogPolEnum dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    PolEnumParams params;
    params.stopAfter = dialog.StopAfter();
    wxStatus status(p_parent, "PolEnumSolve Progress");
    try {
      gList<const NFSupport> singularSupports;
      long nevals;
      double time;
      AllNashSolve(p_support, params, p_solutions, status,
		   nevals, time, singularSupports);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

