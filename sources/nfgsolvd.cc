//
// FILE: nfgsolvd.cc -- Standard solution dialog box for normal form
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "nfg.h"
#include "nfgsolvd.h"

static const char *SOLN_SECT = "Soln-Defaults";

//========================================================================
//                dialogNfgSolveStandard: Member functions
//========================================================================

#define NFG_STANDARD_HELP "Normal Form Standard Solutions"

dialogNfgSolveStandard::dialogNfgSolveStandard(const Nfg &p_nfg,
					       wxWindow *p_parent)
  : wxDialogBox(p_parent, "Standard Solution", TRUE), m_nfg(p_nfg)
{
  gText defaultsFile("gambit.ini");
  int standardType = 0, standardNum = 0, precision = 0;
  wxGetResource(SOLN_SECT, "Nfg-Standard-Type", &standardType,
		defaultsFile);
  wxGetResource(SOLN_SECT, "Nfg-Standard-Num", &standardNum,
		defaultsFile);
  wxGetResource(SOLN_SECT, "Nfg-Standard-Precision", &precision, defaultsFile);

  char *typeChoices[] = { "Nash", "Perfect" };
  m_standardType = new wxRadioBox(this, (wxFunction) CallbackChanged,
				  "Type", -1, -1, -1, -1,
				  (m_nfg.NumPlayers() == 2) ? 2 : 1,
				  typeChoices, 0, wxVERTICAL);
  m_standardType->SetClientData((char *) this);
  m_standardType->SetSelection(standardType);

  char *numChoices[] = { "One", "Two", "All" };
  m_standardNum = new wxRadioBox(this, (wxFunction) CallbackChanged,
				 "Number", -1, -1, -1, -1,
				 3, numChoices, 0, wxVERTICAL);
  m_standardNum->SetClientData((char *) this);
  m_standardNum->SetSelection(standardNum);

  char *precisionChoices[] = { "Float", "Rational" };
  m_precision = new wxRadioBox(this, 0, "Precision", -1, -1, -1, -1,
			       2, precisionChoices, 0, wxVERTICAL);
  m_precision->SetSelection(precision);
  
  NewLine();

  m_description = new wxText(this, 0, "Using algorithm");
  m_description->Enable(FALSE);

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "OK");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  OnChanged();

  Fit();
  Show(TRUE);
}

dialogNfgSolveStandard::~dialogNfgSolveStandard()
{
  if (m_completed == wxOK) {
    gText defaultsFile("gambit.ini");
    wxWriteResource(SOLN_SECT, "Nfg-Standard-Type",
		    m_standardType->GetSelection(), defaultsFile);
    wxWriteResource(SOLN_SECT, "Nfg-Standard-Num",
		    m_standardNum->GetSelection(), defaultsFile);
    wxWriteResource(SOLN_SECT, "Nfg-Standard-Precision",
		    m_precision->GetSelection(), defaultsFile);
  }
}

void dialogNfgSolveStandard::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogNfgSolveStandard::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogNfgSolveStandard::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void dialogNfgSolveStandard::OnChanged(void)
{
  switch (m_standardType->GetSelection()) {
  case 0:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) {
	m_description->SetValue("LpSolve");
	m_precision->Enable(TRUE);
      }
      else if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("LcpSolve");
	m_precision->Enable(TRUE);
      }
      else {
	m_description->SetValue("SimpdivSolve");
	m_precision->Enable(FALSE);
	m_precision->SetSelection(0);
      }
      break;
    case 1:
    case 2:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(TRUE);
      }
      else {
	m_description->SetValue("LiapSolve");
	m_precision->SetSelection(0);
	m_precision->Enable(FALSE);
      }
      break;
    }
    break;

  case 1:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) {
	m_description->SetValue("LpSolve");
	m_precision->Enable(TRUE);
      }
      else if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("LcpSolve");
	m_precision->Enable(TRUE);
      }
      else 
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    case 1:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) {
	m_description->SetValue("LpSolve");
	m_precision->Enable(TRUE);
      }
      else if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("LcpSolve");
	m_precision->Enable(TRUE);
      }
      else
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    case 2:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) { 
	m_description->SetValue("LpSolve");
	m_precision->Enable(TRUE);
      }
      else if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("LcpSolve");
	m_precision->Enable(TRUE);
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
