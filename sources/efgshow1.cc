//
// FILE: efgshow1.cc -- remainder of the extensive form gui stuff
//
//  $Id$
//

#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "efg.h"
#include "efgconst.h"
#include "gambit.h"
#include "treewin.h"
#include "efgshow.h"
#include "efgsoln.h"

#include "efsuptd.h"

#ifdef wx_msw
#include "wx_bbar.h"
#else
#include "wx_tbar.h"
#endif

class EfgShowToolBar:
#ifdef wx_msw
    public wxButtonBar
#else
    public wxToolBar
#endif
{
private:
    wxFrame *parent;

public:
    EfgShowToolBar(wxFrame *frame);
    Bool OnLeftClick(int toolIndex, Bool toggled);
    void OnMouseEnter(int toolIndex);
};


//*********************************************************************
//                  EXTENSIVE FORM GAME SHOW
//*********************************************************************

void EfgShow::MakeMenus(void)
{
    //Note: to insure greatest possible portability we will avoid using
    //      resource files which are inherently non-portable.
    wxMenu *file_menu = new wxMenu;
    file_menu->Append(FILE_SAVE,   "&Save",    "Save the game");
    file_menu->Append(FILE_OUTPUT, "&Output",  "Print or copy the game");
    file_menu->Append(FILE_LOG,    "&Logging", "Start/stop logging gcl", TRUE);
    file_menu->Append(FILE_CLOSE,  "&Close",   "Close this window");

    log_item = file_menu->FindItem("Logging");
    wxMenu *build_menu = new wxMenu;
    wxMenu *node_menu  = new wxMenu;
    node_menu->Append(NODE_ADD,       "&Add",       "Add a node");
    node_menu->Append(NODE_DELETE,    "&Delete",    "Remove cursor node");
    node_menu->Append(NODE_INSERT,    "&Insert",    "Insert node at cursor");
    node_menu->Append(NODE_LABEL,     "&Label",     "Label cursor node");
    node_menu->AppendSeparator();
    node_menu->Append(NODE_SET_MARK,  "Set &Mark",  "Mark cursor node");
    node_menu->Append(NODE_GOTO_MARK, "Go&to Mark", "Goto marked node");

    wxMenu *action_menu = new wxMenu;
    action_menu->Append(ACTION_DELETE, "&Delete", "Delete an action from cursor iset");
    action_menu->Append(ACTION_INSERT, "&Insert", "Delete an action to cursor iset");
    action_menu->Append(ACTION_LABEL,  "&Label");
    action_menu->Append(ACTION_PROBS,  "&Probs",  "Set the chance player probs");

    wxMenu *infoset_menu = new wxMenu;
    infoset_menu->Append(INFOSET_MERGE,  "&Merge",  "Merge cursor iset w/ marked");
    infoset_menu->Append(INFOSET_BREAK,  "&Break",  "Make cursor a new iset");
    infoset_menu->Append(INFOSET_SPLIT,  "&Split",  "Split iset at cursor");
    infoset_menu->Append(INFOSET_JOIN,   "&Join",   "Join cursor to marked iset");
    infoset_menu->Append(INFOSET_LABEL,  "&Label",  "Label cursor iset & actions");
    infoset_menu->Append(INFOSET_SWITCH_PLAYER, "&Player", "Change player of cursor iset");
    infoset_menu->Append(INFOSET_REVEAL, "&Reveal", "Reveal infoset to players");

    wxMenu *tree_menu = new wxMenu;
    tree_menu->Append(TREE_COPY,      "&Copy",     "Copy tree from marked node");
    tree_menu->Append(TREE_MOVE,      "&Move",     "Move tree from marked node");
    tree_menu->Append(TREE_DELETE,    "&Delete",   "Delete recursively from cursor");
    tree_menu->Append(TREE_LABEL,     "&Label",    "Set the game label");
    tree_menu->Append(TREE_PLAYERS,   "&Players",  "Edit/View players");
    tree_menu->Append(TREE_INFOSETS,  "&Infosets", "Edit/View infosets");
    build_menu->Append(BUILD_NODE,    "&Node",     node_menu,    "Edit the node");
    build_menu->Append(BUILD_ACTIONS, "&Actions",  action_menu,  "Edit actions");
    build_menu->Append(BUILD_INFOSET, "&Infoset",  infoset_menu, "Edit infosets");
    build_menu->Append(TREE_OUTCOMES, "&Outcomes", "Edit/View the payoffs");
    build_menu->Append(BUILD_TREE,    "&Tree",     tree_menu,    "Edit the tree");
    build_menu->Append(BUILD_PARAMS,  "&Params",   "Set/Edit parameters");

    wxMenu *subgame_menu = new wxMenu;
    subgame_menu->Append(SUBGAME_SOLVE,        "Mark &All",       "Scan tree for subgames");
    subgame_menu->Append(SUBGAME_SET,          "&Mark",           "Set node subgame root");
    subgame_menu->Append(SUBGAME_CLEARALL,     "Unmark &All",     "Clear all subgame info");
    subgame_menu->Append(SUBGAME_CLEARONE,     "&Unmark",         "Unmark node subgame");
    subgame_menu->Append(SUBGAME_COLLAPSEONE,  "&Collapse Level", "Collapse node subgame");
    subgame_menu->Append(SUBGAME_COLLAPSEALL,  "&Collapse All",   "Collapse all subgames");
    subgame_menu->Append(SUBGAME_EXPANDONE,    "&Expand Level",   "Expand node subgame");
    subgame_menu->Append(SUBGAME_EXPANDBRANCH, "&Expand Branch",  "Expand entire branch");
    subgame_menu->Append(SUBGAME_EXPANDALL,    "&Expand All",     "Expand all subgames");

    wxMenu *supports_menu = new wxMenu;
    supports_menu->Append(SUPPORTS_ELIMDOM,  "&ElimDom",  "Dominated strategies");
    supports_menu->Append(SUPPORTS_SUPPORTS, "S&upports", "Create/view EF supports");

    wxMenu *solve_menu = new wxMenu;
    solve_menu->Append(SOLVE_SOLVE,        "&Solve",       "Start a solution algorithm");
    solve_menu->Append(SOLVE_SOLVE_NORMAL, "Make &NF",     "Create a NF");
    solve_menu->Append(SOLVE_STANDARD,     "S&tandard...", "Standard solutions", TRUE);

    wxMenu *solve_settings_menu = new wxMenu;
    solve_settings_menu->Append(SOLVE_ALGORITHM, "&Algorithm", "Set/Adjust current algorithm");
    solve_settings_menu->Append(SOLVE_DOMINANCE, "&Dominance", "Set ElimDom options");
    solve_settings_menu->Append(SOLVE_SUBGAMES,  "&Subgames",  "Set subgames options");
    solve_menu->Append(SOLVE_SETTINGS,           "&Custom",    solve_settings_menu, 
                       "Control default options");
    solve_menu->Append(SOLVE_GAMEINFO,           "Game&Info",  "Display some game info");

    wxMenu *inspect_menu = new wxMenu;
    inspect_menu->Append(INSPECT_SOLUTIONS,  "&Solutions", "Inspect existing solutions");
    inspect_menu->Append(INSPECT_FEATURES,   "In&fo",      "Advanced solution features");

    wxMenu *prefs_menu = new wxMenu;
    prefs_menu->Append(DISPLAY_SET_ZOOM,    "&Zoom",        "Specify zoom level");
    prefs_menu->Append(DISPLAY_ZOOM_WIN,    "Zoom &Window", "Open zoom-in window", TRUE);
    prefs_menu->Append(DISPLAY_SET_OPTIONS, "&Display",     "Set display options");
    prefs_menu->Append(DISPLAY_LEGENDS,     "&Legend",      "Set legends");
    prefs_menu->Append(DISPLAY_COLORS,      "&Colors",      "Set player colors");
    prefs_menu->Append(DISPLAY_ACCELS,      "&Accels",      "Edit accelerator keys");
    prefs_menu->AppendSeparator();
    prefs_menu->Append(DISPLAY_SAVE_DEFAULT, "Save Default");
    prefs_menu->Append(DISPLAY_LOAD_DEFAULT, "Load Default");
    prefs_menu->Append(DISPLAY_SAVE_CUSTOM,  "Save Custom");
    prefs_menu->Append(DISPLAY_LOAD_CUSTOM,  "Load Custom");
    zoom_win_item = prefs_menu->FindItem("Zoom &Window");

    wxMenu *help_menu = new wxMenu;
    help_menu->Append(GAMBIT_HELP_CONTENTS, "&Contents", "Table of contents");
    help_menu->Append(GAMBIT_HELP_ABOUT,    "&About",    "About this program");

    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu,     "&File");
    menu_bar->Append(build_menu,    "&Build");
    menu_bar->Append(subgame_menu,  "Sub&games");
    menu_bar->Append(supports_menu, "S&upports");
    menu_bar->Append(solve_menu,    "&Solve");
    menu_bar->Append(inspect_menu,  "&Inspect");
    menu_bar->Append(prefs_menu,    "&Prefs");
    menu_bar->Append(help_menu,     "&Help");

    // Set the menu bar
    SetMenuBar(menu_bar);

    // Now check/uncheck all of the checkable menu items
    Bool use_standard;
    char *defaults_file =    "gambit.ini";
    wxGetResource(SOLN_SECT, "Efg-Use-Standard", &use_standard, defaults_file);
    menu_bar->Check(SOLVE_STANDARD, use_standard);
}

Bool EfgShow::OnClose(void)
{
    ChangeSupport(DESTROY_DIALOG);
    ChangeOutcomes(DESTROY_DIALOG);
    InspectSolutions(DESTROY_DIALOG);
    Show(FALSE);
    return TRUE;
}

//---------------------------------------------------------------------
//             EXTENSIVE SHOW: EVENT-HANDLING HOOK MEMBERS
//---------------------------------------------------------------------
void EfgShow::OnSetFocus(void)
{
    tw->SetFocus();
}

void EfgShow::OnSize(int , int )
{
    if (!tw) 
        return;

    int toolbar_height = 40;
    int frame_w, frame_h;
    GetClientSize(&frame_w, &frame_h);
    toolbar->SetSize(0, 0, frame_w, toolbar_height);
    tw->SetSize(0, toolbar_height, frame_w, frame_h-toolbar_height);
    tw->SetFocus();
}


#define     SOLVE_SETUP_CUSTOM      0
#define     SOLVE_SETUP_STANDARD    1


void EfgShow::OnMenuCommand(int id)
{
    switch (id)
    {
        // Node menus
    case NODE_ADD:
        tw->node_add();
        break;

    case NODE_DELETE:
        tw->node_delete();
        break;

    case NODE_INSERT:
        tw->node_add();
        break;

    case NODE_LABEL:
        tw->node_label();
        break;

    case NODE_SET_MARK:
        tw->node_set_mark();
        break;

    case NODE_GOTO_MARK:
        tw->node_goto_mark();
        break;

        // Branch menu
    case ACTION_DELETE:
        tw->action_delete();
        break;

    case ACTION_INSERT:
        tw->action_insert();
        break;

    case ACTION_LABEL:
        tw->action_label();
        break;

    case ACTION_PROBS:
        tw->action_probs();
        break;

        // Tree menu
    case TREE_DELETE:
        tw->tree_delete();
        break;

    case TREE_COPY:
        tw->tree_copy();
        break;

    case TREE_MOVE:
        tw->tree_move();
        break;

    case TREE_LABEL:
        tw->tree_label();
        SetFileName();
        break;

    case TREE_OUTCOMES: 
        ChangeOutcomes(CREATE_DIALOG);
        break;

    case TREE_PLAYERS:
        tw->tree_players();
        break;

    case TREE_INFOSETS:
        tw->tree_infosets();
        break;

        // Infoset menu
    case INFOSET_MERGE:
        tw->infoset_merge();
        break;

    case INFOSET_BREAK:
        tw->infoset_break();
        break;

    case INFOSET_SPLIT:
        tw->infoset_split();
        break;

    case INFOSET_JOIN:
        tw->infoset_join();
        break;

    case INFOSET_LABEL:
        tw->infoset_label();
        break;

    case INFOSET_SWITCH_PLAYER:
        tw->infoset_switch_player();
        break;

    case INFOSET_REVEAL:
        tw->infoset_reveal();
        break;

        // File menu
    case FILE_OUTPUT:
        tw->output();
        break;

    case FILE_SAVE:
        tw->file_save();
        break;

    case FILE_CLOSE:
        Close();
        break;

    case FILE_LOG:
        GetMenuBar()->Check(log_item, tw->logging());
        break;

        // Inspect menu
    case INSPECT_FEATURES:
        SetOptions();
        break;

        // Supports menu
    case SUPPORTS_SUPPORTS: 
        ChangeSupport(CREATE_DIALOG);
        break;

    case SUPPORTS_ELIMDOM: 
        SolveElimDom();
        break;

        // Subgames menu
    case SUBGAME_SOLVE:
        tw->subgame_solve();
        break;

    case SUBGAME_CLEARALL:
        tw->subgame_clear_all();
        break;

    case SUBGAME_CLEARONE:
        tw->subgame_clear_one();
        break;

    case SUBGAME_COLLAPSEALL:
        tw->subgame_collapse_all();
        break;

    case SUBGAME_COLLAPSEONE:
        tw->subgame_collapse_one();
        break;

    case SUBGAME_EXPANDALL:
        tw->subgame_expand_all();
        break;

    case SUBGAME_EXPANDBRANCH:
        tw->subgame_expand_branch();
        break;

    case SUBGAME_EXPANDONE:
        tw->subgame_expand_one();
        break;

    case SUBGAME_SET:
        tw->subgame_set();
        break;

        // Solve menu
    case INSPECT_SOLUTIONS: 
        InspectSolutions(CREATE_DIALOG);
        break;

    case SOLVE_SOLVE: 
        Solve();
        break;

    case SOLVE_SOLVE_NORMAL: 
        SolveNormal();
        break;

    case SOLVE_ALGORITHM: 
        SolveSetup(SOLVE_SETUP_CUSTOM);
        break;

    case SOLVE_STANDARD:
        SolveSetup(SOLVE_SETUP_STANDARD);
        break;

    case SOLVE_DOMINANCE:
    {
        DominanceSettingsDialog EDPD(this);
        break;
    }

    case SOLVE_SUBGAMES: 
        SubgamesSetup();
        break;

    case SOLVE_GAMEINFO: 
        ShowGameInfo();
        break;

        // Display menu
#define ZOOM_DELTA  .2

    case DISPLAY_SET_ZOOM:
        tw->display_set_zoom();
        break;

    case DISPLAY_ZOOM_WIN:
        GetMenuBar()->Check(zoom_win_item, tw->display_zoom_win());
        break;

    case DISPLAY_INC_ZOOM:
        tw->display_set_zoom(tw->display_get_zoom()+ZOOM_DELTA);
        break;

    case DISPLAY_DEC_ZOOM:
        tw->display_set_zoom(tw->display_get_zoom()-ZOOM_DELTA);
        break;

    case DISPLAY_LEGENDS:
        tw->display_legends();
        break;

    case DISPLAY_SET_OPTIONS:
        tw->display_options();
        break;

    case DISPLAY_COLORS:
        tw->display_colors();
        break;

    case DISPLAY_SAVE_DEFAULT:
        tw->display_save_options();
        break;

    case DISPLAY_LOAD_DEFAULT:
        tw->display_load_options();
        break;

    case DISPLAY_SAVE_CUSTOM:
        tw->display_save_options(FALSE);
        break;

    case DISPLAY_LOAD_CUSTOM:
        tw->display_load_options(FALSE);
        break;

    case DISPLAY_REDRAW: 
        /* redraws automatically after switch */ 
        break;

    case DISPLAY_ACCELS:
        EditAccelerators(accelerators, MakeEventNames());
        WriteAccelerators(accelerators, "EfgAccelerators");
        break;

        // Help menu
    case GAMBIT_HELP_ABOUT:
        wxHelpAbout();
        break;

    case GAMBIT_HELP_CONTENTS:
        wxHelpContents(EFG_GUI_HELP);
        break;

    default:
        parent->OnMenuCommand(id);
        break;
    }

    // Most menu selections modify the display somehow, so redraw w/ exceptions
    if (id != FILE_QUIT      &&  id != FILE_CLOSE         && 
        id != TREE_OUTCOMES  &&  id != SUPPORTS_SUPPORTS  && 
        id != BUILD_PARAMS)
    {
        tw->OnPaint();
        tw->SetFocus();
    }
}


#include "efgaccl.h"
gArray<AccelEvent> EfgShow::MakeEventNames(void)
{
    gArray<AccelEvent> events(NUM_EFG_EVENTS);

    for (int i = 0; i < NUM_EFG_EVENTS; i++) 
        events[i+1] = efg_events[i];

    return events;
}


// Check Accelerators
int EfgShow::CheckAccelerators(wxKeyEvent &ev)
{
    int id = ::CheckAccelerators(accelerators, ev);

    if (id) 
        OnMenuCommand(id);

    return id;
}


void EfgShow::SetOptions(void)
{
    struct es_features old_features = features;
    MyDialogBox *options_dialog = new MyDialogBox(this, "Solution features");
    options_dialog->Add(wxMakeFormBool("Hilight infosets", &features.iset_hilight));
    options_dialog->Add(wxMakeFormNewLine());
    options_dialog->Add(wxMakeFormBool("Inspect cursor", &features.node_inspect));
    options_dialog->Go();

    if (options_dialog->Completed() == wxOK)
    {
        if (old_features.iset_hilight && !features.iset_hilight)
        {
            HilightInfoset(0, 0, 1);
            HilightInfoset(0, 0, 2);
        }

        if (old_features.node_inspect && !features.node_inspect)
            NodeInspect(false);

        if (!old_features.node_inspect && features.node_inspect)
            NodeInspect(true);
    }

    delete options_dialog;
}



// if who == 2, hilight in the tree display
// if who == 1, hilight in the solution window display
void EfgShow::HilightInfoset(int pl, int iset, int who)
{
    if (!features.iset_hilight) 
        return;

    if (who == 1)
        soln_show->HilightInfoset(pl, iset);

    if (who == 2) tw->HilightInfoset(pl, iset);
}


//************************** EF SUPPORT FUNCTIONS *********************

#define SUPPORT_CHANGE   2

// Make Support
#define ENTRIES_PER_ROW  6

EFSupport *EfgShow::MakeSupport(void)
{
    MyDialogBox *support = new MyDialogBox(this, "Create Support", EFG_MAKE_SUPPORT_HELP);
    support->SetLabelPosition(wxVERTICAL);
    wxListBox **players = new wxListBox*[ef.NumPlayers()+1];

    for (int i = 1; i <= ef.NumPlayers(); i++)
    {
        EFPlayer *p = ef.Players()[i];
        players[i] = new wxListBox(support, 0, ef.Players()[i]->GetName(), 
                                   TRUE, -1, -1, 80, 100);
        int j;

        for (j = 1; j <= p->NumInfosets(); j++)
        {
            for (int k = 1; k <= p->Infosets()[j]->NumActions(); k++)
                players[i]->Append("(" + ToText(j) + "," + ToText(k) + ")");
        }

        for (j = 0; j < players[i]->Number(); j++) 
            players[i]->SetSelection(j, TRUE);

        if (i%ENTRIES_PER_ROW == 0) support->NewLine();
    }

    support->Go();

    if (support->Completed() == wxOK)
    {
        EFSupport *sup = new EFSupport(ef);
        bool failed = false;

        for (int i = ef.NumPlayers(); i >= 1; i--) // going from bottom up.
        {
            for (int j = 1; j <= ef.Players()[i]->NumInfosets(); j++)
            {
                for (int k = sup->NumActions(i, j); k >= 1; k--)
                {
                    if (!players[i]->Selected(k-1))
                        sup->RemoveAction(ef.Players()[i]->Infosets()[j]->Actions()[k]);
                }

                // Check that each player has at least one action
                if (sup->NumActions(i, j) == 0) 
                    failed = true;
            }
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
                         "Each Player must have at least one Action in each Infoset");
            return 0;
        }
    }

    delete support;
    return 0;
}

// Support Inspect
void EfgShow::ChangeSupport(int what)
{
    if (what == CREATE_DIALOG)
    {
        if (!support_dialog)
        {
            int disp = supports.Find(disp_sup), cur = supports.Find(cur_sup);
            support_dialog = new EFSupportInspectDialog(supports, cur, disp, this);
        }
        else
            support_dialog->SetFocus();
    }

    if (what == DESTROY_DIALOG && support_dialog)
    {
        delete support_dialog;
        support_dialog = 0;
    }

    if (what == SUPPORT_CHANGE)
    {
        assert(support_dialog);
        cur_sup = supports[support_dialog->CurSup()];

        if (supports[support_dialog->DispSup()] != disp_sup ||
            tw->DrawSettings().RootReachable() != support_dialog->RootReachable())
        {
            disp_sup = supports[support_dialog->DispSup()];
            tw->DrawSettings().SetRootReachable(support_dialog->RootReachable());
            tw->SupportChanged();
        }
    }
}

#include "wxstatus.h"
EFSupport *ComputeDominated(EFSupport &S, bool strong,
                            const gArray<int> &players, gOutput &tracefile, gStatus &status);

#include "elimdomd.h"
void EfgShow::SolveElimDom(void)
{
    ElimDomParamsDialog EDPD(ef.NumPlayers(), this);

    if (EDPD.Completed() == wxOK)
    {
        EFSupport *sup = cur_sup;
        wxStatus status(this, "Dominance Elimination");

        if (!EDPD.DomMixed())
        {
            if (EDPD.FindAll())
            {
                while ((sup = ComputeDominated(*sup, EDPD.DomStrong(), 
                                               EDPD.Players(), gnull, status)))
                {
                    supports.Append(sup);
                }
            }
            else
            {
                if ((sup = ComputeDominated(*sup, EDPD.DomStrong(), 
                                            EDPD.Players(), gnull, status)))
                {
                    supports.Append(sup);
                }
            }
        }
        else
        {
            wxMessageBox("Mixed Dominance is not implemented for\nExtensive Form Games");
        }

        if (EDPD.Compress() && disp_sup != sup)
        {
            cur_sup = supports[supports.Length()]; // displaying the last created support
            disp_sup = cur_sup;
            tw->SupportChanged();
        }
    }
}


void EfgShow::GameChanged(void)
{
    static Bool inform_delete = TRUE;

    if (supports.Length() > 1 && inform_delete)
    {
        MyDialogBox inform_delete_dialog(this, "Deleting supports", EFG_MAKE_SUPPORT_HELP);
        inform_delete_dialog.Add(wxMakeFormMessage("Deleting all existing supports."));
        inform_delete_dialog.Add(wxMakeFormNewLine());
        inform_delete_dialog.Add(wxMakeFormBool("Show this note again", &inform_delete));
        inform_delete_dialog.Go();
    }

    while (supports.Length()) 
        delete supports.Remove(1);

    // Create the full support.
    cur_sup = new EFSupport(ef);
    disp_sup = cur_sup;
    supports.Append(cur_sup);
    RemoveStartProfiles();
}


// File name stuff
void EfgShow::SetFileName(void)
{
    // Title the window
#ifndef _HPUX_SOURCE // for some strange reason SetTitle crashes on hp
    SetTitle("[" + gText(wxFileNameFromPath(filename)) + "] " + ef.GetTitle());
#endif
}


void EfgShow::SetFileName(const gText &s)
{
    if (s != "") 
        filename = s;
    else 
        filename = "untitled.efg";

    // Title the window
#ifndef _HPUX_SOURCE // for some strange reason SetTitle crashes on hp
    SetTitle("[" + gText(wxFileNameFromPath(filename)) + "] " + ef.GetTitle());
#endif
}


// Show some game info
bool IsPerfectRecall(const Efg &, Infoset *&, Infoset *&);

void EfgShow::ShowGameInfo(void)
{
    gText tmp;
    char tempstr[200];
    sprintf(tempstr, "Number of Players: %d", ef.NumPlayers());
    tmp += tempstr;
    tmp += "\n";
    sprintf(tempstr, "Is %sconstant sum", ((ef.IsConstSum()) ? "" : "NOT "));
    tmp += tempstr;
    tmp += "\n";
    Infoset *bad1, *bad2;
    sprintf(tempstr, "Is %sperfect recall", ((IsPerfectRecall(ef, bad1, bad2)) ? "" : "NOT "));
    tmp += tempstr;
    tmp += "\n";

    wxMessageBox(tmp, "Efg Game Info", wxOK, this);
}


// which: 0 - current, 1 - display
const EFSupport *EfgShow::GetSupport(int which)
{
    return (which == 0) ? cur_sup : disp_sup;
}





#include "efggui.h"
//***************************************************************************
//                EXTENSIVE FORM GUI
//***************************************************************************

EfgGUI::EfgGUI(Efg *ef, const gText infile_name, 
               EfgNfgInterface *inter, wxFrame *parent)
{
    if (!ef)    // must create a new extensive form from scratch or file
    {
        if (infile_name == gText()) // from scratch
        {
            gArray<gText> names;

            if (GetEFParams(names, parent))
            {
                ef = new Efg;
                for (int i = 1; i <= names.Length(); i++)   
                    ef->NewPlayer()->SetName(names[i]);
            }
        }
        else                        // from data file
        {
	  try { 
            gFileInput infile(infile_name);
            ReadEfgFile(infile, ef);
	    
            if (!ef) 
	      wxMessageBox("ReadFailed::Check the file");
	  }
	  catch(gFileInput::OpenFailed &) {
	    wxMessageBox("ReadFailed::Check the file");
	    return;
	  }
	  
        }
    }

    EfgShow *ef_show = 0;

    if (ef)
    {
        ef_show = new EfgShow(*ef, inter, 1, parent);
    }

    if (ef_show) 
        ef_show->SetFileName(infile_name);
}


#define MAX_PLAYERS           100
#define MAX_STRATEGIES        100
#define NUM_PLAYERS_PER_LINE    8

int EfgGUI::GetEFParams(gArray<gText> &names, wxFrame *parent)
{
    int num_players = 2;

    // Get the number of players first
    MyDialogBox *make_ef_p = 
        new MyDialogBox(parent, "Extensive Form Parameters");
    make_ef_p->Form()->Add(wxMakeFormShort("How many players", 
                                           &num_players, wxFORM_TEXT,
                                           new wxList(wxMakeConstraintRange(1, MAX_PLAYERS), 0), 
                                           NULL, 0, 220));
    make_ef_p->Go();
    int ok = make_ef_p->Completed();
    delete make_ef_p;

    if (ok != wxOK || num_players < 1) 
        return 0;

    // Now get player names
    MyDialogBox *make_ef_names = new MyDialogBox(parent, "Player Names");
    names = gArray<gText>(num_players);
    char **names_str = new char*[num_players+1];

    for (int i = 1; i <= num_players; i++)
    {
        names_str[i] = new char[20];
        strcpy(names_str[i], "Player"+ToText(i));
        make_ef_names->Add(wxMakeFormString(ToText(i), 
                                            &names_str[i], wxFORM_TEXT,
                                            NULL, NULL, 0, 140));

        if (i%(NUM_PLAYERS_PER_LINE/2) == 0) 
            make_ef_names->Add(wxMakeFormNewLine());
    }

    make_ef_names->Go();
    ok = make_ef_names->Completed();
    delete make_ef_names;

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



