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

BEGIN_EVENT_TABLE(dialogEfgSolveStandard, guiAutoDialog)
  EVT_RADIOBOX(idTYPE_RADIOBOX, dialogEfgSolveStandard::OnChanged)
  EVT_RADIOBOX(idNUM_RADIOBOX, dialogEfgSolveStandard::OnChanged)
END_EVENT_TABLE()

dialogEfgSolveStandard::dialogEfgSolveStandard(wxWindow *p_parent,
					       const Efg::Game &p_efg)
  : guiAutoDialog(p_parent, "Standard Solution"), m_efg(p_efg)
{
  wxString standardType = "Nash", standardNum = "One", precision = "Float";
  wxConfig config("Gambit");
  config.Read("Solutions/Efg-Standard-Type", &standardType);
  config.Read("Solutions/Efg-Standard-Num", &standardNum);
  config.Read("Solutions/Efg-Standard-Precision", &precision);

  wxBoxSizer *radioSizer = new wxBoxSizer(wxHORIZONTAL);

  wxString typeChoices[] = { "Nash", "Subgame Perfect", "Sequential" };
  m_standardType = new wxRadioBox(this, idTYPE_RADIOBOX, "Type",
				  wxDefaultPosition, wxDefaultSize,
				  3, typeChoices, 0, wxRA_SPECIFY_ROWS);
  m_standardType->SetStringSelection(standardType);
  radioSizer->Add(m_standardType, 0, wxALL, 5);

  wxString numChoices[] = { "One", "Two", "All" };
  m_standardNum = new wxRadioBox(this, idNUM_RADIOBOX, "Number",
				 wxDefaultPosition, wxDefaultSize,
				 3, numChoices, 0, wxRA_SPECIFY_ROWS);
  m_standardNum->SetStringSelection(standardNum);
  radioSizer->Add(m_standardNum, 0, wxALL, 5);

  wxString precisionChoices[] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 0, wxRA_SPECIFY_ROWS);
  m_precision->SetStringSelection(precision);
  radioSizer->Add(m_precision, 0, wxALL, 5);
  
  m_description = new wxTextCtrl(this, -1);
  m_description->Enable(false);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(radioSizer, 0, wxALL, 5);
  topSizer->Add(m_description, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  wxCommandEvent event;
  OnChanged(event);
  Layout();
}

dialogEfgSolveStandard::~dialogEfgSolveStandard()
{
  wxConfig config("Gambit");
  config.Write("Solutions/Efg-Standard-Type",
	       m_standardType->GetStringSelection());
  config.Write("Solutions/Efg-Standard-Num",
	       m_standardNum->GetStringSelection());
  config.Write("Solutions/Efg-Standard-Precision",
	       m_precision->GetStringSelection());
}

void dialogEfgSolveStandard::OnChanged(wxCommandEvent &)
{
  switch (m_standardType->GetSelection()) {
  case 0:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (IsPerfectRecall(m_efg)) {
	if (m_efg.NumPlayers() == 2 && m_efg.IsConstSum()) {
	  m_description->SetValue("LpSolve[EFG]");
	  m_precision->Enable(true);
	}
	else if (m_efg.NumPlayers() == 2) {
	  m_description->SetValue("LcpSolve[EFG]");
	  m_precision->Enable(true);
	}
	else {
	  m_description->SetValue("SimpdivSolve[NFG]");
	  m_precision->SetSelection(0);
	  m_precision->Enable(false);
	}
      }
      else {
	m_description->SetValue("QreSolve[EFG]");
	m_precision->SetSelection(0);
	m_precision->Enable(false);
      }
      break;
    case 1:
      if (m_efg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve[NFG]");
	m_precision->Enable(true);
      }
      else {
	m_description->SetValue("LiapSolve[EFG]");
	m_precision->SetSelection(0);
	m_precision->Enable(false);
      }
      break;
    case 2:
      if (m_efg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve[NFG]");
	m_precision->Enable(true);
      }
      else {
	m_description->SetValue("PolEnumSolve[EFG]");
	m_precision->SetSelection(1);
	m_precision->Enable(false);
      }
      break;
    }
    break;
  case 1:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (IsPerfectRecall(m_efg)) {
	if (m_efg.NumPlayers() == 2 && m_efg.IsConstSum()) {
	  m_description->SetValue("LpSolve[EFG]");
	  m_precision->Enable(true);
	}
	else if (m_efg.NumPlayers() == 2) {
	  m_description->SetValue("LcpSolve[EFG]");
	  m_precision->Enable(true);
	}
	else {
	  m_description->SetValue("SimpdivSolve[NFG]");
	  m_precision->SetSelection(0);
	  m_precision->Enable(false);
	}
      }
      else {
	m_description->SetValue("QreSolve[EFG]");
	m_precision->SetSelection(0);
	m_precision->Enable(false);
      }
      break;
    case 1:
      if (m_efg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve[NFG]");
	m_precision->Enable(true);
      }
      else {
	m_description->SetValue("LiapSolve[EFG]");
	m_precision->SetSelection(0);
	m_precision->Enable(false);
      }
      break;
    case 2:
      if (m_efg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve[NFG]");
	m_precision->Enable(true);
      }
      else {
	m_description->SetValue("PolEnumSolve[EFG]");
	m_precision->SetSelection(1);
	m_precision->Enable(false);
      }
      break;
    }
    break;
  case 2:
    switch (m_standardNum->GetSelection()) {
    case 0:
      m_description->SetValue("QreSolve[EFG]");
      m_precision->SetSelection(0);
      m_precision->Enable(false);
      break;
    case 1:
    case 2:
      m_description->SetValue("LiapSolve[EFG]");
      m_precision->SetSelection(0);
      m_precision->Enable(false);
      break;
    }
  }
}

efgStandardType dialogEfgSolveStandard::Type(void) const
{
  switch (m_standardType->GetSelection()) {
  case 0:
    return efgSTANDARD_NASH;
  case 1:
    return efgSTANDARD_PERFECT;
  case 2:
    return efgSTANDARD_SEQUENTIAL;
  default:
    return efgSTANDARD_NASH;
  }
}

efgStandardNum dialogEfgSolveStandard::Number(void) const
{
  switch (m_standardNum->GetSelection()) {
  case 0:
    return efgSTANDARD_ONE;
  case 1:
    return efgSTANDARD_TWO;
  case 2:
    return efgSTANDARD_ALL;
  default:
    return efgSTANDARD_ALL;
  }
}

