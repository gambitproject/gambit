//
// FILE: efgshow.cc -- type dependent extensive form gui stuff
//
//  $Id$
//

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


//=====================================================================
//                   EfgShow MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//             EfgShow: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------
void Nodes (const Efg &befg, gList <Node *> &list);

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

EfgShow::EfgShow(Efg &ef_, EfgNfgInterface *nfg, int , wxFrame *frame,
                 const char *title, int x, int y, int w, int h, int type)
    : wxFrame(frame, (char *)title, x, y, w, h, type), 
      EfgNfgInterface(gEFG, nfg), 
      GuiObject(gText("EfgShow")),
      parent(frame), ef(ef_), tw(0)
{
    Show(FALSE);


    //--------------Define all the menus----------------------------
    // Give the frame an icon
    wxIcon *frame_icon;

#ifdef wx_msw
    frame_icon = new wxIcon("efg_icn");
#else
#include "efg.xbm"
    frame_icon = new wxIcon(efg_bits, efg_width, efg_height);
#endif

    SetIcon(frame_icon);
    // No support dialog yet
    support_dialog = 0;
    outcome_dialog = 0;
    // No solution inspect window yet
    soln_show = 0;
    // Create the status bar
    CreateStatusLine();
    // Create the menu bar
    MakeMenus();
    // Create the accelerators (to add an accelerator, see const.h)
    ReadAccelerators(accelerators, "EfgAccelerators");
    
    // Create the canvas(TreeWindow) on which to draw the tree
    tw = new TreeWindow(ef, disp_sup, this);
    // Create the toolbar (must be after the status bar creation)
    toolbar = new EfgShowToolBar(this);
    // Create the all_nodes list.
    Nodes(ef, all_nodes);
    // Now take care of the solution stuff
    cur_soln = 0;
    node_inspect = 0;
    OnSize(-1, -1);
    // now zoom in/out to show the full tree
    tw->display_zoom_fit();
    
    Show(TRUE);
}


// Destructor
EfgShow::~EfgShow(void)
{
    Show(FALSE); delete &ef; delete toolbar; toolbar = 0; delete tw;
}

#include "elimdomd.h"

void EfgShow::NodeInspect(bool insp)
{
    if (insp && !node_inspect)
    {
        node_inspect = new NodeSolnShow(ef.NumPlayers(), this);
        node_inspect->Set(tw->Cursor());
    }
    else
    {
        delete node_inspect;
        node_inspect = 0;
    }
}


void EfgShow::OnSelectedMoved(const Node *n)
{
    if (features.node_inspect) node_inspect->Set(n);
}

//*******************************************************************
//                          SOLUTION ROUTINES                       *
//*******************************************************************

#include "efgsolvd.h"

void EfgShow::SolveStandard(void)
{
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
  if (ESS.AutoInspect()) InspectSolutions(CREATE_DIALOG);
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

        soln_show = new EfgSolnShow(ef, solns, cur_soln, tw->DrawSettings(), sf_options, this);
    }

    if (what == DESTROY_DIALOG && soln_show)
    {
        soln_show = 0;
    }
}


void EfgShow::ChangeSolution(int sol)
{
    if (cur_soln != sol)
    {
        cur_soln = sol;
        tw->OnPaint();
        if (features.node_inspect) node_inspect->Set(tw->Cursor());
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
				 "Efg -> Nfg", 
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
				 "Efg -> Nfg", 
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

template class SolutionList<BehavSolution>;
//***************************************************************************
//                EXTENSIVE FORM TOOLBAR
//***************************************************************************

EfgShowToolBar::EfgShowToolBar(wxFrame *frame):
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
#include "bitmaps/delete.xpm"
#include "bitmaps/solve.xpm"
#include "bitmaps/zoomin.xpm"
#include "bitmaps/zoomout.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/add.xpm"
#include "bitmaps/options.xpm"
#include "bitmaps/makenf.xpm"
#include "bitmaps/inspect.xpm"
#include "bitmaps/payoff.xpm"
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
    wxBitmap *ToolbarPayoffBitmap = new wxBitmap(payoff_xpm);
    
    // Save, Print | Add, Delete, Outcomes | Solve, Inspect, MakeNF | ZoomIn, ZoomOut, Options | Help
    // Create the toolbar
    SetMargins(2, 2);
#ifdef wx_msw
    SetDefaultSize(33, 30);
#endif
    GetDC()->SetBackground(wxLIGHT_GREY_BRUSH);
    
    AddTool(FILE_SAVE, ToolbarSaveBitmap);
    AddTool(FILE_OUTPUT, ToolbarPrintBitmap);
    AddSeparator();
    AddTool(NODE_ADD, ToolbarAddBitmap);
    AddTool(NODE_DELETE, ToolbarDeleteBitmap);
    AddTool(TREE_OUTCOMES, ToolbarPayoffBitmap);
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

Bool EfgShowToolBar::OnLeftClick(int tool, Bool )
{
    parent->OnMenuCommand(tool); return TRUE;
}

void EfgShowToolBar::OnMouseEnter(int tool)
{
    parent->SetStatusText(parent->GetMenuBar()->GetHelpString(tool));
}


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

