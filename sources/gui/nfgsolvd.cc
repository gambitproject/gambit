//
// FILE: nfgsolvd.cc -- Standard solution dialog box for normal form
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"

#include "game/nfg.h"
#include "gambit.h"
#include "nfgsolvd.h"

//========================================================================
//                dialogNfgSolveStandard: Member functions
//========================================================================

BEGIN_EVENT_TABLE(dialogNfgSolveStandard, guiAutoDialog)
  EVT_RADIOBOX(idTYPE_RADIOBOX, dialogNfgSolveStandard::OnChanged)
  EVT_RADIOBOX(idNUM_RADIOBOX, dialogNfgSolveStandard::OnChanged)
END_EVENT_TABLE()

dialogNfgSolveStandard::dialogNfgSolveStandard(wxWindow *p_parent,
					       const Nfg &p_nfg)
  : guiAutoDialog(p_parent, "Standard Solution"), m_nfg(p_nfg)
{
  wxString standardType = "Nash", standardNum = "One", precision = "Float";
  wxConfig config("Gambit");
  config.Read("Solutions/Nfg-Standard-Type", &standardType);
  config.Read("Solutions/Nfg-Standard-Num", &standardNum);
  config.Read("Solutions/Nfg-Standard-Precision", &precision);

  wxBoxSizer *radioSizer = new wxBoxSizer(wxHORIZONTAL);

  wxString typeChoices[] = { "Nash", "Perfect" };
  m_standardType = new wxRadioBox(this, idTYPE_RADIOBOX, "Type",
				  wxDefaultPosition, wxDefaultSize,
				  (m_nfg.NumPlayers() == 2) ? 2 : 1,
				  typeChoices, 0, wxRA_SPECIFY_ROWS);
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

dialogNfgSolveStandard::~dialogNfgSolveStandard()
{
  wxConfig config("Gambit");
  config.Write("Solutions/Nfg-Standard-Type",
	       m_standardType->GetStringSelection());
  config.Write("Solutions/Nfg-Standard-Num",
	       m_standardNum->GetStringSelection());
  config.Write("Solutions/Nfg-Standard-Precision",
	       m_precision->GetStringSelection());
}

void dialogNfgSolveStandard::OnChanged(wxCommandEvent &)
{
  switch (m_standardType->GetSelection()) {
  case 0:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) {
	m_description->SetValue("LpSolve");
	m_precision->Enable(true);
      }
      else if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("LcpSolve");
	m_precision->Enable(true);
      }
      else {
	m_description->SetValue("SimpdivSolve");
	m_precision->Enable(true);
      }
      break;
    case 1:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(true);
      }
      else {
	m_description->SetValue("LiapSolve");
	m_precision->SetSelection(0);
	m_precision->Enable(false);
      }
      break;
    case 2:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(true);
      }
      else {
	m_description->SetValue("PolEnumSolve");
	m_precision->SetSelection(1);
	m_precision->Enable(false);
      }
      break;
    }
    break;

  case 1:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(true);
      }
      else 
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    case 1:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(true);
      }
      else
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    case 2:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(true);
      }
      else
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    }
    break;
  }
}

nfgStandardType dialogNfgSolveStandard::Type(void) const
{
  switch (m_standardType->GetSelection()) {
  case 0:
    return nfgSTANDARD_NASH;
  case 1:
    return nfgSTANDARD_PERFECT;
  default:
    return nfgSTANDARD_NASH;
  }
}

nfgStandardNum dialogNfgSolveStandard::Number(void) const
{
  switch (m_standardNum->GetSelection()) {
  case 0:
    return nfgSTANDARD_ONE;
  case 1:
    return nfgSTANDARD_TWO;
  case 2:
    return nfgSTANDARD_ALL;
  default:
    return nfgSTANDARD_ALL;
  }
}
