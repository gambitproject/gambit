//
// FILE: efgsolvd.cc -- Standard solution methods for extensive form
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"

#include "efg.h"
#include "gambit.h"
#include "efgsolvd.h"

//========================================================================
//                dialogEfgSolveStandard: Member functions
//========================================================================

const int idTYPE_CHOICE = 1000;

BEGIN_EVENT_TABLE(dialogEfgSolveStandard, wxDialog)
  EVT_RADIOBOX(idTYPE_CHOICE, dialogEfgSolveStandard::OnChanged)
END_EVENT_TABLE()

dialogEfgSolveStandard::dialogEfgSolveStandard(wxWindow *p_parent,
					       const Efg::Game &p_efg)
  : wxDialog(p_parent, -1, "Standard Equilibrium Algorithms"), m_efg(p_efg)
{
  wxString typeChoices[] = { "One Nash equilibrium",
			     "Two Nash equilibria",
			     "All Nash equilibria",
			     "One subgame-perfect Nash equilibrium",
			     "Two subgame-perfect Nash equilibria",
			     "All subgame-perfect Nash equilibria",
			     "One sequential Nash equilibrium",
			     "Two sequential Nash equilibria",
			     "All sequential Nash equilibria" };
  m_equilibriumType = new wxRadioBox(this, idTYPE_CHOICE, "What to compute",
				     wxDefaultPosition, wxDefaultSize,
				     9, typeChoices, 3, wxRA_SPECIFY_COLS);

  wxString precisionChoices[] = { "Use floating-point arithmetic (faster)",
				  "Use rational numbers (more precise)" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_COLS);

  wxBoxSizer *algorithmSizer = new wxBoxSizer(wxHORIZONTAL);
  algorithmSizer->Add(new wxStaticText(this, wxID_STATIC, "Algorithm"),
		      0, wxALL | wxCENTER, 5);
  m_algorithm = new wxTextCtrl(this, -1);
  m_algorithm->Enable(false);
  algorithmSizer->Add(m_algorithm, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *detailsSizer = new wxBoxSizer(wxHORIZONTAL);
  detailsSizer->Add(new wxStaticText(this, wxID_STATIC, "Details"),
		    0, wxALL | wxCENTER, 5);
  m_details = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize,
			     wxTE_MULTILINE);
  m_details->SetSize(m_algorithm->GetSize().GetWidth(),
		     2 * m_algorithm->GetSize().GetHeight());
  m_details->Enable(false);
  detailsSizer->Add(m_details, 1, wxALL | wxEXPAND, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton *helpButton = new wxButton(this, wxID_HELP, "Help");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);
  buttonSizer->Add(helpButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_equilibriumType, 0, wxALL | wxCENTER, 5);
  topSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);
  topSizer->Add(algorithmSizer, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(detailsSizer, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  UpdateFields();
}

dialogEfgSolveStandard::~dialogEfgSolveStandard()
{
}

void dialogEfgSolveStandard::OnChanged(wxCommandEvent &)
{
  UpdateFields();
}

void dialogEfgSolveStandard::UpdateFields(void)
{
  switch (m_equilibriumType->GetSelection()) {
  case 0:  // one Nash equilibrium
  case 3:  // one subgame perfect equilibrium
    if (IsPerfectRecall(m_efg)) {
      if (m_efg.NumPlayers() == 2 && m_efg.IsConstSum()) {
	m_algorithm->SetValue("LpSolve[EFG]");
	m_precision->Show(true);
	m_details->SetValue("Uses linear programming on "
			    "the associated sequence form.");
      }
      else if (m_efg.NumPlayers() == 2) {
	m_algorithm->SetValue("LcpSolve[EFG]");
	m_precision->Show(true);
	m_details->SetValue("Uses linear complementarity algorithms on "
			    "the associated sequence form.");
      }
      else {
	m_algorithm->SetValue("SimpdivSolve[NFG]");
	m_precision->Show(false);
	m_details->SetValue("Uses simplicial subdivision on the "
			    "associated reduced normal form.  This "
			    "algorithm uses floating-point arithmetic.");
      }
    }
    else {
      m_algorithm->SetValue("QreSolve[EFG]");
      m_precision->Show(false);
      m_details->SetValue("Uses agent logistic quantal response equilibrium "
			  "correspondence tracing.  This "
			  "algorithm uses floating-point arithmetic.");
    }
    break;
  case 1:  // two Nash equilibria
  case 4:  // two subgame perfect equilibria
    if (m_efg.NumPlayers() == 2) {
      m_algorithm->SetValue("EnumMixedSolve[NFG]");
      m_precision->Show(true);
      m_details->SetValue("Uses enumeration of mixed strategies "
			  "on the associated normal form.");
    }
    else {
      m_algorithm->SetValue("LiapSolve[NFG]");
      m_precision->Show(false);
      m_details->SetValue("Uses Liapunov function minimization. "
			  "Note: this algorithm is not guaranteed to find "
			  "two equilibria. "
			  "This algorithm uses floating-point arithmetic.");
    }
    break;
  case 2:  // all Nash equilibria
  case 5:  // all subgame perfect equilibria
    if (m_efg.NumPlayers() == 2) {
      m_algorithm->SetValue("EnumMixedSolve[NFG]");
      m_precision->Show(true);
      m_details->SetValue("Uses enumeration of mixed strategies "
			  "on the associated normal form.");
    }
    else {
      m_algorithm->SetValue("PolEnumSolve[EFG]");
      m_precision->Show(false);
      m_details->SetValue("Uses enumeration of solutions to systems "
			  "of polynomial equations and inequalities. "
			  "This algorithm uses rational arithmetic.");
    }
  case 6:  // one sequential equilibrium
      m_algorithm->SetValue("QreSolve[EFG]");
      m_precision->Show(false);
      m_details->SetValue("Uses agent logistic quantal response equilibrium "
			  "correspondence tracing.  This "
			  "algorithm uses floating-point arithmetic.");
      break;
  case 7:  // two sequential equilibria
  case 8:  // all sequential equilibria
    m_algorithm->SetValue("LiapSolve[EFG]");
    m_precision->Show(false);
    m_details->SetValue("Uses Liapunov function minimization. "
			"Note: this algorithm is not guaranteed to find "
			"two equilibria. "
			"This algorithm uses floating-point arithmetic.");
    break;
  default:  // shouldn't happen; just clear entries if it does
    m_algorithm->SetValue("");
    m_details->SetValue("");
    break;
  }
}

efgStandardType dialogEfgSolveStandard::Type(void) const
{
  return efgSTANDARD_NASH;
}

efgStandardNum dialogEfgSolveStandard::Number(void) const
{
  return efgSTANDARD_ONE;
}

