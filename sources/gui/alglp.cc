//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to linear-programming equilibrium computation algorithms
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "guishare/valinteger.h"
#include "guishare/wxstatus.h"
#include "gnullstatus.h"
#include "alglp.h"
#include "efgcsum.h"  // for extensive-form algorithm
#include "csumsub.h"  // for efg via nfg algorithm
#include "nfgcsum.h"  // for normal-form algorithm


//========================================================================
//                dialogLp: Parameter-setting dialog
//========================================================================

class dialogLp : public wxDialog {
private:
  wxRadioBox *m_precision;

public:
  dialogLp(wxWindow *p_parent);

  // Data access
  bool SelectSolutions(void) const { return true; }
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

dialogLp::dialogLp(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "LpSolve Parameters")
{
  SetAutoLayout(true);

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
  topSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

//========================================================================
//                         Algorithm classes
//========================================================================

class guiLpEfg : public efgLpSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiLpEfg(wxWindow *p_parent, const CSSeqFormParams &p_params)
    : efgLpSolve(p_params), m_parent(p_parent) { }
  virtual ~guiLpEfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiLpEfg::SelectSolutions(int, const FullEfg &p_efg,
			       gList<BehavSolution> &p_solutions)
{ }

class guiLpNfg : public efgLpNfgSolve {
private:
  wxWindow *m_parent;
  bool m_selectSolutions;

  void SelectSolutions(int, const FullEfg &, gList<BehavSolution> &);

public:
  guiLpNfg(wxWindow *p_parent, const ZSumParams &p_params)
    : efgLpNfgSolve(p_params), m_parent(p_parent) { }
  virtual ~guiLpNfg() { }

  void SetSelectSolutions(bool p_selectSolutions)
    { m_selectSolutions = p_selectSolutions; }
};

void guiLpNfg::SelectSolutions(int, const FullEfg &p_efg,
			       gList<BehavSolution> &p_solutions)
{ }

//========================================================================
//                         Top-level functions
//========================================================================

bool LpEfg(wxWindow *p_parent,
	   const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogLp dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    CSSeqFormParams params;
    params.precision = dialog.Precision();

    guiLpEfg algorithm(p_parent, params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "LpSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool LpNfg(wxWindow *p_parent,
	   const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogLp dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    ZSumParams params;
    params.precision = dialog.Precision();

    guiLpNfg algorithm(p_parent, params);
    algorithm.SetSelectSolutions(dialog.SelectSolutions());
    wxStatus status(p_parent, "LpSolve Progress");
    try {
      p_solutions = algorithm.Solve(p_support, status);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool LpNfg(wxWindow *p_parent,
	   const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogLp dialog(p_parent);
  if (dialog.ShowModal() == wxID_OK) {
    ZSumParams params;
    params.precision = dialog.Precision();

    wxStatus status(p_parent, "LpSolve Progress");
    try {
      int npivots;
      double time;
      ZSum(p_support, params, p_solutions, status, npivots, time);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

