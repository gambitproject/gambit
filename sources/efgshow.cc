//
// FILE: efgshow.cc -- Implementation of class EfgShow
//
//  $Id$
//

#include "wx.h"
#include "wxmisc.h"
#include "efg.h"
#include "efgutils.h"
#include "behavsol.h"

#include "efgconst.h"
#include "treewin.h"
#include "efgshow.h"
#include "efgsoln.h"
#include "nfggui.h"
#include "efgnfgi.h"
#include "efsuptd.h"

//=====================================================================
//                       class EfgShowToolBar
//=====================================================================

#ifdef wx_msw
#include "wx_bbar.h"
#else
#include "wx_tbar.h"
#endif

#ifdef wx_msw
class guiEfgShowToolBar : public wxButtonBar {
#else
class guiEfgShowToolBar : public wxToolBar {
#endif   // wx_msw

private:
  wxFrame *m_parent;

public:
  guiEfgShowToolBar(wxFrame *p_frame);
  ~guiEfgShowToolBar() { }
  Bool OnLeftClick(int p_toolIndex, Bool p_toggled);
  void OnMouseEnter(int p_toolIndex);
};

guiEfgShowToolBar::guiEfgShowToolBar(wxFrame *p_frame)
#ifdef wx_msw
  : wxButtonBar(p_frame, 0, 0, -1, -1, 0, wxHORIZONTAL, 30),
#else
  : wxToolBar(p_frame, 0, 0, -1, -1, 0, wxHORIZONTAL, 30),
#endif  // wx_msw
    m_parent(p_frame)
{
    // Load palette bitmaps
#include "bitmaps/save.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/delete.xpm"
#include "bitmaps/solve.xpm"
#include "bitmaps/zoomin.xpm"
#include "bitmaps/zoomout.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/add.xpm"
#include "bitmaps/options.xpm"
#include "bitmaps/makenf.xpm"
#include "bitmaps/inspect.xpm"
  //#include "bitmaps/payoff.xpm"
  wxBitmap *ToolbarSaveBitmap = new wxBitmap(save_xpm);
  wxBitmap *ToolbarPrintBitmap = new wxBitmap(print_xpm);
  wxBitmap *ToolbarDeleteBitmap = new wxBitmap(delete_xpm);
  wxBitmap *ToolbarSolveBitmap = new wxBitmap(solve_xpm);
  wxBitmap *ToolbarZoominBitmap = new wxBitmap(zoomin_xpm);
  wxBitmap *ToolbarZoomoutBitmap = new wxBitmap(zoomout_xpm);
  wxBitmap *ToolbarHelpBitmap = new wxBitmap(help_xpm);
  wxBitmap *ToolbarAddBitmap = new wxBitmap(add_xpm);
  wxBitmap *ToolbarOptionsBitmap = new wxBitmap(options_xpm);
  wxBitmap *ToolbarMakenfBitmap = new wxBitmap(makenf_xpm);
  wxBitmap *ToolbarInspectBitmap = new wxBitmap(inspect_xpm);
  //wxBitmap *ToolbarPayoffBitmap = new wxBitmap(payoff_xpm);
    
  // Save, Print | Add, Delete, Outcomes | Solve, Inspect, MakeNF |
  // ZoomIn, ZoomOut, Options | Help
  // Create the toolbar
  SetMargins(2, 2);
#ifdef wx_msw
  SetDefaultSize(33, 30);
#endif  // wx_msw
  GetDC()->SetBackground(wxLIGHT_GREY_BRUSH);
    
  AddTool(FILE_SAVE, ToolbarSaveBitmap);
  AddTool(FILE_OUTPUT, ToolbarPrintBitmap);
  AddSeparator();
  AddTool(NODE_ADD, ToolbarAddBitmap);
  AddTool(NODE_DELETE, ToolbarDeleteBitmap);
  // AddTool(TREE_OUTCOMES, ToolbarPayoffBitmap);
  AddSeparator();
  AddTool(SOLVE_STANDARD, ToolbarSolveBitmap);
  AddTool(INSPECT_SOLUTIONS, ToolbarInspectBitmap);
  AddTool(SOLVE_NFG_REDUCED, ToolbarMakenfBitmap);
  AddSeparator();
  AddTool(DISPLAY_INC_ZOOM, ToolbarZoominBitmap);
  AddTool(DISPLAY_DEC_ZOOM, ToolbarZoomoutBitmap);
  AddTool(DISPLAY_SET_OPTIONS, ToolbarOptionsBitmap);
  AddSeparator();
  AddTool(GAMBIT_HELP_CONTENTS, ToolbarHelpBitmap);
  Layout();
}

Bool guiEfgShowToolBar::OnLeftClick(int p_toolIndex, Bool /*p_toggled*/)
{
  m_parent->OnMenuCommand(p_toolIndex);
  return TRUE;
}

void guiEfgShowToolBar::OnMouseEnter(int p_toolIndex)
{
  m_parent->SetStatusText(m_parent->GetMenuBar()->GetHelpString(p_toolIndex));
}

//=====================================================================
//                     EfgShow MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//               EfgShow: Constructor and destructor
//---------------------------------------------------------------------

EfgShow::EfgShow(Efg &p_efg, EfgNfgInterface *p_nfg, int, wxFrame *p_frame,
                 char *p_title, int p_x, int p_y, int p_w, int p_h, int p_type)
  : wxFrame(p_frame, p_title, p_x, p_y, p_w, p_h, p_type), 
    EfgNfgInterface(gEFG, p_nfg), 
    GuiObject(gText("EfgShow")),
    parent(p_frame), ef(p_efg), cur_soln(0), outcome_dialog(0), 
    support_dialog(0), soln_show(0), node_inspect(0), tw(0)
{
  Show(FALSE);

  // Give the frame an icon
#ifdef wx_msw
  wxIcon *frame_icon = new wxIcon("efg_icn");
#else
#include "efg.xbm"
  wxIcon *frame_icon = new wxIcon(efg_bits, efg_width, efg_height);
#endif

  SetIcon(frame_icon);
  // Create the status bar
  CreateStatusLine();
  // Create the menu bar
  MakeMenus();
  // Create the accelerators (to add an accelerator, see const.h)
  ReadAccelerators(accelerators, "EfgAccelerators");
    
  // Create the canvas(TreeWindow) on which to draw the tree
  tw = new TreeWindow(ef, disp_sup, this);
  // Create the toolbar (must be after the status bar creation)
  toolbar = new guiEfgShowToolBar(this);
  // Create the all_nodes list.
  Nodes(ef, all_nodes);

  OnSize(-1, -1);
  // now zoom in/out to show the full tree
  tw->display_zoom_fit();
    
  node_inspect = new NodeSolnShow(ef.NumPlayers(), this);
  node_inspect->Set(tw->Cursor());
  Show(TRUE);
}

EfgShow::~EfgShow(void)
{
  Show(FALSE);
  delete &ef;
  delete toolbar;
  delete tw;
}

#include "elimdomd.h"

void EfgShow::OnSelectedMoved(const Node *n)
{
  // The only time the inspection window won't be around is on construction
  if (node_inspect) {
    node_inspect->Set(n);
  }
}

//*******************************************************************
//                          SOLUTION ROUTINES                       *
//*******************************************************************

#include "efgsolvd.h"

void EfgShow::SolveStandard(void)
{
  // This is a guard against trying to solve the "trivial" game.
  // Most of the GUI code assumes information sets exist.
  if (ef.TotalNumInfosets() == 0)  return;

  // check that the game is perfect recall, if not give a warning
  Infoset *bad1, *bad2;
  if (!IsPerfectRecall(ef, bad1, bad2)) {
    int completed = wxMessageBox("This game is not perfect recall\n"
				 "Do you wish to continue?", 
				 "Solve Warning", 
				 wxOK | wxCANCEL | wxCENTRE, this);
    if (completed != wxOK) return;
  }

  EfgSolveStandardDialog *ESSD = new EfgSolveStandardDialog(ef, this);
  if (ESSD->Completed() != wxOK)  {
    delete ESSD;
    return;
  }
  delete ESSD;

  EfgSolveSettings ESS(ef);

  guiEfgSolution *solver;

  Enable(FALSE);
  wxBeginBusyCursor();

  if (!ESS.ViaNfg()) {
    switch (ESS.GetEfgAlgorithm()) {
    case EFG_PURENASH_SOLUTION:
      solver = new EfgEPureNashG(ef, *cur_sup, this);
      break;
    case EFG_LCP_SOLUTION:
      solver = new EfgSeqFormG(ef, *cur_sup, this);
      break;
    case EFG_CSUM_SOLUTION:
      solver = new EfgCSumG(ef, *cur_sup, this);
      break;
    case EFG_LIAP_SOLUTION:
      solver = new guiEfgSolveLiap(ef, *cur_sup, this);
      break;
    case EFG_QRE_SOLUTION:
      solver = new EfgEQreG(ef, *cur_sup, this);
      break;
    default:
      return;
    }
  }
  else {
    switch (ESS.GetNfgAlgorithm()) {
    case NFG_ENUMPURE_SOLUTION:
      solver = new EfgPureNashG(ef, *cur_sup, this);
      break;
    case NFG_ENUMMIXED_SOLUTION:
      solver = new EfgEnumG(ef, *cur_sup, this);
      break;
    case NFG_LCP_SOLUTION:
      solver = new EfgLemkeG(ef, *cur_sup, this);
      break;
    case NFG_LP_SOLUTION:
      solver = new EfgZSumG(ef, *cur_sup, this);
      break;
    case NFG_LIAP_SOLUTION:
      solver = new EfgNLiapG(ef, *cur_sup, this);
      break;
    case NFG_SIMPDIV_SOLUTION:
      solver = new EfgSimpdivG(ef, *cur_sup, this);
      break;
    case NFG_QRE_SOLUTION:
      solver = new EfgNQreG(ef, *cur_sup, this);
      break;
    case NFG_QREALL_SOLUTION:
      solver = new EfgQreAllG(ef, *cur_sup, this);
      break;
    default:
      return;
    }
  }

  try {
    if (ESS.MarkSubgames())
      tw->subgame_solve();
    else
      tw->subgame_clear_all();
    solns += solver->Solve();
    wxEndBusyCursor();
  }
  catch (gException &E) {
    wxEndBusyCursor();
    guiExceptionDialog(E.Description(), this);
  }

  delete solver;

  ChangeSolution(solns.VisibleLength());
  Enable(TRUE);
  InspectSolutions(CREATE_DIALOG);
}

// Solve
bool IsPerfectRecall(const Efg &, Infoset *&, Infoset *&);

void EfgShow::Solve(int p_algorithm)
{
  // This is a guard against trying to solve the "trivial" game.
  // Most of the GUI code assumes information sets exist.
  if (ef.TotalNumInfosets() == 0)  return;

  // check that the game is perfect recall, if not give a warning
  Infoset *bad1, *bad2;
  if (!IsPerfectRecall(ef, bad1, bad2)) {
    int completed = wxMessageBox("This game is not perfect recall\n"
				 "Do you wish to continue?", 
				 "Solve Warning", 
				 wxOK | wxCANCEL | wxCENTRE, this);
    if (completed != wxOK) return;
  }
    
  //  EfgSolveSettings ESS(ef);
  // do not want users doing anything while solving
  Enable(FALSE);

  guiEfgSolution *solver = 0;

  switch (p_algorithm) {
  case SOLVE_CUSTOM_EFG_ENUMPURE:
    solver = new EfgEPureNashG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_EFG_LCP:
    solver = new EfgSeqFormG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_EFG_LP:
    solver = new EfgCSumG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_EFG_LIAP:
    solver = new guiEfgSolveLiap(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_EFG_POLENUM:
    solver = new guiPolEnumEfg(*cur_sup, this);
    break;
  case SOLVE_CUSTOM_EFG_QRE:
    solver = new EfgEQreG(ef, *cur_sup, this);
    break;

  case SOLVE_CUSTOM_NFG_ENUMPURE: 
    solver = new EfgPureNashG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_NFG_ENUMMIXED:
    solver = new EfgEnumG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_NFG_LCP: 
    solver = new EfgLemkeG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_NFG_LP:
    solver = new EfgZSumG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_NFG_LIAP: 
    solver = new EfgNLiapG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_NFG_SIMPDIV:
    solver = new EfgSimpdivG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_NFG_POLENUM:
    solver = new guiPolEnumEfgNfg(*cur_sup, this);
    break;
  case SOLVE_CUSTOM_NFG_QRE:
    solver = new EfgNQreG(ef, *cur_sup, this);
    break;
  case SOLVE_CUSTOM_NFG_QREGRID: 
    solver = new EfgQreAllG(ef, *cur_sup, this);
    break;
  default:
    // internal error, we'll just ignore silently
    return;
  }

  bool go = solver->SolveSetup();
  
  try {
    if (go) {
      if (solver->MarkSubgames())
	tw->subgame_solve();
      wxBeginBusyCursor();
      solns += solver->Solve();
      wxEndBusyCursor();
    }
  }
  catch (gException &E) {
    wxEndBusyCursor();
    guiExceptionDialog(E.Description(), this);
  }

  delete solver;
 
  ChangeSolution(solns.VisibleLength());
  Enable(TRUE);
  if (go)  InspectSolutions(CREATE_DIALOG);
}


void EfgShow::InspectSolutions(int what)
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

    soln_show = new EfgSolnShow(ef, solns, cur_soln, tw->DrawSettings(), sf_options, this);
  }

  if (what == DESTROY_DIALOG && soln_show) {
    soln_show = 0;
  }
}


void EfgShow::ChangeSolution(int sol)
{
  if (cur_soln != sol) {
    cur_soln = sol;
    tw->OnPaint();
    if (node_inspect) {
      node_inspect->Set(tw->Cursor());
    }
  }
}


// RemoveStartProfiles.  Should be called if game structure changes

void EfgShow::RemoveStartProfiles(void)
{
    starting_points = StartingPoints();
}


// Remove solutions.

void EfgShow::RemoveSolutions(void)
{
    if (soln_show) 
        delete soln_show; 
    soln_show = 0;
    cur_soln = 0;
    solns.Flush();
    Nodes(ef, all_nodes);
    OnSelectedMoved(0); // update the node inspect window if any
}


BehavSolution EfgShow::CreateSolution(void)
{
    return BehavSolution(BehavProfile<gNumber>(*cur_sup));
}

#include "efgoutcd.h"
#define UPDATE1_DIALOG  4
#define PARAMS_ADD_VAR  5       
void EfgShow::ChangeOutcomes(int what, const gText out_name)
{
    if (what == CREATE_DIALOG && !outcome_dialog)
    {
        outcome_dialog = new EfgOutcomeDialog(ef, this);
        if (out_name != "") outcome_dialog->SetOutcome(out_name);
    }

    if (what == DESTROY_DIALOG && outcome_dialog)
    {
        delete outcome_dialog; outcome_dialog = 0;
    }
}

//************************************************************************
//                     ACTUAL SOLUTION FUNCTIONS
//************************************************************************


// SolutionToEfg: overrides the corresponding member of the EfgNfgInterface,
// to allow the NormalForm to send its solutions here

void EfgShow::SolutionToEfg(const BehavProfile<gNumber> &s, bool set)
{
    assert(Interface());    // we better have someone to get a solution from!
    solns.Append(s);

    if (set)
    {
        cur_soln = solns.VisibleLength();
        tw->OnPaint();
    }
}

// Solution access for TreeWindow

gText EfgShow::AsString(TypedSolnValues what, const Node *n, int br) const
{
    int i;
    // Special case that does not fit in ANYWHERE: Chance nodes have probs w/out solutions
    if (what == tBranchProb && n->GetPlayer())
        if (n->GetPlayer()->IsChance())
            return ToText(ef.GetChanceProb(n->GetInfoset(), br));
    
    if (!n || !cur_soln) return "N/A";
    
    const BehavSolution &cur = solns[cur_soln];
    int n_index = all_nodes.Find((Node *const)n);
    
    switch (what)
    {
    case tRealizProb:           // terminal ok
        return ToText(cur.NodeRealizProbs()[n_index]);
    case tBeliefProb: // terminal ok
    {
        if (!n->GetPlayer()) return "N/A";
        if (n->GetPlayer()->IsChance()) return "N/A";
        // Figure out what member # this node is in its iset
        int memb_num;
        for (memb_num = 1; n->GetInfoset()->Members()[memb_num] != n; memb_num++) ;
        
        return ToText((((BehavSolution &)cur).Beliefs())(n->GetPlayer()->GetNumber(),
                                                         n->GetInfoset()->GetNumber(), memb_num));
    }
    case tNodeValue:  // terminal ok
    {
        gText tmp = "(";
        for (i = 1; i <= ef.NumPlayers(); i++)
            tmp += ToText(cur.NodeValues(i)[n_index])+((i == ef.NumPlayers()) ? ")" : ",");
        return tmp;
    }
    case tIsetProb: // terminal not ok
    {
        if (!n->GetPlayer()) return "N/A";
        if (n->GetPlayer()->IsChance()) return "N/A";
        gDPVector<gNumber> value(ef.NumActions());
        gPVector<gNumber> probs(ef.NumInfosets());
        cur.CondPayoff(value, probs);
        return ToText(probs(n->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber()));
    }
    case tBranchVal: // terminal not ok
    {
        if (!n->GetPlayer()) return "N/A";
        if (n->GetPlayer()->IsChance()) return "N/A";
        gDPVector<gNumber> value(ef.NumActions());
        gPVector<gNumber> probs(ef.NumInfosets());
        cur.CondPayoff(value, probs);
        if (probs(n->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber()) > gNumber(0))
            return ToText(value(n->GetPlayer()->GetNumber(),
                                n->GetInfoset()->GetNumber(), br));
        else        // this is due to a bug in the value computation
            return "N/A";
    }
    case tBranchProb:   // terminal not ok
        if (!n->GetPlayer()) return "N/A";
        // For chance node prob, see first line of this function
        return ToText(cur.GetValue(n->GetInfoset(), br));
    case tIsetValue:    // terminal not ok, not implemented
        return "N.I.";
    default:
        return "N/A";
    }
}



// Get Action Prob: used by TreeWindow
#include "infoset.h"
#include "node.h"
#include "efplayer.h"

gNumber EfgShow::BranchProb(const Node *n, int br)
{
    if (n->GetPlayer())
        if (n->GetPlayer()->IsChance())
            return ef.GetChanceProb(n->GetInfoset(), br);
    if (cur_soln && n->GetInfoset())
        return solns[cur_soln].GetValue(n->GetInfoset(), br);
    return -1;
}


#include "nfg.h"
Nfg *MakeReducedNfg(const EFSupport &support);
Nfg *MakeAfg(const Efg &);


bool EfgShow::SolveNormalReduced(void)
{
  // check that the game is perfect recall, if not give a warning
  Infoset *bad1, *bad2;

  if (!IsPerfectRecall(ef, bad1, bad2)) {
    int completed = wxMessageBox("This game is not perfect recall\n"
				 "Do you wish to continue?", 
				 "Reduced normal form", 
				 wxOK|wxCANCEL|wxCENTRE, this);
    
    if (completed != wxOK) 
      return false;
  }
    
  Nfg *N = MakeReducedNfg(*cur_sup);
  if (N) NfgGUI(N, "", (EfgNfgInterface *) this, this);

  return false;
}

bool EfgShow::SolveNormalAgent(void)
{
  // check that the game is perfect recall, if not give a warning
  Infoset *bad1, *bad2;

  if (!IsPerfectRecall(ef, bad1, bad2)) {
    int completed = wxMessageBox("This game is not perfect recall\n"
				 "Do you wish to continue?", 
				 "Agent normal form", 
				 wxOK|wxCANCEL|wxCENTRE, this);
    
    if (completed != wxOK) 
      return false;
  }
    
  Nfg *N = MakeAfg(ef);
  if (N) NfgGUI(N, "", (EfgNfgInterface *) this, this);

  return false;
}

//************************************************************************
//                     INTERFACE TO SOLUTION FUNCTIONS
//************************************************************************


const gText &EfgShow::Filename(void) const
{
    return filename;
}

void EfgShow::SetPickSubgame(const Node *n)
{
    tw->SetSubgamePickNode(n);
}


wxFrame *EfgShow::Frame(void)
{
    return (wxFrame *)this;
}



void EfgShow::PickSolutions(const Efg &p_ef, gList<BehavSolution> &p_solns)
{
    BehavSolutionList temp_solns;   // coerce the list into a sortable
    temp_solns += p_solns;              // format, and automatically number id's
    EfgSolnPicker *pick = new EfgSolnPicker(p_ef, temp_solns,
                                            tw->DrawSettings(), sf_options, this);
    Enable(FALSE);
    while (pick->Completed() == wxRUNNING) wxYield();
    Enable(TRUE);
    p_solns = temp_solns;   // assign back to p_solns for changes to 'take'
    delete pick;
}

// how: 0-default, 1-saved, 2-query

BehavProfile<gNumber> EfgShow::CreateStartProfile(int how)
{
    BehavProfile<gNumber> start(*cur_sup);
    if (how == 0)   start.Centroid();
    if (how == 1 || how == 2)
    {
        if (starting_points.last == -1 || how == 2)
        {
            BSolnSortFilterOptions sf_opts; // no sort, filter
            if (starting_points.profiles.Length() == 0)
                starting_points.profiles += start;
            Ext1SolnPicker *start_dialog = new Ext1SolnPicker(ef, starting_points.profiles, tw->DrawSettings(), sf_opts, this);
            Enable(FALSE);  // disable this window until the edit window is closed
            while (start_dialog->Completed() == wxRUNNING) wxYield();
            Enable(TRUE);
            starting_points.last = start_dialog->Picked();
            delete start_dialog;
        }
        if (starting_points.last)
            start = BehavProfile<gNumber>(starting_points.profiles[starting_points.last]);
    }
    return start;
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

void EfgShow::MakeMenus(void)
{
  // To insure greatest possible portability we will avoid using
  // resource files which are inherently non-portable.

  wxMenu *file_menu = new wxMenu;
  file_menu->Append(FILE_SAVE,   "&Save",    "Save the game");
  file_menu->Append(FILE_OUTPUT, "&Output",  "Print or copy the game");
  file_menu->Append(FILE_CLOSE,  "&Close",   "Close this window");

  wxMenu *edit_menu = new wxMenu;
  wxMenu *nodeMenu  = new wxMenu;
  nodeMenu->Append(NODE_ADD, "&Add Move", "Add a move");
  nodeMenu->Append(NODE_DELETE, "&Delete Move", "Remove move at cursor");
  nodeMenu->Append(NODE_INSERT, "&Insert Move", "Insert move at cursor");
  nodeMenu->Append(NODE_LABEL,     "&Label",     "Label cursor node");
  nodeMenu->AppendSeparator();
  nodeMenu->Append(NODE_SET_MARK,  "Set &Mark",  "Mark cursor node");
  nodeMenu->Append(NODE_GOTO_MARK, "Go&to Mark", "Goto marked node");

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

  wxMenu *outcome_menu = new wxMenu;
  outcome_menu->Append(TREE_OUTCOMES_NEW, "&New",
		       "Create a new outcome");
  outcome_menu->Append(TREE_OUTCOMES_DELETE, "Dele&te",
		       "Delete an outcome");
  outcome_menu->Append(TREE_OUTCOMES_ATTACH, "&Attach",
		       "Attach an outcome to the node at cursor");
  outcome_menu->Append(TREE_OUTCOMES_DETACH, "&Detach",
		       "Detach the outcome from the node at cursor");
  outcome_menu->Append(TREE_OUTCOMES_LABEL, "&Label",
		       "Label the outcome at the node at cursor");
  outcome_menu->Append(TREE_OUTCOMES_PAYOFFS, "&Payoffs",
		       "Set the payoffs for the outcome at the cursor");

  wxMenu *tree_menu = new wxMenu;
  tree_menu->Append(TREE_COPY,      "&Copy",     "Copy tree from marked node");
  tree_menu->Append(TREE_MOVE,      "&Move",     "Move tree from marked node");
  tree_menu->Append(TREE_DELETE,    "&Delete",   "Delete recursively from cursor");
  tree_menu->Append(TREE_LABEL,     "&Label",    "Set the game label");
  tree_menu->Append(TREE_PLAYERS,   "&Players",  "Edit/View players");
  tree_menu->Append(TREE_INFOSETS,  "&Infosets", "Edit/View infosets");
  edit_menu->Append(EDIT_NODE,    "&Node",     nodeMenu,    "Edit the node");
  edit_menu->Append(EDIT_ACTIONS, "&Actions",  action_menu,  "Edit actions");
  edit_menu->Append(EDIT_INFOSET, "&Infoset",  infoset_menu, "Edit infosets");
  edit_menu->Append(TREE_OUTCOMES, "&Outcomes", outcome_menu,
		    "Edit outcomes and payoffs");
  edit_menu->Append(EDIT_TREE,    "&Tree",     tree_menu,    "Edit the tree");

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
  supports_menu->Append(SUPPORTS_SUPPORTS, "&Select", "Select/Create EF supports");
  
  wxMenu *solve_menu = new wxMenu;
  solve_menu->Append(SOLVE_STANDARD, "S&tandard...", "Standard solutions");

  wxMenu *solveCustomMenu = new wxMenu;
  wxMenu *solveCustomEfgMenu = new wxMenu;
  solveCustomEfgMenu->Append(SOLVE_CUSTOM_EFG_ENUMPURE, "EnumPure",
			     "Enumerate pure strategy equilibria");
  solveCustomEfgMenu->Append(SOLVE_CUSTOM_EFG_LCP, "LCP",
			     "Solve by linear complementarity program");
  solveCustomEfgMenu->Append(SOLVE_CUSTOM_EFG_LP, "LP",
			     "Solve by linear program");
  solveCustomEfgMenu->Append(SOLVE_CUSTOM_EFG_LIAP, "Liapunov",
			     "Liapunov function minimization");
  solveCustomEfgMenu->Append(SOLVE_CUSTOM_EFG_POLENUM, "PolEnum",
			     "Enumeration by systems of polynomials", TRUE);
  // FIXME: This item currently disabled since algorithm not implemented yet
  solveCustomEfgMenu->Enable(SOLVE_CUSTOM_EFG_POLENUM, FALSE);
  solveCustomEfgMenu->Append(SOLVE_CUSTOM_EFG_QRE, "QRE",
			     "Compute quantal response equilibria");
  solveCustomMenu->Append(SOLVE_CUSTOM_EFG, "Extensive form",
			  solveCustomEfgMenu,
			  "Solve using extensive form based algorithms");

  wxMenu *solveCustomNfgMenu = new wxMenu;
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_ENUMPURE, "EnumPure",
			     "Enumerate pure strategy equilibria");
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_ENUMMIXED, "EnumMixed",
			     "Enumerate all equilibria");
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_LCP, "LCP",
			     "Solve by linear complementarity program");
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_LP, "LP",
			     "Solve by linear program");
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_LIAP, "Liapunov",
			     "Liapunov function minimization");
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_SIMPDIV, "Simpdiv",
			     "Simplicial subdivision");
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_POLENUM, "PolEnum",
			     "Enumeration by systems of polynomials");
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_QRE, "QRE",
			     "Compute quantal response equilibria");
  solveCustomNfgMenu->Append(SOLVE_CUSTOM_NFG_QREGRID, "QRE Grid",
			     "Compute quantal response equilibria");
  solveCustomMenu->Append(SOLVE_CUSTOM_NFG, "Normal form",
              solveCustomNfgMenu,
              "Solve using normal form based algorithms");

  solve_menu->Append(SOLVE_CUSTOM, "Custom", solveCustomMenu,
             "Select a specific algorithm");

  wxMenu *solveNfgMenu = new wxMenu;
  solveNfgMenu->Append(SOLVE_NFG_REDUCED, "Reduced",
               "Generate reduced normal form");
  solveNfgMenu->Append(SOLVE_NFG_AGENT, "Agent",
               "Generate agent normal form");
  solve_menu->Append(SOLVE_NFG, "Normal form", solveNfgMenu,
             "Create a normal form representation of this game");
  
  wxMenu *inspect_menu = new wxMenu;
  inspect_menu->Append(INSPECT_SOLUTIONS, "&Solutions",
		       "Inspect existing solutions");
  inspect_menu->Append(INSPECT_CURSOR, "&Cursor",
		       "Information about the node at cursor");
  inspect_menu->Append(INSPECT_INFOSETS, "&Infosets",
		       "Inspect information sets", TRUE);
  inspect_menu->Append(INSPECT_ZOOM_WIN, "Zoom &Window",
		       "Open zoom-in window");
  inspect_menu->AppendSeparator();
  inspect_menu->Append(INSPECT_GAMEINFO, "Game&Info",
               "Information about this game");
  m_inspectInfosetsItem = inspect_menu->FindItem("&Infosets");
  
  wxMenu *prefs_menu = new wxMenu;
  prefs_menu->Append(DISPLAY_SET_ZOOM,    "&Zoom",        "Specify zoom level");
  prefs_menu->Append(DISPLAY_SET_OPTIONS, "&Display",     "Set display options");
  prefs_menu->Append(DISPLAY_LEGENDS,     "&Legend",      "Set legends");
  prefs_menu->Append(DISPLAY_COLORS,      "&Colors",      "Set player colors");
  prefs_menu->Append(DISPLAY_ACCELS,      "&Accels",      "Edit accelerator keys");
  prefs_menu->AppendSeparator();
  prefs_menu->Append(DISPLAY_SAVE_DEFAULT, "Save Default");
  prefs_menu->Append(DISPLAY_LOAD_DEFAULT, "Load Default");
  prefs_menu->Append(DISPLAY_SAVE_CUSTOM,  "Save Custom");
  prefs_menu->Append(DISPLAY_LOAD_CUSTOM,  "Load Custom");
  
  wxMenu *help_menu = new wxMenu;
  help_menu->Append(GAMBIT_HELP_CONTENTS, "&Contents", "Table of contents");
  help_menu->Append(GAMBIT_HELP_ABOUT,    "&About",    "About this program");

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu,     "&File");
  menu_bar->Append(edit_menu,     "&Edit");
  menu_bar->Append(subgame_menu,  "Sub&games");
  menu_bar->Append(supports_menu, "S&upports");
  menu_bar->Append(solve_menu,    "&Solve");
  menu_bar->Append(inspect_menu,  "&Inspect");
  menu_bar->Append(prefs_menu,    "&Prefs");
  menu_bar->Append(help_menu,     "&Help");

  // Set the menu bar
  SetMenuBar(menu_bar);
}

Bool EfgShow::OnClose(void)
{
  if (ef.IsDirty()) {
    if (wxMessageBox("Game has been modified.  Close anyway?", "Warning",
		     wxOK | wxCANCEL) == wxCANCEL)
      return FALSE;
    else {
      ChangeSupport(DESTROY_DIALOG);
      ChangeOutcomes(DESTROY_DIALOG);
      InspectSolutions(DESTROY_DIALOG);
      Show(FALSE);
      return TRUE;
    }
  }
  else {
    return TRUE;
  }
}

//---------------------------------------------------------------------
//             EXTENSIVE SHOW: EVENT-HANDLING HOOK MEMBERS
//---------------------------------------------------------------------
void EfgShow::OnSetFocus(void)
{
    tw->SetFocus();
}

#define     SOLVE_SETUP_CUSTOM      0
#define     SOLVE_SETUP_STANDARD    1


void EfgShow::OnMenuCommand(int id)
{
  try {
    switch (id) {
    case NODE_ADD:
      tw->node_add();
      break;
    case NODE_DELETE:
      tw->node_delete();
      break;
    case NODE_INSERT:
      tw->node_insert();
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
    case TREE_OUTCOMES_ATTACH:
      tw->EditOutcomeAttach();
      break;
    case TREE_OUTCOMES_DETACH:
      tw->EditOutcomeDetach();
      break;
    case TREE_OUTCOMES_LABEL:
      tw->EditOutcomeLabel();
      break;
    case TREE_OUTCOMES_PAYOFFS:
      tw->ChangePayoffs();
      break;
    case TREE_OUTCOMES_NEW:
      tw->EditOutcomeNew();
      break;
    case TREE_OUTCOMES_DELETE:
      tw->EditOutcomeDelete();
      break;
    case TREE_PLAYERS:
      tw->tree_players();
      break;
    case TREE_INFOSETS:
      tw->tree_infosets();
      break;

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

    case FILE_OUTPUT:
      tw->output();
      break;
    case FILE_SAVE:
      tw->file_save();
      break;
    case FILE_CLOSE:
      GUI_RECORD("FILE:CLOSE");
      Close();
      break;

    case SUPPORTS_SUPPORTS: 
      ChangeSupport(CREATE_DIALOG);
      break;
    case SUPPORTS_ELIMDOM: 
      SolveElimDom();
      break;

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

    case INSPECT_SOLUTIONS: 
      InspectSolutions(CREATE_DIALOG);
      break;
    case INSPECT_CURSOR:
      node_inspect->Set(tw->Cursor());
      node_inspect->Show(TRUE);
      break;
    case INSPECT_INFOSETS:
      features.iset_hilight = !features.iset_hilight;
      if (!features.iset_hilight) {
	HilightInfoset(0, 0, 1);
	HilightInfoset(0, 0, 2);
      }
      GetMenuBar()->Check(m_inspectInfosetsItem, features.iset_hilight);
      break;
    case INSPECT_ZOOM_WIN:
      tw->display_zoom_win();
      break;
    case INSPECT_GAMEINFO: 
      ShowGameInfo();
      break;

    case SOLVE_CUSTOM_EFG_ENUMPURE:
    case SOLVE_CUSTOM_EFG_LCP:
    case SOLVE_CUSTOM_EFG_LP:
    case SOLVE_CUSTOM_EFG_LIAP:
    case SOLVE_CUSTOM_EFG_POLENUM:
    case SOLVE_CUSTOM_EFG_QRE:
    case SOLVE_CUSTOM_NFG_ENUMPURE:
    case SOLVE_CUSTOM_NFG_ENUMMIXED:
    case SOLVE_CUSTOM_NFG_LCP:
    case SOLVE_CUSTOM_NFG_LP:
    case SOLVE_CUSTOM_NFG_LIAP:
    case SOLVE_CUSTOM_NFG_SIMPDIV:
    case SOLVE_CUSTOM_NFG_POLENUM:
    case SOLVE_CUSTOM_NFG_QRE:
    case SOLVE_CUSTOM_NFG_QREGRID:
      GUI_RECORD("SOLVE:SOLVE");
      Solve(id);
      break;
    case SOLVE_NFG_REDUCED: 
      SolveNormalReduced();
      break;
    case SOLVE_NFG_AGENT:
      SolveNormalAgent();
      break;
    case SOLVE_STANDARD:
      SolveStandard();
      break;

#define ZOOM_DELTA  .2
    case DISPLAY_SET_ZOOM:
      tw->display_set_zoom();
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
    if (id != FILE_QUIT && id != FILE_CLOSE && id != TREE_OUTCOMES
	&& id != SUPPORTS_SUPPORTS) {
      tw->OnPaint();
      tw->SetFocus();
    }
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), this);
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
#include "elimdomd.h"
#include "efdom.h"

void EfgShow::SolveElimDom(void)
{
    ElimDomParamsDialog EDPD(ef.NumPlayers(), this);

    if (EDPD.Completed() == wxOK) {
      EFSupport *sup = cur_sup;
      wxStatus status(this, "Dominance Elimination");

      if (!EDPD.DomMixed()) {
	if (EDPD.FindAll()) {
	  while ((sup = SupportWithoutDominatedOfPlayerList(*sup, EDPD.DomStrong(), false,
						  EDPD.Players(), gnull, status)))
	    {
                    supports.Append(sup);
                }
            }
            else
            {
                if ((sup = SupportWithoutDominatedOfPlayerList(*sup, EDPD.DomStrong(), false, 
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


void EfgShow::SetFileName(void)
{
  SetTitle("[" + filename + "] " + ef.GetTitle());
}

void EfgShow::SetFileName(const gText &p_name)
{
  if (p_name != "")
    filename = p_name;
  else
    filename = "untitled.efg";

  SetTitle("[" + filename + "] " + ef.GetTitle());
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


void EfgShow::UpdateMenus(Node *p_cursor, Node *p_markNode)
{
  wxMenuBar *menuBar = GetMenuBar();
  menuBar->Enable(NODE_ADD, (p_cursor->NumChildren() > 0) ? FALSE : TRUE);
  menuBar->Enable(NODE_DELETE, (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(NODE_GOTO_MARK, (p_markNode) ? TRUE : FALSE);
  menuBar->Enable(INFOSET_MERGE, (p_markNode && p_markNode->GetInfoset() &&
				  p_cursor->GetInfoset() &&
				  p_markNode->GetSubgameRoot() == p_cursor->GetSubgameRoot() &&
				  p_markNode->GetPlayer() == p_cursor->GetPlayer()) ? TRUE : FALSE);
  menuBar->Enable(INFOSET_BREAK, (p_cursor->GetInfoset()) ? TRUE : FALSE);
  menuBar->Enable(INFOSET_SPLIT, (p_cursor->GetInfoset()) ? TRUE : FALSE);
  menuBar->Enable(INFOSET_JOIN, (p_markNode && p_markNode->GetInfoset() &&
				 p_cursor->GetInfoset() &&
				 p_markNode->GetSubgameRoot() == p_cursor->GetSubgameRoot()) ? TRUE : FALSE);
  menuBar->Enable(INFOSET_LABEL, (p_cursor->GetInfoset()) ? TRUE : FALSE);
  menuBar->Enable(INFOSET_SWITCH_PLAYER,
		  (p_cursor->GetInfoset()) ? TRUE : FALSE);
  menuBar->Enable(INFOSET_REVEAL, (p_cursor->GetInfoset()) ? TRUE : FALSE);

  menuBar->Enable(ACTION_LABEL,
		  (p_cursor->GetInfoset() &&
		   p_cursor->GetInfoset()->NumActions() > 0) ? TRUE : FALSE);
  menuBar->Enable(ACTION_INSERT, (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(ACTION_DELETE, (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(ACTION_PROBS,
		  (p_cursor->GetInfoset() &&
		   p_cursor->GetPlayer()->IsChance()) ? TRUE : FALSE);

  menuBar->Enable(TREE_DELETE, (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(TREE_COPY,
		  (p_markNode &&
		   p_cursor->GetSubgameRoot() == p_markNode->GetSubgameRoot()) ? TRUE : FALSE);
  menuBar->Enable(TREE_MOVE,
		  (p_markNode &&
		   p_cursor->GetSubgameRoot() == p_markNode->GetSubgameRoot()) ? TRUE : FALSE);

  menuBar->Enable(TREE_OUTCOMES_ATTACH,
		  (ef.NumOutcomes() > 0) ? TRUE : FALSE);
  menuBar->Enable(TREE_OUTCOMES_DETACH,
		  (p_cursor->GetOutcome()) ? TRUE : FALSE);
  menuBar->Enable(TREE_OUTCOMES_LABEL,
		  (p_cursor->GetOutcome()) ? TRUE : FALSE);
  menuBar->Enable(TREE_OUTCOMES_DELETE,
		  (ef.NumOutcomes() > 0) ? TRUE : FALSE);
}




#include "efggui.h"

//-------------------------------------------------------------------------
//                        EfgGUI: Member functions
//-------------------------------------------------------------------------

EfgGUI::EfgGUI(Efg *p_efg, const gText &p_filename,
               EfgNfgInterface *p_interface, wxFrame *p_parent)
{
  if (!p_efg) {
    // must create a new extensive form from scratch or file
    if (p_filename == "") {  // from scratch
      gArray<gText> names;

      if (GetEfgParams(names, p_parent)) {
	p_efg = new Efg;
	for (int i = 1; i <= names.Length(); i++)   
	  p_efg->NewPlayer()->SetName(names[i]);
      }
    }
    else {
      // from data file
      try {
	gFileInput infile(p_filename);
	ReadEfgFile(infile, p_efg);
                
	if (!p_efg) 
	  wxMessageBox(p_filename + " is not a valid .efg file");
      }
      catch (gFileInput::OpenFailed &) { 
	wxMessageBox("Could not open " + p_filename + " for reading");
	return;
      }
    }
  }

  EfgShow *efgShow = 0;

  if (p_efg) {
    efgShow = new EfgShow(*p_efg, p_interface, 1, p_parent);
  }

  if (efgShow) 
    efgShow->SetFileName(p_filename);
}


#define MAX_PLAYERS           100
#define NUM_PLAYERS_PER_LINE    8

int EfgGUI::GetEfgParams(gArray<gText> &names, wxFrame *parent)
{
  static int num_players = 2;

  // Get the number of players first
  MyDialogBox *make_ef_p = new MyDialogBox(parent,
					   "Extensive Form Parameters");
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

  for (int i = 1; i <= num_players; i++) {
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

  for (int i = 1; i <= num_players; i++) {
    names[i] = names_str[i];
    delete [] names_str[i];
  }

  delete [] names_str;
  
  return 1;
}

template class SolutionList<BehavSolution>;



// Gui playback code:

void EfgShow::ExecuteLoggedCommand(const gText& command,
                                   const gList<gText>& arglist)
{
#ifdef GUIPB_DEBUG
    printf("in EfgShow::ExecuteLoggedCommand...\n");
    printf("command: %s\n", (char *)command);
    
    for (int i = 1; i <= arglist.Length(); i++)
        printf("arglist[%d] = %s\n", i, (char *)arglist[i]);
#endif
    
    // FIXME! add commands.
    // FIXME! this has been changed since solve menu was rearranged!
    if (command == "SOLVE:SOLVE") {
      Solve(SOLVE_CUSTOM_EFG_LIAP);
    }
    else if (command == "FILE:CLOSE")
    {
        Close();
    }
    else
    {
        throw InvalidCommand();
    }
}

