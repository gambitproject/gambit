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
#include "nfgoutcd.h"

extern Bool LongStringConstraint(int type, char *value, char *label,
                 char *msg_buffer);

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
  outcome_dialog = 0;  // no outcome dialog yet.
  SetPlayers(pl1, pl2, true);

  // Create the accelerators
  ReadAccelerators(accelerators, "NfgAccelerators");
  
  UpdateVals();
  spread->Redraw();
}


// ****************************** NORM SHOW::UPDATE Vals

void NfgShow::UpdateVals(void)
{
    int i, j;

    if (!(nf_iter.Support() == *disp_sup)) 
        nf_iter = NfgIter(*disp_sup);

    for (i = 1; i <= rows; i++)
    {
        for (j = 1; j <= cols; j++)
        {
            nf_iter.Set(pl1, i);
            nf_iter.Set(pl2, j);
            gText pay_str;
            NFOutcome *outcome = nf_iter.GetOutcome();
            bool hilight = false;

            if (draw_settings.OutcomeDisp() == OUTCOME_VALUES)
            {
                for (int k = 1; k <= nf.NumPlayers(); k++)
                {
                    pay_str += ("\\C{"+ToText(draw_settings.GetPlayerColor(k))+"}");
                    pay_str += ToText(nf.Payoff(outcome, k));

                    if (k != nf.NumPlayers())
                        pay_str += ',';
                }
            }
            else
            {
                if (outcome)
                {
                    pay_str = outcome->GetName();

                    if (pay_str == "")
                        pay_str = "Outcome"+ToText(outcome->GetNumber());
                }
                else
                {
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


void NfgShow::ChangeOutcomes(int what)
{
    if (what == CREATE_DIALOG && !outcome_dialog)
        outcome_dialog = new NfgOutcomeDialog(nf, this);

    if (what == DESTROY_DIALOG && outcome_dialog)
    {
        delete outcome_dialog;
        outcome_dialog = 0;
    }
}




Nfg *CompressNfg(const Nfg &nfg, const NFSupport &S); // in nfgutils.cc

void NfgShow::Save(void)
{
  static int s_nDecimals = 6;
  gText filename = Filename();
  gText s = wxFileSelector("Save data file", (char *)gPathOnly(filename), 
               (char *)gFileNameFromPath(filename), ".nfg", "*.nfg", 
               wxSAVE | wxOVERWRITE_PROMPT);

#ifdef __GNUG__
    // Overwrite protection doesn't work in Unix, so we
    // have to check explicitly.

    if (wxFileExists((char *) s))  {  // Ask for confirmation.
      if (wxMessageBox("File exists.  Overwrite?", "Confirm", wxOK | wxCANCEL) 
      != wxOK) {
    return;
      }
    }
#endif  // __GNUG__

  if (s != "") {
    // Allow to change description 
    if (filename != "untitled.nfg") {
      char *label = new char[256];
      
      strcpy(label, nf.GetTitle());
      MyDialogBox *nfg_save_dialog = 
	new MyDialogBox(spread, "Label Game", NFG_EDIT_HELP);
      nfg_save_dialog->Add(wxMakeFormString("Label", &label, wxFORM_DEFAULT,
	new wxList(wxMakeConstraintFunction(LongStringConstraint), 0), 0, 0, 350));
      nfg_save_dialog->Add(wxMakeFormNewLine());
      nfg_save_dialog->Add(wxMakeFormShort("Decimals", &s_nDecimals, wxFORM_DEFAULT,
	new wxList(wxMakeConstraintRange(0, 25), 0)));
      nfg_save_dialog->Go();
      
      if (nfg_save_dialog->Completed() == wxOK) {
	nf.SetTitle(label);
	SetFileName(Filename()); // updates the title
      }
      
      delete nfg_save_dialog;
      delete [] label;
    }
    
    gFileOutput out(s);
    
    // Compress the nfg to the current support
    Nfg *N = CompressNfg(nf, *cur_sup);
    N->WriteNfgFile(out, s_nDecimals);
    delete N;
    SetFileName(s);
  }
}


#include "nfgciter.h"

void NfgShow::DumpAscii(Bool all_cont)
{
    char *s = wxFileSelector("Save", NULL, NULL, NULL, "*.asc", wxSAVE);

    if (s)
    {
        gFileOutput outfilef(s);

        // I have no idea why this works, but otherwise we get ambiguities:
        gOutput &outfile = outfilef;  
  
        gArray<int> profile(nf.NumPlayers());

        if (all_cont)       // if we need to save ALL the contingencies
        {
            NfgContIter nf_citer(*cur_sup);
            gBlock<int> hold_const(2);
            hold_const[1] = pl1;
            hold_const[2] = pl2;
            nf_citer.Freeze(hold_const);

            do
            {
                outfile << nf_citer.Get() << '\n';

                for (int i = 1; i <= rows; i++)
                {
                    for (int j = 1; j <= cols; j++)
                    {
                        nf_citer.Set(pl1, i);
                        nf_citer.Set(pl2, j);
                        outfile << "{ ";

                        for (int k = 1; k <= nf.NumPlayers(); k++)
                            outfile << nf.Payoff(nf_iter.GetOutcome(), k) << ' ';
                        outfile << " }  ";
                    }
                    outfile << "\n";
                }
            } while (nf_citer.NextContingency()) ;
        }
        else    // if we only need to save the contingency currently displayed
        {
            nf_iter.Get(profile);
            outfile << profile << '\n';

            for (int i = 1; i <= rows; i++)
            {
                for (int j = 1; j <= cols; j++)
                {
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
    int i, j;

    if (cur_soln)       // if there are solutions to clear, and they are displayed
    {
        for (i = 1; i <= rows; i++)
            for (j = 1; j <= cols; j++)
                spread->HiLighted(i, j, 0, FALSE);
    }

    if (spread->HaveProbs())
    {
        for (i = 1; i <= cols; i++) spread->SetCell(rows+1, i, "");

        for (i = 1; i <= rows; i++)
            spread->SetCell(i, cols+1, "");

        spread->SetCell(rows+1, cols+1, "");
    }

    if (spread->HaveDom())  // if there exist the dominance row/col
    {
        int dom_pos = 1+spread->HaveProbs();
        for (i = 1; i <= cols; i++)
            spread->SetCell(rows+dom_pos, i, "");

        for (i = 1; i <= rows; i++)
            spread->SetCell(i, cols+dom_pos, "");
    }

    if (spread->HaveVal())
    {
        int val_pos = 1+spread->HaveProbs()+spread->HaveDom();
        for (i = 1; i <= cols; i++)
            spread->SetCell(rows+val_pos, i, "");

        for (i = 1; i <= rows; i++)
            spread->SetCell(i, cols+val_pos, "");
    }
}


// ****************************** NORM SHOW::CHANGE Solution

void NfgShow::ChangeSolution(int sol)
{
    ClearSolutions();

    if (sol)
    {
        if (solns[sol].Support().IsSubset(*disp_sup))
        {
            cur_soln = sol;

            if (cur_soln)
                UpdateSoln();
        }
        else
        {
            int ok = wxMessageBox("This solution was created in a support that is not\n "
                                  "a subset of the currently displayed support.\n"
                                  "Displayed probabilities may not add up to 1.\n"
                                  "Are you sure you want to display this solution?", 
                                  "Not a subset", wxYES_NO | wxCENTRE);

            if (ok == wxYES)
            {
                cur_soln = sol;

                if (cur_soln)
                    UpdateSoln();
            }
        }
    }
    else
    {
        cur_soln = 0;
        spread->Repaint();
    }
}


// Remove solutions-permanently removes any solutions

void NfgShow::RemoveSolutions(void)
{
    if (soln_show)
    {
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
    if (soln_show)
    {
        soln_show->OnOk();
    }

    ChangeSupport(DESTROY_DIALOG);

    if (outcome_dialog)
        delete outcome_dialog;

    spread->Close();
    delete &nf;
}


void NfgShow::InspectSolutions(int what)
{
    if (what == CREATE_DIALOG)
    {
        if (solns.Length() == 0)
        {
            wxMessageBox("Solution list currently empty");
            return;
        }

        if (soln_show)
        {
            soln_show->Show(FALSE);
            delete soln_show;
        }

        soln_show = new NfgSolnShow(solns, nf.NumPlayers(), 
                                    gmax(nf.NumStrats()), 
                                    cur_soln, draw_settings, 
                                    sf_options, this, spread);
    }

    if (what == DESTROY_DIALOG && soln_show)
    {
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
    NfgSolveStandardDialog *NSSD = new NfgSolveStandardDialog(nf, (wxWindow *) spread);
    if (NSSD->Completed() != wxOK)  return;
    delete NSSD;
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


const gText &NfgShow::Filename(void) const
{ return filename; }

wxFrame *NfgShow::Frame(void)
{ return spread; }


// how: 0-default, 1-saved, 2-query

MixedProfile<gNumber> NfgShow::CreateStartProfile(int how)
{
    MixedProfile<gNumber> start(*cur_sup);

    if (how == 0)
        start.Centroid();

    if (how == 1 || how == 2)
    {
        if (starting_points.last == -1 || how == 2)
        {
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

//****************************************************************************
//                           NORMAL SOLUTIONS
//****************************************************************************

#include "gstatus.h"
NFSupport *ComputeDominated(const Nfg &, NFSupport &S, bool strong,
                            const gArray<int> &players, gOutput &tracefile, 
                            gStatus &status); // in nfdom.cc


NFSupport *NfgShow::MakeSolnSupport(void)
{
    NFSupport *sup = new NFSupport(nf);
    DominanceSettings DS;  // reads in dominance defaults
    gArray<int> players(nf.NumPlayers());

    for (int i = 1; i <= nf.NumPlayers(); i++) 
        players[i] = i;

    if (DS.UseElimDom())
    {
        NFSupport *temp_sup;

        if (DS.FindAll())
        {
            while ((temp_sup = ComputeDominated(sup->Game(), *sup, DS.DomStrong(), 
                                                players, gnull, gstatus)))
                sup = temp_sup;
        }
        else
        {
            if ((temp_sup = ComputeDominated(sup->Game(), *sup, DS.DomStrong(),
                                             players, gnull, gstatus)))
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

    if (_pl1 == _pl2)
    {
        if (num_players != 2)   // do nothing
        {
            wxMessageBox("Can not use the same player for both row and col!");
            spread->SetRowPlayer(pl1);
            spread->SetColPlayer(pl2);
            return;
        }
        else    // switch row/col
        {
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
    int i;

    for (i = 1; i <= rows; i++)
    {
        label = disp_sup->Strategies(pl1)[i]->Name();

        if (label == "") 
            label = ToText(i);

        spread->SetLabelRow(i, label);
    }

    for (i = 1; i <= cols; i++)
    {
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

    if (!first_time)
    {
        UpdateVals();
        UpdateSoln();
        UpdateDom();
        spread->Redraw();
        spread->Repaint();
    }
}


//******************* NFSUPPORTS CODE ***************
// MakeSupport -- build a new support

NFSupport *NfgShow::MakeSupport(void)
{
    MyDialogBox *support = new MyDialogBox(spread, 
                                           "Create Support", 
                                           NFG_MAKE_SUPPORT_HELP);

    support->SetLabelPosition(wxVERTICAL);
    wxListBox **players = new wxListBox*[nf.NumPlayers() + 1];

    for (int i = 1; i <= nf.NumPlayers(); i++)
    {
        int num_strats = nf.NumStrats(i);
        char **strats = new char *[num_strats];
        int j;

        for (j = 0; j < num_strats; j++) 
            strats[j] = copystring(nf.Strategies(i)[j+1]->Name());

        players[i] = new wxListBox(support, 0, nf.Players()[i]->GetName(), 
                                   TRUE, -1, -1, 80, 100,
                                   num_strats, strats);

        for (j = 0; j < num_strats; j++) 
            players[i]->SetSelection(j, TRUE);

        for (j = 0; j < num_strats; j++) 
            delete [] strats[j];

        delete [] strats;
    }

    support->Go();

    if (support->Completed() == wxOK)
    {
        NFSupport *sup = new NFSupport(nf);
        bool failed = false;

        for (int i = 1; i <= nf.NumPlayers(); i++)
        {
            int num_strats = sup->NumStrats(i);

            for (int j = num_strats; j >= 1; j--)
            {
                if (!players[i]->Selected(j - 1))
                    sup->RemoveStrategy(nf.Players()[i]->Strategies()[j]);
            }

            // Check that each player has at least one strategy.
            if (sup->NumStrats(i) == 0) 
                failed = true; 
        }

        delete support;

        if (!failed)
        {
            supports.Append(sup);
            return sup;
        }
        else
        {
            wxMessageBox("This support is invalid!\n"
                         "Each player must have at least one strategy");
            return 0;
        }
    }

    delete support;
    return 0;
}


//=========================================================================
//                        class nfgOutcomePayoffsDialog 
//=========================================================================

class nfgOutcomePayoffsDialog : public MyDialogBox {
private:
  NFOutcome *m_outcome;
  const Nfg &m_nfg;
  gArray<gNumber> m_payoffs;
  gText m_name;
  int m_completed;
  wxText *m_outcomeName;
  wxText **m_outcomePayoffs;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((nfgOutcomePayoffsDialog *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((nfgOutcomePayoffsDialog *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);

public:
  nfgOutcomePayoffsDialog(NFOutcome *, bool, wxWindow *parent);
  virtual ~nfgOutcomePayoffsDialog() { }

  int Completed(void) const { return m_completed; }
  const gArray<gNumber> &Payoffs(void) const { return m_payoffs; }
  gText Name(void) const { return m_outcomeName->GetValue(); }
};

nfgOutcomePayoffsDialog::nfgOutcomePayoffsDialog(NFOutcome *p_outcome,
						 bool p_solutions,
						 wxWindow *p_parent)
  : MyDialogBox(p_parent, "Change Payoffs"),
    m_outcome(p_outcome), m_nfg(*p_outcome->Game()),
    m_payoffs(p_outcome->Game()->NumPlayers())
{
  new wxMessage(this, "Change payoffs for outcome:");
  NewLine();

  m_outcomeName = new wxText(this, 0, "Outcome");
  NewLine();

  if (p_solutions) {
    new wxMessage(this, "Pressing OK will delete computed solutions");
    NewLine();
  }

  m_outcomePayoffs = new wxText *[m_nfg.NumPlayers()];

  const int ENTRIES_PER_ROW = 3;

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_outcomePayoffs[pl - 1] = new wxText(this, 0, "");
    m_outcomePayoffs[pl - 1]->SetValue(ToText(m_nfg.Payoff(p_outcome, pl)));
    if (pl % ENTRIES_PER_ROW == 0)
      NewLine();
  }

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void nfgOutcomePayoffsDialog::OnOK(void)
{
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++)
    FromText(m_outcomePayoffs[pl - 1]->GetValue(), m_payoffs[pl]);
  m_name = m_outcomeName->GetValue();

  m_completed = wxOK;
  Show(FALSE);
}

void nfgOutcomePayoffsDialog::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

//=========================================================================
//                        class nfgOutcomeSelectDialog 
//=========================================================================

class nfgOutcomeSelectDialog : public wxDialogBox {
private:
  Nfg &m_nfg;
  int m_outcomeSelected, m_completed;
  wxListBox *m_outcomeList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((nfgOutcomeSelectDialog *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((nfgOutcomeSelectDialog *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);

public:
  nfgOutcomeSelectDialog(Nfg &, wxWindow * = 0);
  virtual ~nfgOutcomeSelectDialog() { }

  int Completed(void) const { return m_completed; }
  NFOutcome *GetOutcome(void); 
};

nfgOutcomeSelectDialog::nfgOutcomeSelectDialog(Nfg &p_nfg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select Outcome", TRUE), m_nfg(p_nfg)
{
  m_outcomeList = new wxListBox(this, 0, "Outcome");
  
  for (int outc = 1; outc <= m_nfg.NumOutcomes(); outc++) {
    NFOutcome *outcome = m_nfg.Outcomes()[outc];
    gText item = outcome->GetName();
    if (item == "")
      item = "Outcome" + ToText(outc);

    item += (" (" + ToText(m_nfg.Payoff(outcome, 1)) + ", " +
	     ToText(m_nfg.Payoff(outcome, 2)));
    if (m_nfg.NumPlayers() > 2) {
      item += ", " + ToText(m_nfg.Payoff(outcome, 3));
      if (m_nfg.NumPlayers() > 3) 
	item += ",...)";
      else
	item += ")";
    }
    else
      item += ")";

    m_outcomeList->Append(item);
  }

  m_outcomeList->SetSelection(0);

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);
  
  Fit();
  Show(TRUE);
}

void nfgOutcomeSelectDialog::OnOK(void)
{
  m_outcomeSelected = m_outcomeList->GetSelection();
  m_completed = wxOK;
  Show(FALSE);
}

void nfgOutcomeSelectDialog::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

NFOutcome *nfgOutcomeSelectDialog::GetOutcome(void)
{
  return m_nfg.Outcomes()[m_outcomeSelected];
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

  nfgOutcomePayoffsDialog dialog(nf.GetOutcome(profile), solns.Length() > 0,
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

  nfgOutcomeSelectDialog dialog(nf, spread);
    
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
  nfgOutcomePayoffsDialog dialog(0, solns.Length() > 0, pframe);

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

  nfgOutcomeSelectDialog dialog(nf, spread);
    
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


//**************************** OUTCOMES STUFF *********************************
#define UPDATE1_DIALOG  4
#define PARAMS_ADD_VAR  5

void NfgShow::SetOutcome(int out, int x, int y)
{
  if (out > nf.NumOutcomes()) {
    MyMessageBox("This outcome is not defined yet", 
		 "Outcome", NFG_OUTCOME_HELP, spread);
    UpdateVals();
  }
  else {
    gArray<int> cur_profile(spread->GetProfile());

    if (x != -1) {    // dropped an outcome at the coordinates (x,y)
      spread->GetSheet()->ScreenToClient(&x, &y);  
      // Convert to logical coordinates.
      // This takes into account the current scrollbar position.
      x = (int)(spread->GetSheet()->GetDC()->DeviceToLogicalX((float)x));
      y = (int)(spread->GetSheet()->GetDC()->DeviceToLogicalY((float)y));

      int row, col;

      if (spread->XYtoRowCol(x, y, &row, &col)) {
	cur_profile[pl1] = row;
	cur_profile[pl2] = col;
	// MCV: commented this out because it makes the cursor
	// jump around unnecessarily.
        // spread->SetProfile(cur_profile);
      }
      else  {
	return;
      }
    }

    if (out > 0) {
      nf.SetOutcome(cur_profile, nf.Outcomes()[out]);
      RemoveSolutions();
    }
    else if (out == 0) {
      nf.SetOutcome(cur_profile, 0);
      RemoveSolutions();
    }
    else if (out == -1) { } // just update all outcomes

    UpdateVals();
  }
}


//**************************** DOMINATED STRATEGY STUFF ************************
// SolveElimDom
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

    if (EDPD.Completed() == wxOK)
    {
        NFSupport *sup = cur_sup;
        wxStatus status(spread, "Dominance Elimination");

        if (!EDPD.DomMixed())
        {
            if (EDPD.FindAll())
            {
                while ((sup = ComputeDominated(sup->Game(), 
                                               *sup, EDPD.DomStrong(), 
                                               EDPD.Players(), gnull, status)))
                    supports.Append(sup);
            }
            else
            {
                if ((sup = ComputeDominated(sup->Game(), 
                                            *sup, EDPD.DomStrong(), 
                                            EDPD.Players(), gnull, status)))
                    supports.Append(sup);
            }
        }
        else
        {
            if (EDPD.FindAll())
            {
                while ((sup = ComputeMixedDominated(sup->Game(), 
                                                    *sup, 
                                                    EDPD.DomStrong(), 
                                                    EDPD.Players(), gnull, status)))
                    supports.Append(sup);
            }
            else
            {
                if ((sup = ComputeMixedDominated(sup->Game(), 
                                                 *sup, EDPD.DomStrong(), 
                                                 EDPD.Players(), gnull, status)))
                    supports.Append(sup);
            }
        }

        if (EDPD.Compress() && disp_sup != sup)
        {
            disp_sup = supports[supports.Length()]; // displaying the last created support
            SetPlayers(pl1, pl2);
        }
        else
        {
            spread->MakeDomDisp();
            spread->Redraw();
        }

        UpdateDom();
        UpdateSoln();
        return 1;
    }

    return 0;
}


void NfgShow::DominanceSetup(void)
{
    DominanceSettingsDialog EDPD(spread);
}


// Support Inspect
void NfgShow::ChangeSupport(int what)
{
    if (what == CREATE_DIALOG && !support_dialog)
    {
        int disp = supports.Find(disp_sup), cur = supports.Find(cur_sup);
        support_dialog = new NFSupportInspectDialog(supports, cur, disp, this, spread);
    }

    if (what == DESTROY_DIALOG && support_dialog)
    {
        delete support_dialog;
        support_dialog = 0;
    }

    if (what == UPDATE_DIALOG)
    {
        assert(support_dialog);
        cur_sup = supports[support_dialog->CurSup()];

        if (supports[support_dialog->DispSup()] != disp_sup)
        {
            ChangeSolution(0);  // chances are, the current solution will not work.
            disp_sup = supports[support_dialog->DispSup()];
            SetPlayers(pl1, pl2);
        }
    }
}


// Update Dominated Strategy info
void NfgShow::UpdateDom(void)
{
/* DOM
   if (spread->HaveDom()) // Display the domination info, if its turned on
   {
   int rows = nf.NumStrats(pl1,disp_sset);
   int cols = nf.NumStrats(pl2,disp_sset);
   int dom_pos = spread->HaveProbs() + 1;
   for (int i = 1; i <= rows; i++)
   if (nf.IsDominated(pl1,i,disp_sset))
   spread->SetCell(i,cols + dom_pos,ToText(nf.GetDominator(pl1,i,disp_sset)));
   for (i = 1; i <= cols; i++)
   if (nf.IsDominated(pl2,i,disp_sset))
   spread->SetCell(rows + dom_pos,i,ToText(nf.GetDominator(pl2,i,disp_sset)));
   }
   spread->Repaint();
   */
}


// Print
void NfgShow::Print(void)
{
    wxStringList extras("ASCII", 0);
    wxOutputDialogBox print_dialog(&extras, spread);

    if (print_dialog.Completed() == wxOK)
    {
        if (!print_dialog.ExtraMedia())
        {
            spread->Print(print_dialog.GetMedia(), print_dialog.GetOption());
        }
        else    // must be dump_ascii
        {
            Bool all_cont = FALSE;
            MyDialogBox cont_dialog(spread, "Continencies");
            cont_dialog.Add(wxMakeFormBool("All Contingencies", &all_cont));
            cont_dialog.Go();
            DumpAscii(all_cont);
        }
    }
}


//**********************************LABELING CODE**************************
// SetLabels: what == 0: game, what == 1: strats, what == 2: players

#define LABEL_LENGTH    20
#define ENTRIES_PER_ROW 3

// Call Spread->SetLabels afterwards to update the display
void NfgShow::SetLabels(int what)
{
    int num_players = nf.NumPlayers();

    if (what == 0)  // label game
    {
        char *label = new char[256];

        strcpy(label, nf.GetTitle());
        MyDialogBox *nfg_label_dialog = 
            new MyDialogBox(spread, "Label Game", NFG_EDIT_HELP);
        nfg_label_dialog->Add(wxMakeFormString("Label", &label, wxFORM_DEFAULT,
            new wxList(wxMakeConstraintFunction(LongStringConstraint), 0), 0, 0, 350));
        nfg_label_dialog->Go();

        if (nfg_label_dialog->Completed() == wxOK)
        {
            nf.SetTitle(label);
            SetFileName(Filename()); // updates the title
        }

        delete nfg_label_dialog;
        delete [] label;
    }

    if (what == 1) // label strategies
    {
        int max_strats = 0, i;

        for (i = 1; i <= num_players; i++)
        {
            if (max_strats < disp_sup->NumStrats(i)) 
                max_strats = disp_sup->NumStrats(i);
        }

        SpreadSheet3D *labels = 
            new SpreadSheet3D(num_players, max_strats, 1, "Label Strategies", spread);
        labels->DrawSettings()->SetLabels(S_LABEL_ROW);

        for (i = 1; i <= num_players; i++)
        {
            int j;

            for (j = 1; j <= disp_sup->NumStrats(i); j++)
            {
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

        if (labels->Completed() == wxOK)
        {
            for (i = 1; i <= num_players; i++)
                for (int j = 1; j <= disp_sup->NumStrats(i); j++)
                    disp_sup->Strategies(i)[j]->SetName(labels->GetCell(i, j));
        }

        delete labels;
    }

    if (what == 2) // label players
    {
        MyDialogBox *labels = new MyDialogBox(spread, "Label Players", NFG_EDIT_HELP);
        char **player_labels = new char *[num_players+1];
        int i;

        for (i = 1; i <= num_players; i++)
        {
            player_labels[i] = new char[LABEL_LENGTH];
            strcpy(player_labels[i], nf.Players()[i]->GetName());
            labels->Add(wxMakeFormString(ToText(i), &player_labels[i]));

            if (i % ENTRIES_PER_ROW == 0) 
                labels->Add(wxMakeFormNewLine());
        }

        labels->Go();

        if (labels->Completed() == wxOK)
        {
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


//********************************* CONFIGURATION STUFF ********************
// SetColors

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

    //wxFormItem *dom_fitem = 
    norm_options->Add(wxMakeFormBool("Display dominance", &disp_dom));
    norm_options->AssociatePanel();

    if (!cur_soln && !disp_probs) 
        ((wxCheckBox *)prob_fitem->GetPanelItem())->Enable(FALSE);

    if (!cur_soln && !disp_val) 
        ((wxCheckBox *)val_fitem->GetPanelItem())->Enable(FALSE);

    // DOM if (nf.NumStratSets() == 1 && !disp_dom) 
    //       ((wxCheckBox *)dom_fitem->GetPanelItem())->Enable(FALSE);

    norm_options->Go1();

    if (norm_options->Completed() == wxOK)
    {
        bool change = false;

        if (!disp_probs && spread->HaveProbs())
        {
            spread->RemoveProbDisp();
            change = true;
        }

        if (!disp_dom && spread->HaveDom())
        {
            spread->RemoveDomDisp();
            change = true;
        }

        if (!disp_val && spread->HaveVal())
        {
            spread->RemoveValDisp();
            change = true;
        }

        if (disp_probs && !spread->HaveProbs() && cur_soln)
        {
            spread->MakeProbDisp();
            change = true;
        }

        /* DOM
           if (disp_dom && !spread->HaveDom() && disp_sset != 1)
           {
           spread->MakeDomDisp();
           change = true;
           }
           */

        if (disp_val && !spread->HaveVal() && cur_soln)
        {
            spread->MakeValDisp();
            change = true;
        }

        if (change)
        {
            UpdateSoln();
            UpdateDom();
            spread->Redraw();
        }
    }

    delete norm_options;
}


// Process Accelerators

#include "nfgaccl.h"
#include "sprdaccl.h"

// These events include those for NormShow and those for SpreadSheet3D
gArray<AccelEvent> NfgShow::MakeEventNames(void)
{
    gArray<AccelEvent> events(NUM_NFG_EVENTS + NUM_SPREAD_EVENTS);
    int i;

    for (i = 0; i < NUM_SPREAD_EVENTS; i++) 
        events[i+1] = spread_events[i];

    for (i = NUM_SPREAD_EVENTS; i < NUM_NFG_EVENTS + NUM_SPREAD_EVENTS; i++)
        events[i+1] = nfg_events[i - NUM_SPREAD_EVENTS];

    return events;
}


// Check Accelerators
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

    for (int i = 1; i <= num_players; i++)
    {
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

    for (int i = 1; i <= num_players; i++)
    {
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

    for (int i = 1; i <= num_players; i++)
    {
        names[i] = names_str[i];
        delete [] names_str[i];
    }

    delete [] names_str;

    return 1;
}





