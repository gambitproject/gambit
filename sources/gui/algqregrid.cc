//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to quantal-response correspondence grid search
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/valnumber.h"
#include "guishare/wxstatus.h"
#include "algqregrid.h"
#include "nash/nfgqregrid.h"

//========================================================================
//                 dialogQreGrid: Parameter-setting dialog
//========================================================================

class dialogQreGrid : public wxDialog {
private:
  wxTextCtrl *m_lambda, *m_delp1, *m_delp2, *m_tol1, *m_tol2;
  wxString m_lambdaValue, m_delp1Value, m_delp2Value, m_tol1Value, m_tol2Value;

public:
  dialogQreGrid(wxWindow *p_parent);

  double Lambda(void) const
    { return ToNumber(m_lambda->GetValue().c_str()); }
  double DelP1(void) const
    { return ToNumber(m_delp1->GetValue().c_str()); }
  double Tol1(void) const
    { return ToNumber(m_tol1->GetValue().c_str()); }
  double DelP2(void) const
    { return ToNumber(m_delp2->GetValue().c_str()); }
  double Tol2(void) const
    { return ToNumber(m_tol2->GetValue().c_str()); }
};


dialogQreGrid::dialogQreGrid(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "QreGridSolve Parameters")
{
  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Lambda"),
		 0, wxALL, 5);
  m_lambdaValue = wxString::Format("%f", 1.0);
  m_lambda = new wxTextCtrl(this, -1, m_lambdaValue,
			    wxDefaultPosition, wxDefaultSize,
			    0, gNumberValidator(&m_lambdaValue, 0, 1000),
			    "Lambda");
  gridSizer->Add(m_lambda, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Initial mesh size"),
		 0, wxALL, 5);
  m_delp1Value = wxString::Format(".01");
  m_delp1 = new wxTextCtrl(this, -1, m_delp1Value,
			   wxDefaultPosition, wxDefaultSize,
			   0, gNumberValidator(&m_delp1Value, 0, .1),
			   "Initial mesh size");
  gridSizer->Add(m_delp1, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC,
				  "Initial search tolerance"), 0, wxALL, 5);
  m_tol1Value = wxString::Format(".01");
  m_tol1 = new wxTextCtrl(this, -1, m_tol1Value,
			  wxDefaultPosition, wxDefaultSize,
			  0, gNumberValidator(&m_tol1Value, 0, .1),
			  "Initial search tolerance");
  gridSizer->Add(m_tol1, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Refined mesh size"),
		 0, wxALL, 5);
  m_delp2Value = wxString::Format(".0001");
  m_delp2 = new wxTextCtrl(this, -1, m_delp2Value,
			   wxDefaultPosition, wxDefaultSize,
			   0, gNumberValidator(&m_delp2Value, 0, .1),
			   "Refined mesh size");
  gridSizer->Add(m_delp2, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC,
				  "Refined search tolerance"), 0, wxALL, 5);
  m_tol2Value = wxString::Format(".0001");
  m_tol2 = new wxTextCtrl(this, -1, m_tol2Value,
			  wxDefaultPosition, wxDefaultSize,
			  0, gNumberValidator(&m_tol2Value, 0, .1),
			  "Initial search tolerance");
  gridSizer->Add(m_tol2, 0, wxALL, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton *helpButton = new wxButton(this, wxID_HELP, "Help");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);
  buttonSizer->Add(helpButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(gridSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

//========================================================================
//                         Top-level functions
//========================================================================

bool QreGridNfg(wxWindow *p_parent,
		const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogQreGrid dialog(p_parent);

  if (dialog.ShowModal() == wxID_OK) {
    QreNfgGrid algorithm;
    algorithm.SetMinLambda(dialog.Lambda());
    algorithm.SetMaxLambda(dialog.Lambda());
    algorithm.SetDelP1(dialog.DelP1());
    algorithm.SetTol1(dialog.Tol1());
    algorithm.SetDelP2(dialog.DelP2());
    algorithm.SetTol2(dialog.Tol2());

    wxStatus status(p_parent, "QreGridSolve Progress");
    try {
      Nfg *N = MakeReducedNfg(EFSupport(p_support.GetGame()));
      gList<MixedSolution> solutions;
      algorithm.Solve(NFSupport(*N), gnull, status, solutions);
      for (int i = 1; i <= solutions.Length(); i++) {
	p_solutions.Append(BehavSolution(BehavProfile<gNumber>(solutions[i])));
      }
      delete N;
    }
    catch (gSignalBreak &) { }
    return true;
  }

  return false;
}

bool QreGridNfg(wxWindow *p_parent,
		const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogQreGrid dialog(p_parent);

  if (dialog.ShowModal() == wxID_OK) {
    QreNfgGrid algorithm;
    algorithm.SetMinLambda(dialog.Lambda());
    algorithm.SetMaxLambda(dialog.Lambda());
    algorithm.SetDelP1(dialog.DelP1());
    algorithm.SetTol1(dialog.Tol1());
    algorithm.SetDelP2(dialog.DelP2());
    algorithm.SetTol2(dialog.Tol2());

    wxStatus status(p_parent, "QreGridSolve Progress");
    try {
      algorithm.Solve(p_support.Game(), gnull, status, p_solutions);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}
