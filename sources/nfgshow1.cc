//
// FILE: nfgshow1.cc -- remainder of the normal form GUI
//
// $Id$
//

#include "nfgshow.h"
#include "nfplayer.h"
#include "nfgoutcd.h"

#include "gambit.h"

#include "nfgconst.h"
#include "nfgsolvd.h"

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

    if (opt_list->Nth(outcome_disp) == NULL)
    {
        guiExceptionDialog("Invalid value for NFOutcome-Display; check \"gambit.ini\" file.\n",
                           main_gambit_frame);
    }
    else
    {
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
  edit_menu->Append(NFG_EDIT_GAME,      "&Game",      "Edit the entire game");
  edit_menu->Append(NFG_EDIT_STRATS,    "&Strats",    "Edit player strategies");
  edit_menu->Append(NFG_EDIT_PLAYERS,   "&Players",   "Edit players");
  edit_menu->Append(NFG_EDIT_OUTCOMES,  "&Outcomes",  "Set/Edit outcomes");

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

    case NFG_SOLVE_DOMINANCE_MENU: 
      parent->DominanceSetup();
      break;

    case NFG_SOLVE_GAMEINFO_MENU: 
      parent->ShowGameInfo();
      break;

    case NFG_EDIT_GAME: 
      parent->SetLabels(0);
      break;

    case NFG_EDIT_STRATS: 
      parent->SetLabels(1);
      break;

    case NFG_EDIT_PLAYERS: 
      parent->SetLabels(2);
      break;

    case NFG_EDIT_OUTCOMES: 
      parent->ChangeOutcomes(CREATE_DIALOG);
      break;

    case NFG_FILE_SAVE:
      parent->Save();
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
#include "bitmaps/payoff.xpm"
#include "bitmaps/solve.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/options.xpm"
#include "bitmaps/inspect.xpm"
    wxBitmap *ToolbarSaveBitmap = new wxBitmap(save_xpm);
    wxBitmap *ToolbarPrintBitmap = new wxBitmap(print_xpm);
    wxBitmap *ToolbarSolveBitmap = new wxBitmap(solve_xpm);
    wxBitmap *ToolbarPayoffBitmap = new wxBitmap(payoff_xpm);
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
    AddTool(NFG_EDIT_OUTCOMES, ToolbarPayoffBitmap);
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

