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
				  2, typeChoices, 0, wxVERTICAL);
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
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) 
	m_description->SetValue("LpSolve");
      else if (m_nfg.NumPlayers() == 2 && !IsConstSum(m_nfg))
	m_description->SetValue("LcpSolve");
      else
	m_description->SetValue("SimpdivSolve");
      m_precision->Enable(TRUE);
      break;
    case 1:
    case 2:
      if (m_nfg.NumPlayers() == 2) {
	m_description->SetValue("EnumMixedSolve");
	m_precision->Enable(TRUE);
      }
      else {
	m_description->SetValue("LiapSolve");
	m_precision->Enable(FALSE);
      }
      break;
    }
    break;

  case 1:
    switch (m_standardNum->GetSelection()) {
    case 0:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) 
	m_description->SetValue("LpSolve");
      else if (m_nfg.NumPlayers() == 2 && !IsConstSum(m_nfg))
	m_description->SetValue("LcpSolve");
      else
	m_description->SetValue("SimpdivSolve");
      m_precision->Enable(TRUE);
      break;
    case 1:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) 
	m_description->SetValue("LpSolve");
      else if (m_nfg.NumPlayers() == 2 && !IsConstSum(m_nfg))
	m_description->SetValue("LcpSolve");
      else
	m_description->SetValue("NOT IMPLEMENTED");
      break;
    case 2:
      if (m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg)) 
	m_description->SetValue("LpSolve");
      else if (m_nfg.NumPlayers() == 2 && !IsConstSum(m_nfg))
	m_description->SetValue("LcpSolve");
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

#ifdef UNUSED
void NfgSolveStandardDialog::StandardSettings(void)
{
  int  stopAfter=1, dom_type = DOM_WEAK;
  bool use_elimdom = TRUE, all=TRUE;

  // a separate case for each of the possible alg/num/game combinations
  // One Nash 2 person
  if (standard_type == STANDARD_NASH && 
      standard_num == STANDARD_ONE && nf.NumPlayers() == 2) {
    algorithm = (IsConstSum(nf)) ? NFG_LP_SOLUTION : NFG_LCP_SOLUTION;
    stopAfter = 1;
    dom_type = DOM_WEAK;
    all = TRUE;
    use_elimdom = TRUE;
  }

  // One Nash n person
  if (standard_type == STANDARD_NASH && 
      standard_num == STANDARD_ONE && nf.NumPlayers() != 2) {
    algorithm = NFG_SIMPDIV_SOLUTION;
    stopAfter = 1;
    dom_type = DOM_WEAK;
    all = TRUE;
    use_elimdom = TRUE;
  }

  // Two Nash 2 person
  if (standard_type == STANDARD_NASH && 
      standard_num == STANDARD_TWO && nf.NumPlayers() == 2) {
    algorithm = NFG_ENUMMIXED_SOLUTION;
    stopAfter = 2;
    dom_type = DOM_STRONG;
    all = TRUE;
    use_elimdom = TRUE;
  }

  // Two Nash n person
  if (standard_type == STANDARD_NASH && 
      standard_num == STANDARD_TWO && nf.NumPlayers() != 2) {
    algorithm = NFG_LIAP_SOLUTION;
    stopAfter = 2;
    dom_type = DOM_STRONG;
    all = TRUE;
    use_elimdom = TRUE;
    wxWriteResource(PARAMS_SECTION, "Liap-Ntries", 2 * stopAfter,
		    defaults_file);
  }

  // All Nash 2 person
  if (standard_type == STANDARD_NASH && 
      standard_num == STANDARD_ALL && nf.NumPlayers() == 2) {
    algorithm = NFG_ENUMMIXED_SOLUTION;
    stopAfter = 0;
    dom_type = DOM_STRONG;
    all = TRUE;
    use_elimdom = TRUE;
  }

  // All Nash n person
  if (standard_type == STANDARD_NASH && 
      standard_num == STANDARD_ALL && nf.NumPlayers() != 2) {
    algorithm = NFG_LIAP_SOLUTION;
    stopAfter = 0;
    dom_type = DOM_STRONG;
    all = TRUE;
    use_elimdom = TRUE;
    Warn("Not guaranteed to find all solutions for "
	 "'All Nash' n-person games\n");
    wxWriteResource(PARAMS_SECTION, "Liap-Ntries", 2*stopAfter, defaults_file);
  }

  // One Perfect 2 person
  if (standard_type == STANDARD_PERFECT && 
      standard_num == STANDARD_ONE && nf.NumPlayers() == 2) {
    algorithm = (IsConstSum(nf)) ? NFG_LP_SOLUTION : NFG_LCP_SOLUTION;
    stopAfter = 1;
    dom_type = DOM_WEAK;
    all = TRUE;
    use_elimdom = TRUE;
  }

  // One Perfect n person
  if (standard_type == STANDARD_PERFECT && 
      standard_num == STANDARD_ONE && nf.NumPlayers() != 2) {
    algorithm = NFG_SIMPDIV_SOLUTION;
    stopAfter = 1;
    use_elimdom = true;
    all = true;
    dom_type = DOM_WEAK;
  }

  // Two Perfect 2 person
  if (standard_type == STANDARD_PERFECT
      && standard_num == STANDARD_TWO
      && nf.NumPlayers() == 2) {
    algorithm = (IsConstSum(nf)) ? NFG_LP_SOLUTION : NFG_LCP_SOLUTION;
    stopAfter = 2;
    dom_type = DOM_WEAK;
    all = TRUE;
    use_elimdom = TRUE;
    Warn("Not guaranteed to find 2 solutions for 'Two Perfect'");
  }

  // Two Perfect n person
  if (standard_type == STANDARD_PERFECT
      && standard_num == STANDARD_TWO
      && nf.NumPlayers() != 2) {
    throw guiBadStandardSolve("Two Perfect not implemented for n person games");
  }

  // All Perfect 2 person
  if (standard_type == STANDARD_PERFECT && 
      standard_num == STANDARD_ALL && nf.NumPlayers() == 2) {
    algorithm = (IsConstSum(nf)) ? NFG_LP_SOLUTION : NFG_LCP_SOLUTION;
    stopAfter = 0;
    dom_type = DOM_WEAK;
    all = TRUE;
    use_elimdom = TRUE;
    Warn("Not guaranteed to find all solutions for "
	 "'All Perfect' 2-person games\n");
  }

  // All Perfect n person
  if (standard_type == STANDARD_PERFECT && 
      standard_num == STANDARD_ALL && nf.NumPlayers() != 2) {
    throw guiBadStandardSolve("All Perfect not implemented for n-person games");
  }

  // -------- now write the new settings to file
  wxWriteResource(SOLN_SECT, "Nfg-Algorithm", algorithm, defaults_file);
  wxWriteResource(PARAMS_SECTION, "Stop-After", stopAfter, defaults_file);
  wxWriteResource(SOLN_SECT, "Nfg-ElimDom-All", (int) all, defaults_file);
  wxWriteResource(SOLN_SECT, "Nfg-ElimDom-Type", dom_type, defaults_file);
  wxWriteResource(SOLN_SECT, "Nfg-ElimDom-Use", (int) use_elimdom,
		  defaults_file);
}
#endif  // UNUSED

