//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to quantal-response correspondence tracing
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/valnumber.h"
#include "guishare/wxstatus.h"
#include "algqre.h"
#include "efgqre.h"
#include "nfgqre.h"

//========================================================================
//                  dialogQre: Parameter-setting dialog
//========================================================================

class dialogQre : public wxDialog {
private:
  wxRadioBox *m_finiteLambda;
  wxTextCtrl *m_maxLambda, *m_stepSize;
  wxCheckBox *m_pxiFile;
  wxString m_maxLambdaValue, m_stepSizeValue;

  // Event handlers
  void OnStoppingLambda(wxCommandEvent &);

public:
  dialogQre(wxWindow *p_parent);

  // Returning data from dialog; only valid if ShowModal() returns OK.
  bool FiniteLambda(void) const;
  bool GeneratePXIFile(void) const;
  
  double MaxLambda(void) const;
  double StepSize(void) const;

  DECLARE_EVENT_TABLE()
};

int idQRE_TO_INFINITY = 2000;

BEGIN_EVENT_TABLE(dialogQre, wxDialog)
  EVT_RADIOBOX(idQRE_TO_INFINITY, dialogQre::OnStoppingLambda)
END_EVENT_TABLE()

dialogQre::dialogQre(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "QreSolve Parameters")
{
  wxString lambdaStrings[2] = { "Finite", "Infinite" };
  m_finiteLambda = new wxRadioBox(this, idQRE_TO_INFINITY, "Stopping lambda",
				  wxDefaultPosition, wxDefaultSize,
				  2, lambdaStrings, 1, wxRA_SPECIFY_ROWS);

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Maximum lambda:"),
		 0, wxALL, 5);
  m_maxLambdaValue = wxString::Format("%d", 100);
  m_maxLambda = new wxTextCtrl(this, -1, m_maxLambdaValue,
			       wxDefaultPosition, wxDefaultSize,
			       0, gNumberValidator(&m_maxLambdaValue, 0),
			       "maximum lambda");
  gridSizer->Add(m_maxLambda, 0, wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, "Step size:"),
		 0, wxALL, 5);
  m_stepSizeValue = wxString::Format("%f", .0001);
  m_stepSize = new wxTextCtrl(this, -1, m_stepSizeValue,
			      wxDefaultPosition, wxDefaultSize,
			      0, gNumberValidator(&m_stepSizeValue, 0, .01),
			      "step size");
  gridSizer->Add(m_stepSize, 0, wxALL, 5);

  m_pxiFile = new wxCheckBox(this, -1, "Generate .pxi file");

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_finiteLambda, 0, wxALL | wxCENTER, 5);
  topSizer->Add(gridSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(m_pxiFile, 0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void dialogQre::OnStoppingLambda(wxCommandEvent &)
{
  m_maxLambda->Enable(m_finiteLambda->GetSelection() == 0);
}

bool dialogQre::FiniteLambda(void) const
{
  return (m_finiteLambda->GetSelection() == 0);
}

bool dialogQre::GeneratePXIFile(void) const
{
  return m_pxiFile->GetValue();
}

double dialogQre::MaxLambda(void) const
{
  return ToNumber(m_maxLambda->GetValue().c_str());
}

double dialogQre::StepSize(void) const
{
  return ToNumber(m_stepSize->GetValue().c_str());
}

//========================================================================
//                         Top-level functions
//========================================================================

bool QreEfg(wxWindow *p_parent,
	    const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogQre dialog(p_parent);

  if (dialog.ShowModal() == wxID_OK) {
    class QreEfg algorithm;
    algorithm.SetFullGraph(false);

    wxStatus status(p_parent, "QreSolve Progress");
    try {
      algorithm.Solve(p_support.GetGame(), gnull, status, p_solutions);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}

bool QreNfg(wxWindow *p_parent,
	    const EFSupport &p_support, gList<BehavSolution> &p_solutions)
{
  dialogQre dialog(p_parent);

  if (dialog.ShowModal() == wxID_OK) {
    class QreNfg algorithm;
    algorithm.SetFullGraph(false);

    wxStatus status(p_parent, "QreSolve Progress");
    Correspondence<double, MixedSolution> correspondence;
    try {
      Nfg *N = MakeReducedNfg(EFSupport(p_support.GetGame()));
      algorithm.Solve(*N, gnull, status, correspondence);
      p_solutions.Append(BehavSolution(BehavProfile<gNumber>(correspondence.GetPoint(1, 1))));
      delete N;
    }
    catch (gSignalBreak &) { }
    return true;
  }

  return false;
}

bool QreNfg(wxWindow *p_parent,
	    const NFSupport &p_support, gList<MixedSolution> &p_solutions)
{
  dialogQre dialog(p_parent);

  if (dialog.ShowModal() == wxID_OK) {
    class QreNfg algorithm;
    algorithm.SetFullGraph(false);

    wxStatus status(p_parent, "QreSolve Progress");
    Correspondence<double, MixedSolution> correspondence;
    try {
      algorithm.Solve(p_support.Game(), gnull, status, correspondence);
      p_solutions.Append(correspondence.GetPoint(1, 1));
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}
