//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to select from standard algorithms for normal form games
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "game/nfg.h"
#include "nfgshow.h"
#include "dlnfgstandard.h"

//========================================================================
//                dialogNfgSolveStandard: Member functions
//========================================================================

const int idTYPE_CHOICE = 1000;

BEGIN_EVENT_TABLE(dialogNfgSolveStandard, wxDialog)
  EVT_RADIOBOX(idTYPE_CHOICE, dialogNfgSolveStandard::OnChanged)
END_EVENT_TABLE()

dialogNfgSolveStandard::dialogNfgSolveStandard(wxWindow *p_parent,
					       const Nfg &p_nfg)
  : wxDialog(p_parent, -1, "Standard Equilibrium Algorithms"), m_nfg(p_nfg)
{
  if (p_nfg.NumPlayers() == 2) {
    wxString typeChoices[] = { "One Nash equilibrium",
			       "Two Nash equilibria",
			       "All Nash equilibria",
			       "One perfect Nash equilibrium",
			       "Two perfect Nash equilibria",
			       "All perfect Nash equilibria" };
    m_equilibriumType = new wxRadioBox(this, idTYPE_CHOICE, "What to compute",
				       wxDefaultPosition, wxDefaultSize,
				       6, typeChoices, 2, wxRA_SPECIFY_COLS);
  }
  else {

    wxString typeChoices[] = { "One Nash equilibrium",
			       "Two Nash equilibria",
			       "All Nash equilibria" };
    m_equilibriumType = new wxRadioBox(this, idTYPE_CHOICE, "What to compute",
				       wxDefaultPosition, wxDefaultSize,
				       3, typeChoices, 1, wxRA_SPECIFY_COLS);
  }

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

dialogNfgSolveStandard::~dialogNfgSolveStandard()
{
}

void dialogNfgSolveStandard::OnChanged(wxCommandEvent &)
{
  UpdateFields();
}

void dialogNfgSolveStandard::UpdateFields(void)
{
  switch (m_equilibriumType->GetSelection()) {
  case 0:  // one Nash equilibrium
    if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) {
      m_algorithm->SetValue("LpSolve[NFG]");
      m_precision->Enable(true);
      m_details->SetValue("Uses linear programming, after eliminating "
			  "weakly dominated strategies.");
    }
    else if (m_nfg.NumPlayers() == 2) {
      m_algorithm->SetValue("LcpSolve[NFG]");
      m_precision->Enable(true);
      m_details->SetValue("Uses linear complementarity algorithms, "
			  "after eliminating weakly dominated strategies.");

    }
    else {
      m_algorithm->SetValue("SimpdivSolve[NFG]");
	m_precision->Enable(false);
	m_precision->SetSelection(0);
	m_details->SetValue("Uses simplicial subdivision on the "
			    "associated reduced normal form.  This "
			    "algorithm uses floating-point arithmetic.");
    }
    break;
  case 1:  // two Nash equilibria
    if (m_nfg.NumPlayers() == 2) {
      m_algorithm->SetValue("EnumMixedSolve[NFG]");
      m_precision->Enable(true);
      m_details->SetValue("Uses enumeration of mixed strategies, "
			  "after eliminating strictly dominated strategies");
    }
    else {
      m_algorithm->SetValue("LiapSolve[NFG]");
      m_precision->Enable(false);
      m_precision->SetSelection(0);
      m_details->SetValue("Uses Liapunov function minimization, "
			  "after eliminating strictly dominated strategies. "
			  "Note: this algorithm is not guaranteed to find "
			  "two equilibria. "
			  "This algorithm uses floating-point arithmetic.");
    }
    break;
  case 2:  // all Nash equilibria
    if (m_nfg.NumPlayers() == 2) {
      m_algorithm->SetValue("EnumMixedSolve[NFG]");
      m_precision->Enable(true);
      m_details->SetValue("Uses enumeration of mixed strategies, "
			  "after eliminating strictly dominated strategies");
    }
    else {
      m_algorithm->SetValue("PolEnumSolve[NFG]");
      m_precision->Enable(false);
      m_precision->SetSelection(1);
      m_details->SetValue("Uses enumeration of solutions to systems "
			  "of polynomial equations and inequalities. "
			  "This algorithm uses rational arithmetic.");
    }
    break;
  case 3:  // one perfect Nash equilibrium
    m_algorithm->SetValue("LcpSolve[NFG]");
    m_precision->Enable(true);
    m_details->SetValue("Uses linear complementarity algorithms, "
			"after eliminating weakly dominated strategies.");
    break;
  case 4:  // two perfect Nash equilibria
    m_algorithm->SetValue("EnumMixedSolve[NFG]");
    m_precision->Enable(true);
    m_details->SetValue("Uses enumeration of mixed strategies, "
			"after eliminating weakly dominated strategies");
    break;
  case 5:  // all perfect Nash equilibria
    m_algorithm->SetValue("EnumMixedSolve[NFG]");
    m_precision->Enable(true);
    m_details->SetValue("Uses enumeration of mixed strategies, "
			"after eliminating weakly dominated strategies");
    break;
  default:  // shouldn't happen; just clear entries if it does
    m_algorithm->SetValue("");
    m_details->SetValue("");
    break;
  }
}
