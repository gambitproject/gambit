//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to Lyapunov function minimization for computing equilibrium
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "guishare/valinteger.h"
#include "guishare/wxstatus.h"
#include "algliap.h"
#include "nash/eliap.h"  // for extensive-form algorithm
#include "nash/liapsub.h"  // for efg via nfg
#include "nash/nliap.h"  // for normal-form algorithm


//========================================================================
//                dialogLiap: Parameter-setting dialog
//========================================================================

const int idSTOPAFTER = 9000;

class dialogLiap : public wxDialog {
private:
  wxRadioBox *m_stopAfter;
  wxStaticText *m_stopAfterText;
  wxTextCtrl *m_stopAfterCount, *m_nTries, *m_accuracy;
  wxString m_stopAfterValue, m_nTriesValue, m_accuracyValue;

  // Event handlers
  void OnStopAfter(wxCommandEvent &);

public:
  dialogLiap(wxWindow *p_parent);

  // Data access
  bool SelectSolutions(void) const { return true; }
  int StopAfter(void) const;
  int NumTries(void) const { return atoi(m_nTries->GetValue()); }
  int Accuracy(void) const { return atoi(m_accuracy->GetValue()); }

  // Redefined virtual functions
  bool Validate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dialogLiap, wxDialog)
  EVT_RADIOBOX(idSTOPAFTER, dialogLiap::OnStopAfter)
END_EVENT_TABLE()

dialogLiap::dialogLiap(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "LiapSolve Parameters"),
    m_stopAfterValue("1"), m_nTriesValue("10"), m_accuracyValue("4")
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
  
  wxBoxSizer *triesSizer = new wxBoxSizer(wxHORIZONTAL);
  triesSizer->Add(new wxStaticText(this, wxID_STATIC, "Number of tries"),
		    0, wxALL, 5);
  m_nTries = new wxTextCtrl(this, -1, "",
			    wxDefaultPosition, wxDefaultSize, 0,
			    gIntegerValidator(&m_nTriesValue, 0),
			    "Number of tries");
  triesSizer->Add(m_nTries, 0, wxALL, 5);

  wxBoxSizer *accuracySizer = new wxBoxSizer(wxHORIZONTAL);
  accuracySizer->Add(new wxStaticText(this, wxID_STATIC, "Accuracy"),
		     0, wxALL, 5);
  m_accuracy = new wxTextCtrl(this, -1, "",
			      wxDefaultPosition, wxDefaultSize, 0,
			      gIntegerValidator(&m_accuracyValue, 0),
			      "Accuracy");
  accuracySizer->Add(m_accuracy, 0, wxALL, 5);

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
  topSizer->Add(triesSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(accuracySizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

void dialogLiap::OnStopAfter(wxCommandEvent &)
{
  m_stopAfterText->Show(m_stopAfter->GetSelection() == 2);
  m_stopAfterCount->Show(m_stopAfter->GetSelection() == 2);
}

int dialogLiap::StopAfter(void) const
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

bool dialogLiap::Validate(void)
{
  return (m_stopAfter->GetSelection() != 2 || wxWindow::Validate());
}

//========================================================================
//                         Algorithm classes
//========================================================================

class guiLiapEfg : public efgLiapSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiLiapEfg(wxWindow *p_parent, const Efg::Game &p_efg,
	     const EFLiapParams &p_params)
    : efgLiapSolve(p_efg, p_params, 
		   BehavProfile<gNumber>(EFSupport(p_efg))),
      m_parent(p_parent) { }
  virtual ~guiLiapEfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiLiapEfg::SelectSolutions(int, const FullEfg &p_efg,
				 gList<BehavSolution> &p_solutions)
{ }

class guiLiapNfg : public efgLiapNfgSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiLiapNfg(wxWindow *p_parent, const Efg::Game &p_efg,
	     const NFLiapParams &p_params)
    : efgLiapNfgSolve(p_efg, p_params, 
		      BehavProfile<gNumber>(EFSupport(p_efg))),
      m_parent(p_parent) { }
  virtual ~guiLiapNfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiLiapNfg::SelectSolutions(int, const FullEfg &p_efg,
				 gList<BehavSolution> &p_solutions)
{ }

//========================================================================
//                         Top-level functions
//========================================================================

bool LiapEfg(wxWindow *p_parent,
	     const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogLiap dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    EFLiapParams params;
    params.stopAfter = dialog.StopAfter();
    params.nTries = dialog.NumTries();
    params.SetAccuracy(dialog.Accuracy());
 
    guiLiapEfg algorithm(p_parent, p_support.GetGame(), params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "LiapSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool LiapNfg(wxWindow *p_parent,
	     const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogLiap dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    NFLiapParams params;
    params.stopAfter = dialog.StopAfter();
    params.nTries = dialog.NumTries();
    params.SetAccuracy(dialog.Accuracy());
 
    guiLiapNfg algorithm(p_parent, p_support.GetGame(), params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "LiapSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool LiapNfg(wxWindow *p_parent,
	     const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogLiap dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    NFLiapParams params;
    params.stopAfter = dialog.StopAfter();
    params.nTries = dialog.NumTries();
    params.SetAccuracy(dialog.Accuracy());

    wxStatus status(p_parent, "LiapSolve Progress");
    try {
      long nevals, niters;
      Liap(p_support.Game(), params, MixedProfile<gNumber>(p_support),
	   p_solutions, status, nevals, niters);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

