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
  wxTextCtrl *m_maxLambda;
  wxStaticText *m_maxLambdaText;
  wxCheckBox *m_generatePxiFile;
  wxString m_maxLambdaValue;
  gOutput *m_pxiFile;

  // Event handlers
  void OnStoppingLambda(wxCommandEvent &);
  void OnOK(wxCommandEvent &);

public:
  dialogQre(wxWindow *p_parent);
  virtual ~dialogQre();

  // Returning data from dialog; only valid if ShowModal() returns OK.
  bool FiniteLambda(void) const;
  bool GeneratePxiFile(void) const;
  gOutput *PxiFile(void) const { return m_pxiFile; }
  double MaxLambda(void) const;

  DECLARE_EVENT_TABLE()
};

int idQRE_TO_INFINITY = 2000;

BEGIN_EVENT_TABLE(dialogQre, wxDialog)
  EVT_RADIOBOX(idQRE_TO_INFINITY, dialogQre::OnStoppingLambda)
  EVT_BUTTON(wxID_OK, dialogQre::OnOK)
END_EVENT_TABLE()

dialogQre::dialogQre(wxWindow *p_parent)
  : wxDialog(p_parent, -1, "QreSolve Parameters"), m_pxiFile(0)
{
  SetAutoLayout(true);
  
  wxBoxSizer *paramSizer = new wxBoxSizer(wxHORIZONTAL);

  wxString lambdaChoices[2] = { "Infinity", "Finite" };
  m_finiteLambda = new wxRadioBox(this, idQRE_TO_INFINITY, "Stopping lambda",
				  wxDefaultPosition, wxDefaultSize,
				  2, lambdaChoices, 1, wxRA_SPECIFY_COLS);
  paramSizer->Add(m_finiteLambda, 0, wxALL, 5);
  
  wxBoxSizer *maxLambdaSizer = new wxBoxSizer(wxVERTICAL);
  m_maxLambdaText = new wxStaticText(this, wxID_STATIC, "Maximum lambda");
  maxLambdaSizer->Add(m_maxLambdaText, 0, wxALL | wxCENTER, 5);
  m_maxLambdaValue = wxString::Format("%d", 100);
  m_maxLambda = new wxTextCtrl(this, -1, m_maxLambdaValue,
			       wxDefaultPosition, wxDefaultSize, 0,
			       gNumberValidator(&m_maxLambdaValue, 0, 10000),
			       "maximum lambda");
  maxLambdaSizer->Add(m_maxLambda, 0, wxALL | wxCENTER, 5);
  paramSizer->Add(maxLambdaSizer, 0, wxALL | wxCENTER, 5);

  m_generatePxiFile = new wxCheckBox(this, -1, "Generate .pxi file");

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(paramSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(m_generatePxiFile, 0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  m_finiteLambda->SetSelection(0);
  m_maxLambdaText->Show(false);
  m_maxLambda->Show(false);
}

dialogQre::~dialogQre()
{
  if (m_pxiFile) {
    delete m_pxiFile;
  }
}

void dialogQre::OnStoppingLambda(wxCommandEvent &)
{
  m_maxLambda->Show(m_finiteLambda->GetSelection() == 1);
  m_maxLambdaText->Show(m_finiteLambda->GetSelection() == 1);
}

void dialogQre::OnOK(wxCommandEvent &)
{
  if (m_generatePxiFile->GetValue()) {
    // Prompt user for PXI filename; if 'cancel', return to dialog
    wxFileDialog dialog(this, "Choose file for PXI output", "", "", "*.pxi");

    if (dialog.ShowModal() == wxID_OK) {
      try {
	m_pxiFile = new gFileOutput(dialog.GetPath());
	EndModal(wxID_OK);
      }
      catch (...) {
	wxMessageDialog message(&dialog, 
				wxString("Unable to open file '") +
				dialog.GetPath() + 
				wxString("' for writing."),
				"Error", wxOK);
	message.ShowModal();
	m_pxiFile = 0;
	return;
      }
    }
    // If cancelled, we fall through, and return to parameter dialog
    return;
  }

  EndModal(wxID_OK);
}

bool dialogQre::FiniteLambda(void) const
{
  return (m_finiteLambda->GetSelection() == 1);
}

bool dialogQre::GeneratePxiFile(void) const
{
  return m_generatePxiFile->GetValue();
}

double dialogQre::MaxLambda(void) const
{
  return ToNumber(m_maxLambda->GetValue().c_str());
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
    algorithm.SetMaxLambda(dialog.FiniteLambda() ? dialog.MaxLambda() : 10000);

    wxStatus status(p_parent, "QreSolve Progress");
    try {
      algorithm.Solve(p_support.GetGame(), 
		      (dialog.GeneratePxiFile()) ? *dialog.PxiFile() : gnull,
		      status, p_solutions);
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
    algorithm.SetMaxLambda(dialog.FiniteLambda() ? dialog.MaxLambda() : 10000);

    wxStatus status(p_parent, "QreSolve Progress");
    gList<MixedSolution> correspondence;
    try {
      Nfg *N = MakeReducedNfg(EFSupport(p_support.GetGame()));
      algorithm.Solve(*N,
		      (dialog.GeneratePxiFile()) ? *dialog.PxiFile() : gnull,
		      status, correspondence);
      p_solutions.Append(BehavSolution(BehavProfile<gNumber>(correspondence[1])));
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
    algorithm.SetMaxLambda(dialog.FiniteLambda() ? dialog.MaxLambda() : 10000);

    wxStatus status(p_parent, "QreSolve Progress");
    gList<MixedSolution> correspondence;
    try {
      algorithm.Solve(p_support.Game(),
		      (dialog.GeneratePxiFile()) ? *dialog.PxiFile() : gnull,
		      status, correspondence);
      p_solutions.Append(correspondence[1]);
    }
    catch (gSignalBreak &) { }
    return true;
  }
  return false;
}
