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

#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgnewsupport.h"
#include "dlnfgsave.h"

//======================================================================
//                 NfgShow: Constructor and destructor
//======================================================================

NfgShow::NfgShow(Nfg &N, EfgNfgInterface *efg, wxFrame *pframe_)
  : EfgNfgInterface(gNFG, efg), nf(N), nf_iter(N), pframe(pframe_)

{
  pl1 = 1;
  pl2 = 2;        // use the defaults
  cur_soln = 0;
  cur_sup = new NFSupport(nf);    // base support
  disp_sup = cur_sup;
  supports.Append(cur_sup);
  
  spread = new NormalSpread(disp_sup, pl1, pl2, this, pframe);

  support_dialog = 0;  // no support dialog yet
  soln_show      = 0;  // no solution inspect window yet.
  SetPlayers(pl1, pl2, true);

  // Create the accelerators
  ReadAccelerators(accelerators, "NfgAccelerators");
  
  UpdateVals();
  spread->Redraw();

  nf.SetIsDirty(false);
}

void NfgShow::UpdateVals(void)
{
  if (!(nf_iter.Support() == *disp_sup)) 
    nf_iter = NfgIter(*disp_sup);

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
	  pay_str = "Outcome 0";
	}
      }

      spread->SetCell(i, j, pay_str);
      spread->HiLighted(i, j, 0, hilight);
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
  // displaying solutions created for supports other than disp_sup

  MixedSolution soln = solns[cur_soln];
  gNumber t_max;
  gArray<int> profile(nf.NumPlayers());

  // Figure out the index in the disp_sup, then map it onto the full support
  for (int pl = 1; pl <= nf.NumPlayers(); pl++) {
    profile[pl] = 1;
    t_max = soln(nf.Players()[pl]->Strategies()[1]);

    for (int st1 = 1; st1 <= disp_sup->NumStrats(pl); st1++) {
      if (soln(disp_sup->Strategies(pl)[st1]) > t_max) {
    profile[pl] = st1;
    t_max = soln(disp_sup->Strategies(pl)[st1]);
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
      if (soln(disp_sup->Strategies(pl1)[st1]) > eps
      && soln(disp_sup->Strategies(pl2)[st2]) > eps)
    spread->HiLighted(st1, st2, 0, TRUE);
      else
    spread->HiLighted(st1, st2, 0, FALSE);
    }
  }


  if (spread->HaveProbs()) {
    // Print out the probability in the next column/row
    for (int i = 1; i <= rows; i++)
      spread->SetCell(i, cols+1,
              ToText(soln(disp_sup->Strategies(pl1)[i])));

    for (int i = 1; i <= cols; i++)
      spread->SetCell(rows+1, i, 
              ToText(soln(disp_sup->Strategies(pl2)[i])));
  }

  if (spread->HaveVal()) {
    // Print out the probability in the last column/row
    for (int i = 1; i <= rows; i++) {
      spread->SetCell(i, cols+spread->HaveProbs()+spread->HaveDom()+1, 
              ToText(soln.Payoff(nf.Players()[pl1],
                     disp_sup->Strategies(pl1)[i])));
    }
    
    for (int j = 1; j <= cols; j++) {
      spread->SetCell(rows+spread->HaveProbs()+spread->HaveDom()+1, j, 
              ToText(soln.Payoff(nf.Players()[pl2],
                     disp_sup->Strategies(pl2)[j])));
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
  dialogNfgSave dialog(Filename(), nf.GetTitle(), s_nDecimals, pframe);

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

  if (spread->HaveDom()) {  // if there exist the dominance row/col
    int dom_pos = 1+spread->HaveProbs();
    for (int i = 1; i <= cols; i++)
      spread->SetCell(rows+dom_pos, i, "");

    for (int i = 1; i <= rows; i++)
      spread->SetCell(i, cols+dom_pos, "");
  }

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
    if (solns[sol].Support().IsSubset(*disp_sup)) {
      cur_soln = sol;

      if (cur_soln)
	UpdateSoln();
    }
    else {
      int ok = wxMessageBox("This solution was created in a support that is not\n "
			    "a subset of the currently displayed support.\n"
			    "Displayed probabilities may not add up to 1.\n"
			    "Are you sure you want to display this solution?", 
			    "Not a subset", wxYES_NO | wxCENTRE);

      if (ok == wxYES) {
	cur_soln = sol;

	if (cur_soln)
	  UpdateSoln();
      }
    }
  }
  else {
    cur_soln = 0;
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
  spread->EnableInspect(FALSE);
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

  ChangeSupport(DESTROY_DIALOG);

  spread->Close();
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
#include "elimdomd.h"
#include "nfgsolng.h"

void NfgShow::Solve(int id)
{
  NfgSolveSettings NSD(nf);

  // If we have more than 1 support, we must have created it explicitly.
  // In that case use the currently set support.  Otherwise, only the
  // default support exists and we should use a support dictated by
  // dominance defaults.
  NFSupport *sup = (supports.Length() > 1) ? cur_sup : 0;

  if (!sup)
    sup = MakeSolnSupport();

  int old_max_soln = solns.Length();  // used for extensive update

  NfgSolutionG *solver;

  switch (id) {
  case NFG_SOLVE_CUSTOM_ENUMPURE:
    solver = new NfgEnumPureG(nf, *sup, this);
    break;
  case NFG_SOLVE_CUSTOM_ENUMMIXED:
    solver = new NfgEnumG(nf, *sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LCP:      
    solver = new NfgLemkeG(nf, *sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LP:       
    solver = new NfgZSumG(nf, *sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LIAP:
    solver = new NfgLiapG(nf, *sup, this);
    break;
  case NFG_SOLVE_CUSTOM_SIMPDIV:
    solver = new NfgSimpdivG(nf, *sup, this);
    break;
  case NFG_SOLVE_CUSTOM_POLENUM:
    solver = new guiPolEnumNfg(nf, *sup, this);
    break;
  case NFG_SOLVE_CUSTOM_QRE:
    solver = new NfgQreG(nf, *sup, this);
    break;
  case NFG_SOLVE_CUSTOM_QREGRID:
    solver = new NfgQreAllG(nf, *sup, this);
    break;
  default:
    // shouldn't happen.  we'll ignore silently
    return;
  }

  bool go = solver->SolveSetup();

  wxBeginBusyCursor();

  try {
    if (go)
      solns += solver->Solve();
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
    if (NSD.GetExtensive()) {
      for (int i = old_max_soln+1; i <= solns.Length(); i++) 
    SolutionToExtensive(solns[i]);
    }

    if (!spread->HaveProbs()) {
      spread->MakeProbDisp();
      spread->Redraw();
    }

    ChangeSolution(solns.VisibleLength());
    spread->EnableInspect(TRUE);
    
    InspectSolutions(CREATE_DIALOG);
  }
}


void NfgShow::SolveStandard(void)
{ 
  try {
    if (NfgSolveStandardDialog(nf, (wxWindow *) spread).Completed() != wxOK)
      return;
  }
  catch (guiBadStandardSolve &E) {
    wxMessageBox((char *) E.Description(), "Standard Solution");
    return;
  }

  NfgSolveSettings NSD(nf);

  NFSupport *sup = (supports.Length() > 1) ? cur_sup : 0;

  if (!sup)
    sup = MakeSolnSupport();

  int old_max_soln = solns.Length();  // used for extensive update

  NfgSolutionG *solver;

  switch (NSD.GetAlgorithm()) {
  case NFG_ENUMPURE_SOLUTION:
    solver = new NfgEnumPureG(nf, *sup, this);
    break;
  case NFG_ENUMMIXED_SOLUTION:
    solver = new NfgEnumG(nf, *sup, this);
    break;
  case NFG_LCP_SOLUTION:
    solver = new NfgLemkeG(nf, *sup, this);
    break;
  case NFG_LP_SOLUTION:
    solver = new NfgZSumG(nf, *sup, this);
    break;
  case NFG_LIAP_SOLUTION:
    solver = new NfgLiapG(nf, *sup, this);
    break;
  case NFG_SIMPDIV_SOLUTION:
    solver = new NfgSimpdivG(nf, *sup, this);
    break;
  case NFG_QRE_SOLUTION:
    solver = new NfgQreG(nf, *sup, this);
    break;
  case NFG_QREALL_SOLUTION:
    solver = new NfgQreAllG(nf, *sup, this);
    break;
  default:
    // shouldn't happen.  we'll ignore silently
    return;
  }

  wxBeginBusyCursor();

  try {
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
    if (NSD.GetExtensive()) {
      for (int i = old_max_soln+1; i <= solns.Length(); i++) 
    SolutionToExtensive(solns[i]);
    }

    if (!spread->HaveProbs()) {
      spread->MakeProbDisp();
      spread->Redraw();
    }

    ChangeSolution(solns.VisibleLength());
    spread->EnableInspect(TRUE);
    
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

#include "gstatus.h"
extern NFSupport *ComputeDominated(const Nfg &, NFSupport &S, bool strong,
				   const gArray<int> &players,
				   gOutput &tracefile, 
				   gStatus &status); // in nfdom.cc


NFSupport *NfgShow::MakeSolnSupport(void)
{
  NFSupport *sup = new NFSupport(nf);
  DominanceSettings DS;  // reads in dominance defaults
  gArray<int> players(nf.NumPlayers());

  for (int i = 1; i <= nf.NumPlayers(); i++) 
    players[i] = i;

  if (DS.UseElimDom()) {
    NFSupport *temp_sup;

    if (DS.FindAll()) {
      while ((temp_sup = ComputeDominated(sup->Game(), *sup, DS.DomStrong(), 
					  players, gnull, gstatus)) != 0)
	sup = temp_sup;
    }
    else {
      if ((temp_sup = ComputeDominated(sup->Game(), *sup, DS.DomStrong(),
				       players, gnull, gstatus)) != 0)
	sup = temp_sup;
    }
  }

  return sup;
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

  if (AssociatedNfg(efg) != &nf) 
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
  BehavProfile<gNumber> bp(S);
  MixedToBehav(mp.Game(), mp, S.Game(), bp);
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
  
  rows = disp_sup->NumStrats(pl1);
  cols = disp_sup->NumStrats(pl2);

  int features = spread->HaveDom() + spread->HaveProbs() + spread->HaveVal();
  spread->SetDimensions(rows + features, cols + features, 1);

  // Must set dimensionality in case it changed due to elim dom
  spread->SetDimensionality(disp_sup);

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
    label = disp_sup->Strategies(pl1)[i]->Name();

    if (label == "") 
      label = ToText(i);

    spread->SetLabelRow(i, label);
  }

  for (int i = 1; i <= cols; i++) {
    label = disp_sup->Strategies(pl2)[i]->Name();

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
  dialogNfgNewSupport dialog(nf, spread);

  if (dialog.Completed() == wxOK) {
    NFSupport *support = dialog.CreateSupport();
    supports.Append(support);
    return support;
  }
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

  dialogNfgPayoffs dialog(nf, nf.GetOutcome(profile), solns.Length() > 0,
			  spread);

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
  dialogNfgPayoffs dialog(nf, 0, solns.Length() > 0, pframe);

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

void NfgShow::OutcomeLabel(void)
{
  gArray<int> profile(spread->GetProfile());
  if (!nf.GetOutcome(profile))
    return;

  char *name = new char[40];
  strncpy(name, nf.GetOutcome(profile)->GetName(), 40);

  MyDialogBox *dialog = new MyDialogBox(spread, "Label outcome");
  dialog->Form()->Add(wxMakeFormString("New outcome label", &name, wxFORM_TEXT,
				       0, 0, 0, 220));
  dialog->Go();

  if (dialog->Completed() == wxOK) {
    nf.GetOutcome(profile)->SetName(name);
  }
  
  delete dialog;
  delete [] name;

  UpdateVals();
}

#include "wxstatus.h"
NFSupport *ComputeDominated(const Nfg &N, NFSupport &S, bool strong,
                            const gArray<int> &players,
                            gOutput &tracefile, gStatus &status); // nfdom.cc

NFSupport *ComputeMixedDominated(const Nfg &N, NFSupport &S, 
                                 bool strong,
                                 const gArray<int> &players,
                                 gOutput &tracefile, gStatus &status); // nfdommix.cc

#include "elimdomd.h"
#include "nfsuptd.h"

int NfgShow::SolveElimDom(void)
{
  ElimDomParamsDialog EDPD(nf.NumPlayers(), spread);

  if (EDPD.Completed() == wxOK) {
    NFSupport *sup = cur_sup;
    wxStatus status(spread, "Dominance Elimination");

    if (!EDPD.DomMixed()) {
      if (EDPD.FindAll()) {
	while ((sup = ComputeDominated(sup->Game(), 
				       *sup, EDPD.DomStrong(), 
				       EDPD.Players(), gnull, status)) != 0)
	  supports.Append(sup);
      }
      else {
	if ((sup = ComputeDominated(sup->Game(), 
				    *sup, EDPD.DomStrong(), 
				    EDPD.Players(), gnull, status)) != 0)
	  supports.Append(sup);
      }
    }
    else {
      if (EDPD.FindAll()) {
	while ((sup = ComputeMixedDominated(sup->Game(), *sup, 
					    EDPD.DomStrong(), EDPD.Players(),
					    gnull, status)) != 0)
	  supports.Append(sup);
      }
      else {
	if ((sup = ComputeMixedDominated(sup->Game(), *sup,
					 EDPD.DomStrong(), EDPD.Players(),
					 gnull, status)) != 0)
	  supports.Append(sup);
      }
    }
    
    if (EDPD.Compress() && disp_sup != sup) {
      disp_sup = supports[supports.Length()]; // displaying the last created support
      SetPlayers(pl1, pl2);
    }
    else {
      spread->MakeDomDisp();
      spread->Redraw();
    }

    UpdateSoln();
    return 1;
  }

  return 0;
}

// Support Inspect
void NfgShow::ChangeSupport(int what)
{
  if (what == CREATE_DIALOG && !support_dialog) {
    int disp = supports.Find(disp_sup), cur = supports.Find(cur_sup);
    support_dialog = new NFSupportInspectDialog(supports, cur, disp, this, spread);
    }
  
  if (what == DESTROY_DIALOG && support_dialog) {
    delete support_dialog;
    support_dialog = 0;
  }

  if (what == UPDATE_DIALOG) {
    assert(support_dialog);
    cur_sup = supports[support_dialog->CurSup()];

    if (supports[support_dialog->DispSup()] != disp_sup) {
      ChangeSolution(0);  // chances are, the current solution will not work.
      disp_sup = supports[support_dialog->DispSup()];
      SetPlayers(pl1, pl2);
    }
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
      Bool all_cont = FALSE;
      MyDialogBox cont_dialog(spread, "Continencies");
      cont_dialog.Add(wxMakeFormBool("All Contingencies", &all_cont));
      cont_dialog.Go();
      DumpAscii(all_cont);
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

// Call Spread->SetLabels afterwards to update the display
void NfgShow::SetLabels(int what)
{
  int num_players = nf.NumPlayers();
  
  if (what == 1) { // label strategies
    int max_strats = 0, i;

    for (i = 1; i <= num_players; i++) {
      if (max_strats < disp_sup->NumStrats(i)) 
	max_strats = disp_sup->NumStrats(i);
    }

    SpreadSheet3D *labels = 
      new SpreadSheet3D(num_players, max_strats, 1, "Label Strategies", spread);
    labels->DrawSettings()->SetLabels(S_LABEL_ROW);
	
    for (i = 1; i <= num_players; i++) { 
      int j;

      for (j = 1; j <= disp_sup->NumStrats(i); j++) {
	labels->SetCell(i, j, disp_sup->Strategies(i)[j]->Name());
	labels->SetType(i, j, 1, gSpreadStr);
      } // note that we continue using j

      for (; j <= max_strats; j++)
	labels->HiLighted(i, j, 1, TRUE);

      labels->SetLabelRow(i, nf.Players()[i]->GetName());
    }

    labels->Redraw();
    labels->Show(TRUE);

    while (labels->Completed() == wxRUNNING) 
      wxYield(); // wait for ok/cancel

    if (labels->Completed() == wxOK) {
      for (i = 1; i <= num_players; i++)
	for (int j = 1; j <= disp_sup->NumStrats(i); j++)
	  disp_sup->Strategies(i)[j]->SetName(labels->GetCell(i, j));
    }
    
    delete labels;
  }

  if (what == 2)  { // label players
    MyDialogBox *labels = new MyDialogBox(spread, "Label Players", NFG_EDIT_HELP);
    char **player_labels = new char *[num_players+1];
    int i;

    for (i = 1; i <= num_players; i++) {
      player_labels[i] = new char[LABEL_LENGTH];
      strcpy(player_labels[i], nf.Players()[i]->GetName());
      labels->Add(wxMakeFormString(ToText(i), &player_labels[i]));
      
      if (i % ENTRIES_PER_ROW == 0) 
	labels->Add(wxMakeFormNewLine());
    }

    labels->Go();

    if (labels->Completed() == wxOK) {
      for (i = 1; i <= num_players; i++) 
	nf.Players()[i]->SetName(player_labels[i]);
    }

    for (i = 1; i <= num_players; i++) 
      delete [] player_labels[i];

    delete [] player_labels;
  }

  spread->SetLabels(disp_sup, what);
  UpdateVals();
}


void NfgShow::ShowGameInfo(void)
{
  gText tmp;
  char tempstr[200];
  sprintf(tempstr, "Number of Players: %d", nf.NumPlayers());
  tmp += tempstr;
  tmp += "\n";
  sprintf(tempstr, "Is %sconstant sum", (IsConstSum(nf)) ? "" : "NOT ");
  tmp += tempstr;
  tmp += "\n";
  wxMessageBox(tmp, "Nfg Game Info", wxOK, spread);
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


void NfgShow::SetOptions(void)
{
  Bool disp_probs = spread->HaveProbs();
  Bool disp_dom   = spread->HaveDom();
  Bool disp_val   = spread->HaveVal();

  MyDialogBox *norm_options = 
    new MyDialogBox(spread, "Normal GUI Options", NFG_FEATURES_HELP);
  wxFormItem *prob_fitem = 
        norm_options->Add(wxMakeFormBool("Display strategy probs", &disp_probs));
  norm_options->Add(wxMakeFormNewLine());
  wxFormItem *val_fitem = 
    norm_options->Add(wxMakeFormBool("Display strategy values", &disp_val));
  norm_options->Add(wxMakeFormNewLine());
  
  norm_options->Add(wxMakeFormBool("Display dominance", &disp_dom));
  norm_options->AssociatePanel();

  if (!cur_soln && !disp_probs) 
    ((wxCheckBox *)prob_fitem->GetPanelItem())->Enable(FALSE);

  if (!cur_soln && !disp_val) 
    ((wxCheckBox *)val_fitem->GetPanelItem())->Enable(FALSE);

  norm_options->Go1();
  
  if (norm_options->Completed() == wxOK) {
    bool change = false;
    if (!disp_probs && spread->HaveProbs()) {
      spread->RemoveProbDisp();
      change = true;
    }

    if (!disp_dom && spread->HaveDom()) {
      spread->RemoveDomDisp();
      change = true;
    }

    if (!disp_val && spread->HaveVal()) {
      spread->RemoveValDisp();
      change = true;
    }

    if (disp_probs && !spread->HaveProbs() && cur_soln) {
      spread->MakeProbDisp();
      change = true;
    }

    if (disp_val && !spread->HaveVal() && cur_soln) {
      spread->MakeValDisp();
      change = true;
    }

    if (change) {
      UpdateSoln();
      spread->Redraw();
    }
  }
  
  delete norm_options;
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
  WriteAccelerators(accelerators, "NfgAccelerators");
}


template class SolutionList<MixedSolution>;

//******************** NORMAL FORM GUI ******************************
#include "nfggui.h"
NfgGUI::NfgGUI(Nfg *nf, const gText infile_name, EfgNfgInterface *inter,
	       wxFrame *parent)
{
  // an already created normal form has been passed

  if (nf == 0)  {  // must create a new normal form, from scratch or from file
    gArray<int> dimensionality;
    gArray<gText> names;

    if (infile_name == "") {  // from scratch
      if (GetNFParams(dimensionality, names, parent)) {
	nf = new Nfg(dimensionality);

	for (int i = 1; i <= names.Length(); i++) 
	  nf->Players()[i]->SetName(names[i]);
      }
    }
    else {   // from data file
      try {
        gFileInput infile(infile_name);
        ReadNfgFile((gInput &) infile, nf);

	if (!nf) {
	  wxMessageBox(infile_name + " is not a valid .nfg file");
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
      MyMessageBox("Single player normal form games are not supported in the GUI", 
		   "Error", NFG_GUI_HELP, parent);
    }
  }

  if (nf_show)
    nf_show->SetFileName(infile_name);
}

// Create a normal form
#define MAX_PLAYERS 100
#define MAX_STRATEGIES  100
#define NUM_PLAYERS_PER_LINE 8
int NfgGUI::GetNFParams(gArray<int> &dimensionality, gArray<gText> &names, wxFrame *parent)
{
  int num_players = 2;
  // Get the number of players first
  MyDialogBox *make_nf_p = new MyDialogBox(parent, "Normal Form Parameters");
  make_nf_p->Form()->Add(wxMakeFormShort("How many players", &num_players, wxFORM_TEXT,
					 new wxList(wxMakeConstraintRange(2, MAX_PLAYERS), 0), 
					 NULL, 0, 220));
  make_nf_p->Go();
  int ok = make_nf_p->Completed();
  delete make_nf_p;
  
  if (ok != wxOK || num_players < 1)
    return 0;
  
  // if we got a valid # of players, go on to get the dimensionality
  MyDialogBox *make_nf_dim = new MyDialogBox(parent, "Normal Form Parameters");
  make_nf_dim->Add(wxMakeFormMessage("How many strategies for\neach player?"));
  dimensionality = gArray<int>(num_players);
  
  for (int i = 1; i <= num_players; i++) {
    dimensionality[i] = 2;  // Why 2?  why not?
    make_nf_dim->Add(wxMakeFormShort(ToText(i), &dimensionality[i], wxFORM_TEXT,
				     new wxList(wxMakeConstraintRange(1, MAX_STRATEGIES), 0), 
				     NULL, 0, 70));
    
    if (i % NUM_PLAYERS_PER_LINE == 0)
      make_nf_dim->Add(wxMakeFormNewLine());
  }
  
  make_nf_dim->Go();
  ok = make_nf_dim->Completed();
  delete make_nf_dim;
  
  if (ok != wxOK)
    return 0;

  // Now get player names
  MyDialogBox *make_nf_names = new MyDialogBox(parent, "Player Names");
  names = gArray<gText>(num_players);
  char **names_str = new char*[num_players+1];

  for (int i = 1; i <= num_players; i++) {
    names_str[i] = new char[20];
    strcpy(names_str[i], "Player"+ToText(i));
    make_nf_names->Add(wxMakeFormString(ToText(i), &names_str[i], wxFORM_TEXT,
					NULL, NULL, 0, 140));
    
    if (i%(NUM_PLAYERS_PER_LINE/2) == 0) 
      make_nf_names->Add(wxMakeFormNewLine());
  }

  make_nf_names->Go();
  ok = make_nf_names->Completed();
  delete make_nf_names;
    
  if (ok != wxOK)
    return 0;

  for (int i = 1; i <= num_players; i++) {
    names[i] = names_str[i];
    delete [] names_str[i];
  }

  delete [] names_str;
  
  return 1;
}

//**********************************************************************
//                       NORMAL DRAW OPTIONS
//**********************************************************************

NormalDrawSettings::NormalDrawSettings(void)
  : output_precision(2)
{
  char *defaults_file = "gambit.ini";
  wxGetResource("Gambit", "NFOutcome-Display", &outcome_disp, defaults_file);
}


void NormalDrawSettings::OutcomeOptions(void)
{
  MyDialogBox *options_dialog = new MyDialogBox(0, "Outcome Display");
  wxStringList *opt_list = new wxStringList("Payoff Values", "Outcome Name", 0);
  char *opt_str = new char[25];
    
  if (opt_list->Nth(outcome_disp) == NULL) {
    guiExceptionDialog("Invalid value for NFOutcome-Display; check \"gambit.ini\" file.\n",
		       main_gambit_frame);
  }
  else {
    strcpy(opt_str, (char *)opt_list->Nth(outcome_disp)->Data());
    options_dialog->Add(wxMakeFormString("Display as", &opt_str, wxFORM_RADIOBOX,
					 new wxList(wxMakeConstraintStrings(opt_list), 0)));
    options_dialog->Go();
    
    if (options_dialog->Completed() == wxOK) {
      char *defaults_file = "gambit.ini";
      outcome_disp = wxListFindString(opt_list, opt_str);
      wxWriteResource("Gambit", "NFOutcome-Display", outcome_disp, defaults_file);
    }
  }

  delete options_dialog;
  delete opt_str;
  delete opt_list;
}



//**********************************************************************
//                       NORMAL SPREAD
//**********************************************************************

#ifdef wx_msw
#include "wx_bbar.h"
#else
#include "wx_tbar.h"
#endif

class NfgShowToolBar :   // no reason to have yet another .h file for just this
#ifdef wx_msw
    public wxButtonBar
#else
public wxToolBar
#endif
{
private:
    wxFrame *parent;

public:
    NfgShowToolBar(wxFrame *frame);
    Bool OnLeftClick(int toolIndex, Bool toggled);
    void OnMouseEnter(int toolIndex);
};


// Note that the menubar required for the normal form display is too
// different from the default SpreadSheet3D one to warrant an override of
// the MakeMenuBar function.  Thus, no features are selected in the main
// SpreadSheet3D constructor, except for the panel.

NormalSpread::NormalSpread(const NFSupport *sup, int _pl1, int _pl2, NfgShow *p, wxFrame *pframe) 
    : SpreadSheet3D(sup->NumStrats(_pl1), sup->NumStrats(_pl2), 1, "", pframe, ANY_BUTTON),
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

    int i;
    wxPanel *sub_panel = Panel();

    // Create the list boxes for choosing which players to display for row/col
    const Nfg &nf = sup->Game();
    char **player_names = new char *[num_players];

    for (i = 0; i < num_players; i++)
    {
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

    for (i = 0; i < num_players; i++) 
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

    for (i = 1; i <= num_players; i++)
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
}

void NormalSpread::SetProfile(const gArray<int> &profile)
{
  for (int i = 1; i <= strat_profile.Length(); i++) 
    strat_profile[i]->SetSelection(profile[i]-1);

  SetCurRow(profile[pl1]);
  SetCurCol(profile[pl2]);
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
  edit_menu->Append(NFG_EDIT_STRATS,    "&Strats",    "Edit player strategies");
  edit_menu->Append(NFG_EDIT_PLAYERS,   "&Players",   "Edit players");

  wxMenu *editOutcomesMenu = new wxMenu;
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_NEW, "&New",
			   "Create a new outcome");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_DELETE, "Dele&te",
			   "Delete an outcome");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_ATTACH, "&Attach",
			   "Attach an outcome to the current contingency");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_DETACH, "&Detach",
			   "Set the outcome for the current contingency to null");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_LABEL, "&Label",
			   "Label the outcome for the current contingency");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_PAYOFFS, "&Payoffs",
			   "Set the payoffs for outcome of the current contingency");
  edit_menu->Append(NFG_EDIT_OUTCOMES,  "&Outcomes",  editOutcomesMenu,
		    "Set/Edit outcomes");

  wxMenu *supports_menu = new wxMenu;
  supports_menu->Append(NFG_SOLVE_COMPRESS_MENU, "&ElimDom",  "Dominated strategies");
  supports_menu->Append(NFG_SOLVE_SUPPORTS_MENU, "&Select", "Select/Create NF support");

  wxMenu *solve_menu = new wxMenu;
  solve_menu->Append(NFG_SOLVE_STANDARD_MENU,  "S&tandard...",
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

  
  wxMenu *inspect_menu = new wxMenu;
  inspect_menu->Append(NFG_SOLVE_INSPECT_MENU,   "&Solutions", "Inspect existing solutions");
  inspect_menu->Append(NFG_SOLVE_FEATURES_MENU,  "In&fo",      "Advanced solution features");
  inspect_menu->Append(NFG_SOLVE_GAMEINFO_MENU,  "Game&Info", "Display some game info");
  
  wxMenu *prefs_menu = new wxMenu;
  prefs_menu->Append(NFG_PREFS_OUTCOMES_MENU, "&Outcomes", "Configure outcome display");
  prefs_menu->Append(OPTIONS_MENU,            "&Display",  "Configure display options");
  prefs_menu->Append(NFG_DISPLAY_COLORS,      "Colors",    "Set Player Colors");
  prefs_menu->Append(NFG_DISPLAY_ACCELS,      "Accels",    "Edit Accelerators");

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(HELP_MENU_ABOUT,    "&About");
  help_menu->Append(HELP_MENU_CONTENTS, "&Contents");

  wxMenuBar *tmp_menubar = new wxMenuBar;
  tmp_menubar->Append(file_menu,     "&File");
  tmp_menubar->Append(edit_menu,     "&Edit");
  tmp_menubar->Append(supports_menu, "S&upports");
  tmp_menubar->Append(solve_menu,    "&Solve");
  tmp_menubar->Append(inspect_menu,  "&Inspect");
  tmp_menubar->Append(prefs_menu,    "&Prefs");
  tmp_menubar->Append(help_menu,     "&Help");
  
  // Need these to enable/disable them depending on existance of solutions
  inspect_item = tmp_menubar->FindMenuItem("Solve", "Inspect");
  tmp_menubar->Enable(inspect_item, FALSE);

  return tmp_menubar;
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

void NormalSpread::SetLabels(const NFSupport *disp_sup, int what)
{
    if (what == 1) // strategies
    {
        gArray<int> profile = GetProfile();
        // update the profile choiceboxes
        int i;

        for (i = 1; i <= dimensionality.Length(); i++)
        {
            strat_profile[i]->Clear();

            for (int j = 1; j <= disp_sup->NumStrats(i); j++)
                strat_profile[i]->Append(disp_sup->Strategies(i)[j]->Name());
        }

        // Update the row/col labels
        for (i = 1; i <= disp_sup->NumStrats(pl1); i++)
            SetLabelRow(i, disp_sup->Strategies(pl1)[i]->Name());

        for (i = 1; i <= disp_sup->NumStrats(pl2); i++)
            SetLabelCol(i, disp_sup->Strategies(pl2)[i]->Name());

        for (i = 1; i <= dimensionality.Length(); i++) 
            strat_profile[i]->SetSelection(profile[i] - 1);

        Redraw();
    }

    if (what == 2) // players
    {
        // the row, col player choicebox
        const Nfg &nf = disp_sup->Game();
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

    if (!features.prob)
    {
        AddRow(row);
        AddCol(col);
        DrawSettings()->SetColWidth((3 + ToTextPrecision()), col);
    }

    // Note: this insures that Prob is always the FIRST extra after the
    // regular data, and Domin is AFTER the prob.
    SetLabelRow(row, "Prob");
    SetLabelCol(col, "Prob");
    features.prob = 1;
}


void NormalSpread::RemoveProbDisp(void)
{

    if (features.prob)
    {
        int row = dimensionality[pl1] + 1;
        int col = dimensionality[pl2] + 1;
        DelRow(row);
        DelCol(col);
        features.prob = 0;
    }
}


// Dominance display
void NormalSpread::MakeDomDisp(void)
{
    int row = dimensionality[pl1] + features.prob + 1;
    int col = dimensionality[pl2] + features.prob + 1;

    if (!features.dom)
    {
        AddRow(row);
        AddCol(col);
        DrawSettings()->SetColWidth(5, col);
    }

    SetLabelRow(row, "Domin");
    SetLabelCol(col, "Domin");
    features.dom = 1;
}


void NormalSpread::RemoveDomDisp(void)
{

    if (features.dom)
    {
        int row = dimensionality[pl1] + features.prob + 1;
        int col = dimensionality[pl2] + features.prob + 1;
        DelRow(row);
        DelCol(col);
        features.dom = 0;
    }
}


// Value display
void NormalSpread::MakeValDisp(void)
{
    int row = dimensionality[pl1] + features.prob + features.dom + 1;
    int col = dimensionality[pl2] + features.prob + features.dom + 1;

    if (!features.val)
    {
        AddRow(row);
        AddCol(col);
        DrawSettings()->SetColWidth((3 + ToTextPrecision()), col);
    }

    SetLabelRow(row, "Value");
    SetLabelCol(col, "Value");
    features.val = 1;
}


void NormalSpread::RemoveValDisp(void)
{
    if (features.val)
    {
        int row = dimensionality[pl1] + features.prob + features.dom + 1;
        int col = dimensionality[pl2] + features.prob + features.dom + 1;
        DelRow(row);
        DelCol(col);
        features.val = 0;
    }
}


void NormalSpread::SetDimensionality(const NFSupport *sup)
{
    gArray<int> dim = sup->NumStrats();
    assert(dim.Length() == dimensionality.Length());

    for (int i = 1; i <= dim.Length(); i++)
    {
        if (dimensionality[i] != dim[i]) // dimensionality changed
        {
            strat_profile[i]->Clear();

            for (int j = 1; j <= dim[i]; j++)
                strat_profile[i]->Append(sup->Strategies(i)[j]->Name());

            strat_profile[i]->SetSelection(0);
            dimensionality[i] = dim[i];
        }
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
    case NFG_PREFS_OUTCOMES_MENU: 
      parent->OutcomeOptions();
      break;

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

    case NFG_DISPLAY_COLORS: 
      parent->SetColors();
      break;

    case NFG_DISPLAY_ACCELS: 
      parent->EditAccelerators();
      break;

    case NFG_SOLVE_FEATURES_MENU: 
      parent->SetOptions();
      break;

    case NFG_SOLVE_INSPECT_MENU: 
      parent->InspectSolutions(CREATE_DIALOG);
      break;

    case NFG_SOLVE_COMPRESS_MENU: 
      parent->SolveElimDom();
      break;

    case NFG_SOLVE_SUPPORTS_MENU: 
      parent->ChangeSupport(CREATE_DIALOG);
      break;
      
    case NFG_SOLVE_STANDARD_MENU: 
      parent->SolveStandard();
      break;

    case NFG_SOLVE_GAMEINFO_MENU: 
      parent->ShowGameInfo();
      break;

    case NFG_EDIT_GAME: 
      parent->EditLabel();
      break;
    case NFG_EDIT_STRATS: 
      parent->SetLabels(1);
      break;
    case NFG_EDIT_PLAYERS: 
      parent->SetLabels(2);
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
    case NFG_EDIT_OUTCOMES_LABEL:
      parent->OutcomeLabel();
      break;
    case NFG_EDIT_OUTCOMES_PAYOFFS:
      parent->OutcomePayoffs(CurRow(), CurCol(), false);
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


//****************************************************************************
//                                  NORMAL FORM TOOLBAR
//****************************************************************************

NfgShowToolBar::NfgShowToolBar(wxFrame *frame):
#ifdef wx_msw
    wxButtonBar(frame, 0, 0, -1, -1, 0, wxHORIZONTAL, 30)
#else
    wxToolBar(frame, 0, 0, -1, -1, 0, wxHORIZONTAL, 30)
#endif
{
    parent = frame;

    // Load palette bitmaps
#include "bitmaps/save.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/solve.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/options.xpm"
#include "bitmaps/inspect.xpm"
    wxBitmap *ToolbarSaveBitmap = new wxBitmap(save_xpm);
    wxBitmap *ToolbarPrintBitmap = new wxBitmap(print_xpm);
    wxBitmap *ToolbarSolveBitmap = new wxBitmap(solve_xpm);
    wxBitmap *ToolbarHelpBitmap = new wxBitmap(help_xpm);
    wxBitmap *ToolbarOptionsBitmap = new wxBitmap(options_xpm);
    wxBitmap *ToolbarInspectBitmap = new wxBitmap(inspect_xpm);

    // Save, Print | Outcomes | Solve, Inspect | Options | Help
    // Create the toolbar
    SetMargins(2, 2);
#ifdef wx_msw
    SetDefaultSize(33, 30);
#endif
    GetDC()->SetBackground(wxLIGHT_GREY_BRUSH);

    AddTool(NFG_FILE_SAVE, ToolbarSaveBitmap);
    AddTool(OUTPUT_MENU, ToolbarPrintBitmap);
    AddSeparator();
    AddTool(NFG_SOLVE_STANDARD_MENU, ToolbarSolveBitmap);
    AddTool(NFG_SOLVE_INSPECT_MENU, ToolbarInspectBitmap);
    AddSeparator();
    AddTool(OPTIONS_MENU, ToolbarOptionsBitmap);
    AddSeparator();
    AddTool(HELP_MENU_CONTENTS, ToolbarHelpBitmap);
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





