//
// FILE: NfgShow.cc -- normal form GUI
//
// @(#)nfgshow.cc   1.9 02/16/98
//

#include "wx.h"
#include "nfgshow.h"
#include "nfgsoln.h"
#include "nfgconst.h"
#include "nfplayer.h" // need this for player->GetName
#include "nfgoutcd.h"

static int gmax(const gArray<int> &a)
{
    int t = a[1];

    for (int i = 2; i <= a.Length(); i++)
    {
        if (a[i] > t)
            t = a[i];
    }

    return t;
}

//**********************************************************************
//                                     NORM SHOW
//**********************************************************************
// Constructor

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

    // Create the accelerators (to add an accelerator, see const.h)
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


// ****************************** NORM SHOW::UPDATE Soln

MixedProfile<gNumber> FitToSupport(const MixedProfile<gNumber> &from, const NFSupport &sup_to)
{
    const NFSupport &sup_from = from.Support();

    if (sup_to == sup_from)
        return from;

    gArray<int> dim_from = sup_from.NumStrats();
    gArray<int> dim_to = sup_to.NumStrats();
    assert(dim_from.Length() == dim_to.Length());
    MixedProfile<gNumber> to(sup_to);

    for (int i = 1; i <= dim_to.Length(); i++)
    {
        for (int j = 1; j <= dim_to[i]; j++)
        {
            Strategy *s = sup_to.Strategies(i)[j];
            int strat = sup_from.Strategies(i).Find(s);

            if (strat)
                to(i, j) = from(i, strat);
            else 
                to(i, j) = 0;
        }
    }

    return to;
}


void NfgShow::UpdateSoln(void)
{
    if (!cur_soln)
        return;

    // The profile is obvious for pure strategy: just set the display strat
    // to the nonzero solution strategy.  However, for mixed equs, we set
    // the display strategy to the highest soluton strat.  (Note that
    // MixedSolution.Pure() is not yet implemented :( Add support for
    // displaying solutions created for supports other than disp_sup

    MixedProfile<gNumber> soln = 
        FitToSupport((const MixedProfile<gNumber> &)solns[cur_soln], *disp_sup);
    gNumber t_max;
    gArray<int> profile(nf.NumPlayers());

    // Figure out the index in the disp_sup, then map it onto the full support
    int pl;

    for (pl = 1; pl <= nf.NumPlayers(); pl++)
    {
        profile[pl] = 1;
        t_max = soln(pl, 1);

        for (int st1 = 1; st1 <= disp_sup->NumStrats(pl); st1++)
        {
            if (soln(pl, st1) > t_max)
            {
                profile[pl] = st1;
                t_max = soln(pl, st1);
            }
        }
    }

    UpdateProfile(profile);

    // Set the profile boxes to correct values if this is a pure equ
    spread->SetProfile(profile);

    // Hilight the cells w/ nonzero prob
    int st1;
    gNumber eps;
    gEpsilon(eps, spread->DrawSettings()->NumPrec()+1);

    for (st1 = 1; st1 <= rows; st1++)
    {
        for (int st2 = 1; st2 <= cols; st2++)
        {
            if (soln(pl1, st1) > eps && soln(pl2, st2) > eps)
                spread->HiLighted(st1, st2, 0, TRUE);
            else
                spread->HiLighted(st1, st2, 0, FALSE);
        }
    }


    if (spread->HaveProbs())
    {
        // Print out the probability in the next column/row
        int i;

        for (i = 1; i <= rows; i++)
            spread->SetCell(i, cols+1, ToText(soln(pl1, i)));

        for (i = 1; i <= cols; i++)
            spread->SetCell(rows+1, i, ToText(soln(pl2, i)));
    }

    if (spread->HaveVal())
    {
        // Print out the probability in the last column/row
        int i, j;

        for (i = 1; i <= rows; i++)
        {
            spread->SetCell(i, cols+spread->HaveProbs()+spread->HaveDom()+1, 
                            ToText(soln.Payoff(pl1, pl1, i)));
        }

        for (j = 1; j <= cols; j++)
        {
            spread->SetCell(rows+spread->HaveProbs()+spread->HaveDom()+1, j, 
                            ToText(soln.Payoff(pl2, pl2, j)));
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

    for (int i = 1; i <= nf.NumPlayers(); i++)
    {
        if (i != pl1 && i != pl2)
            cont_prob *= soln(i, profile[i]);
    }

    spread->SetCell(rows+1, cols+1, ToText(cont_prob));
}



void NfgShow::UpdateProfile(gArray<int> &profile)
{
    nf_iter.Set(profile);
    UpdateContingencyProb(profile);
    UpdateVals();
}


#define ENTRIES_PER_ROW 5

class NFChangePayoffs: public MyDialogBox
{
    const gArray<int> &profile;
    Nfg &nf;

    wxText **payoff_items;
    wxChoice *outcome_item;

    static void outcome_func(wxChoice &ob, wxEvent &)
    { ((NFChangePayoffs *)ob.GetClientData())->OnOutcome(); }

    void OnOutcome(void);

public:
    NFChangePayoffs(Nfg &nf, const gArray<int> &profile, wxWindow *parent);
    int OutcomeNum(void);
    gArray<gNumber> Payoffs(void);
};



NFChangePayoffs::NFChangePayoffs(Nfg &nf_, const gArray<int> &profile_, wxWindow *parent)
    : MyDialogBox(parent, "ChangePayoffs"), profile(profile_), nf(nf_)
{
    int i;
    Add(wxMakeFormMessage("This invalidates all solutions!"));
    Add(wxMakeFormNewLine());
    Add(wxMakeFormMessage("Change payoffs for profile:"));
    gText profile_str = "(";

    for (i = 1; i <= profile.Length(); i++)
        profile_str += ToText(profile[i])+((i == profile.Length()) ? ")" : ",");

    Add(wxMakeFormMessage(profile_str));
    Add(wxMakeFormNewLine());

    // Outcome item
    wxStringList *outcome_list = new wxStringList;
    char *outcome_name = new char[20];
    NFOutcome *outc = nf.GetOutcome(profile);

    for (i = 1; i <= nf.NumOutcomes(); i++)
    {
        NFOutcome *outc_tmp = nf.Outcomes()[i];
        gText outc_name;

        if (outc_tmp->GetName() != "")
            outc_name = outc_tmp->GetName();
        else
            outc_name = "Outcome "+ToText(i);

        outcome_list->Add(outc_name);

        if (outc_tmp == outc)
            strcpy(outcome_name, outc_name);
    }

    outcome_list->Add("New Outcome");

    if (outc == 0)
        strcpy(outcome_name, "New Outcome");

    wxFormItem *outcome_fitem = 
        Add(wxMakeFormString("Outcome", &outcome_name,
                             wxFORM_CHOICE, 
                             new wxList(wxMakeConstraintStrings(outcome_list), 0)));

    Add(wxMakeFormNewLine());

    // Payoff items
    char **new_payoffs = new char *[profile.Length()+1];
    wxFormItem **payoff_fitems = new wxFormItem *[profile.Length()+1];
    payoff_items = new wxText *[profile.Length()+1];

    for (i = 1; i <= nf.NumPlayers(); i++)
    {
        new_payoffs[i] = new char[40];
        payoff_fitems[i] = Add(wxMakeFormString("", &(new_payoffs[i]), wxFORM_TEXT, 0, 0, 0, 160));

        if (i % ENTRIES_PER_ROW == 0)
            Add(wxMakeFormNewLine());
    }

    AssociatePanel();

    for (i = 1; i <= nf.NumPlayers(); i++)
        payoff_items[i] = (wxText *)payoff_fitems[i]->GetPanelItem();

    outcome_item = (wxChoice *)outcome_fitem->GetPanelItem();
    outcome_item->Callback((wxFunction)outcome_func);
    outcome_item->SetClientData((char *)this);
    OnOutcome(); // update the outcome values
    Go1();

    for (i = 1; i <= profile.Length(); i++) 
        delete [] new_payoffs[i];

    delete [] new_payoffs;
}


void NFChangePayoffs::OnOutcome(void)
{
    int outc_num = outcome_item->GetSelection()+1;
    NFOutcome *outc = 0;

    if (outc_num <= nf.NumOutcomes())
        outc = nf.Outcomes()[outc_num];

    for (int i = 1; i <= nf.NumPlayers(); i++)
    {
        gNumber payoff = 0;

        if (outc)
            payoff = nf.Payoff(outc, i);

        payoff_items[i]->SetValue(ToText(payoff));
    }
    payoff_items[1]->SetFocus();
}


int NFChangePayoffs::OutcomeNum(void)
{
    return outcome_item->GetSelection()+1;
}


gArray<gNumber> NFChangePayoffs::Payoffs(void)
{
    gArray<gNumber> payoffs(nf.NumPlayers());

    for (int i = 1; i <= nf.NumPlayers(); i++)
        FromText(payoff_items[i]->GetValue(), payoffs[i]);

    return payoffs;
}


void NfgShow::ChangePayoffs(int st1, int st2, bool next)
{
    if (st1 > rows || st2 > cols)
        return;

    if (next) // facilitates quickly editing the nf -> automatically moves to next cell
    {
        if (st2 < cols)
        {
            st2++;
        }
        else
        {
            if (st1 < rows)
            {
                st1++;
                st2 = 1;
            }
            else
            {
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

    NFChangePayoffs *payoffs_dialog = new NFChangePayoffs(nf, profile, spread);

    if (payoffs_dialog->Completed() == wxOK)
    {
        NFOutcome *outc;
        int outc_num = payoffs_dialog->OutcomeNum();
        gArray<gNumber> payoffs(payoffs_dialog->Payoffs());

        if (outc_num > nf.NumOutcomes())
            outc = nf.NewOutcome();
        else
            outc = nf.Outcomes()[outc_num];

        for (int i = 1; i <= nf.NumPlayers(); i++)
            nf.SetPayoff(outc, i, payoffs[i]);

        nf.SetOutcome(profile, outc);
        UpdateVals();
        RemoveSolutions();
        InterfaceDied();
    }

    delete payoffs_dialog;
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
    gText filename = Filename();
    gText s = wxFileSelector("Save data file", wxPathOnly(filename), 
                             wxFileNameFromPath(filename), ".nfg", "*.nfg", 
                             wxSAVE|wxOVERWRITE_PROMPT);

    if (s != "")
    {
        // Change description if saving under a different filename
        if (filename != "untitled.nfg" && s != filename)
            SetLabels(0);

        gFileOutput out(s);

        // Compress the nfg to the current support
        Nfg *N = CompressNfg(nf, *cur_sup);
        N->WriteNfgFile(out);
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


#define SUPPORT_CLOSE       1 // in elimdomd.h

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
// Solve

void NfgShow::Solve(void)
{
    NfgSolveSettings NSD(nf);

    // If we have more than 1 support, we must have created it explicitly.
    // In that case use the currently set support.  Otherwise, only the
    // default support exists and we should use a support dictated by
    // dominance defaults.
    NFSupport *sup = (supports.Length() > 1) ? cur_sup : 0;

    //bool solved = false;

    if (!sup)
        sup = MakeSolnSupport();

    int old_max_soln = solns.Length();  // used for extensive update

    switch (NSD.GetAlgorithm())
    {
    case NFG_ENUMPURE_SOLUTION: 
        solns += NfgEnumPureG(nf, *cur_sup, this).Solve();
        break;

    case NFG_LCP_SOLUTION:      
        solns += NfgLemkeG(nf, *cur_sup, this).Solve();
        break;

    case NFG_LIAP_SOLUTION:      
        solns += NfgLiapG(nf, *cur_sup, this).Solve();
        break;

    case NFG_GOBITALL_SOLUTION: 
        solns += NfgGobitAllG(nf, *cur_sup, this).Solve();
        break;

    case NFG_GOBIT_SOLUTION:     
        solns += NfgGobitG(nf, *cur_sup, this).Solve();
        break;

    case NFG_SIMPDIV_SOLUTION:   
        solns += NfgSimpdivG(nf, *cur_sup, this).Solve();
        break;

    case NFG_ENUMMIXED_SOLUTION:
        solns += NfgEnumG(nf, *cur_sup, this).Solve();
        break;

    case NFG_LP_SOLUTION:       
        solns += NfgZSumG(nf, *cur_sup, this).Solve();
        break;

    default:
        wxError("Internal Error!\nUnknown NFG algorithm\nContact the author");
        break;
    }

    if (old_max_soln != solns.Length())
    {
        if (NSD.GetExtensive()) // Now, transfer the NEW solutions to extensive form if requested
        {
            for (int i = old_max_soln+1; i <= solns.Length(); i++) 
                SolutionToExtensive(solns[i]);
        }

        if (!spread->HaveProbs())
        {
            spread->MakeProbDisp();
            spread->Redraw();
        }

        ChangeSolution(solns.VisibleLength());
        spread->EnableInspect(TRUE);

        if (NSD.AutoInspect()) InspectSolutions(CREATE_DIALOG);
    }
}


#define     SOLVE_SETUP_CUSTOM      0
#define     SOLVE_SETUP_STANDARD    1

void NfgShow::SolveSetup(int what)
{
    if (what == SOLVE_SETUP_CUSTOM)
    {
        NfgSolveParamsDialog NSD(nf, InterfaceOk(), (wxWindow *)spread);
        //   bool from_efg = 0;

        if (NSD.GetResult() == SD_PARAMS)
        {
            switch (NSD.GetAlgorithm())
            {
            case NFG_ENUMPURE_SOLUTION: 
                NfgEnumPureG(nf, *cur_sup, this).SolveSetup();
                break;

            case NFG_LCP_SOLUTION:       
                NfgLemkeG(nf, *cur_sup, this).SolveSetup();
                break;

            case NFG_LIAP_SOLUTION:      
                NfgLiapG(nf, *cur_sup, this).SolveSetup();
                break;

            case NFG_GOBITALL_SOLUTION: 
                NfgGobitAllG(nf, *cur_sup, this).SolveSetup();
                break;

            case NFG_GOBIT_SOLUTION:     
                NfgGobitG(nf, *cur_sup, this).SolveSetup();
                break;

            case NFG_SIMPDIV_SOLUTION:   
                NfgSimpdivG(nf, *cur_sup, this).SolveSetup();
                break;

            case NFG_ENUMMIXED_SOLUTION:
                NfgEnumG(nf, *cur_sup, this).SolveSetup();
                break;

            case NFG_LP_SOLUTION:        
                NfgLiapG(nf, *cur_sup, this).SolveSetup();
                break;

            default:                    
                assert(0 && "Unknown NFG algorithm");
                break;
            }
        }

        if (NSD.GetResult() != SD_CANCEL)
            spread->GetMenuBar()->Check(NFG_SOLVE_STANDARD_MENU, FALSE);
    }
    else    // SOLVE_SETUP_STANDARD
    {
        NfgSolveStandardDialog(nf, (wxWindow *)spread);
        spread->GetMenuBar()->Check(NFG_SOLVE_STANDARD_MENU, TRUE); // using standard now
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

#ifndef NFG_ONLY
#include "efg.h"
void MixedToBehav(const Nfg &N, const MixedProfile<gNumber> &mp, 
                  const Efg &E, BehavProfile<gNumber> &bp);
#endif

void NfgShow::SolutionToExtensive(const MixedSolution &mp, bool set)
{
#ifndef NFG_ONLY
    assert(InterfaceOk());  // we better have someone to send solutions to

    // assert(mp != Solution());
    EFSupport S(*InterfaceObjectEfg());
    BehavProfile<gNumber> bp(S);
    MixedToBehav(nf, mp, *InterfaceObjectEfg(), bp);
    SolutionToEfg(bp, set);
#endif
}


template class SolutionList<MixedSolution>;

//******************** NORMAL FORM GUI ******************************
#include "nfggui.h"
NfgGUI::NfgGUI(Nfg *nf, const gText infile_name, EfgNfgInterface *inter, wxFrame *parent)
{
    // an already created normal form has been passed

    if (nf == 0) // must create a new normal form, from scratch or from file
    {
        gArray<int> dimensionality;
        gArray<gText> names;

        if (infile_name == gText()) // from scratch
        {
            if (GetNFParams(dimensionality, names, parent))
            {
                nf = new Nfg(dimensionality);

                for (int i = 1; i <= names.Length(); i++) 
                    nf->Players()[i]->SetName(names[i]);
            }
        }
        else  // from data file
        {
            gFileInput infile(infile_name);
            bool valid = infile.IsValid();

            if (!valid)
            {
                wxMessageBox("ReadFailed:FileInvalid::Check the file");
                return;
            }

            ReadNfgFile((gInput &) infile, nf);

            if (!nf)
                wxMessageBox("ReadFailed:FormatInvalid::Check the file");
        }
    }

    NfgShow *nf_show = 0;

    if (nf)
    {
        if (nf->NumPlayers() > 1)
        {
            nf_show= new NfgShow(*nf, inter, parent);
        }
        else
        {
            delete nf;
            MyMessageBox("Single player Normal Form games are not supported in the GUI", 
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





