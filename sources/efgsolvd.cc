//
// FILE: efgsolvd.cc -- Standard solution methods for extensive form
//
//
//

// Most of these probably aren't necessary... figure out which ones are!

#include "wx.h"
#include "wxmisc.h"
#include "efg.h"
#include "efgconst.h"
#include "treewin.h"
#include "efgshow.h"
#include "efgsoln.h"
#include "nfggui.h"
#include "efgnfgi.h"
#include "behavsol.h"

#include "efgsolvd.h"

//=========================================================================
//               EfgSolveSettings: Private member functions
//=========================================================================

void EfgSolveSettings::Warn(const char *p_warning) 
{
  if (solving) {
    wxMessageBox((char *) p_warning, "Standard Solution");
  }
}

//=========================================================================
//              EfgSolveSettings: Constructor and destructor
//=========================================================================

EfgSolveSettings::EfgSolveSettings(const Efg &p_efg, bool p_solving /*= true*/)
  : solving(p_solving), ef(p_efg)
{
  result = SD_SAVE;
  defaults_file = "gambit.ini";
  wxGetResource(SOLN_SECT, "Use-Nfg", &use_nfg, defaults_file);
  char *alg_sect=(use_nfg) ? "Nfg-Algorithm" : "Efg-Algorithm";
  wxGetResource(SOLN_SECT,alg_sect,&algorithm,defaults_file);
  wxGetResource(SOLN_SECT,"Efg-Nfg",&normal,defaults_file);
  wxGetResource(SOLN_SECT,"Efg-Mark-Subgames",&subgames,defaults_file);
  wxGetResource(SOLN_SECT,"Efg-Interactive-Solns",&pick_solns,defaults_file);
  wxGetResource(SOLN_SECT,"Efg-Auto-Inspect-Solns",&auto_inspect,defaults_file);
}

EfgSolveSettings::~EfgSolveSettings()
{
  if (result != SD_CANCEL) {
    wxWriteResource(SOLN_SECT,"Use-Nfg",use_nfg,defaults_file);
    char *alg_sect=(use_nfg) ? "Nfg-Algorithm" : "Efg-Algorithm";
    wxWriteResource(SOLN_SECT,alg_sect,algorithm,defaults_file);
    wxWriteResource(SOLN_SECT,"Efg-Nfg",normal,defaults_file);
    wxWriteResource(SOLN_SECT,"Efg-Interactive-Solns",pick_solns,defaults_file);
    wxWriteResource(SOLN_SECT,"Efg-Auto-Inspect-Solns",auto_inspect,defaults_file);
  }
}

//========================================================================
//                EfgSolveStandardDialog: Member functions
//========================================================================

EfgSolveStandardDialog::EfgSolveStandardDialog(const Efg &p_efg,
					       wxWindow *p_parent)
  : EfgSolveSettings(p_efg, false),
    MyDialogBox(p_parent, "Standard Solution", EFG_STANDARD_HELP),
    m_standardTypeStr(new char[20]), m_standardNumStr(new char[20]),
    m_precisionStr(new char[20])
{
  gText defaultsFile("gambit.ini");
  wxGetResource(SOLN_SECT, "Efg-Standard-Type", &m_standardType, 
		defaultsFile);
  wxGetResource(SOLN_SECT, "Efg-Standard-Num", &m_standardNum,
		defaultsFile);
  wxGetResource(PARAMS_SECTION, "Precision", &m_precisionStr, defaultsFile);

  m_standardTypeList = new wxStringList("Nash", "Subgame Perfect",
					"Sequential", 0);
  m_standardNumList = new wxStringList("One", "Two", "All", 0);
  m_precisionList = new wxStringList("Float", "Rational", 0);

  strcpy(m_standardTypeStr,
	 (char *) m_standardTypeList->Nth(m_standardType)->Data());
  strcpy(m_standardNumStr,
	 (char *) m_standardNumList->Nth(m_standardNum)->Data());

  Add(wxMakeFormString("Type", &m_standardTypeStr, wxFORM_RADIOBOX,
		       new wxList(wxMakeConstraintStrings(m_standardTypeList),
				  0)));
  Add(wxMakeFormString("Number", &m_standardNumStr, wxFORM_RADIOBOX,
		       new wxList(wxMakeConstraintStrings(m_standardNumList),
				  0)));
  Add(wxMakeFormNewLine());
  Add(wxMakeFormString("Precision", &m_precisionStr, wxFORM_RADIOBOX,
		       new wxList(wxMakeConstraintStrings(m_precisionList),
				  0), 0, wxHORIZONTAL));
  Go();

  if (Completed() == wxOK) {
    m_standardType = wxListFindString(m_standardTypeList, m_standardTypeStr);
    m_standardNum = wxListFindString(m_standardNumList, m_standardNumStr);
    m_precision = ((wxListFindString(m_precisionList, m_precisionStr) == 0)
		   ? precRATIONAL : precDOUBLE);
    StandardSettings();
    result = SD_SAVE;
  }
  else
    result = SD_CANCEL;
}

EfgSolveStandardDialog::~EfgSolveStandardDialog()
{
  gText defaultsFile("gambit.ini");
  wxWriteResource(SOLN_SECT, "Efg-Standard-Type", m_standardType,
		  defaultsFile);
  wxWriteResource(SOLN_SECT, "Efg-Standard-Num", m_standardNum,
		  defaultsFile);
  wxWriteResource(PARAMS_SECTION, "Precision", m_precisionStr,
		  defaultsFile);

  delete [] m_standardTypeStr;
  delete [] m_standardNumStr;
  delete [] m_precisionStr;

  delete m_standardTypeList;
  delete m_standardNumList;
  delete m_precisionList;
}

void EfgSolveStandardDialog::StandardSettings(void)
{
  int stopAfter=1,max_solns=1,dom_type=DOM_WEAK;
  bool use_elimdom=true,all=true;

  Infoset *bad1,*bad2;
  bool perf=IsPerfectRecall(ef,bad1,bad2);

  // a separate case for each of the possible alg/num/game combinations
  // One Nash for 2 person
  if (m_standardType == efgSTANDARD_NASH && m_standardNum == efgSTANDARD_ONE
      && ef.NumPlayers() == 2) {
    use_nfg=FALSE;
    if (perf)
      algorithm=(ef.IsConstSum()) ? EFG_CSUM_SOLUTION : EFG_LCP_SOLUTION;
    else
      algorithm=EFG_QRE_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=true;all=true;dom_type=DOM_WEAK;
    subgames=TRUE;
  }

  // One Nash for n person
  if (m_standardType==efgSTANDARD_NASH && m_standardNum==efgSTANDARD_ONE
      && ef.NumPlayers()!=2) {
    use_nfg=TRUE;algorithm=NFG_SIMPDIV_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=true;all=true;dom_type=DOM_WEAK;
    subgames=TRUE;
  }
	
  // Two Nash 2 person
  if (m_standardType==efgSTANDARD_NASH && m_standardNum==efgSTANDARD_TWO
      && ef.NumPlayers()==2) {
    use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
  }
	
  // Two Nash n person
  if (m_standardType==efgSTANDARD_NASH && m_standardNum==efgSTANDARD_TWO 
      && ef.NumPlayers()!=2) {
    use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
  }
	
  // All Nash 2 person
  if (m_standardType==efgSTANDARD_NASH && m_standardNum==efgSTANDARD_ALL 
      && ef.NumPlayers()==2) {
    use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
    stopAfter=0;max_solns=0;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
  }
	
  // ALL Nash n person
  if (m_standardType==efgSTANDARD_NASH && m_standardNum==efgSTANDARD_ALL
      && ef.NumPlayers()!=2) {
    use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
    stopAfter=0;max_solns=0;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
    Warn("Not guaranteed to find all solutions for 'All Nash n-person'\n");
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
  }

  // One Subgame Perfect (same as One Nash)
  // One Subgame Perfect for 2 person
  if (m_standardType==efgSTANDARD_PERFECT && m_standardNum==efgSTANDARD_ONE 
      && ef.NumPlayers()==2) {
    use_nfg=FALSE;
    if (perf)
      algorithm=(ef.IsConstSum()) ? EFG_CSUM_SOLUTION : EFG_LCP_SOLUTION;
    else
      algorithm=EFG_QRE_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=true;all=true;dom_type=DOM_WEAK;
    subgames=TRUE;
  }
  
  // One Subgame Pefect for n person
  if (m_standardType==efgSTANDARD_PERFECT && m_standardNum==efgSTANDARD_ONE
      && ef.NumPlayers()!=2) {
    use_nfg=TRUE;algorithm=NFG_SIMPDIV_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=true;all=true;dom_type=DOM_WEAK;
    subgames=TRUE;
  }
	
  // Two Subgame Perfect 2 person
  if (m_standardType==efgSTANDARD_PERFECT && m_standardNum==efgSTANDARD_TWO 
      && ef.NumPlayers()==2) {
    use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=TRUE;
    Warn("Not guaranteed to find 2 solutions for 'Two Perfect'");
  }
	
  // Two Subgame Perfect n person
  if (m_standardType==efgSTANDARD_PERFECT && m_standardNum==efgSTANDARD_TWO
      && ef.NumPlayers()!=2) {
    use_nfg=TRUE;algorithm=NFG_LIAP_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=TRUE;
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
    Warn("Not guaranteed to find 2 solutions for 'Two Perfect'");
  }
	
  // All Subgame Perfect 2 person
  if (m_standardType==efgSTANDARD_PERFECT && m_standardNum==efgSTANDARD_ALL 
      && ef.NumPlayers()==2) {
    use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
    stopAfter=0;max_solns=0;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=TRUE;
  }
	
  // All Subgame Perfect n person
  if (m_standardType==efgSTANDARD_PERFECT && m_standardNum==efgSTANDARD_ALL 
      && ef.NumPlayers()!=2) {
    use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
    stopAfter=0;max_solns=0;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=TRUE;
    Warn("Not guaranteed to find all solutions for 'All Subgame Perfect n-person'\n");
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
  }
	
  // One Sequential
  if (m_standardType==efgSTANDARD_SEQUENTIAL && m_standardNum==efgSTANDARD_ONE) {
    use_nfg=FALSE;algorithm=EFG_QRE_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=false;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
  }
	
  // Two Sequential
  if (m_standardType==efgSTANDARD_SEQUENTIAL && m_standardNum==efgSTANDARD_TWO) {
    use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=false;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
    Warn("Not guaranteed to find all solutions for 'Two Sequential'\n");
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
  }
	
  // All Sequential
  if (m_standardType==efgSTANDARD_SEQUENTIAL && m_standardNum==efgSTANDARD_ALL) {
    use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
    stopAfter=0;max_solns=0;
    use_elimdom=false;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
    Warn("Not guaranteed to find all solutions for 'All Sequential'\n");
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
  }

  pick_solns=false; // pick solution subgames off for all standard solns
  // -------- now write the new settings to file
  wxWriteResource(SOLN_SECT,"Use-Nfg",use_nfg,defaults_file);
  char *alg_sect=(use_nfg) ? "Nfg-Algorithm" : "Efg-Algorithm";
  wxWriteResource(SOLN_SECT,alg_sect,algorithm,defaults_file);
  wxWriteResource(PARAMS_SECTION,"Stop-After",stopAfter,defaults_file);
  wxWriteResource(PARAMS_SECTION,"Max-Solns",max_solns,defaults_file);
  wxWriteResource(SOLN_SECT,"Nfg-ElimDom-All",(int)all,defaults_file);
  wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Type",dom_type,defaults_file);
  wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Use",(int)use_elimdom,defaults_file);
  wxWriteResource(SOLN_SECT,"Efg-Mark-Subgames",subgames,defaults_file);
  wxWriteResource(SOLN_SECT,"Efg-Interactive-Solns",pick_solns,defaults_file);

  wxWriteResource(PARAMS_SECTION, "Precision", m_precisionStr,
		  defaults_file);
}


