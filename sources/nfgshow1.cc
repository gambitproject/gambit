//
// FILE: nfgshow1.cc -- remainder of the normal form GUI
// contains classes NfgShow and NormalSpread
//
// $Id$
//

#include "nfgshow.h"
#include "nfplayer.h"
#include "nfgoutcd.h"

//**************************************************************************
//                         NORMAL FORM GAME SHOW
//**************************************************************************

#include "nfgconst.h"

// SetPlayers
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


//**************************** OUTCOMES STUFF *********************************
#define UPDATE1_DIALOG  4
#define PARAMS_ADD_VAR  5

void NfgShow::SetOutcome(int out, int x, int y)
{
    if (out > nf.NumOutcomes())
    {
        MyMessageBox("This outcome is not defined yet", 
                     "Outcome", NFG_OUTCOME_HELP, spread);
    }
    else
    {
        gArray<int> cur_profile(spread->GetProfile());

        if (x != -1)    // dropped an outcome at the coordinates (x,y)
        {
            spread->GetSheet()->ScreenToClient(&x, &y);  // x,y are absolute screen coordinates
            int row, col;

            if (spread->XYtoRowCol(x, y, &row, &col))
            {
                cur_profile[pl1] = row;
                cur_profile[pl2] = col;
                spread->SetProfile(cur_profile);
            }
            else
            {
                return;
            }
        }

        if (out > 0)
            nf.SetOutcome(cur_profile, nf.Outcomes()[out]);

        if (out == 0)
            nf.SetOutcome(cur_profile, 0);

        if (out == -1) ; // just update all outcomes

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

static Bool LongStringConstraint(int type, char *value, char *label, char *msg_buffer)
{
    if (value && (strlen(value) >= 255) && (type == wxFORM_STRING))
    {
        sprintf(msg_buffer, "Value for %s should be %d characters or less\n",
                label, 255);
        return FALSE;
    }
    else 
    {
        return TRUE;
    }
}


// Call Spread->SetLabels afterwards to update the display
void NfgShow::SetLabels(int what)
{
    int num_players = nf.NumPlayers();

    if (what == 0)  // label game
    {
        char *label = new char[256];

        strcpy(label, nf.GetTitle());
        MyDialogBox *nfg_label_dialog = 
            new MyDialogBox(spread, "Label Game", NFG_LABEL_HELP);
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
                    disp_sup->Strategies(i)[j]->SetName() = labels->GetCell(i, j);
        }

        delete labels;
    }

    if (what == 2) // label players
    {
        MyDialogBox *labels = new MyDialogBox(spread, "Label Players", NFG_LABEL_HELP);
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
    strcpy(opt_str, (char *)opt_list->Nth(outcome_disp)->Data());
    options_dialog->Add(wxMakeFormString("Display as", &opt_str, wxFORM_RADIOBOX,
                                         new wxList(wxMakeConstraintStrings(opt_list), 0)));
    options_dialog->Go();

    if (options_dialog->Completed() == wxOK)
    {
        char *defaults_file = "gambit.ini";
        outcome_disp = wxListFindString(opt_list, opt_str);
        wxWriteResource("Gambit", "NFOutcome-Display", outcome_disp, defaults_file);
    }

    delete options_dialog;
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

    // Now check/uncheck all of the checkable menu items
    Bool use_standard;
    char *defaults_file = "gambit.ini";
    wxGetResource(SOLN_SECT, "Nfg-Use-Standard", &use_standard, defaults_file);
    GetMenuBar()->Check(NFG_SOLVE_STANDARD_MENU, use_standard);
    Redraw();
    Show(TRUE);
}


// Overriding the MakeMenuBar... Note that menus is not used.
wxMenuBar *NormalSpread::MakeMenuBar(long )
{
    wxMenu *file_menu = new wxMenu;
    file_menu->Append(NFG_FILE_SAVE, "&Save",    "Save the game");
    file_menu->Append(OUTPUT_MENU,   "Out&put",  "Output to any device");
    file_menu->Append(CLOSE_MENU,    "&Close",   "Exit");

    wxMenu *label_menu = new wxMenu;
    label_menu->Append(NFG_LABEL_GAME,     "&Game",    "Label the entire game");
    label_menu->Append(NFG_LABEL_STRATS,   "&Strats",  "Label player strategies");
    label_menu->Append(NFG_LABEL_PLAYERS,  "&Players", "Label players");

    wxMenu *supports_menu = new wxMenu;
    supports_menu->Append(NFG_SOLVE_COMPRESS_MENU, "&ElimDom",  "Dominated strategies");
    supports_menu->Append(NFG_SOLVE_SUPPORTS_MENU, "&Supports", "Change support");

    wxMenu *solve_menu = new wxMenu;
    solve_menu->Append(NFG_SOLVE_OUTCOMES_MENU,  "&Outcomes",    "Set/Edit outcomes");
    solve_menu->Append(NFG_SOLVE_PARAMS_MENU,    "&Params",      "Set/Edit parameters");
    solve_menu->Append(NFG_SOLVE_SOLVE_MENU,     "&Solve",       "Solution modules");
    solve_menu->Append(NFG_SOLVE_STANDARD_MENU,  "S&tandard...", "Standard solutions", TRUE);

    wxMenu *solve_settings_menu = new wxMenu;
    solve_settings_menu->Append(NFG_SOLVE_ALGORITHM_MENU, "&Algorithm", 
                                "Choose current algorithm");
    solve_settings_menu->Append(NFG_SOLVE_DOMINANCE_MENU, "&Dominance", "ElimDom options");
    solve_menu->Append(NFG_SOLVE_SETTINGS_MENU,  "&Custom", solve_settings_menu, 
                       "Control default options");
    solve_menu->Append(NFG_SOLVE_GAMEINFO_MENU,  "Game&Info", "Display some game info");

    wxMenu *inspect_menu = new wxMenu;
    inspect_menu->Append(NFG_SOLVE_INSPECT_MENU,   "&Solutions", "Inspect existing solutions");
    inspect_menu->Append(NFG_SOLVE_FEATURES_MENU,  "In&fo",      "Advanced solution features");

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
    tmp_menubar->Append(label_menu,    "&Label");
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
// OnMenuCommand

#define     SOLVE_SETUP_CUSTOM      0
#define     SOLVE_SETUP_STANDARD    1

void NormalSpread::OnMenuCommand(int id)
{
    switch (id)
    {
    case NFG_SOLVE_OUTCOMES_MENU: 
        parent->ChangeOutcomes(CREATE_DIALOG);
        break;

    case NFG_PREFS_OUTCOMES_MENU: 
        parent->OutcomeOptions();
        break;

    case NFG_SOLVE_SOLVE_MENU: 
        parent->Solve();
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

    case NFG_SOLVE_ALGORITHM_MENU: 
        parent->SolveSetup(SOLVE_SETUP_CUSTOM);
        break;

    case NFG_SOLVE_STANDARD_MENU: 
        parent->SolveSetup(SOLVE_SETUP_STANDARD);
        break;

    case NFG_SOLVE_DOMINANCE_MENU: 
        parent->DominanceSetup();
        break;

    case NFG_SOLVE_GAMEINFO_MENU: 
        parent->ShowGameInfo();
        break;

    case NFG_LABEL_GAME: 
        parent->SetLabels(0);
        break;

    case NFG_LABEL_STRATS: 
        parent->SetLabels(1);
        break;

    case NFG_LABEL_PLAYERS: 
        parent->SetLabels(2);
        break;

    case NFG_FILE_SAVE:
        parent->Save();
        break;

    case  NFG_ACCL_PAYOFF: 
        parent->ChangePayoffs(CurRow(), CurCol());
        break;

    case NFG_ACCL_NEXT_PAYOFF: 
        parent->ChangePayoffs(CurRow(), CurCol(), true);
        break;

    default: 
        SpreadSheet3D::OnMenuCommand(id);
        break;
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

// OnSolve
//void NormalSpread::OnSolve(void)
//{
//  parent->OnSolve();
//}


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
#include "bitmaps/payoff.xpm"
#include "bitmaps/solve.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/options.xpm"
#include "bitmaps/inspect.xpm"
#include "bitmaps/params.xpm"
    wxBitmap *ToolbarSaveBitmap = new wxBitmap(save_xpm);
    wxBitmap *ToolbarPrintBitmap = new wxBitmap(print_xpm);
    wxBitmap *ToolbarSolveBitmap = new wxBitmap(solve_xpm);
    wxBitmap *ToolbarPayoffBitmap = new wxBitmap(payoff_xpm);
    wxBitmap *ToolbarHelpBitmap = new wxBitmap(help_xpm);
    wxBitmap *ToolbarOptionsBitmap = new wxBitmap(options_xpm);
    wxBitmap *ToolbarInspectBitmap = new wxBitmap(inspect_xpm);
    wxBitmap *ToolbarParamBitmap = new wxBitmap(params_xpm);

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
    AddTool(NFG_SOLVE_OUTCOMES_MENU, ToolbarPayoffBitmap);
    AddTool(NFG_SOLVE_PARAMS_MENU, ToolbarParamBitmap);
    AddSeparator();
    AddTool(NFG_SOLVE_SOLVE_MENU, ToolbarSolveBitmap);
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

