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

// call this to convert standard settings to actual solution parameters

void EfgSolveSettings::StandardSettings(void)
{
  int stopAfter,max_solns,dom_type;
  bool use_elimdom,all;

  Infoset *bad1,*bad2;
  bool perf=IsPerfectRecall(ef,bad1,bad2);

  // a separate case for each of the possible alg/num/game combinations
  // One Nash for 2 person
  if (standard_type == STANDARD_NASH && standard_num == STANDARD_ONE
      && ef.NumPlayers() == 2) {
    use_nfg=FALSE;
    if (perf)
      algorithm=(ef.IsConstSum()) ? EFG_CSUM_SOLUTION : EFG_LCP_SOLUTION;
    else
      algorithm=EFG_GOBIT_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=true;all=true;dom_type=DOM_WEAK;
    subgames=TRUE;
  }

  // One Nash for n person
  if (standard_type==STANDARD_NASH && standard_num==STANDARD_ONE
      && ef.NumPlayers()!=2) {
    use_nfg=TRUE;algorithm=NFG_SIMPDIV_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=true;all=true;dom_type=DOM_WEAK;
    subgames=TRUE;
  }
	
  // Two Nash 2 person
  if (standard_type==STANDARD_NASH && standard_num==STANDARD_TWO
      && ef.NumPlayers()==2) {
    use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
  }
	
  // Two Nash n person
  if (standard_type==STANDARD_NASH && standard_num==STANDARD_TWO 
      && ef.NumPlayers()!=2) {
    use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
  }
	
  // All Nash 2 person
  if (standard_type==STANDARD_NASH && standard_num==STANDARD_ALL 
      && ef.NumPlayers()==2) {
    use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
    stopAfter=0;max_solns=0;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
  }
	
  // ALL Nash n person
  if (standard_type==STANDARD_NASH && standard_num==STANDARD_ALL
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
  if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ONE 
      && ef.NumPlayers()==2) {
    use_nfg=FALSE;
    if (perf)
      algorithm=(ef.IsConstSum()) ? EFG_CSUM_SOLUTION : EFG_LCP_SOLUTION;
    else
      algorithm=EFG_GOBIT_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=true;all=true;dom_type=DOM_WEAK;
    subgames=TRUE;
  }
  
  // One Subgame Pefect for n person
  if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ONE
      && ef.NumPlayers()!=2) {
    use_nfg=TRUE;algorithm=NFG_SIMPDIV_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=true;all=true;dom_type=DOM_WEAK;
    subgames=TRUE;
  }
	
  // Two Subgame Perfect 2 person
  if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_TWO 
      && ef.NumPlayers()==2) {
    use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=TRUE;
    Warn("Not guaranteed to find 2 solutions for 'Two Perfect'");
  }
	
  // Two Subgame Perfect n person
  if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_TWO
      && ef.NumPlayers()!=2) {
    use_nfg=TRUE;algorithm=NFG_LIAP_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=TRUE;
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
    Warn("Not guaranteed to find 2 solutions for 'Two Perfect'");
  }
	
  // All Subgame Perfect 2 person
  if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ALL 
      && ef.NumPlayers()==2) {
    use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
    stopAfter=0;max_solns=0;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=TRUE;
  }
	
  // All Subgame Perfect n person
  if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ALL 
      && ef.NumPlayers()!=2) {
    use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
    stopAfter=0;max_solns=0;
    use_elimdom=true;all=true;dom_type=DOM_STRONG;
    subgames=TRUE;
    Warn("Not guaranteed to find all solutions for 'All Subgame Perfect n-person'\n");
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
  }
	
  // One Sequential
  if (standard_type==STANDARD_SEQUENTIAL && standard_num==STANDARD_ONE) {
    use_nfg=FALSE;algorithm=EFG_GOBIT_SOLUTION;
    stopAfter=1;max_solns=1;
    use_elimdom=false;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
  }
	
  // Two Sequential
  if (standard_type==STANDARD_SEQUENTIAL && standard_num==STANDARD_TWO) {
    use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
    stopAfter=2;max_solns=2;
    use_elimdom=false;all=true;dom_type=DOM_STRONG;
    subgames=FALSE;
    Warn("Not guaranteed to find all solutions for 'Two Sequential'\n");
    wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
  }
	
  // All Sequential
  if (standard_type==STANDARD_SEQUENTIAL && standard_num==STANDARD_ALL) {
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
}

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
  wxGetResource(SOLN_SECT,"Efg-Use-Standard",&use_standard,defaults_file);
  wxGetResource(SOLN_SECT,"Efg-Standard-Type",&standard_type,defaults_file);
  wxGetResource(SOLN_SECT,"Efg-Standard-Num",&standard_num,defaults_file);
  if (use_standard) StandardSettings();
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
    wxWriteResource(SOLN_SECT,"Efg-Use-Standard",use_standard,defaults_file);
    wxWriteResource(SOLN_SECT,"Efg-Standard-Type",standard_type,defaults_file);
    wxWriteResource(SOLN_SECT,"Efg-Standard-Num",standard_num,defaults_file);
  }
}
