//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for computing quantal response equilibria
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "nash/nfgqre.h"
#include "dlnfgqre.h"

const int idRADIOBOX_SOLVEUSING = 2000;
const int idCHECKBOX_MULTIGRID = 2001;

BEGIN_EVENT_TABLE(dialogNfgQre, wxDialog)
  EVT_RADIOBOX(idRADIOBOX_SOLVEUSING, dialogNfgQre::OnSolveUsing)
  EVT_CHECKBOX(idCHECKBOX_MULTIGRID, dialogNfgQre::OnMultiGrid)
END_EVENT_TABLE()

dialogNfgQre::dialogNfgQre(wxWindow *p_parent, const NFSupport &p_support)
  : wxDialog(p_parent, -1, "Compute quantal response equilibria")
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString solveChoices[] = { "Trace principal branch",
			      "Use grid search" };
  m_solveUsing = new wxRadioBox(this, idRADIOBOX_SOLVEUSING, "Method",
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  m_solveUsing->SetSelection(0);
  topSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

  wxStaticBox *gridBox = new wxStaticBox(this, wxID_STATIC, 
					 "Grid search parameters");
  wxStaticBoxSizer *gridSizer = new wxStaticBoxSizer(gridBox, wxVERTICAL);

  wxStaticBox *lambdaBox = new wxStaticBox(this, wxID_STATIC,
					   "Search over lambda");
  wxStaticBoxSizer *lambdaSizer = new wxStaticBoxSizer(lambdaBox, wxVERTICAL);

  wxBoxSizer *startLambdaSizer = new wxBoxSizer(wxHORIZONTAL);
  startLambdaSizer->Add(new wxStaticText(this, wxID_STATIC, "Start at lambda"),
			0, wxALL | wxCENTER, 5);
  m_startLambda = new wxTextCtrl(this, -1, ".01");
  m_startLambda->Enable(false);
  startLambdaSizer->Add(m_startLambda, 0, wxALL | wxCENTER, 5);
  lambdaSizer->Add(startLambdaSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  wxBoxSizer *stopLambdaSizer = new wxBoxSizer(wxHORIZONTAL);
  stopLambdaSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop at lambda"),
		       0, wxALL | wxCENTER, 5);
  m_stopLambda = new wxTextCtrl(this, -1, "3");
  m_stopLambda->Enable(false);
  stopLambdaSizer->Add(m_stopLambda, 0, wxALL | wxCENTER, 5);
  lambdaSizer->Add(stopLambdaSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  wxBoxSizer *stepLambdaSizer = new wxBoxSizer(wxHORIZONTAL);
  stepLambdaSizer->Add(new wxStaticText(this, wxID_STATIC, "Increment by"),
		       0, wxALL | wxCENTER, 5);
  m_stepLambda = new wxTextCtrl(this, -1, ".1");
  m_stepLambda->Enable(false);
  stepLambdaSizer->Add(m_stepLambda, 0, wxALL | wxCENTER, 5);
  lambdaSizer->Add(stepLambdaSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  gridSizer->Add(lambdaSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *coarseBox = new wxStaticBox(this, wxID_STATIC,
					   "Settings for coarse grid");
  wxStaticBoxSizer *coarseSizer = new wxStaticBoxSizer(coarseBox, wxVERTICAL);

  wxBoxSizer *coarseGridSizer = new wxBoxSizer(wxHORIZONTAL);
  coarseGridSizer->Add(new wxStaticText(this, wxID_STATIC, "Grid size"),
			0, wxALL | wxCENTER, 5);
  m_del1 = new wxTextCtrl(this, -1, ".01");
  m_del1->Enable(false);
  coarseGridSizer->Add(m_del1, 0, wxALL | wxCENTER, 5);
  coarseSizer->Add(coarseGridSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  wxBoxSizer *coarseTolSizer = new wxBoxSizer(wxHORIZONTAL);
  coarseTolSizer->Add(new wxStaticText(this, wxID_STATIC, "Tolerance"),
		      0, wxALL | wxCENTER, 5);
  m_tol1 = new wxTextCtrl(this, -1, ".01");
  m_tol1->Enable(false);
  coarseTolSizer->Add(m_tol1, 0, wxALL | wxCENTER, 5);
  coarseSizer->Add(coarseTolSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  gridSizer->Add(coarseSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *fineBox = new wxStaticBox(this, wxID_STATIC,
					 "Settings for fine grid");
  wxStaticBoxSizer *fineSizer = new wxStaticBoxSizer(fineBox, wxVERTICAL);

  m_multiGrid = new wxCheckBox(this, idCHECKBOX_MULTIGRID, "Use fine grid");
  m_multiGrid->SetValue(true);
  m_multiGrid->Enable(false);
  fineSizer->Add(m_multiGrid, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *fineGridSizer = new wxBoxSizer(wxHORIZONTAL);
  fineGridSizer->Add(new wxStaticText(this, wxID_STATIC, "Grid size"),
		     0, wxALL | wxCENTER, 5);
  m_del2 = new wxTextCtrl(this, -1, ".0001");
  m_del2->Enable(false);
  fineGridSizer->Add(m_del2, 0, wxALL | wxCENTER, 5);
  fineSizer->Add(fineGridSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  wxBoxSizer *fineTolSizer = new wxBoxSizer(wxHORIZONTAL);
  fineTolSizer->Add(new wxStaticText(this, wxID_STATIC, "Tolerance"),
		    0, wxALL | wxCENTER, 5);
  m_tol2 = new wxTextCtrl(this, -1, ".0001");
  m_tol2->Enable(false);
  fineTolSizer->Add(m_tol2, 0, wxALL | wxCENTER, 5);
  fineSizer->Add(fineTolSizer, 0, wxLEFT | wxRIGHT | wxCENTER, 5);

  gridSizer->Add(fineSizer, 0, wxALL | wxCENTER, 5);

  topSizer->Add(gridSizer, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void dialogNfgQre::OnSolveUsing(wxCommandEvent &)
{
  m_startLambda->Enable(m_solveUsing->GetSelection() == 1);
  m_stopLambda->Enable(m_solveUsing->GetSelection() == 1);
  m_stepLambda->Enable(m_solveUsing->GetSelection() == 1);
  m_multiGrid->Enable(m_solveUsing->GetSelection() == 1);
  m_del1->Enable(m_solveUsing->GetSelection() == 1);
  m_tol1->Enable(m_solveUsing->GetSelection() == 1);
  m_del2->Enable(m_solveUsing->GetSelection() == 1 && m_multiGrid->GetValue());
  m_tol2->Enable(m_solveUsing->GetSelection() == 1 && m_multiGrid->GetValue());
}

void dialogNfgQre::OnMultiGrid(wxCommandEvent &)
{
  m_del2->Enable(m_multiGrid->GetValue());
  m_tol2->Enable(m_multiGrid->GetValue());
}
