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
    
  // Save, Print | Add, Delete, Outcomes | Solve, Inspect, MakeNF |
  // ZoomIn, ZoomOut, Options | Help
  // Create the toolbar
  SetMargins(2, 2);
#ifdef wx_msw
  SetDefaultSize(33, 30);
#endif  // wx_msw
  GetDC()->SetBackground(wxLIGHT_GREY_BRUSH);
    
  AddTool(efgmenuFILE_SAVE, ToolbarSaveBitmap);
  AddTool(efgmenuFILE_OUTPUT, ToolbarPrintBitmap);
  AddSeparator();
  AddTool(efgmenuEDIT_NODE_ADD, ToolbarAddBitmap);
  AddTool(efgmenuEDIT_NODE_DELETE, ToolbarDeleteBitmap);
  AddSeparator();
  AddTool(efgmenuSOLVE_STANDARD, ToolbarSolveBitmap);
  AddTool(efgmenuINSPECT_SOLUTIONS, ToolbarInspectBitmap);
  AddTool(efgmenuSOLVE_NFG_REDUCED, ToolbarMakenfBitmap);
  AddSeparator();
  AddTool(efgmenuPREFS_INC_ZOOM, ToolbarZoominBitmap);
  AddTool(efgmenuPREFS_DEC_ZOOM, ToolbarZoomoutBitmap);
  AddTool(efgmenuPREFS_DISPLAY, ToolbarOptionsBitmap);
  AddSeparator();
  AddTool(efgmenuHELP_CONTENTS, ToolbarHelpBitmap);
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
    parent(p_frame), ef(p_efg), cur_soln(0),
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

  ef.SetIsDirty(false);
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
      solver = new guiefgEnumPureEfg(*cur_sup, this);
      break;
    case EFG_LCP_SOLUTION:
      solver = new guiefgLcpEfg(*cur_sup, this);
      break;
    case EFG_CSUM_SOLUTION:
      solver = new guiefgLpEfg(*cur_sup, this);
      break;
    case EFG_LIAP_SOLUTION:
      solver = new guiefgLiapEfg(*cur_sup, this);
      break;
    case EFG_QRE_SOLUTION:
      solver = new guiefgQreEfg(*cur_sup, this);
      break;
    default:
      return;
    }
  }
  else {
    switch (ESS.GetNfgAlgorithm()) {
    case NFG_ENUMPURE_SOLUTION:
      solver = new guiefgEnumPureNfg(*cur_sup, this);
      break;
    case NFG_ENUMMIXED_SOLUTION:
      solver = new guiefgEnumMixedNfg(*cur_sup, this);
      break;
    case NFG_LCP_SOLUTION:
      solver = new guiefgLcpNfg(*cur_sup, this);
      break;
    case NFG_LP_SOLUTION:
      solver = new guiefgLpNfg(*cur_sup, this);
      break;
    case NFG_LIAP_SOLUTION:
      solver = new guiefgLiapNfg(*cur_sup, this);
      break;
    case NFG_SIMPDIV_SOLUTION:
      solver = new guiefgSimpdivNfg(*cur_sup, this);
      break;
    case NFG_QRE_SOLUTION:
      solver = new guiefgQreNfg(*cur_sup, this);
      break;
    case NFG_QREALL_SOLUTION:
      solver = new guiefgQreAllNfg(*cur_sup, this);
      break;
    default:
      return;
    }
  }

  try {
    if (ESS.MarkSubgames())  
      tw->SubgameMarkAll();
    else
      tw->SubgameUnmarkAll();

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
  if (!IsPerfectRecall(ef)) {
    int completed = wxMessageBox("This game is not perfect recall\n"
				 "Do you wish to continue?", 
				 "Solve Warning", 
				 wxOK | wxCANCEL | wxCENTRE, this);
    if (completed != wxOK) return;
  }
    
  // do not want users doing anything while solving
  Enable(FALSE);

  guiEfgSolution *solver = 0;

  switch (p_algorithm) {
  case efgmenuSOLVE_CUSTOM_EFG_ENUMPURE:
    solver = new guiefgEnumPureEfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_LCP:
    solver = new guiefgLcpEfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_LP:
    solver = new guiefgLpEfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_LIAP:
    solver = new guiefgLiapEfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_POLENUM:
    solver = new guiefgPolEnumEfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_QRE:
    solver = new guiefgQreEfg(*cur_sup, this);
    break;

  case efgmenuSOLVE_CUSTOM_NFG_ENUMPURE: 
    solver = new guiefgEnumPureNfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_ENUMMIXED:
    solver = new guiefgEnumMixedNfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_LCP: 
    solver = new guiefgLcpNfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_LP:
    solver = new guiefgLpNfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_LIAP: 
    solver = new guiefgLiapNfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_SIMPDIV:
    solver = new guiefgSimpdivNfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_POLENUM:
    solver = new guiefgPolEnumNfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_QRE:
    solver = new guiefgQreNfg(*cur_sup, this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_QREGRID: 
    solver = new guiefgQreAllNfg(*cur_sup, this);
    break;
  default:
    // internal error, we'll just ignore silently
    return;
  }

  bool go = solver->SolveSetup();
  
  try {
    if (go) {
      if (solver->MarkSubgames())
	tw->SubgameMarkAll();
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

gNumber EfgShow::ActionProb(const Node *n, int br)
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

wxFrame *EfgShow::Frame(void)
{
    return (wxFrame *)this;
}



void EfgShow::PickSolutions(const Efg &p_efg, Node *p_rootnode,
			    gList<BehavSolution> &p_solns)
{
  try {
    tw->SetSubgamePickNode(p_rootnode);
    BehavSolutionList temp_solns;
    temp_solns += p_solns;       
    EfgSolnPicker *pick = new EfgSolnPicker(p_efg, temp_solns,
					    tw->DrawSettings(),
					    sf_options, this);
    Enable(FALSE);
    while (pick->Completed() == wxRUNNING) wxYield();
    Enable(TRUE);
    p_solns = temp_solns; 
    delete pick;
    tw->SetSubgamePickNode(0);
  }
  catch (...) {
    tw->SetSubgamePickNode(0);
    throw;
  }
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
  wxMenu *file_menu = new wxMenu;
  file_menu->Append(efgmenuFILE_SAVE, "&Save", "Save the game");
  file_menu->Append(efgmenuFILE_OUTPUT, "&Output", "Print or copy the game");
  file_menu->Append(efgmenuFILE_CLOSE, "&Close", "Close this window");

  wxMenu *edit_menu = new wxMenu;
  wxMenu *nodeMenu  = new wxMenu;
  nodeMenu->Append(efgmenuEDIT_NODE_ADD, "&Add Move", "Add a move");
  nodeMenu->Append(efgmenuEDIT_NODE_DELETE, "&Delete Move", "Remove move at cursor");
  nodeMenu->Append(efgmenuEDIT_NODE_INSERT, "&Insert Move", "Insert move at cursor");
  nodeMenu->Append(efgmenuEDIT_NODE_LABEL,     "&Label",     "Label cursor node");
  nodeMenu->AppendSeparator();
  nodeMenu->Append(efgmenuEDIT_NODE_SET_MARK,  "Set &Mark",  "Mark cursor node");
  nodeMenu->Append(efgmenuEDIT_NODE_GOTO_MARK, "Go&to Mark", "Goto marked node");

  wxMenu *action_menu = new wxMenu;
  action_menu->Append(efgmenuEDIT_ACTION_DELETE, "&Delete", "Delete an action from cursor information set");
  action_menu->Append(efgmenuEDIT_ACTION_INSERT, "&Insert", "Insert an action in the cursor's information set");
  action_menu->Append(efgmenuEDIT_ACTION_APPEND, "&Append", "Append an action to the cursor's information set");
  action_menu->Append(efgmenuEDIT_ACTION_LABEL, "&Label", "Label the actions of the cursor's information set");
  action_menu->Append(efgmenuEDIT_ACTION_PROBS, "&Probabilities", "Set chance probabilities for the cursor's information set");

  wxMenu *infoset_menu = new wxMenu;
  infoset_menu->Append(efgmenuEDIT_INFOSET_MERGE,  "&Merge",  "Merge cursor iset w/ marked");
  infoset_menu->Append(efgmenuEDIT_INFOSET_BREAK,  "&Break",  "Make cursor a new iset");
  infoset_menu->Append(efgmenuEDIT_INFOSET_SPLIT,  "&Split",  "Split iset at cursor");
  infoset_menu->Append(efgmenuEDIT_INFOSET_JOIN,   "&Join",   "Join cursor to marked iset");
  infoset_menu->Append(efgmenuEDIT_INFOSET_LABEL,  "&Label",  "Label cursor iset & actions");
  infoset_menu->Append(efgmenuEDIT_INFOSET_PLAYER, "&Player", "Change player of cursor iset");
  infoset_menu->Append(efgmenuEDIT_INFOSET_REVEAL, "&Reveal", "Reveal infoset to players");

  wxMenu *outcome_menu = new wxMenu;
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_NEW, "&New",
		       "Create a new outcome");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_DELETE, "Dele&te",
		       "Delete an outcome");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_ATTACH, "&Attach",
		       "Attach an outcome to the node at cursor");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_DETACH, "&Detach",
		       "Detach the outcome from the node at cursor");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_LABEL, "&Label",
		       "Label the outcome at the node at cursor");
  outcome_menu->Append(efgmenuEDIT_OUTCOMES_PAYOFFS, "&Payoffs",
		       "Set the payoffs for the outcome at the cursor");

  wxMenu *tree_menu = new wxMenu;
  tree_menu->Append(efgmenuEDIT_TREE_COPY, "&Copy",
		    "Copy tree from marked node");
  tree_menu->Append(efgmenuEDIT_TREE_MOVE, "&Move",
		    "Move tree from marked node");
  tree_menu->Append(efgmenuEDIT_TREE_DELETE, "&Delete",
		    "Delete recursively from cursor");
  tree_menu->Append(efgmenuEDIT_TREE_LABEL, "&Label",
		    "Set the game label");
  tree_menu->Append(efgmenuEDIT_TREE_PLAYERS, "&Players",
		    "Edit/View players");
  tree_menu->Append(efgmenuEDIT_TREE_INFOSETS, "&Infosets",
		    "Edit/View infosets");

  edit_menu->Append(efgmenuEDIT_NODE, "&Node", nodeMenu, "Edit the node");
  edit_menu->Append(efgmenuEDIT_ACTIONS, "&Actions", action_menu, 
		    "Edit actions");
  edit_menu->Append(efgmenuEDIT_INFOSET, "&Infoset", infoset_menu,
		    "Edit infosets");
  edit_menu->Append(efgmenuEDIT_OUTCOMES, "&Outcomes", outcome_menu,
		    "Edit outcomes and payoffs");
  edit_menu->Append(efgmenuEDIT_TREE, "&Tree", tree_menu,
		    "Edit the tree");

  wxMenu *subgame_menu = new wxMenu;
  subgame_menu->Append(efgmenuSUBGAME_MARKALL, "Mark &All",
		       "Scan tree for subgames");
  subgame_menu->Append(efgmenuSUBGAME_MARK, "&Mark",
		       "Set node subgame root");
  subgame_menu->Append(efgmenuSUBGAME_UNMARKALL, "Unmark &All",
		       "Clear all subgame info");
  subgame_menu->Append(efgmenuSUBGAME_UNMARK, "&Unmark",
		       "Unmark node subgame");
  subgame_menu->Append(efgmenuSUBGAME_COLLAPSE, "Collapse &Level",
		       "Collapse node subgame");
  subgame_menu->Append(efgmenuSUBGAME_COLLAPSEALL, "&Collapse All", 
		       "Collapse all subgames");
  subgame_menu->Append(efgmenuSUBGAME_EXPAND, "&Expand Level",
		       "Expand node subgame");
  subgame_menu->Append(efgmenuSUBGAME_EXPANDBRANCH, "&Expand Branch", 
		       "Expand entire branch");
  subgame_menu->Append(efgmenuSUBGAME_EXPANDALL, "&Expand All",
		       "Expand all subgames");
  
  wxMenu *supports_menu = new wxMenu;
  supports_menu->Append(efgmenuSUPPORTS_ELIMDOM, "&ElimDom",
			"Compute dominated strategies");
  supports_menu->Append(efgmenuSUPPORTS_SELECT, "&Select",
			"Select and create supports");
  
  wxMenu *solve_menu = new wxMenu;
  solve_menu->Append(efgmenuSOLVE_STANDARD, "S&tandard...", "Standard solutions");

  wxMenu *solveCustomMenu = new wxMenu;
  wxMenu *solveCustomEfgMenu = new wxMenu;
  solveCustomEfgMenu->Append(efgmenuSOLVE_CUSTOM_EFG_ENUMPURE, "EnumPure",
			     "Enumerate pure strategy equilibria");
  solveCustomEfgMenu->Append(efgmenuSOLVE_CUSTOM_EFG_LCP, "LCP",
			     "Solve by linear complementarity program");
  solveCustomEfgMenu->Append(efgmenuSOLVE_CUSTOM_EFG_LP, "LP",
			     "Solve by linear program");
  solveCustomEfgMenu->Append(efgmenuSOLVE_CUSTOM_EFG_LIAP, "Liapunov",
			     "Liapunov function minimization");
  solveCustomEfgMenu->Append(efgmenuSOLVE_CUSTOM_EFG_POLENUM, "PolEnum",
			     "Enumeration by systems of polynomials");
  // FIXME: This item currently disabled since algorithm not implemented yet
  solveCustomEfgMenu->Enable(efgmenuSOLVE_CUSTOM_EFG_POLENUM, FALSE);
  solveCustomEfgMenu->Append(efgmenuSOLVE_CUSTOM_EFG_QRE, "QRE",
			     "Compute quantal response equilibria");
  solveCustomMenu->Append(efgmenuSOLVE_CUSTOM_EFG, "Extensive form",
			  solveCustomEfgMenu,
			  "Solve using extensive form based algorithms");

  wxMenu *solveCustomNfgMenu = new wxMenu;
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_ENUMPURE, "EnumPure",
			     "Enumerate pure strategy equilibria");
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_ENUMMIXED, "EnumMixed",
			     "Enumerate all equilibria");
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_LCP, "LCP",
			     "Solve by linear complementarity program");
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_LP, "LP",
			     "Solve by linear program");
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_LIAP, "Liapunov",
			     "Liapunov function minimization");
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_SIMPDIV, "Simpdiv",
			     "Simplicial subdivision");
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_POLENUM, "PolEnum",
			     "Enumeration by systems of polynomials");
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_QRE, "QRE",
			     "Compute quantal response equilibria");
  solveCustomNfgMenu->Append(efgmenuSOLVE_CUSTOM_NFG_QREGRID, "QRE Grid",
			     "Compute quantal response equilibria");
  solveCustomMenu->Append(efgmenuSOLVE_CUSTOM_NFG, "Normal form",
              solveCustomNfgMenu,
              "Solve using normal form based algorithms");

  solve_menu->Append(efgmenuSOLVE_CUSTOM, "Custom", solveCustomMenu,
             "Select a specific algorithm");

  wxMenu *solveNfgMenu = new wxMenu;
  solveNfgMenu->Append(efgmenuSOLVE_NFG_REDUCED, "Reduced",
               "Generate reduced normal form");
  solveNfgMenu->Append(efgmenuSOLVE_NFG_AGENT, "Agent",
               "Generate agent normal form");
  solve_menu->Append(efgmenuSOLVE_NFG, "Normal form", solveNfgMenu,
             "Create a normal form representation of this game");
  
  wxMenu *inspect_menu = new wxMenu;
  inspect_menu->Append(efgmenuINSPECT_SOLUTIONS, "&Solutions",
		       "Inspect existing solutions");
  inspect_menu->Append(efgmenuINSPECT_CURSOR, "&Cursor",
		       "Information about the node at cursor");
  inspect_menu->Append(efgmenuINSPECT_INFOSETS, "&Infosets",
		       "Inspect information sets", TRUE);
  inspect_menu->Append(efgmenuINSPECT_ZOOM_WIN, "Zoom &Window",
		       "Open zoom-in window");
  inspect_menu->AppendSeparator();
  inspect_menu->Append(efgmenuINSPECT_GAMEINFO, "Game&Info",
		       "Information about this game");
  
  wxMenu *prefs_menu = new wxMenu;
  prefs_menu->Append(efgmenuPREFS_SET_ZOOM, "&Zoom",
		     "Specify zoom level");
  prefs_menu->Append(efgmenuPREFS_DISPLAY, "&Display",
		     "Set display options");
  prefs_menu->Append(efgmenuPREFS_LEGEND, "&Legend",
		     "Set legends");
  prefs_menu->Append(efgmenuPREFS_COLORS, "&Colors",
		     "Set player colors");
  prefs_menu->Append(efgmenuPREFS_ACCELS, "&Accels",
		     "Edit accelerator keys");
  prefs_menu->AppendSeparator();
  prefs_menu->Append(efgmenuPREFS_SAVE_DEFAULT, "Save Default");
  prefs_menu->Append(efgmenuPREFS_LOAD_DEFAULT, "Load Default");
  prefs_menu->Append(efgmenuPREFS_SAVE_CUSTOM,  "Save Custom");
  prefs_menu->Append(efgmenuPREFS_LOAD_CUSTOM,  "Load Custom");
  
  wxMenu *help_menu = new wxMenu;
  help_menu->Append(efgmenuHELP_CONTENTS, "&Contents", "Table of contents");
  help_menu->Append(efgmenuHELP_ABOUT, "&About", "About this program");

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

void EfgShow::OnMenuCommand(int id)
{
  const double ZOOM_DELTA = 0.2;

  try {
    switch (id) {
    case efgmenuFILE_OUTPUT:
      tw->output();
      break;
    case efgmenuFILE_SAVE:
      tw->file_save();
      break;
    case efgmenuFILE_CLOSE:
      GUI_RECORD("FILE:CLOSE");
      Close();
      break;

    case efgmenuEDIT_NODE_ADD:
      tw->node_add();
      break;
    case efgmenuEDIT_NODE_DELETE:
      tw->node_delete();
      break;
    case efgmenuEDIT_NODE_INSERT:
      tw->node_insert();
      break;
    case efgmenuEDIT_NODE_LABEL:
      tw->node_label();
      break;
    case efgmenuEDIT_NODE_SET_MARK:
      tw->node_set_mark();
      break;
    case efgmenuEDIT_NODE_GOTO_MARK:
      tw->node_goto_mark();
      break;

    case efgmenuEDIT_ACTION_DELETE:
      tw->action_delete();
      break;
    case efgmenuEDIT_ACTION_INSERT:
      tw->action_insert();
      break;
    case efgmenuEDIT_ACTION_APPEND:
      tw->action_append();
      break;
    case efgmenuEDIT_ACTION_LABEL:
      tw->action_label();
      break;
    case efgmenuEDIT_ACTION_PROBS:
      tw->action_probs();
      break;

    case efgmenuEDIT_INFOSET_MERGE:
      tw->infoset_merge();
      break;
    case efgmenuEDIT_INFOSET_BREAK:
      tw->infoset_break();
      break;
    case efgmenuEDIT_INFOSET_SPLIT:
      tw->infoset_split();
      break;
    case efgmenuEDIT_INFOSET_JOIN:
      tw->infoset_join();
      break;
    case efgmenuEDIT_INFOSET_LABEL:
      tw->infoset_label();
      break;
    case efgmenuEDIT_INFOSET_PLAYER:
      tw->infoset_switch_player();
      break;
    case efgmenuEDIT_INFOSET_REVEAL:
      tw->infoset_reveal();
      break;

    case efgmenuEDIT_OUTCOMES_ATTACH:
      tw->EditOutcomeAttach();
      break;
    case efgmenuEDIT_OUTCOMES_DETACH:
      tw->EditOutcomeDetach();
      break;
    case efgmenuEDIT_OUTCOMES_LABEL:
      tw->EditOutcomeLabel();
      break;
    case efgmenuEDIT_OUTCOMES_PAYOFFS:
      tw->EditOutcomePayoffs();
      break;
    case efgmenuEDIT_OUTCOMES_NEW:
      tw->EditOutcomeNew();
      break;
    case efgmenuEDIT_OUTCOMES_DELETE:
      tw->EditOutcomeDelete();
      break;

    case efgmenuEDIT_TREE_DELETE:
      tw->tree_delete();
      break;
    case efgmenuEDIT_TREE_COPY:
      tw->tree_copy();
      break;
    case efgmenuEDIT_TREE_MOVE:
      tw->tree_move();
      break;
    case efgmenuEDIT_TREE_LABEL:
      tw->tree_label();
      SetFileName();
      break;
    case efgmenuEDIT_TREE_PLAYERS:
      tw->tree_players();
      break;
    case efgmenuEDIT_TREE_INFOSETS:
      tw->tree_infosets();
      break;

    case efgmenuSUBGAME_MARKALL:
      tw->SubgameMarkAll();
      break;
    case efgmenuSUBGAME_MARK:
      tw->SubgameMark();
      break;
    case efgmenuSUBGAME_UNMARKALL:
      tw->SubgameUnmarkAll();
      break;
    case efgmenuSUBGAME_UNMARK:
      tw->SubgameUnmark();
      break;
    case efgmenuSUBGAME_COLLAPSEALL:
      tw->SubgameCollapseAll();
      break;
    case efgmenuSUBGAME_COLLAPSE:
      tw->SubgameCollapse();
      break;
    case efgmenuSUBGAME_EXPANDALL:
      tw->SubgameExpandAll();
      break;
    case efgmenuSUBGAME_EXPANDBRANCH:
      tw->SubgameExpandBranch();
      break;
    case efgmenuSUBGAME_EXPAND:
      tw->SubgameExpand();
      break;

    case efgmenuSUPPORTS_ELIMDOM: 
      SolveElimDom();
      break;
    case efgmenuSUPPORTS_SELECT:
      ChangeSupport(CREATE_DIALOG);
      break;

    case efgmenuINSPECT_SOLUTIONS: 
      InspectSolutions(CREATE_DIALOG);
      break;
    case efgmenuINSPECT_CURSOR:
      node_inspect->Set(tw->Cursor());
      node_inspect->Show(TRUE);
      break;
    case efgmenuINSPECT_INFOSETS:
      features.iset_hilight = !features.iset_hilight;
      if (!features.iset_hilight) {
	HilightInfoset(0, 0, 1);
	HilightInfoset(0, 0, 2);
      }
      GetMenuBar()->Check(efgmenuINSPECT_INFOSETS, features.iset_hilight);
      break;
    case efgmenuINSPECT_ZOOM_WIN:
      tw->display_zoom_win();
      break;
    case efgmenuINSPECT_GAMEINFO: 
      ShowGameInfo();
      break;

    case efgmenuSOLVE_CUSTOM_EFG_ENUMPURE:
    case efgmenuSOLVE_CUSTOM_EFG_LCP:
    case efgmenuSOLVE_CUSTOM_EFG_LP:
    case efgmenuSOLVE_CUSTOM_EFG_LIAP:
    case efgmenuSOLVE_CUSTOM_EFG_POLENUM:
    case efgmenuSOLVE_CUSTOM_EFG_QRE:
    case efgmenuSOLVE_CUSTOM_NFG_ENUMPURE:
    case efgmenuSOLVE_CUSTOM_NFG_ENUMMIXED:
    case efgmenuSOLVE_CUSTOM_NFG_LCP:
    case efgmenuSOLVE_CUSTOM_NFG_LP:
    case efgmenuSOLVE_CUSTOM_NFG_LIAP:
    case efgmenuSOLVE_CUSTOM_NFG_SIMPDIV:
    case efgmenuSOLVE_CUSTOM_NFG_POLENUM:
    case efgmenuSOLVE_CUSTOM_NFG_QRE:
    case efgmenuSOLVE_CUSTOM_NFG_QREGRID:
      GUI_RECORD("SOLVE:SOLVE");
      Solve(id);
      break;
    case efgmenuSOLVE_NFG_REDUCED: 
      SolveNormalReduced();
      break;
    case efgmenuSOLVE_NFG_AGENT:
      SolveNormalAgent();
      break;
    case efgmenuSOLVE_STANDARD:
      SolveStandard();
      break;

    case efgmenuPREFS_SET_ZOOM:
      tw->display_set_zoom();
      break;
    case efgmenuPREFS_INC_ZOOM:
      tw->display_set_zoom(tw->display_get_zoom() + ZOOM_DELTA);
      break;
    case efgmenuPREFS_DEC_ZOOM:
      tw->display_set_zoom(tw->display_get_zoom() - ZOOM_DELTA);
      break;
    case efgmenuPREFS_LEGEND:
      tw->display_legends();
      break;
    case efgmenuPREFS_DISPLAY:
      tw->display_options();
      break;
    case efgmenuPREFS_COLORS:
      tw->display_colors();
      break;
    case efgmenuPREFS_SAVE_DEFAULT:
      tw->display_save_options();
      break;
    case efgmenuPREFS_LOAD_DEFAULT:
      tw->display_load_options();
      break;
    case efgmenuPREFS_SAVE_CUSTOM:
      tw->display_save_options(FALSE);
      break;
    case efgmenuPREFS_LOAD_CUSTOM:
      tw->display_load_options(FALSE);
      break;
    case efgmenuPREFS_REDRAW: 
      /* redraws automatically after switch */ 
      break;
    case efgmenuPREFS_ACCELS:
      EditAccelerators(accelerators, MakeEventNames());
      WriteAccelerators(accelerators, "EfgAccelerators");
      break;

    case efgmenuHELP_ABOUT:
      wxHelpAbout();
      break;
    case efgmenuHELP_CONTENTS:
      wxHelpContents(EFG_GUI_HELP);
      break;
    default:
      parent->OnMenuCommand(id);
      break;
    }

    // Most menu selections modify the display somehow, so redraw w/ exceptions
    if (id != efgmenuFILE_CLOSE && id != efgmenuSUPPORTS_SELECT) {
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
							  EDPD.Players(), gnull, status)) != 0) {
	  supports.Append(sup);
	}
      }
      else {
	if ((sup = SupportWithoutDominatedOfPlayerList(*sup, EDPD.DomStrong(), false, 
						       EDPD.Players(), gnull, status)) != 0) {
	  supports.Append(sup);
	}
      }
    }
    else {
      wxMessageBox("Mixed dominance is not implemented for\n"
		   "Extensive form games");
      }
    
    if (EDPD.Compress() && disp_sup != sup) {
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
  menuBar->Enable(efgmenuEDIT_NODE_ADD,
		  (p_cursor->NumChildren() > 0) ? FALSE : TRUE);
  menuBar->Enable(efgmenuEDIT_NODE_DELETE,
		  (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_NODE_GOTO_MARK, (p_markNode) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_INFOSET_MERGE,
		  (p_markNode && p_markNode->GetInfoset() &&
		   p_cursor->GetInfoset() &&
		   p_markNode->GetSubgameRoot() == p_cursor->GetSubgameRoot() &&
		   p_markNode->GetPlayer() == p_cursor->GetPlayer()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_INFOSET_BREAK, 
		  (p_cursor->GetInfoset()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_INFOSET_SPLIT,
		  (p_cursor->GetInfoset()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_INFOSET_JOIN, 
		  (p_markNode && p_markNode->GetInfoset() &&
		   p_cursor->GetInfoset() &&
		   p_markNode->GetSubgameRoot() == p_cursor->GetSubgameRoot()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_INFOSET_LABEL,
		  (p_cursor->GetInfoset()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_INFOSET_PLAYER,
		  (p_cursor->GetInfoset() &&
		   !p_cursor->GetPlayer()->IsChance()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_INFOSET_REVEAL, 
		  (p_cursor->GetInfoset()) ? TRUE : FALSE);

  menuBar->Enable(efgmenuEDIT_ACTION_LABEL,
		  (p_cursor->GetInfoset() &&
		   p_cursor->GetInfoset()->NumActions() > 0) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_ACTION_INSERT,
		  (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_ACTION_APPEND,
		  (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_ACTION_DELETE, 
		  (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_ACTION_PROBS,
		  (p_cursor->GetInfoset() &&
		   p_cursor->GetPlayer()->IsChance()) ? TRUE : FALSE);

  menuBar->Enable(efgmenuEDIT_TREE_DELETE,
		  (p_cursor->NumChildren() > 0) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_TREE_COPY,
		  (p_markNode &&
		   p_cursor->GetSubgameRoot() == p_markNode->GetSubgameRoot()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_TREE_MOVE,
		  (p_markNode &&
		   p_cursor->GetSubgameRoot() == p_markNode->GetSubgameRoot()) ? TRUE : FALSE);

  menuBar->Enable(efgmenuEDIT_OUTCOMES_ATTACH,
		  (ef.NumOutcomes() > 0) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_OUTCOMES_DETACH,
		  (p_cursor->GetOutcome()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_OUTCOMES_LABEL,
		  (p_cursor->GetOutcome()) ? TRUE : FALSE);
  menuBar->Enable(efgmenuEDIT_OUTCOMES_DELETE,
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
#ifdef GUIPB_DEBUG
                                   const gList<gText>& arglist)
#else
                                   const gList<gText>& /*arglist*/)
#endif  // GUIPB_DEBUG
{
#ifdef GUIPB_DEBUG
    printf("in EfgShow::ExecuteLoggedCommand...\n");
    printf("command: %s\n", (char *)command);
    
    for (int i = 1; i <= arglist.Length(); i++)
        printf("arglist[%d] = %s\n", i, (char *)arglist[i]);
#endif  // GUIPB_DEBUG
    
    // FIXME! add commands.
    // FIXME! this has been changed since solve menu was rearranged!
    if (command == "SOLVE:SOLVE") {
      Solve(efgmenuSOLVE_CUSTOM_EFG_LIAP);
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

