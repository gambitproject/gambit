//
// FILE: nfgshow.cc -- Implementation of NfgShow class
//
// $Id$
//

#include "wx.h"
#include "nfgshow.h"
#include "nfgsoln.h"
#include "nfgconst.h"
#include "nfplayer.h"
#include "nfdom.h"

#include "gambit.h"
#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgeditsupport.h"
#include "dlnfgplayers.h"
#include "dlstrategies.h"
#include "dlnfgsave.h"

#include "dlelim.h"
#include "dlsupportselect.h"

//======================================================================
//                 NfgShow: Constructor and destructor
//======================================================================

NfgShow::NfgShow(Nfg &N, EfgNfgInterface *efg, wxFrame *p_frame)
  : EfgNfgInterface(gNFG, efg), nf(N), nf_iter(N), m_frame(p_frame)
{
  pl1 = 1;
  pl2 = 2;        // use the defaults
  cur_soln = 0;
  cur_sup = new NFSupport(nf);    // base support
  cur_sup->SetName("Full Support");
  supports.Append(cur_sup);
  
  spread = new NormalSpread(cur_sup, pl1, pl2, this, m_frame);

  soln_show      = 0;  // no solution inspect window yet.
  SetPlayers(pl1, pl2, true);

  // Create the accelerators
  ReadAccelerators(accelerators, "NfgAccelerators", gambitApp.ResourceFile());
  
  UpdateVals();
  spread->Redraw();

  nf.SetIsDirty(false);
}

NfgShow::~NfgShow()
{ 

}

void NfgShow::UpdateVals(void)
{
  if (!(nf_iter.Support() == *cur_sup)) 
    nf_iter = NfgIter(*cur_sup);

  for (int i = 1; i <= rows; i++) {
    for (int j = 1; j <= cols; j++) {
      nf_iter.Set(pl1, i);
      nf_iter.Set(pl2, j);
      gText pay_str;
      NFOutcome *outcome = nf_iter.GetOutcome();
      bool hilight = false;

      if (draw_settings.OutcomeDisp() == OUTCOME_VALUES) {
	for (int k = 1; k <= nf.NumPlayers(); k++) {
	  pay_str += ("\\C{"+ToText(draw_settings.GetPlayerColor(k))+"}");
	  pay_str += ToText(nf.Payoff(outcome, k));
	  
	  if (k != nf.NumPlayers())
	    pay_str += ',';
	}
      }
      else {
	if (outcome) {
	  pay_str = outcome->GetName();

	  if (pay_str == "")
	    pay_str = "Outcome"+ToText(outcome->GetNumber());
	}
	else {
	  pay_str = "Null";
	}
      }

      spread->SetCell(i, j, pay_str);
      spread->HiLighted(i, j, 0, hilight);
    }

    if (spread->HaveDom()) { 
      int dom_pos = 1+spread->HaveProbs();
      Strategy *strategy = cur_sup->Strategies(pl1)[i];
      if (cur_sup->IsDominated(strategy, true)) {
	spread->SetCell(i, cols+dom_pos, "S");
      }
      else if (cur_sup->IsDominated(strategy, false)){
	spread->SetCell(i, cols+dom_pos, "W");
      }
      else {
        spread->SetCell(i, cols+dom_pos, "N");
      }
    }

  }

  for (int j = 1; j <= cols; j++) {
    nf_iter.Set(pl2, j);

    if (spread->HaveDom()) { 
      int dom_pos = 1+spread->HaveProbs();
      Strategy *strategy = cur_sup->Strategies(pl2)[j];
      if (cur_sup->IsDominated(strategy, true)) { 
	spread->SetCell(rows+dom_pos, j, "S");
      }
      else if (cur_sup->IsDominated(strategy, false)) {
	spread->SetCell(rows+dom_pos, j, "W");
      }
      else {
	spread->SetCell(rows+dom_pos, j, "N");
      }
    }
  }

  nf_iter.Set(pl1, 1);
  nf_iter.Set(pl2, 1);

  spread->Repaint();
}


void NfgShow::UpdateSoln(void)
{
  if (!cur_soln)  return;

  // The profile is obvious for pure strategy: just set the display strat
  // to the nonzero solution strategy.  However, for mixed equs, we set
  // the display strategy to the highest soluton strat.  (Note that
  // MixedSolution.Pure() is not yet implemented :( Add support for
  // displaying solutions created for supports other than cur_sup

  MixedSolution soln = solns[cur_soln];
  gNumber t_max;
  gArray<int> profile(nf.NumPlayers());

  // Figure out the index in the cur_sup, then map it onto the full support
  for (int pl = 1; pl <= nf.NumPlayers(); pl++) {
    profile[pl] = 1;
    t_max = soln(nf.Players()[pl]->Strategies()[1]);

    for (int st1 = 1; st1 <= cur_sup->NumStrats(pl); st1++) {
      if (soln(cur_sup->Strategies(pl)[st1]) > t_max) {
    profile[pl] = st1;
    t_max = soln(cur_sup->Strategies(pl)[st1]);
      }
    }
  }

  UpdateProfile(profile);

  // Set the profile boxes to correct values if this is a pure equ
  spread->SetProfile(profile);

  // Hilight the cells w/ nonzero prob
  gNumber eps;
  gEpsilon(eps, spread->DrawSettings()->NumPrec()+1);
 
  for (int st1 = 1; st1 <= rows; st1++) {
    for (int st2 = 1; st2 <= cols; st2++) {
      if (soln(cur_sup->Strategies(pl1)[st1]) > eps
      && soln(cur_sup->Strategies(pl2)[st2]) > eps)
    spread->HiLighted(st1, st2, 0, TRUE);
      else
    spread->HiLighted(st1, st2, 0, FALSE);
    }
  }


  if (spread->HaveProbs()) {
    // Print out the probability in the next column/row
    for (int i = 1; i <= rows; i++)
      spread->SetCell(i, cols+1,
              ToText(soln(cur_sup->Strategies(pl1)[i])));

    for (int i = 1; i <= cols; i++)
      spread->SetCell(rows+1, i, 
              ToText(soln(cur_sup->Strategies(pl2)[i])));
  }

  if (spread->HaveVal()) {
    // Print out the probability in the last column/row
    for (int i = 1; i <= rows; i++) {
      spread->SetCell(i, cols+spread->HaveProbs()+spread->HaveDom()+1, 
              ToText(soln.Payoff(nf.Players()[pl1],
                     cur_sup->Strategies(pl1)[i])));
    }
    
    for (int j = 1; j <= cols; j++) {
      spread->SetCell(rows+spread->HaveProbs()+spread->HaveDom()+1, j, 
              ToText(soln.Payoff(nf.Players()[pl2],
                     cur_sup->Strategies(pl2)[j])));
    }
  }

  spread->Repaint();
}



void NfgShow::UpdateContingencyProb(const gArray<int> &profile)
{
  if (!cur_soln || !spread->HaveProbs()) 
    return;

  // The value in the maximum row&col cell corresponds to prob of being
  // at this contingency = Product(Prob(strat_here), all players except pl1, pl2)
  const MixedSolution &soln = solns[cur_soln];

  gNumber cont_prob(1);

  for (int i = 1; i <= nf.NumPlayers(); i++) {
    if (i != pl1 && i != pl2) {
      NFPlayer *player = nf.Players()[i];
      cont_prob *= soln(player->Strategies()[profile[i]]);
    }
  }

  spread->SetCell(rows+1, cols+1, ToText(cont_prob));
}



void NfgShow::UpdateProfile(gArray<int> &profile)
{
  nf_iter.Set(profile);
  UpdateContingencyProb(profile);
  UpdateVals();
}

extern Nfg *CompressNfg(const Nfg &nfg, const NFSupport &S); // in nfgutils.cc

Bool NfgShow::Save(void)
{
  static int s_nDecimals = 6;
  dialogNfgSave dialog(Filename(), nf.GetTitle(), s_nDecimals, m_frame);

  if (dialog.Completed() == wxOK) {
    if (wxFileExists(dialog.Filename())) {
      if (wxMessageBox("File " + dialog.Filename() + " exists.  Overwrite?",
		       "Confirm", wxOK | wxCANCEL) != wxOK) {
	return FALSE;
      }
    }

    nf.SetTitle(dialog.Label());

    Nfg *nfg = 0;
    try {
      gFileOutput file(dialog.Filename());
      nfg = CompressNfg(nf, *cur_sup);
      nfg->WriteNfgFile(file, s_nDecimals);
      delete nfg;
      SetFileName(dialog.Filename());
    }
    catch (gFileOutput::OpenFailed &) {
      wxMessageBox("Could not open " + dialog.Filename() + " for writing.",
		   "Error", wxOK);
      if (nfg)  delete nfg;
    }
    catch (gFileOutput::WriteFailed &) {
      wxMessageBox("Write error occurred in saving " + dialog.Filename(),
		   "Error", wxOK);
      if (nfg)  delete nfg;
    }
    catch (gException &) {
      wxMessageBox("Internal exception in Gambit", "Error", wxOK);
      if (nfg)  delete nfg;
    }
    return TRUE;
  }
  else {
    return FALSE;
  }
}

gText NfgShow::UniqueSupportName(void) const
{
  int number = supports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= supports.Length(); i++) {
      if (supports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > supports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

#include "nfgciter.h"

void NfgShow::DumpAscii(Bool all_cont)
{
  char *s = wxFileSelector("Save", NULL, NULL, NULL, "*.asc", wxSAVE);

  if (s) {
    gFileOutput outfilef(s);

    // I have no idea why this works, but otherwise we get ambiguities:
    gOutput &outfile = outfilef;  
  
    gArray<int> profile(nf.NumPlayers());

    if (all_cont) {       // if we need to save ALL the contingencies
      NfgContIter nf_citer(*cur_sup);
      gBlock<int> hold_const(2);
      hold_const[1] = pl1;
      hold_const[2] = pl2;
      nf_citer.Freeze(hold_const);

      do {
	outfile << nf_citer.Get() << '\n';

	for (int i = 1; i <= rows; i++) {
	  for (int j = 1; j <= cols; j++) {
	    nf_citer.Set(pl1, i);
	    nf_citer.Set(pl2, j);
	    outfile << "{ ";

	    for (int k = 1; k <= nf.NumPlayers(); k++)
	      outfile << nf.Payoff(nf_iter.GetOutcome(), k) << ' ';
	    outfile << " }  ";
	  }
	  outfile << "\n";
	}
      } while (nf_citer.NextContingency());
    }
    else  {   // if we only need to save the contingency currently displayed
      nf_iter.Get(profile);
      outfile << profile << '\n';

      for (int i = 1; i <= rows; i++) {
	for (int j = 1; j <= cols; j++) {
	  nf_iter.Set(pl1, i);
	  nf_iter.Set(pl2, j);
	  outfile << "{ ";

	  for (int k = 1; k <= nf.NumPlayers(); k++)
	    outfile << nf.Payoff(nf_iter.GetOutcome(), k) << ' ';
	  outfile << "}\n";
	}
      }
    }
  }
}


// Clear solutions-just updates the spreadsheet to remove any hilights
void NfgShow::ClearSolutions(void)
{
  if (cur_soln)  {
    // if there are solutions to clear, and they are displayed
    for (int i = 1; i <= rows; i++)
      for (int j = 1; j <= cols; j++)
	spread->HiLighted(i, j, 0, FALSE);
  }

  if (spread->HaveProbs()) {
    for (int i = 1; i <= cols; i++) spread->SetCell(rows+1, i, "");

    for (int i = 1; i <= rows; i++)
      spread->SetCell(i, cols+1, "");
    
    spread->SetCell(rows+1, cols+1, "");
  }

  /*
  if (spread->HaveDom()) {  // if there exist the dominance row/col
    int dom_pos = 1+spread->HaveProbs();
    for (int i = 1; i <= cols; i++)
      spread->SetCell(rows+dom_pos, i, "");

    for (int i = 1; i <= rows; i++)
      spread->SetCell(i, cols+dom_pos, "");
  }
  */

  if (spread->HaveVal()) {
    int val_pos = 1+spread->HaveProbs()+spread->HaveDom();
    for (int i = 1; i <= cols; i++)
      spread->SetCell(rows+val_pos, i, "");

    for (int i = 1; i <= rows; i++)
      spread->SetCell(i, cols+val_pos, "");
  }
}

void NfgShow::ChangeSolution(int sol)
{
  ClearSolutions();

  if (sol) {
    cur_soln = sol;
    
    if (cur_soln)
      UpdateSoln();
  }
  else {
    cur_soln = 0;
    spread->RemoveProbDisp();
    spread->RemoveValDisp();
    UpdateVals();
    spread->Repaint();
  }
}


// Remove solutions-permanently removes any solutions
void NfgShow::RemoveSolutions(void)
{
  if (soln_show) {
    soln_show->Show(FALSE);
    delete soln_show;
    soln_show = 0;
  }

  ClearSolutions();

  if (cur_soln)
    spread->Repaint();

  cur_soln = 0;
  solns.Flush();
}



MixedSolution NfgShow::CreateSolution(void)
{
  return MixedSolution(MixedProfile<gNumber>(*cur_sup));
}


void NfgShow::OnOk(void)
{
  if (soln_show) {
    soln_show->OnOk();
  }

  spread->Close();
  draw_settings.SaveSettings();
  delete &nf;
}


void NfgShow::InspectSolutions(int what)
{
  if (what == CREATE_DIALOG) {
    if (solns.Length() == 0) {
      wxMessageBox("Solution list currently empty");
      return;
    }

    if (soln_show) {
      soln_show->Show(FALSE);
      delete soln_show;
    }

    soln_show = new NfgSolnShow(solns, nf.NumPlayers(), 
				gmax(nf.NumStrats()), 
				cur_soln, draw_settings, 
				sf_options, this, spread);
  }

  if (what == DESTROY_DIALOG && soln_show) {
    soln_show = 0;
  }
}


#include "nfgsolvd.h"
#include "nfgsolng.h"

void NfgShow::Solve(int id)
{
  // If we have more than 1 support, we must have created it explicitly.
  // In that case use the currently set support. 
  // Otherwise, use the full support.
  NFSupport *sup = (supports.Length() > 1) ? cur_sup : 0;

  if (!sup)
    sup = new NFSupport(nf);

  int old_max_soln = solns.Length();  // used for extensive update

  guiNfgSolution *solver;

  switch (id) {
  case NFG_SOLVE_CUSTOM_ENUMPURE:
    solver = new guinfgEnumPure(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_ENUMMIXED:
    solver = new guinfgEnumMixed(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LCP:      
    solver = new guinfgLcp(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LP:       
    solver = new guinfgLp(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LIAP:
    solver = new guinfgLiap(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_SIMPDIV:
    solver = new guinfgSimpdiv(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_POLENUM:
    solver = new guinfgPolEnum(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_QRE:
    solver = new guinfgQre(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_QREGRID:
    solver = new guinfgQreAll(*sup, this);
    break;
  default:
    // shouldn't happen.  we'll ignore silently
    return;
  }

  bool go = solver->SolveSetup();

  wxBeginBusyCursor();

  try {
    if (go) {
      solver->Eliminate();
      solns += solver->Solve();
    }
    wxEndBusyCursor();
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), Frame());
    wxEndBusyCursor();
  }
    
  delete solver;

  if (!go)  return;

  if (old_max_soln != solns.Length()) {
    // Now, transfer the NEW solutions to extensive form if requested
    /*
    if (NSD.GetExtensive()) {
      for (int i = old_max_soln+1; i <= solns.Length(); i++) 
    SolutionToExtensive(solns[i]);
    }
    */
    if (!spread->HaveProbs()) {
      spread->MakeProbDisp();
      spread->Redraw();
    }

    ChangeSolution(solns.VisibleLength());
    
    InspectSolutions(CREATE_DIALOG);
  }
}


void NfgShow::SolveStandard(void)
{ 
  dialogNfgSolveStandard dialog(nf, (wxWindow *) spread);
  
  if (dialog.Completed() != wxOK)
    return;

  NFSupport support(nf);

  int old_max_soln = solns.Length();  // used for extensive update

  guiNfgSolution *solver = 0;

  switch (dialog.Number()) {
  case nfgSTANDARD_ONE:
    if (dialog.Type() == nfgSTANDARD_NASH) {
      if (nf.NumPlayers() == 2) {
	if (IsConstSum(nf))
	  solver = new guinfgLp(support, this, 1, dialog.Precision(), true);
	else
	  solver = new guinfgLcp(support, this, 1, dialog.Precision(), true);
      }
      else
	solver = new guinfgSimpdiv(support, this, 1, dialog.Precision(), true);
    }
    else {  // nfgSTANDARD_PERFECT
      if (nf.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(support, this, 1, dialog.Precision(), true);
      }
      else {
	wxMessageBox("One-Perfect not implemented", "Standard Solution");
	return;
      }
    }
    break;

  case nfgSTANDARD_TWO:
    if (dialog.Type() == nfgSTANDARD_NASH) {
      if (nf.NumPlayers() == 2)
	solver = new guinfgEnumMixed(support, this, 2, dialog.Precision(), false);
      else
	solver = new guinfgLiap(support, this, 2, 10, false);
    }
    else {  // nfgSTANDARD_PERFECT
      if (nf.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(support, this, 2, dialog.Precision(), true);
	wxMessageBox("Not guaranteed to find 2 solutions", "Warning");
      }
      else {
	wxMessageBox("Two-Perfect not implemented", "Standard Solution");
	return;
      }
    }
    break;

  case nfgSTANDARD_ALL:
    if (dialog.Type() == nfgSTANDARD_NASH) {
      if (nf.NumPlayers() == 2)
	solver = new guinfgEnumMixed(support, this, 0, dialog.Precision(), false);
      else {
	solver = new guinfgLiap(support, this, 0, 0, false);
	wxMessageBox("Not guaranteed to find all solutions", "Warning");
      }
    }
    else {  // nfgSTANDARD_PERFECT
      if (nf.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(support, this, 0, dialog.Precision(), true);
	wxMessageBox("Not guaranteed to find all solutions", "Warning");
      }
      else {
	wxMessageBox("All-Perfect not implemented", "Standard Solution");
	return;
      }
    }
    break;
  }

  wxBeginBusyCursor();

  try {
    solver->Eliminate();
    solns += solver->Solve();
    wxEndBusyCursor();
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), Frame());
    wxEndBusyCursor();
  }
    
  delete solver;

  if (old_max_soln != solns.Length()) {
    // Now, transfer the NEW solutions to extensive form if requested
    /*
    if (NSD.GetExtensive()) {
      for (int i = old_max_soln+1; i <= solns.Length(); i++) 
    SolutionToExtensive(solns[i]);
    }
    */
    if (!spread->HaveProbs()) {
      spread->MakeProbDisp();
      spread->Redraw();
    }

    ChangeSolution(solns.VisibleLength());
    
    InspectSolutions(CREATE_DIALOG);
  }  

}

void NfgShow::SetFileName(const gText &s)
{
  if (s != "")
    filename = s;
  else 
    filename = "untitled.nfg";

  // Title the window
  spread->SetTitle("[" + filename + "] " + nf.GetTitle());
}

// how: 0-default, 1-saved, 2-query
MixedProfile<gNumber> NfgShow::CreateStartProfile(int how)
{
  MixedProfile<gNumber> start(*cur_sup);

  if (how == 0)
    start.Centroid();

  if (how == 1 || how == 2) {
    if (starting_points.last == -1 || how == 2) {
      MSolnSortFilterOptions sf_opts; // no sort, filter

      if (starting_points.profiles.Length() == 0)
	starting_points.profiles += start;

      Nfg1SolnPicker *start_dialog = 
	new Nfg1SolnPicker(starting_points.profiles, 
			   nf.NumPlayers(), 
			   gmax(nf.NumStrats()), 0, 
			   draw_settings, sf_opts, this, spread);
      
      spread->Enable(FALSE);  // disable this window until the edit window is closed
      
      while (start_dialog->Completed() == wxRUNNING) 
	wxYield();
      
      spread->Enable(TRUE);
      starting_points.last = start_dialog->Picked();
      delete start_dialog;
    }

    if (starting_points.last)
      start = starting_points.profiles[starting_points.last];
  }

  return start;
}


// Solution To Extensive
#include "efg.h"
#include "efgutils.h"

void NfgShow::SolutionToExtensive(const MixedSolution &mp, bool set)
{
  if (!InterfaceOk()) {  // we better have someone to send solutions to
    return;
  }
  
  const Efg *efg = InterfaceObjectEfg();

  if (efg->AssociatedNfg() != &nf) 
    return;

  if (!IsPerfectRecall(*efg)) {
    if (wxMessageBox("May not be able to find valid behavior strategy\n"
		     "for game of imperfect recall\n"
		     "Continue anyway?",
		     "Convert to behavior strategy",
		     wxOK | wxCANCEL | wxCENTRE) != wxOK)   
      return;
  }

  EFSupport S(*InterfaceObjectEfg());
  BehavProfile<gNumber> bp(mp);
  SolutionToEfg(bp, set);
}

void NfgShow::SetPlayers(int _pl1, int _pl2, bool first_time)
{
  int num_players = nf.NumPlayers();

  if (_pl1 == _pl2) {
    if (num_players != 2) {  // do nothing
      wxMessageBox("Can not use the same player for both row and col!");
      spread->SetRowPlayer(pl1);
      spread->SetColPlayer(pl2);
      return;
    }
    else {    // switch row/col
      _pl1 = pl2;
      _pl2 = pl1;
    }
  }

  pl1 = _pl1;
  pl2 = _pl2;
  
  rows = cur_sup->NumStrats(pl1);
  cols = cur_sup->NumStrats(pl2);

  int features = spread->HaveDom() + spread->HaveProbs() + spread->HaveVal();
  //  spread->SetDimensions(rows + features, cols + features, 1);

  if (rows + features != spread->GetRows()) {
    while (rows + features > spread->GetRows()) {
      spread->AddRow(1);
    }
    while (rows + features < spread->GetRows()) {
      spread->DelRow(1);
    }
  }

  if (cols + features != spread->GetCols()) {
    int colWidth = spread->DrawSettings()->GetColWidthRaw(1);
    while (cols + features > spread->GetCols()) {
      spread->AddCol(1);
      spread->DrawSettings()->SetColWidth(colWidth, 1);
    }
    while (cols + features < spread->GetCols()) {
      spread->DelCol(1);
    }
  }

  // Must reset this if support changes
  spread->SetDimensionality(cur_sup);

  if (spread->HaveProbs()) 
    spread->MakeProbDisp();
  
  if (spread->HaveDom()) 
    spread->MakeDomDisp();

  if (spread->HaveVal()) 
    spread->MakeValDisp();

  // Set new title
  spread->SetTitle(nf.GetTitle() + " : " + 
		   nf.Players()[pl1]->GetName() +
		   " x " + nf.Players()[pl2]->GetName());

  // Set new labels
  gText label;
  
  for (int i = 1; i <= rows; i++) {
    label = cur_sup->Strategies(pl1)[i]->Name();

    if (label == "") 
      label = ToText(i);

    spread->SetLabelRow(i, label);
  }

  for (int i = 1; i <= cols; i++) {
    label = cur_sup->Strategies(pl2)[i]->Name();

    if (label == "") 
      label = ToText(i);

    spread->SetLabelCol(i, label);
  }

  // Update the sheet's players.
  spread->SetRowPlayer(pl1);
  spread->SetColPlayer(pl2);

  // This is really odd.  If I call UpdateVals during construction, the
  // virtual function table is not yet created causing a crash.  Thus, we
  // can only call it if this is NOT the first time this is called

  if (!first_time) {
    UpdateVals();
    UpdateSoln();
    spread->Redraw();
    spread->Repaint();
  }
}

NFSupport *NfgShow::MakeSupport(void)
{
  /*
  dialogNfgEditSupport dialog(nf, spread);

  if (dialog.Completed() == wxOK) {
    NFSupport *support = dialog.CreateSupport();
    supports.Append(support);
    return support;
  }
  */
  return 0;
}


void NfgShow::OutcomePayoffs(int st1, int st2, bool next)
{
  if (st1 > rows || st2 > cols)
    return;

  if (next) { 
    if (st2 < cols) {
      st2++;
    }
    else {
      if (st1 < rows) {
	st1++;
	st2 = 1;
      }
      else {
	st1 = 1;
	st2 = 1;
      }
    }

    spread->SetCurRow(st1);
    spread->SetCurCol(st2);
  }

  gArray<int> profile(spread->GetProfile());
  profile[pl1] = st1;
  profile[pl2] = st2;
  nf_iter.Set(pl1, st1);
  nf_iter.Set(pl2, st2);

  dialogNfgPayoffs dialog(nf, nf.GetOutcome(profile), spread);

  if (dialog.Completed() == wxOK) {
    NFOutcome *outc = nf.GetOutcome(profile);
    gArray<gNumber> payoffs(dialog.Payoffs());

    if (!outc) {
      outc = nf.NewOutcome();
      nf.SetOutcome(profile, outc);
    }

    for (int i = 1; i <= nf.NumPlayers(); i++)
      nf.SetPayoff(outc, i, payoffs[i]);
    outc->SetName(dialog.Name());

    UpdateVals();
    RemoveSolutions();
    InterfaceDied();
  }
}

void NfgShow::OutcomeAttach(void)
{
  if (nf.NumOutcomes() == 0)
    return;

  dialogNfgOutcomeSelect dialog(nf, spread);
    
  if (dialog.Completed() == wxOK) {
    nf.SetOutcome(spread->GetProfile(), dialog.GetOutcome());
    InterfaceDied();
    UpdateVals();
  }
}

void NfgShow::OutcomeDetach(void)
{
  nf.SetOutcome(spread->GetProfile(), 0);
  InterfaceDied();
  UpdateVals();
}

void NfgShow::OutcomeNew(void)
{
  dialogNfgPayoffs dialog(nf, 0, m_frame);

  if (dialog.Completed() == wxOK) {
    NFOutcome *outc = nf.NewOutcome();
    gArray<gNumber> payoffs(dialog.Payoffs());

    for (int pl = 1; pl <= nf.NumPlayers(); pl++)
      nf.SetPayoff(outc, pl, payoffs[pl]);
    outc->SetName(dialog.Name());
    InterfaceDied();
  }
}

void NfgShow::OutcomeDelete(void)
{
  if (nf.NumOutcomes() == 0)
    return;

  dialogNfgOutcomeSelect dialog(nf, spread);
    
  if (dialog.Completed() == wxOK) {
    nf.DeleteOutcome(dialog.GetOutcome());
    InterfaceDied();
    UpdateVals();
  }
}

#include "wxstatus.h"

int NfgShow::SolveElimDom(void)
{
  gArray<gText> playerNames(nf.NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++)
    playerNames[pl] = nf.Players()[pl]->GetName();
  dialogElim dialog(playerNames, true, spread);

  if (dialog.Completed() == wxOK) {
    NFSupport *sup = cur_sup;
    wxStatus status(spread, "Dominance Elimination");

    try {
      if (!dialog.DomMixed()) {
	if (dialog.FindAll()) {
	  while ((sup = sup->Undominated(dialog.DomStrong(), 
					 dialog.Players(), gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    supports.Append(sup);
	  }
	}
	else {
	  if ((sup = sup->Undominated(dialog.DomStrong(), 
				      dialog.Players(), gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    supports.Append(sup);
	  }
	}
      }
      else {
	if (dialog.FindAll()) {
	  while ((sup = sup->MixedUndominated(dialog.DomStrong(), precRATIONAL,
					      dialog.Players(),
					      gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    supports.Append(sup);
	  }
	}
	else {
	  if ((sup = sup->MixedUndominated(dialog.DomStrong(), precRATIONAL,
					   dialog.Players(),
					   gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    supports.Append(sup);
	  }
	}
      }
    }
    catch (gSignalBreak &) { }

    if (dialog.Compress() && cur_sup != sup) {
      cur_sup = supports[supports.Length()]; // displaying the last created support
      SetPlayers(pl1, pl2);
    }
    else {
      spread->MakeDomDisp();
      UpdateVals();
      spread->Redraw();
    }

    UpdateSoln();
    return 1;
  }

  return 0;
}

void NfgShow::SupportNew(void)
{
  NFSupport newSupport(nf);
  newSupport.SetName(UniqueSupportName());
  dialogNfgEditSupport dialog(newSupport, spread);

  if (dialog.Completed() == wxOK) {
    try {
      NFSupport *support = new NFSupport(dialog.Support());
      support->SetName(dialog.Name());
      supports.Append(support);

      ChangeSolution(0);  // chances are, the current solution will not work.
      cur_sup = support;
      SetPlayers(pl1, pl2);
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), spread);
    }
  }
}

void NfgShow::SupportEdit(void)
{
  dialogNfgEditSupport dialog(*cur_sup, spread);

  if (dialog.Completed() == wxOK) {
    try {
      *cur_sup = dialog.Support();
      cur_sup->SetName(dialog.Name());
      SetPlayers(pl1, pl2);
      ChangeSolution(0);
      UpdateVals();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), spread);
    }
  }
}

void NfgShow::SupportDelete(void)
{
  if (supports.Length() == 1)  return;

  dialogSupportSelect dialog(supports, cur_sup, "Delete Support", spread);

  if (dialog.Completed() == wxOK) {
    try {
      delete supports.Remove(dialog.Selected());
      if (!supports.Find(cur_sup)) {
	cur_sup = supports[1];
	SetPlayers(pl1, pl2);
	ChangeSolution(0);
      }
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), spread);
    }
  }
}

void NfgShow::SupportSelectFromList(void)
{
  dialogSupportSelect dialog(supports, cur_sup, "Select Support", spread);

  if (dialog.Completed() == wxOK) {
    try {
      cur_sup = supports[dialog.Selected()];
      SetPlayers(pl1, pl2);
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), spread);
    }
  }
}

void NfgShow::SupportSelectPrevious(void)
{
  int index = supports.Find(cur_sup);
  if (index == 1) {
    cur_sup = supports[supports.Length()];
  }
  else {
    cur_sup = supports[index - 1];
  }
  SetPlayers(pl1, pl2);
}

void NfgShow::SupportSelectNext(void)
{
  int index = supports.Find(cur_sup);
  if (index == supports.Length()) {
    cur_sup = supports[1];
  }
  else {
    cur_sup = supports[index + 1];
  }
  SetPlayers(pl1, pl2);
}

void NfgShow::PrefsDisplayColumns(void)
{
  guiSliderDialog dialog(spread, "Column width", 0, 100,
			 spread->DrawSettings()->GetColWidthRaw(spread->CurCol()));

  if (dialog.Completed() == wxOK) {
    for (int i = 1; i <= cur_sup->NumStrats(pl2); i++) {
      spread->DrawSettings()->SetColWidth(dialog.GetValue(), i);
    }
    spread->Repaint();
  }
}

void NfgShow::PrefsDisplayDecimals(void)
{
  guiSliderDialog dialog(spread, "Decimal places", 0, 25, ToTextPrecision());

  if (dialog.Completed() == wxOK) {
    ToTextPrecision(dialog.GetValue());
    UpdateVals();
  }
}

void NfgShow::PrefsFont(void)
{
  FontDialogBox dialog(spread, spread->DrawSettings()->GetDataFont());
    
  if (dialog.Completed() == wxOK) {
    spread->DrawSettings()->SetDataFont(dialog.MakeFont());
    spread->Repaint();
  }
}

void NfgShow::Print(void)
{
  wxStringList extras("ASCII", 0);
  wxOutputDialogBox print_dialog(&extras, spread);

  if (print_dialog.Completed() == wxOK) {
    if (!print_dialog.ExtraMedia()) {
      spread->Print(print_dialog.GetMedia(), print_dialog.GetOption());
    }
    else {    // must be dump_ascii
      DumpAscii(TRUE);
    }
  }
}

void NfgShow::EditLabel(void)
{
  char *label = wxGetTextFromUser("Label of game", "Label Game",
				  nf.GetTitle());
  if (label) {
    nf.SetTitle(label);
    SetFileName(Filename());
  }
}


#define LABEL_LENGTH    20
#define ENTRIES_PER_ROW 3

void NfgShow::SetStrategyLabels(void)
{
  dialogStrategies dialog(nf, m_frame);

  if (dialog.GameChanged()) {
    spread->SetStrategyLabels(cur_sup);
    UpdateVals();
  }
}

void NfgShow::SetPlayerLabels(void)
{
  dialogNfgPlayers dialog(nf, spread);
  spread->SetPlayerLabels(cur_sup);
  UpdateVals();
}


void NfgShow::ShowGameInfo(void)
{
  gText message = "Number of Players: " + ToText(nf.NumPlayers()) + "\n";
  message += "Is";
  message += ((IsConstSum(nf)) ? " " : "NOT ");
  message += "constant sum\n";
  message += "Game ";
  message += ((nf.IsDirty()) ? "HAS " : "has not ");
  message += "been modified\n";

  wxMessageBox(message, "Game Information", wxOK, spread);
}


void NfgShow::SetColors(void)
{
  gArray<gText> names(nf.NumPlayers());

  for (int i = 1; i <= names.Length(); i++) 
    names[i] = ToText(i);

  draw_settings.PlayerColorDialog(names);
  UpdateVals();
  spread->Repaint();
}

#include "nfgaccl.h"
#include "sprdaccl.h"

// These events include those for NormShow and those for SpreadSheet3D
gArray<AccelEvent> NfgShow::MakeEventNames(void)
{
  gArray<AccelEvent> events(NUM_NFG_EVENTS + NUM_SPREAD_EVENTS);

  for (int i = 0; i < NUM_SPREAD_EVENTS; i++) 
    events[i+1] = spread_events[i];

  for (int i = NUM_SPREAD_EVENTS; i < NUM_NFG_EVENTS + NUM_SPREAD_EVENTS; i++)
    events[i+1] = nfg_events[i - NUM_SPREAD_EVENTS];

  return events;
}


int NfgShow::CheckAccelerators(wxKeyEvent &ev)
{
  int id = ::CheckAccelerators(accelerators, ev);

  if (id) 
    spread->OnMenuCommand(id);

  return id;
}

void NfgShow::EditAccelerators(void)
{
  ::EditAccelerators(accelerators, MakeEventNames());
  WriteAccelerators(accelerators, "NfgAccelerators", gambitApp.ResourceFile());
}


template class SolutionList<MixedSolution>;

//=========================================================================
//                       class NfgGUI: Member functions
//=========================================================================

#include "nfggui.h"

NfgGUI::NfgGUI(Nfg *nf, const gText infile_name, EfgNfgInterface *inter,
	       wxFrame *parent)
{
  // an already created normal form has been passed

  if (nf == 0)  {  // must create a new normal form, from scratch or from file
    if (infile_name == "") {  // from scratch
      int numPlayers = GetPlayers(parent);
      if (numPlayers >= 2) {
	gArray<int> dimensionality(numPlayers);
	if (GetStrategies(dimensionality, parent)) {
	  nf = new Nfg(dimensionality);
	}
      }
    }
    else {   // from data file
      try {
        gFileInput infile(infile_name);
        ReadNfgFile((gInput &) infile, nf);

	if (!nf) {
	  wxMessageBox(infile_name + " is not a valid .nfg file");
	}
	else {
	  gambitApp.AddFileToHistory(infile_name);
	}
      }
      catch (gFileInput::OpenFailed &) {
	wxMessageBox("Could not open " + infile_name + " for reading");
	return;
      }
      
    }
  }

  NfgShow *nf_show = 0;

  if (nf) {
    if (nf->NumPlayers() > 1) {
      nf_show = new NfgShow(*nf, inter, parent);
    }
    else {
      delete nf;
      wxMessageBox("Single player normal form games are not supported", 
		   "Error");
    }
  }

  if (nf_show)
    nf_show->SetFileName(infile_name);
}

class dialogDimensionality : public guiPagedDialog {
public:
  dialogDimensionality(wxWindow *p_parent, int p_numPlayers);
  virtual ~dialogDimensionality() { }
};

dialogDimensionality::dialogDimensionality(wxWindow *p_parent,
					   int p_numPlayers)
  : guiPagedDialog(p_parent, "Number of Strategies", p_numPlayers)
{
  for (int pl = 1; pl <= p_numPlayers; pl++) {
    SetValue(pl, "2");
  }
  Go();
}

int NfgGUI::GetPlayers(wxFrame *p_parent)
{
  int numPlayers = 2;

  char *label = wxGetTextFromUser("Number of players",
				  "Create new normal form",
				  ToText(numPlayers));
  if (label) {
    numPlayers = ToNumber(label);
    if (numPlayers < 1) {
      return 0;
    }
    else {
      return numPlayers;
    }
  }
  else {
    return 0;
  }
}
 
int NfgGUI::GetStrategies(gArray<int> &p_dimensionality, wxFrame *p_parent)
{
  dialogDimensionality dialog(p_parent, p_dimensionality.Length());
  
  if (dialog.Completed() == wxOK) {
    for (int pl = 1; pl <= p_dimensionality.Length(); pl++) {
      p_dimensionality[pl] = ToNumber(dialog.GetValue(pl));
    }
    return 1;
  }
  else {
    return 0;
  }
}

//**********************************************************************
//                       NORMAL DRAW OPTIONS
//**********************************************************************

NormalDrawSettings::NormalDrawSettings(void)
  : output_precision(2)
{
  wxGetResource("Gambit", "NFOutcome-Display", &outcome_disp, 
		gambitApp.ResourceFile());
}

NormalDrawSettings::~NormalDrawSettings()
{
  SaveSettings();
}

void NormalDrawSettings::SaveSettings(void) const
{
  wxWriteResource("Gambit", "NFOutcome-Display", outcome_disp,
		  gambitApp.ResourceFile());
}

//**********************************************************************
//                       NORMAL SPREAD
//**********************************************************************

#ifdef wx_msw
#include "wx_bbar.h"
#else
#include "wx_tbar.h"
#endif

//=====================================================================
//                       class GambitToolBar
//=====================================================================

#ifdef wx_msw
class NfgShowToolBar : public wxButtonBar {
#else
class NfgShowToolBar : public wxToolBar {
#endif
private:
  wxFrame *parent;

public:
  NfgShowToolBar(wxFrame *frame);
  Bool OnLeftClick(int toolIndex, Bool toggled);
  void OnMouseEnter(int toolIndex);
};

NfgShowToolBar::NfgShowToolBar(wxFrame *frame)
#ifdef wx_msw
  : wxButtonBar(frame, 0, 0, -1, -1, 0, wxVERTICAL, 1), parent(frame)
#else
  : wxToolBar(frame, 0, 0, -1, -1, 0, wxHORIZONTAL, 30), parent(frame)
#endif
{
#ifdef wx_msw
  wxBitmap *saveBitmap = new wxBitmap("SAVE_BITMAP");
  wxBitmap *printBitmap = new wxBitmap("PRINT_BITMAP");
  wxBitmap *solveBitmap = new wxBitmap("SOLVE_BITMAP");
  wxBitmap *helpBitmap = new wxBitmap("HELP_BITMAP");
  wxBitmap *optionsBitmap = new wxBitmap("OPTIONS_BITMAP");
  wxBitmap *inspectBitmap = new wxBitmap("INSPECT_BITMAP");
#else
#include "bitmaps/save.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/solve.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/options.xpm"
#include "bitmaps/inspect.xpm"
  wxBitmap *saveBitmap = new wxBitmap(save_xpm);
  wxBitmap *printBitmap = new wxBitmap(print_xpm);
  wxBitmap *solveBitmap = new wxBitmap(solve_xpm);
  wxBitmap *helpBitmap = new wxBitmap(help_xpm);
  wxBitmap *optionsBitmap = new wxBitmap(options_xpm);
  wxBitmap *inspectBitmap = new wxBitmap(inspect_xpm);
#endif // wx_msw

  SetMargins(2, 2);
#ifdef wx_msw
  SetDefaultSize(33, 30);
#endif
  GetDC()->SetBackground(wxLIGHT_GREY_BRUSH);

  AddTool(NFG_FILE_SAVE, saveBitmap);
  AddTool(OUTPUT_MENU, printBitmap);
  AddSeparator();
  AddTool(NFG_SOLVE_STANDARD, solveBitmap);
  AddTool(NFG_VIEW_SOLUTIONS, inspectBitmap);
  AddSeparator();
  AddTool(OPTIONS_MENU, optionsBitmap);
  AddSeparator();
  AddTool(HELP_MENU_CONTENTS, helpBitmap);

  CreateTools();
  Layout();
}


Bool NfgShowToolBar::OnLeftClick(int tool, Bool )
{
    parent->OnMenuCommand(tool);
    return TRUE;
}


void NfgShowToolBar::OnMouseEnter(int tool)
{
    parent->SetStatusText(parent->GetMenuBar()->GetHelpString(tool));
}

// Note that the menubar required for the normal form display is too
// different from the default SpreadSheet3D one to warrant an override of
// the MakeMenuBar function.  Thus, no features are selected in the main
// SpreadSheet3D constructor, except for the panel.

NormalSpread::NormalSpread(const NFSupport *sup, int _pl1, int _pl2, NfgShow *p, wxFrame *p_frame) 
    : SpreadSheet3D(sup->NumStrats(_pl1), sup->NumStrats(_pl2), 1, 3, "", p_frame, ANY_BUTTON),
      strat_profile(sup->Game().NumPlayers()), 
      parent(p), pl1(_pl1), pl2(_pl2),
      dimensionality(sup->NumStrats())
{
#ifdef wx_xview // xview has a bug that will not display any menus if the window is too small
    SetSize(200, 600);
#endif

    int num_players = dimensionality.Length();

    // column widths
    DrawSettings()->SetColWidth(num_players*(3 + ToTextPrecision()));
    DrawSettings()->SetLabels(S_LABEL_ROW|S_LABEL_COL);

    //------------------take care of the frame/window stuff
    // Give the frame an icon
    wxIcon *frame_icon;
#ifdef wx_msw
    frame_icon = new wxIcon("nfg_icn");
#else
#include "nfg.xbm"
    frame_icon = new wxIcon(nfg_bits, nfg_width, nfg_height);
#endif

    SetIcon(frame_icon);
    toolbar = new NfgShowToolBar(this);

    wxPanel *sub_panel = Panel();

    // Create the list boxes for choosing which players to display for row/col
    const Nfg &nf = sup->Game();
    char **player_names = new char *[num_players];

    for (int i = 0; i < num_players; i++) {
        if (features.verbose && nf.Players()[i+1]->GetName() != "")
            player_names[i] = copystring(nf.Players()[i+1]->GetName());
        else
            player_names[i] = copystring(ToText(i + 1));
    }

    sub_panel->SetLabelPosition(wxVERTICAL);
    row_choice = new wxChoice(sub_panel, (wxFunction)NormalSpread::normal_player_func, 
                              "Row Player", -1, -1, -1, -1, num_players, player_names);
    col_choice = new wxChoice(sub_panel, (wxFunction)NormalSpread::normal_player_func, 
                              "Col Player", -1, -1, -1, -1, num_players, player_names);
    sub_panel->SetLabelPosition(wxHORIZONTAL);

    for (int i = 0; i < num_players; i++) 
    {
        delete [] player_names[i];
    }

    delete [] player_names;

    row_choice->SetClientData((char *)this);
    col_choice->SetClientData((char *)this);
    row_choice->SetSelection(_pl1 - 1);
    col_choice->SetSelection(_pl2 - 1);
    sub_panel->NewLine();

    // Create the list boxes for strategies for each player
    (void)new wxMessage(sub_panel, "Profile:");
    sub_panel->NewLine();

    for (int i = 1; i <= num_players; i++)
    {
        char **strat_profile_str = new char *[dimensionality[i]];
        int j;

        for (j = 0; j < dimensionality[i]; j++)
        {
            if (features.verbose)
                strat_profile_str[j] = copystring(sup->Strategies(i)[j+1]->Name());
            else
                strat_profile_str[j] = copystring(ToText(j + 1));
        }

        strat_profile[i] = new wxChoice(sub_panel, 
                                        (wxFunction)NormalSpread::normal_strat_func, 
                                        "", -1, -1, -1, -1, dimensionality[i], 
                                        strat_profile_str);
        strat_profile[i]->SetSelection(0);
        strat_profile[i]->SetClientData((char *)this);

        for (j = 0; j < dimensionality[i]; j++) 
            delete [] strat_profile_str[j];

        delete [] strat_profile_str;
    }

    sub_panel->Fit();
    SetEditable(FALSE);
    SetMenuBar(MakeMenuBar(0));
    UpdateMenus();
    Redraw();
    Show(TRUE);
}

void NormalSpread::UpdateProfile(void)
{
  gArray<int> profile(strat_profile.Length());

  for (int i = 1; i <= strat_profile.Length(); i++)
    profile[i] = strat_profile[i]->GetSelection()+1;

  parent->UpdateProfile(profile);
  SetCurRow(profile[pl1]);
  SetCurCol(profile[pl2]);

  UpdateMenus();
}

void NormalSpread::SetProfile(const gArray<int> &profile)
{
  for (int i = 1; i <= strat_profile.Length(); i++) 
    strat_profile[i]->SetSelection(profile[i]-1);

  SetCurRow(profile[pl1]);
  SetCurCol(profile[pl2]);
  UpdateMenus();
}

gArray<int> NormalSpread::GetProfile(void)
{
  gArray<int> profile(strat_profile.Length());
  
  for (int i = 1; i <= strat_profile.Length(); i++) {
    // Moving about the normal form table by clicking cells does not
    // update the selection dropdown lists.  We must explicitly
    // check the location of the selected cell to figure out the
    // (current) row and column players' strategies
    if (i == pl1)
      profile[i] = CurRow();
    else if (i == pl2)
      profile[i] = CurCol();
    else
      profile[i] = strat_profile[i]->GetSelection()+1;
  }

  return profile;
}

// Overriding the MakeMenuBar... Note that menus is not used.
wxMenuBar *NormalSpread::MakeMenuBar(long )
{
  wxMenu *file_menu = new wxMenu;
  file_menu->Append(NFG_FILE_SAVE, "&Save",    "Save the game");
  file_menu->Append(OUTPUT_MENU,   "Out&put",  "Output to any device");
  file_menu->Append(CLOSE_MENU,    "&Close",   "Exit");
  
  wxMenu *edit_menu = new wxMenu;
  edit_menu->Append(NFG_EDIT_GAME, "&Label", "Set the label of the game");
  edit_menu->Append(NFG_EDIT_PLAYERS, "&Players", "Edit player names");
  edit_menu->Append(NFG_EDIT_STRATS, "&Strategies", "Edit strategy names");

  wxMenu *editOutcomesMenu = new wxMenu;
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_NEW, "&New",
			   "Create a new outcome");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_DELETE, "Dele&te",
			   "Delete an outcome");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_ATTACH, "&Attach",
			   "Attach an outcome to the current contingency");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_DETACH, "&Detach",
			   "Set the outcome for the current contingency to null");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_PAYOFFS, "&Payoffs",
			   "Set the payoffs for outcome of the current contingency");
  edit_menu->Append(NFG_EDIT_OUTCOMES,  "&Outcomes",  editOutcomesMenu,
		    "Set/Edit outcomes");

  wxMenu *supports_menu = new wxMenu;
  supports_menu->Append(NFG_SUPPORT_UNDOMINATED, "&Undominated",
			"Find undominated strategies");
  supports_menu->Append(NFG_SUPPORT_NEW, "&New",
			"Create a new support");
  supports_menu->Append(NFG_SUPPORT_EDIT, "&Edit",
			"Edit the currently displayed support");
  supports_menu->Append(NFG_SUPPORT_DELETE, "&Delete",
			"Delete a support");
  wxMenu *supportsSelectMenu = new wxMenu;
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_FROMLIST, "From &List...",
			     "Select a support from the list of defined supports");
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_PREVIOUS, "&Previous",
			     "Select the previous support from the list");
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_NEXT, "&Next",
			     "Select the next support from the list");
  supports_menu->Append(NFG_SUPPORT_SELECT, "&Select", supportsSelectMenu,
			"Change the current support");

  wxMenu *solve_menu = new wxMenu;
  solve_menu->Append(NFG_SOLVE_STANDARD,  "S&tandard...",
		     "Standard solutions");
  
  wxMenu *solveCustomMenu = new wxMenu;
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_ENUMPURE, "EnumPure",
			  "Enumerate pure strategy equilibria");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_ENUMMIXED, "EnumMixed",
			  "Enumerate mixed strategy equilibria");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LCP, "LCP",
			  "Solve via linear complementarity program");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LP, "LP",
			  "Solve via linear program");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LIAP, "Liapunov",
			  "Minimization of liapunov function");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_SIMPDIV, "Simpdiv",
			  "Solve via simplicial subdivision");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_POLENUM, "PolEnum",
			  "Enumeration by systems of polynomials");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_QRE, "QRE",
			  "Compute quantal response equilibrium");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_QREGRID, "QREGrid",
			  "Compute quantal response equilibrium");
  solve_menu->Append(NFG_SOLVE_CUSTOM, "Custom", solveCustomMenu,
		     "Solve with a particular algorithm");

  
  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(NFG_VIEW_SOLUTIONS, "&Solutions",
		   "Display solutions");
  viewMenu->Append(NFG_VIEW_DOMINANCE, "&Dominance",
		   "Display dominance information", TRUE);
  viewMenu->Append(NFG_VIEW_PROBABILITIES, "&Probabilities",
		   "Display solution probabilities", TRUE);
  viewMenu->Append(NFG_VIEW_VALUES, "&Values",
		   "Display strategy values", TRUE);
  viewMenu->Append(NFG_VIEW_OUTCOMES, "&Outcomes",
		   "Display outcome names", TRUE);
  viewMenu->Append(NFG_VIEW_GAMEINFO, "Game&Info",
		   "Display information about the game");
  
  wxMenu *prefsMenu = new wxMenu;
  wxMenu *prefsDisplayMenu = new wxMenu;
  prefsDisplayMenu->Append(NFG_PREFS_DISPLAY_COLUMNS, "&Column Width",
			   "Set column width");
  prefsDisplayMenu->Append(NFG_PREFS_DISPLAY_DECIMALS, "&Decimal Places",
			   "Set number of decimal places to display");

  prefsMenu->Append(NFG_PREFS_DISPLAY, "&Display", prefsDisplayMenu,
		    "Configure display options");
  prefsMenu->Append(NFG_PREFS_FONT, "&Font", "Set font");
  prefsMenu->Append(NFG_PREFS_COLORS, "&Colors", "Set player colors");
  prefsMenu->Append(NFG_PREFS_ACCELS, "&Accels", "Edit accelerators");

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(HELP_MENU_ABOUT,    "&About");
  help_menu->Append(HELP_MENU_CONTENTS, "&Contents");

  wxMenuBar *tmp_menubar = new wxMenuBar;
  tmp_menubar->Append(file_menu,     "&File");
  tmp_menubar->Append(edit_menu,     "&Edit");
  tmp_menubar->Append(supports_menu, "S&upports");
  tmp_menubar->Append(solve_menu,    "&Solve");
  tmp_menubar->Append(viewMenu,  "&View");
  tmp_menubar->Append(prefsMenu,    "&Prefs");
  tmp_menubar->Append(help_menu,     "&Help");

  if (parent->getNormalDrawSettings().OutcomeDisp()) {
    viewMenu->Check(NFG_VIEW_OUTCOMES, TRUE);
  }
  else {
    viewMenu->Check(NFG_VIEW_OUTCOMES, FALSE);
  }
  
  return tmp_menubar;
}

void NormalSpread::UpdateMenus(void)
{
  wxMenuBar *menu = GetMenuBar();
  const Nfg &nfg = parent->Game();
  gArray<int> profile(GetProfile());

  menu->Enable(NFG_EDIT_OUTCOMES_DELETE, nfg.NumOutcomes() > 0);
  menu->Enable(NFG_EDIT_OUTCOMES_ATTACH, nfg.NumOutcomes() > 0);
  menu->Enable(NFG_EDIT_OUTCOMES_DETACH, nfg.GetOutcome(profile) != 0);
  menu->Enable(NFG_EDIT_OUTCOMES_PAYOFFS, nfg.GetOutcome(profile) != 0);

  menu->Enable(NFG_SUPPORT_SELECT_FROMLIST, parent->NumSupports() > 1);
  menu->Enable(NFG_SUPPORT_SELECT_PREVIOUS, parent->NumSupports() > 1);
  menu->Enable(NFG_SUPPORT_SELECT_NEXT, parent->NumSupports() > 1);

  menu->Enable(NFG_SOLVE_CUSTOM_ENUMMIXED, nfg.NumPlayers() == 2);
  menu->Enable(NFG_SOLVE_CUSTOM_LP, nfg.NumPlayers() == 2 && IsConstSum(nfg));
  menu->Enable(NFG_SOLVE_CUSTOM_LCP, nfg.NumPlayers() == 2);

  menu->Enable(NFG_VIEW_PROBABILITIES, parent->NumSolutions() > 0);
  menu->Enable(NFG_VIEW_VALUES, parent->NumSolutions() > 0);

  SetStatusText("Support: " + parent->CurrentSupport()->GetName(), 1);
  if (parent->CurrentSolution() > 0) {
    SetStatusText("Solution: " + 
		  ToText((int) parent->Solutions()[parent->CurrentSolution()].Id()), 2);
  }
  else {
    SetStatusText("No solution displayed", 2);
  }
}

Bool NormalSpread::OnClose(void)
{
  if (parent->GameIsDirty()) {
    if (wxMessageBox("Game has been modified.  Close anyway?", "Warning",
		     wxOK | wxCANCEL) == wxCANCEL)
      return FALSE;
    else {
      parent->InterfaceDied();
      Show(FALSE);
      return TRUE;
    }
  }
  else {
    return TRUE;
  }
}

void NormalSpread::SetStrategyLabels(const NFSupport *cur_sup)
{
  gArray<int> profile = GetProfile();
  // update the profile choiceboxes
  int i;

  for (i = 1; i <= dimensionality.Length(); i++)
    {
      strat_profile[i]->Clear();
      
      for (int j = 1; j <= cur_sup->NumStrats(i); j++)
	strat_profile[i]->Append(cur_sup->Strategies(i)[j]->Name());
    }

  // Update the row/col labels
  for (i = 1; i <= cur_sup->NumStrats(pl1); i++)
    SetLabelRow(i, cur_sup->Strategies(pl1)[i]->Name());
  
  for (i = 1; i <= cur_sup->NumStrats(pl2); i++)
    SetLabelCol(i, cur_sup->Strategies(pl2)[i]->Name());
  
  for (i = 1; i <= dimensionality.Length(); i++) 
    strat_profile[i]->SetSelection(profile[i] - 1);

  Redraw();
}

void NormalSpread::SetPlayerLabels(const NFSupport *cur_sup)
{
  // the row, col player choicebox
  const Nfg &nf = cur_sup->Game();
  row_choice->Clear();
  col_choice->Clear();
  
  for (int i = 1; i <= nf.NumPlayers(); i++)
    {
      row_choice->Append(nf.Players()[i]->GetName());
      col_choice->Append(nf.Players()[i]->GetName());
    }

  row_choice->SetSelection(pl1 - 1);
  col_choice->SetSelection(pl2 - 1);
  
  // Set new title
  SetTitle(nf.GetTitle() + " : " + nf.Players()[pl1]->GetName() + " x " + 
	   nf.Players()[pl2]->GetName());
}


//************ extra features for displaying dominance, probs, vals **********
// these features each create an extra row and columns.  They always go in
// order: 1. Prob, 2. Domin, 3. Value.  Make sure to update the labels if a
// feature is turned on/off.  Note that if you turn on a feature that is
// already on, no new rows/cols will be created, but the labels will be
// updated.

// Probability display
void NormalSpread::MakeProbDisp(void)
{
  int row = dimensionality[pl1] + 1;
  int col = dimensionality[pl2] + 1;

  if (!features.prob) {
    AddRow(row);
    SetSelectableRow(row, FALSE);
    AddCol(col);
    SetSelectableCol(col, FALSE);
    DrawSettings()->SetColWidth((3 + ToTextPrecision()), col);
  }

  // Note: this insures that Prob is always the FIRST extra after the
  // regular data, and Domin is AFTER the prob.
  SetLabelRow(row, "Prob");
  SetLabelCol(col, "Prob");
  features.prob = 1;
  GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, TRUE);
}


void NormalSpread::RemoveProbDisp(void)
{
  if (features.prob) {
    int row = dimensionality[pl1] + 1;
    int col = dimensionality[pl2] + 1;
    DelRow(row);
    DelCol(col);
    features.prob = 0;
  }
  GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, FALSE);
}


// Dominance display
void NormalSpread::MakeDomDisp(void)
{
  int row = dimensionality[pl1] + features.prob + 1;
  int col = dimensionality[pl2] + features.prob + 1;

  if (!features.dom) {
    AddRow(row);
    SetSelectableRow(row, FALSE);
    AddCol(col);
    SetSelectableCol(col, FALSE);
    DrawSettings()->SetColWidth(5, col);
  }

  SetLabelRow(row, "Domin");
  SetLabelCol(col, "Domin");
  features.dom = 1;

  GetMenuBar()->Check(NFG_VIEW_DOMINANCE, TRUE);
}


void NormalSpread::RemoveDomDisp(void)
{
  if (features.dom) {
    int row = dimensionality[pl1] + features.prob + 1;
    int col = dimensionality[pl2] + features.prob + 1;
    DelRow(row);
    DelCol(col);
    features.dom = 0;
  }
  GetMenuBar()->Check(NFG_VIEW_DOMINANCE, FALSE);
}


// Value display
void NormalSpread::MakeValDisp(void)
{
  int row = dimensionality[pl1] + features.prob + features.dom + 1;
  int col = dimensionality[pl2] + features.prob + features.dom + 1;

  if (!features.val) {
    AddRow(row);
    SetSelectableRow(row, FALSE);
    AddCol(col);
    SetSelectableCol(col, FALSE);
    DrawSettings()->SetColWidth((3 + ToTextPrecision()), col);
  }
  
  SetLabelRow(row, "Value");
  SetLabelCol(col, "Value");
  features.val = 1;
  GetMenuBar()->Check(NFG_VIEW_VALUES, TRUE);
}


void NormalSpread::RemoveValDisp(void)
{
  if (features.val) {
    int row = dimensionality[pl1] + features.prob + features.dom + 1;
    int col = dimensionality[pl2] + features.prob + features.dom + 1;
    DelRow(row);
    DelCol(col);
    features.val = 0;
  }
  GetMenuBar()->Check(NFG_VIEW_VALUES, FALSE);
}


void NormalSpread::SetDimensionality(const NFSupport *p_support)
{
  gArray<int> dim = p_support->NumStrats();

  for (int i = 1; i <= dim.Length(); i++) {
    strat_profile[i]->Clear();

    for (int j = 1; j <= dim[i]; j++)
      strat_profile[i]->Append(p_support->Strategies(i)[j]->Name());

    strat_profile[i]->SetSelection(0);
    dimensionality[i] = dim[i];
  }
}


// OnPrint
void NormalSpread::OnPrint(void)
{
    parent->Print();
}


//****************************WINDOW EVENT HANDLERS************************

void NormalSpread::OnMenuCommand(int id)
{
  try {
    switch (id) {
    case NFG_SOLVE_CUSTOM_ENUMPURE:
    case NFG_SOLVE_CUSTOM_ENUMMIXED:
    case NFG_SOLVE_CUSTOM_LCP:
    case NFG_SOLVE_CUSTOM_LP:
    case NFG_SOLVE_CUSTOM_LIAP:
    case NFG_SOLVE_CUSTOM_SIMPDIV:
    case NFG_SOLVE_CUSTOM_POLENUM:
    case NFG_SOLVE_CUSTOM_QRE:
    case NFG_SOLVE_CUSTOM_QREGRID:
      parent->Solve(id);
      break;

    case NFG_VIEW_SOLUTIONS:
      parent->InspectSolutions(CREATE_DIALOG);
      break;
    case NFG_VIEW_DOMINANCE:
      if (HaveDom()) {
	RemoveDomDisp();
      }
      else {
	MakeDomDisp();
      }
      parent->SetPlayers(pl1, pl2);
      parent->UpdateVals();
      Redraw();
      break;
    case NFG_VIEW_PROBABILITIES:
      if (HaveProbs()) {
	RemoveProbDisp();
      }
      else {
	MakeProbDisp();
      }
      parent->UpdateVals();
      parent->UpdateSoln();
      Redraw();
      break;
    case NFG_VIEW_VALUES:
      if (HaveVal()) {
	RemoveValDisp();
      }
      else {
	MakeValDisp();
      }
      parent->UpdateVals();
      parent->UpdateSoln();
      Redraw();
      break;
    case NFG_VIEW_OUTCOMES:
      parent->OutcomeOptions();
      if (parent->getNormalDrawSettings().OutcomeDisp()) {
	GetMenuBar()->Check(NFG_VIEW_OUTCOMES, TRUE);
      }
      else {
	GetMenuBar()->Check(NFG_VIEW_OUTCOMES, FALSE);
      }
      break;
    case NFG_VIEW_GAMEINFO:
      parent->ShowGameInfo();
      break;


    case NFG_SUPPORT_UNDOMINATED:
      parent->SolveElimDom();
      break;
    case NFG_SUPPORT_NEW:
      parent->SupportNew();
      break;
    case NFG_SUPPORT_EDIT:
      parent->SupportEdit();
      break;
    case NFG_SUPPORT_DELETE:
      parent->SupportDelete();
      break;
    case NFG_SUPPORT_SELECT_FROMLIST:
      parent->SupportSelectFromList();
      break;
    case NFG_SUPPORT_SELECT_PREVIOUS:
      parent->SupportSelectPrevious();
      break;
    case NFG_SUPPORT_SELECT_NEXT:
      parent->SupportSelectNext();
      break;

    case NFG_SOLVE_STANDARD: 
      parent->SolveStandard();
      break;

    case NFG_EDIT_GAME: 
      parent->EditLabel();
      break;
    case NFG_EDIT_STRATS: 
      parent->SetStrategyLabels();
      break;
    case NFG_EDIT_PLAYERS: 
      parent->SetPlayerLabels();
      break;
    case NFG_EDIT_OUTCOMES_NEW:
      parent->OutcomeNew();
      break;
    case NFG_EDIT_OUTCOMES_DELETE:
      parent->OutcomeDelete();
      break;
    case NFG_EDIT_OUTCOMES_ATTACH:
      parent->OutcomeAttach();
      break;
    case NFG_EDIT_OUTCOMES_DETACH:
      parent->OutcomeDetach();
      break;
    case NFG_EDIT_OUTCOMES_PAYOFFS:
      parent->OutcomePayoffs(CurRow(), CurCol(), false);
      break;

    case NFG_PREFS_DISPLAY_COLUMNS:
      parent->PrefsDisplayColumns();
      break;
    case NFG_PREFS_DISPLAY_DECIMALS:
      parent->PrefsDisplayDecimals();
      break;
    case NFG_PREFS_FONT:
      parent->PrefsFont();
      break;
    case NFG_PREFS_COLORS: 
      parent->SetColors();
      break;
    case NFG_PREFS_ACCELS: 
      parent->EditAccelerators();
      break;

    case NFG_FILE_SAVE:
      parent->Save();
      break;

    case NFG_ACCL_PAYOFF:
      parent->OutcomePayoffs(CurRow(), CurCol(), false);
      break;

    case NFG_ACCL_NEXT_PAYOFF:
      parent->OutcomePayoffs(CurRow(), CurCol(), true);
      break;

    default: 
      SpreadSheet3D::OnMenuCommand(id);
      break;
    }
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), this);
  }

  if (id != CLOSE_MENU)
    UpdateMenus();
}


// OnCharNew.  Return TRUE if this key has been handled by parent.
Bool NormalSpread::OnCharNew(wxKeyEvent &ev)
{
    return (parent->CheckAccelerators(ev)) ? TRUE: FALSE;
}


// OnHelp
void NormalSpread::OnHelp(int help_type)
{
    if (!help_type) // contents
        wxHelpContents(NFG_GUI_HELP);
    else
        wxHelpAbout();
}


void NormalSpread::OnOptionsChanged(unsigned int options)
{
    if (options&S_PREC_CHANGED)
    {
        // column widths
        DrawSettings()->SetColWidth(dimensionality.Length() * (3 + ToTextPrecision()));
        parent->UpdateVals();
        Redraw();
        Repaint();
    }
}







