//
// FILE: nfgsolvd.cc -- Standard solution dialog box for normal form
//
//
//

#include "wx.h"
#include "wxmisc.h"
#include "nfgsolvd.h"

//========================================================================
//               NfgSolveSettings: Private member functions
//========================================================================

void NfgSolveSettings::Warn(const char *warning) 
{
  wxMessageBox((char *) warning, "Standard Solution");
}

//========================================================================
//             NfgSolveSettings: Constructor and destructor
//========================================================================

NfgSolveSettings::NfgSolveSettings(const Nfg &p_nfg, bool p_solving /*= true*/)
  : solving(p_solving), nf(p_nfg)
{
  result = SD_SAVE;
  defaults_file = "gambit.ini";
  wxGetResource(SOLN_SECT, "Nfg-Algorithm", &algorithm, defaults_file);
  wxGetResource(SOLN_SECT, "Nfg-Efg", &extensive, defaults_file);
  wxGetResource(SOLN_SECT, "Nfg-Standard-Type", &standard_type, defaults_file);
  wxGetResource(SOLN_SECT, "Nfg-Standard-Num", &standard_num, defaults_file);
  wxGetResource(SOLN_SECT, "Nfg-Auto-Inspect-Solns", &auto_inspect,
		defaults_file);
}

NfgSolveSettings::~NfgSolveSettings()
{
  if (result != SD_CANCEL) {
    char *defaults_file = "gambit.ini";
    wxWriteResource(SOLN_SECT, "Nfg-Algorithm", algorithm, defaults_file);
    wxWriteResource(SOLN_SECT, "Nfg-Efg", extensive, defaults_file);
    wxWriteResource(SOLN_SECT, "Nfg-Standard-Type", standard_type,
		    defaults_file);
    wxWriteResource(SOLN_SECT, "Nfg-Standard-Num", standard_num,
		    defaults_file);
    wxWriteResource(SOLN_SECT, "Nfg-Auto-Inspect-Solns", auto_inspect,
		    defaults_file);
  }
}

//========================================================================
//                NfgSolveStandardDialog: Member functions
//========================================================================

#define NFG_STANDARD_HELP "Normal Form Standard Solutions"

NfgSolveStandardDialog::NfgSolveStandardDialog(const Nfg &p_nfg,
					       wxWindow *p_parent)
  : NfgSolveSettings(p_nfg, false),
    MyDialogBox(p_parent, "Standard Solution", NFG_STANDARD_HELP),
    m_standardTypeStr(new char[20]), m_standardNumStr(new char[20]),
    m_precisionStr(new char[20])
{
  gText defaultsFile("gambit.ini");
  wxGetResource(PARAMS_SECTION, "Precision", &m_precisionStr, defaultsFile);

  m_standardTypeList = new wxStringList("Nash", "Perfect", 0);
  m_standardNumList = new wxStringList("One", "Two", "All", 0);
  m_precisionList = new wxStringList("Float", "Rational", 0);

  strcpy(m_standardTypeStr,
	 (char *) m_standardTypeList->Nth(standard_type)->Data());
  strcpy(m_standardNumStr,
	 (char *) m_standardNumList->Nth(standard_num)->Data());

  Add(wxMakeFormString("Type", &m_standardTypeStr, wxFORM_RADIOBOX,
		       new wxList(wxMakeConstraintStrings(m_standardTypeList),
				  0)));
  Add(wxMakeFormString("Number", &m_standardNumStr, wxFORM_RADIOBOX,
		       new wxList(wxMakeConstraintStrings(m_standardNumList),
				  0)));
  Add(wxMakeFormString("Precision", &m_precisionStr, wxFORM_RADIOBOX,
		       new wxList(wxMakeConstraintStrings(m_precisionList),
				  0), 0, wxHORIZONTAL));
  Go();

  if (Completed() == wxOK) {
    standard_type = wxListFindString(m_standardTypeList, m_standardTypeStr);
    standard_num = wxListFindString(m_standardNumList, m_standardNumStr);
    m_precision = ((wxListFindString(m_precisionList, m_precisionStr) == 0)
		   ? precDOUBLE : precRATIONAL);
    StandardSettings();
    result = SD_SAVE;
  }
  else {
    result = SD_CANCEL;
  } 
}

NfgSolveStandardDialog::~NfgSolveStandardDialog()
{
  gText defaultsFile("gambit.ini");
  wxWriteResource(PARAMS_SECTION, "Precision", m_precisionStr,
		  defaultsFile);

  delete [] m_standardTypeStr;
  delete [] m_standardNumStr;
  delete [] m_precisionStr;
  delete m_standardTypeList;
  delete m_standardNumList;
  delete m_precisionList;
}

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


