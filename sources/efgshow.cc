//
// FILE: efgshow.cc -- Implementation of class EfgShow
//
// $Id$
//

#include "wx/wx.h"
#include "wx/notebook.h"
#include "wx/fontdlg.h"
#include "wx/colordlg.h"
#include "wx/printdlg.h"
#include "wx/wizard.h"
#include "wxmisc.h"
#include "wxstatus.h"

#include "math/math.h"
#include "efg.h"
#include "infoset.h"
#include "node.h"
#include "efplayer.h"
#include "efgutils.h"
#include "behavsol.h"
#include "efdom.h"
#include "nfg.h"

#include "efgconst.h"
#include "treewin.h"
#include "treezoom.h"
#include "efgprint.h"
#include "efgshow.h"
#include "efgprofile.h"
#include "efgcursor.h"
#include "efgsoln.h"
#include "efgsolng.h"
#include "nfgshow.h"
#include "efgsolvd.h"

#include "dlefgsave.h"
#include "dlmoveadd.h"
#include "dlnodedelete.h"
#include "dlefgplayer.h"
#include "dlefgoutcome.h"
#include "dlefgpayoff.h"
#include "dlefgreveal.h"
#include "dlactionselect.h"
#include "dlactionlabel.h"
#include "dlactionprobs.h"
#include "dlefgplayers.h"
#include "dlinfosets.h"

#include "dllayout.h"
#include "dllegends.h"

#include "dlelim.h"
#include "dlsupportselect.h"
#include "dlefgeditsupport.h"

#include "behavedit.h"

//=====================================================================
//                     EfgShow MEMBER FUNCTIONS
//=====================================================================

const int idTREEWINDOW = 999;
const int idNODEWINDOW = 998;
const int idTOOLWINDOW = 997;
const int idSOLUTIONWINDOW = 996;

BEGIN_EVENT_TABLE(EfgShow, wxFrame)
  EVT_MENU(efgmenuFILE_SAVE, EfgShow::OnFileSave)
  EVT_MENU(efgmenuFILE_PAGE_SETUP, EfgShow::OnFilePageSetup)
  EVT_MENU(efgmenuFILE_PRINT_PREVIEW, EfgShow::OnFilePrintPreview)
  EVT_MENU(efgmenuFILE_PRINT, EfgShow::OnFilePrint)
  EVT_MENU(efgmenuFILE_CLOSE, EfgShow::Close)
  EVT_MENU(efgmenuEDIT_NODE_ADD, EfgShow::OnEditNodeAdd)
  EVT_MENU(efgmenuEDIT_NODE_DELETE, EfgShow::OnEditNodeDelete)
  EVT_MENU(efgmenuEDIT_NODE_INSERT, EfgShow::OnEditNodeInsert)
  EVT_MENU(efgmenuEDIT_NODE_LABEL, EfgShow::OnEditNodeLabel)
  EVT_MENU(efgmenuEDIT_NODE_SET_MARK, EfgShow::OnEditNodeSetMark)
  EVT_MENU(efgmenuEDIT_NODE_GOTO_MARK, EfgShow::OnEditNodeGotoMark)
  EVT_MENU(efgmenuEDIT_ACTION_DELETE, EfgShow::OnEditActionDelete)
  EVT_MENU(efgmenuEDIT_ACTION_INSERT, EfgShow::OnEditActionInsert)
  EVT_MENU(efgmenuEDIT_ACTION_APPEND, EfgShow::OnEditActionAppend)
  EVT_MENU(efgmenuEDIT_ACTION_LABEL, EfgShow::OnEditActionLabel)
  EVT_MENU(efgmenuEDIT_ACTION_PROBS, EfgShow::OnEditActionProbs)
  EVT_MENU(efgmenuEDIT_INFOSET_MERGE, EfgShow::OnEditInfosetMerge)
  EVT_MENU(efgmenuEDIT_INFOSET_BREAK, EfgShow::OnEditInfosetBreak)
  EVT_MENU(efgmenuEDIT_INFOSET_SPLIT, EfgShow::OnEditInfosetSplit)
  EVT_MENU(efgmenuEDIT_INFOSET_JOIN, EfgShow::OnEditInfosetJoin)
  EVT_MENU(efgmenuEDIT_INFOSET_LABEL, EfgShow::OnEditInfosetLabel)
  EVT_MENU(efgmenuEDIT_INFOSET_PLAYER, EfgShow::OnEditInfosetPlayer)
  EVT_MENU(efgmenuEDIT_INFOSET_REVEAL, EfgShow::OnEditInfosetReveal)
  EVT_MENU(efgmenuEDIT_OUTCOMES_ATTACH, EfgShow::OnEditOutcomesAttach)
  EVT_MENU(efgmenuEDIT_OUTCOMES_DETACH, EfgShow::OnEditOutcomesDetach)
  EVT_MENU(efgmenuEDIT_OUTCOMES_LABEL, EfgShow::OnEditOutcomesLabel)
  EVT_MENU(efgmenuEDIT_OUTCOMES_PAYOFFS, EfgShow::OnEditOutcomesPayoffs)
  EVT_MENU(efgmenuEDIT_OUTCOMES_NEW, EfgShow::OnEditOutcomesNew)
  EVT_MENU(efgmenuEDIT_OUTCOMES_DELETE, EfgShow::OnEditOutcomesDelete)
  EVT_MENU(efgmenuEDIT_TREE_DELETE, EfgShow::OnEditTreeDelete)
  EVT_MENU(efgmenuEDIT_TREE_COPY, EfgShow::OnEditTreeCopy)
  EVT_MENU(efgmenuEDIT_TREE_MOVE, EfgShow::OnEditTreeMove)
  EVT_MENU(efgmenuEDIT_TREE_LABEL, EfgShow::OnEditTreeLabel)
  EVT_MENU(efgmenuEDIT_TREE_PLAYERS, EfgShow::OnEditTreePlayers)
  EVT_MENU(efgmenuEDIT_TREE_INFOSETS, EfgShow::OnEditTreeInfosets)
  EVT_MENU(efgmenuSUBGAME_MARKALL, EfgShow::OnSubgamesMarkAll)
  EVT_MENU(efgmenuSUBGAME_MARK, EfgShow::OnSubgamesMark)
  EVT_MENU(efgmenuSUBGAME_UNMARKALL, EfgShow::OnSubgamesUnMarkAll)
  EVT_MENU(efgmenuSUBGAME_UNMARK, EfgShow::OnSubgamesUnMark)
  EVT_MENU(efgmenuSUBGAME_COLLAPSEALL, EfgShow::OnSubgamesCollapseAll) 
  EVT_MENU(efgmenuSUBGAME_COLLAPSE, EfgShow::OnSubgamesCollapse)
  EVT_MENU(efgmenuSUBGAME_EXPANDALL, EfgShow::OnSubgamesExpandAll)
  EVT_MENU(efgmenuSUBGAME_EXPANDBRANCH, EfgShow::OnSubgamesExpandBranch)
  EVT_MENU(efgmenuSUBGAME_EXPAND, EfgShow::OnSubgamesExpand)
  EVT_MENU(efgmenuSUPPORT_UNDOMINATED, EfgShow::OnSupportUndominated)
  EVT_MENU(efgmenuSUPPORT_NEW, EfgShow::OnSupportNew)
  EVT_MENU(efgmenuSUPPORT_EDIT, EfgShow::OnSupportEdit)
  EVT_MENU(efgmenuSUPPORT_DELETE, EfgShow::OnSupportDelete)
  EVT_MENU(efgmenuSUPPORT_SELECT_FROMLIST, EfgShow::OnSupportSelectFromList)
  EVT_MENU(efgmenuSUPPORT_SELECT_PREVIOUS, EfgShow::OnSupportSelectPrevious)
  EVT_MENU(efgmenuSUPPORT_SELECT_NEXT, EfgShow::OnSupportSelectNext)
  EVT_MENU(efgmenuSUPPORT_REACHABLE, EfgShow::OnSupportReachable)
  EVT_MENU(efgmenuSOLVE_STANDARD, EfgShow::OnSolveStandard)
  EVT_MENU(efgmenuSOLVE_CUSTOM_EFG_ENUMPURE, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_EFG_LCP, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_EFG_LP, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_EFG_LIAP, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_EFG_POLENUM, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_EFG_QRE, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_ENUMPURE, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_ENUMMIXED, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_LCP, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_LP, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_LIAP, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_SIMPDIV, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_POLENUM, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_QRE, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_CUSTOM_NFG_QREGRID, EfgShow::OnSolveCustom)
  EVT_MENU(efgmenuSOLVE_NFG_REDUCED, EfgShow::OnSolveNormalReduced)
  EVT_MENU(efgmenuSOLVE_NFG_AGENT, EfgShow::OnSolveNormalAgent)
  EVT_MENU(efgmenuSOLVE_WIZARD, EfgShow::OnSolveWizard)
  EVT_MENU(efgmenuINSPECT_SOLUTIONS, EfgShow::OnInspectSolutions)
  EVT_MENU(efgmenuINSPECT_CURSOR, EfgShow::OnInspectCursor)
  EVT_MENU(efgmenuINSPECT_INFOSETS, EfgShow::OnInspectInfosets)
  EVT_MENU(efgmenuINSPECT_ZOOM_WIN, EfgShow::OnInspectZoom)
  EVT_MENU(efgmenuINSPECT_GAMEINFO, EfgShow::OnInspectGameInfo)
  EVT_MENU(efgmenuINSPECT_SCRIPT, EfgShow::OnInspectScript)
  EVT_MENU(efgmenuPREFS_ZOOMIN, EfgShow::OnPrefsZoomIn)
  EVT_MENU(efgmenuPREFS_ZOOMOUT, EfgShow::OnPrefsZoomOut)
  EVT_MENU(efgmenuPREFS_LEGEND, EfgShow::OnPrefsLegend)
  EVT_MENU(efgmenuPREFS_FONTS_ABOVENODE, EfgShow::OnPrefsFontsAboveNode)
  EVT_MENU(efgmenuPREFS_FONTS_BELOWNODE, EfgShow::OnPrefsFontsBelowNode)
  EVT_MENU(efgmenuPREFS_FONTS_AFTERNODE, EfgShow::OnPrefsFontsAfterNode)
  EVT_MENU(efgmenuPREFS_FONTS_ABOVEBRANCH, EfgShow::OnPrefsFontsAboveBranch)
  EVT_MENU(efgmenuPREFS_FONTS_BELOWBRANCH, EfgShow::OnPrefsFontsBelowBranch)
  EVT_MENU(efgmenuPREFS_DISPLAY_LAYOUT, EfgShow::OnPrefsDisplayLayout)
  EVT_MENU(efgmenuPREFS_DISPLAY_DECIMALS, EfgShow::OnPrefsDisplayDecimals)
  EVT_MENU(efgmenuPREFS_COLORS, EfgShow::OnPrefsColors)
  EVT_MENU(efgmenuPREFS_SAVE, EfgShow::OnPrefsSave)
  EVT_MENU(efgmenuPREFS_LOAD, EfgShow::OnPrefsLoad)
  EVT_MENU(efgmenuPROFILES_NEW, EfgShow::OnProfilesNew)
  EVT_MENU(efgmenuPROFILES_CLONE, EfgShow::OnProfilesClone)
  EVT_MENU(efgmenuPROFILES_RENAME, EfgShow::OnProfilesRename)
  EVT_MENU(efgmenuPROFILES_EDIT, EfgShow::OnProfilesEdit)
  EVT_LIST_ITEM_ACTIVATED(idEFG_SOLUTION_LIST, EfgShow::OnProfilesEdit)
  EVT_MENU(efgmenuPROFILES_DELETE, EfgShow::OnProfilesDelete)
  EVT_SET_FOCUS(EfgShow::OnFocus)
  EVT_SIZE(EfgShow::OnSize)
  EVT_CLOSE(EfgShow::OnCloseWindow)
  EVT_SASH_DRAGGED_RANGE(idSOLUTIONWINDOW, idTREEWINDOW, EfgShow::OnSashDrag)
  EVT_ACTIVATE(EfgShow::OnActivate)
END_EVENT_TABLE()

//---------------------------------------------------------------------
//               EfgShow: Constructor and destructor
//---------------------------------------------------------------------

EfgShow::EfgShow(FullEfg &p_efg, GambitFrame *p_parent)
  : wxFrame(p_parent, -1, "", wxPoint(0, 0), wxSize(600, 400)),
    EfgClient(&p_efg),
    m_parent(p_parent), m_efg(p_efg), m_treeWindow(0), 
    m_treeZoomWindow(0), cur_soln(0),
    m_solutionTable(0), m_solutionSashWindow(0), m_cursorWindow(0)
{
  SetSizeHints(300, 300);

  // Give the frame an icon
#ifdef __WXMSW__
  SetIcon(wxIcon("efg_icn"));
#else
#include "efg.xbm"
  SetIcon(wxIcon(efg_bits, efg_width, efg_height));
#endif

  CreateStatusBar();

  wxAcceleratorEntry entries[8];
  entries[0].Set(wxACCEL_CTRL, (int) 'N', wxID_NEW);
  entries[1].Set(wxACCEL_CTRL, (int) 'O', wxID_OPEN);
  entries[2].Set(wxACCEL_CTRL, (int) 'S', efgmenuFILE_SAVE);
  entries[3].Set(wxACCEL_CTRL, (int) 'P', efgmenuFILE_PRINT);
  entries[4].Set(wxACCEL_CTRL, (int) 'X', wxID_EXIT);
  entries[5].Set(wxACCEL_NORMAL, WXK_F1, wxID_HELP_CONTENTS);
  entries[6].Set(wxACCEL_NORMAL, (int) '+', efgmenuPREFS_ZOOMIN);
  entries[7].Set(wxACCEL_NORMAL, (int) '-', efgmenuPREFS_ZOOMOUT);
  wxAcceleratorTable accel(8, entries);
  SetAcceleratorTable(accel);

  MakeMenus();
    
  m_currentSupport = new EFSupport(m_efg);
  m_currentSupport->SetName("Full Support");
  m_supports.Append(m_currentSupport);

  MakeToolbar();
  
  m_nodeSashWindow = new wxSashWindow(this, idNODEWINDOW,
				      wxPoint(0, 40), wxSize(200, 200),
				      wxNO_BORDER | wxSW_3D);
  m_nodeSashWindow->SetSashVisible(wxSASH_RIGHT, true);

  m_treeWindow = new TreeWindow(this, this);
  m_treeWindow->SetSize(200, 40, 200, 200);

  wxNotebook *notebook = new wxNotebook(m_nodeSashWindow, -1);

  m_cursorWindow = new EfgCursorWindow(this, notebook);
  m_cursorWindow->Set(m_treeWindow->Cursor());
  m_cursorWindow->SetSize(200, 200);
  notebook->AddPage(m_cursorWindow, "Cursor");

  wxPanel *outcomePanel = new wxPanel(notebook, -1);
  (void) new wxStaticText(outcomePanel, -1, 
			  "Hi!  I'm going to be the outcome window");
  notebook->AddPage(outcomePanel, "Outcomes");

  wxPanel *supportPanel = new wxPanel(notebook, -1);
  (void) new wxStaticText(supportPanel, -1,
			  "Hi!  I'm going to be the support window");
  notebook->AddPage(supportPanel, "Supports");
  notebook->SetSelection(0);

  m_nodeSashWindow->Show(false);

  m_solutionSashWindow = new wxSashWindow(this, idSOLUTIONWINDOW,
					  wxDefaultPosition,
					  wxSize(600, 100));
  m_solutionSashWindow->SetSashVisible(wxSASH_TOP, true);

  m_solutionTable = new EfgProfileList(this, m_solutionSashWindow);
  m_solutionTable->Show(true);
  m_solutionSashWindow->Show(false);

  m_efg.SetIsDirty(false);

#ifdef ZOOM_WINDOW
  m_treeZoomWindow = new TreeZoomWindow(this, m_treeWindow);
#endif  // ZOOM_WINDOW

  AdjustSizes();
  m_treeWindow->FitZoom();

  Show(true);
}

EfgShow::~EfgShow()
{
  m_parent->RemoveGame(&m_efg);
}

void EfgShow::OnSelectedMoved(const Node *n)
{
  // The only time the inspection window won't be around is on construction
  if (m_cursorWindow) {
    m_cursorWindow->Set(n);
  }
#ifdef ZOOM_WINDOW
  m_treeZoomWindow->UpdateCursor();
#endif  // ZOOM_WINDOW
  UpdateMenus();
}

void EfgShow::ChangeSolution(int sol)
{
  cur_soln = sol;
  m_treeWindow->Refresh();
  if (m_cursorWindow) {
    m_cursorWindow->Set(m_treeWindow->Cursor());
  }
  if (m_solutionTable) {
    m_solutionTable->UpdateValues();
  }
}


void EfgShow::RemoveSolutions(void)
{
  cur_soln = 0;
  m_solutionTable->Flush();
  OnSelectedMoved(0); // update the node inspect window if any
}

void EfgShow::OnProfilesNew(wxCommandEvent &)
{
  BehavSolution profile = BehavProfile<gNumber>(EFSupport(m_efg));

  dialogBehavEditor dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddSolution(dialog.GetProfile(), true);
    ChangeSolution(m_solutionTable->Length());
  }
}

void EfgShow::OnProfilesClone(wxCommandEvent &)
{
  BehavSolution profile((*m_solutionTable)[cur_soln]);

  dialogBehavEditor dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddSolution(dialog.GetProfile(), true);
    ChangeSolution(m_solutionTable->Length());
  }
}

void EfgShow::OnProfilesRename(wxCommandEvent &)
{
  if (cur_soln > 0) {
    wxTextEntryDialog dialog(this, "Enter new name for profile",
			     "Rename profile",
			     (char *) (*m_solutionTable)[cur_soln].GetName());

    if (dialog.ShowModal() == wxID_OK) {
      (*m_solutionTable)[cur_soln].SetName(dialog.GetValue().c_str());
      m_solutionTable->UpdateValues();
    }
  }
}

void EfgShow::OnProfilesEdit(wxCommandEvent &)
{
  if (cur_soln > 0) {
    dialogBehavEditor dialog(this, (*m_solutionTable)[cur_soln]);

    if (dialog.ShowModal() == wxID_OK) {
      (*m_solutionTable)[cur_soln] = dialog.GetProfile();
      ChangeSolution(cur_soln);
    }
  }
}

void EfgShow::OnProfilesDelete(wxCommandEvent &)
{
  m_solutionTable->Remove(cur_soln);
  cur_soln = (m_solutionTable->Length() > 0) ? 1 : 0;
  ChangeSolution(cur_soln);
}

//************************************************************************
//                     ACTUAL SOLUTION FUNCTIONS
//************************************************************************

void EfgShow::AddSolution(const BehavSolution &p_profile, bool p_map)
{
  m_solutionTable->Append(p_profile);

  if (m_efg.AssociatedNfg() && p_map) {
    m_parent->GetWindow(m_efg.AssociatedNfg())->AddSolution(MixedProfile<gNumber>(p_profile), false);
  }

  m_solutionTable->UpdateValues();
  UpdateMenus();
}

// Solution access for TreeWindow

gText EfgShow::AsString(TypedSolnValues what, const Node *n, int br) const
{
  int i;
  // Special case that does not fit in ANYWHERE: Chance nodes have probs w/out solutions
  if (what == tBranchProb && n->GetPlayer())
    if (n->GetPlayer()->IsChance())
      return ToText(m_efg.GetChanceProb(n->GetInfoset(), br),
		    m_treeWindow->NumDecimals());
  
  if (!n || !cur_soln) return "N/A";
  
  const BehavSolution &cur = (*m_solutionTable)[cur_soln];
  
  switch (what) 
    {
    case tRealizProb:           // terminal ok
      return ToText(cur.RealizProb(n), m_treeWindow->NumDecimals());
    case tBeliefProb: // terminal ok
      {
	if (!n->GetPlayer()) return "N/A";
	return ToText(cur.BeliefProb(n), m_treeWindow->NumDecimals());
      }
    case tNodeValue:  // terminal ok
      {
	gText tmp = "(";
	for (i = 1; i <= m_efg.NumPlayers(); i++)
	  tmp += ToText(cur.NodeValue(n)[i], m_treeWindow->NumDecimals())+((i == m_efg.NumPlayers()) ? ")" : ",");
	return tmp;
      }
    case tIsetProb: // terminal not ok
      {
	if (!n->GetPlayer()) return "N/A";
	return ToText(cur.IsetProb(n->GetInfoset()), m_treeWindow->NumDecimals());
      }
    case tBranchVal: // terminal not ok
      {
	if (!n->GetPlayer()) return "N/A";
	if (n->GetPlayer()->IsChance()) return "N/A";
	if (cur.IsetProb(n->GetInfoset()) > gNumber(0))
	  return ToText(cur.ActionValue(n->GetInfoset()->Actions()[br]),m_treeWindow->NumDecimals());
	else        // this is due to a bug in the value computation
	  return "N/A";
      }
    case tBranchProb:   // terminal not ok
      if (!n->GetPlayer()) return "N/A";
      // For chance node prob, see first line of this function
      return ToText(cur.ActionProb(n->GetInfoset()->Actions()[br]),m_treeWindow->NumDecimals());
    case tIsetValue:    // terminal not ok, not implemented
      {
	if (!n->GetPlayer()) return "N/A";
	if (cur.IsetProb(n->GetInfoset()) > gNumber(0))
	  return ToText(cur.IsetValue(n->GetInfoset()), m_treeWindow->NumDecimals());
	else        // this is due to a bug in the value computation
	  return "N/A";
      }
    default:
      return "N/A";
    }
}

gNumber EfgShow::ActionProb(const Node *p_node, int p_action)
{
  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    return m_efg.GetChanceProb(p_node->GetInfoset(), p_action);
  }

  if (cur_soln && p_node->GetInfoset()) {
    return (*m_solutionTable)[cur_soln](p_node->GetInfoset()->Actions()[p_action]);
  }
  return -1;
}

const gText &EfgShow::Filename(void) const
{
    return filename;
}


void EfgShow::PickSolutions(const Efg &p_efg, Node *p_rootnode,
			    gList<BehavSolution> &p_solns)
{
#ifdef NOT_PORTED_YET
  try {
    m_treeWindow->SetSubgamePickNode(p_rootnode);
    BehavSolutionList temp_solns;
    temp_solns += p_solns;       
    EfgSolnPicker *pick = new EfgSolnPicker(p_efg, temp_solns,
					    m_treeWindow->DrawSettings(),
					    sf_options, this);
    Enable(false);
    while (pick->Completed() == wxRUNNING) wxYield();
    Enable(true);
    p_solns = temp_solns; 
    delete pick;
    m_treeWindow->SetSubgamePickNode(0);
  }
  catch (...) {
    m_treeWindow->SetSubgamePickNode(0);
    throw;
  }
#endif // NOT_PORTED_YET
}

void EfgShow::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new game");
  fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a saved game");
  fileMenu->Append(efgmenuFILE_CLOSE, "&Close", "Close this window");
  fileMenu->AppendSeparator();
  fileMenu->Append(efgmenuFILE_SAVE, "&Save\tCtrl-S", "Save this game");
  fileMenu->AppendSeparator();
  fileMenu->Append(efgmenuFILE_PAGE_SETUP, "Page Se&tup",
		   "Set up preferences for printing");
  fileMenu->Append(efgmenuFILE_PRINT_PREVIEW, "Print Pre&view",
		   "View a preview of the game printout");
  fileMenu->Append(efgmenuFILE_PRINT, "&Print\tCtrl-P", "Print this game");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, "E&xit\tCtrl-X", "Exit Gambit");

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
  subgame_menu->Append(efgmenuSUBGAME_UNMARKALL, "UnMark &All",
		       "Clear all subgame info");
  subgame_menu->Append(efgmenuSUBGAME_UNMARK, "&UnMark",
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
  supports_menu->Append(efgmenuSUPPORT_UNDOMINATED, "&Undominated",
			"Find undominated strategies");
  supports_menu->Append(efgmenuSUPPORT_NEW, "&New",
			"Create a new support");
  supports_menu->Append(efgmenuSUPPORT_EDIT, "&Edit",
			"Edit the currently displayed support");
  supports_menu->Append(efgmenuSUPPORT_DELETE, "&Delete",
			"Delete a support");
  wxMenu *supportsSelectMenu = new wxMenu;
  supportsSelectMenu->Append(efgmenuSUPPORT_SELECT_FROMLIST, "From &List...",
			     "Select a support from the list of defined supports");
  supportsSelectMenu->Append(efgmenuSUPPORT_SELECT_PREVIOUS, "&Previous",
			     "Select the previous support from the list");
  supportsSelectMenu->Append(efgmenuSUPPORT_SELECT_NEXT, "&Next",
			     "Select the next support from the list");
  supports_menu->Append(efgmenuSUPPORT_SELECT, "&Select", supportsSelectMenu,
			"Change the current support");
  supports_menu->AppendSeparator();
  supports_menu->Append(efgmenuSUPPORT_REACHABLE, "&Root Reachable",
			"Display only nodes that are support-reachable",
			true);
  
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

  solve_menu->AppendSeparator();
  solve_menu->Append(efgmenuSOLVE_WIZARD, "&Wizard",
		     "Experimental wizard for algorithms");
  
  wxMenu *inspect_menu = new wxMenu;
  inspect_menu->Append(efgmenuINSPECT_SOLUTIONS, "&Solutions",
		       "Inspect existing solutions", true);
  inspect_menu->Append(efgmenuINSPECT_CURSOR, "&Cursor",
		       "Information about the node at cursor", true);
  inspect_menu->Append(efgmenuINSPECT_INFOSETS, "&Infosets",
		       "Inspect information sets", true);
  inspect_menu->Append(efgmenuINSPECT_ZOOM_WIN, "Zoom &Window",
		       "Open zoom window", true);
  inspect_menu->AppendSeparator();
  inspect_menu->Append(efgmenuINSPECT_GAMEINFO, "Game&Info",
		       "Information about this game");
  inspect_menu->Append(efgmenuINSPECT_SCRIPT, "Scri&pt",
		       "View GCL script log");
  
  wxMenu *prefsMenu = new wxMenu;
  wxMenu *prefsDisplayMenu = new wxMenu;
  prefsDisplayMenu->Append(efgmenuPREFS_DISPLAY_DECIMALS, "&Decimal Places",
			   "Set number of decimal places to display");
  prefsDisplayMenu->Append(efgmenuPREFS_DISPLAY_LAYOUT, "&Layout",
			   "Set tree layout parameters");
  prefsMenu->Append(efgmenuPREFS_DISPLAY, "&Display", prefsDisplayMenu,
		     "Set display options");
  prefsMenu->Append(efgmenuPREFS_LEGEND, "&Legends...",
		     "Set legends");

  wxMenu *prefsFontsMenu = new wxMenu;
  prefsFontsMenu->Append(efgmenuPREFS_FONTS_ABOVENODE, "Above Node",
			 "Font for label above nodes");
  prefsFontsMenu->Append(efgmenuPREFS_FONTS_BELOWNODE, "Below Node",
			 "Font for label below nodes");
  prefsFontsMenu->Append(efgmenuPREFS_FONTS_AFTERNODE, "After Node",
			 "Font for label to right of nodes");
  prefsFontsMenu->Append(efgmenuPREFS_FONTS_ABOVEBRANCH, "Above Branch",
			 "Font for label above branches");
  prefsFontsMenu->Append(efgmenuPREFS_FONTS_BELOWBRANCH, "Below Branch",
			 "Font for label below branches");
  prefsMenu->Append(efgmenuPREFS_FONTS, "&Fonts", prefsFontsMenu,
		     "Set display fonts");

  prefsMenu->Append(efgmenuPREFS_COLORS, "&Colors",
		     "Set player colors");
  prefsMenu->AppendSeparator();
  prefsMenu->Append(efgmenuPREFS_ZOOMIN, "Zoom &in\t+",
		    "Increase display magnification");
  prefsMenu->Append(efgmenuPREFS_ZOOMOUT, "Zoom &out\t-",
		    "Decrease display magnification");
  prefsMenu->AppendSeparator();
  prefsMenu->Append(efgmenuPREFS_SAVE, "&Save");
  prefsMenu->Append(efgmenuPREFS_LOAD, "&Load");
  
  wxMenu *help_menu = new wxMenu;
  help_menu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");
  help_menu->Append(wxID_ABOUT, "&About", "About Gambit");

  wxMenuBar *menu_bar = new wxMenuBar(wxMB_DOCKABLE);
  menu_bar->Append(fileMenu, "&File");
  menu_bar->Append(edit_menu,     "&Edit");
  menu_bar->Append(subgame_menu,  "Sub&games");
  menu_bar->Append(supports_menu, "S&upports");
  menu_bar->Append(solve_menu,    "&Solve");
  menu_bar->Append(inspect_menu,  "&Inspect");
  menu_bar->Append(prefsMenu,     "&Prefs");
  menu_bar->Append(help_menu,     "&Help");

  // Set the menu bar
  SetMenuBar(menu_bar);
}

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/preview.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/table.xpm"
#include "bitmaps/help.xpm"

void EfgShow::MakeToolbar(void)
{
  wxToolBar *toolBar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE |
				     wxTB_HORIZONTAL);
  toolBar->SetMargins(4, 4);

  toolBar->AddTool(wxID_NEW, wxBITMAP(new), wxNullBitmap, false,
		   -1, -1, 0, "New game", "Create a new game");
  toolBar->AddTool(wxID_OPEN, wxBITMAP(open), wxNullBitmap, false,
		   -1, -1, 0, "Open file", "Open a saved game");
  toolBar->AddTool(efgmenuFILE_SAVE, wxBITMAP(save), wxNullBitmap, false,
		   -1, -1, 0, "Save game", "Save this game");
  toolBar->AddSeparator();

  toolBar->AddTool(efgmenuFILE_PRINT_PREVIEW, wxBITMAP(preview), wxNullBitmap,
		   false, -1, -1, 0, "Print Preview",
		   "View a preview of the game printout");
  toolBar->AddTool(efgmenuFILE_PRINT, wxBITMAP(print), wxNullBitmap, false,
		   -1, -1, 0, "Print", "Print this game");
  toolBar->AddSeparator();

  toolBar->AddTool(efgmenuSOLVE_NFG_REDUCED, wxBITMAP(table), wxNullBitmap,
		   false, -1, -1, 0, "Normal form",
		   "Generate reduced normal form");
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_HELP, wxBITMAP(help), wxNullBitmap, false,
		   -1, -1, 0, "Help", "Table of contents");

  toolBar->Realize();
  toolBar->SetRows(1);
}

// if who == 2, hilight in the tree display
// if who == 1, hilight in the solution window display
void EfgShow::HilightInfoset(int pl, int iset, int who)
{
  if (!features.iset_hilight) 
    return;

#ifdef NOT_PORTED_YET
  if (who == 1)
    m_solutionTable->HilightInfoset(pl, iset);
#endif  // NOT_PORTED_YET

  if (who == 2) m_treeWindow->HilightInfoset(pl, iset);
}

gText EfgShow::UniqueSupportName(void) const
{
  int number = m_supports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_supports.Length(); i++) {
      if (m_supports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_supports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

void EfgShow::OnFileSave(wxCommandEvent &)
{
  static int s_nDecimals = 6;
  dialogEfgSave dialog(Filename(), m_efg.GetTitle(), s_nDecimals, this);

  if (dialog.ShowModal() == wxID_OK) {
    if (wxFileExists((char *) dialog.Filename())) {
      if (wxMessageBox((char *) ("File " + dialog.Filename() +
				 " exists.  Overwrite?"),
		       "Confirm", wxOK | wxCANCEL) != wxOK) {
	return;
      }
    }

    m_efg.SetTitle(dialog.Label());

    FullEfg *efg = 0;
    try {
      gFileOutput file(dialog.Filename());
      efg = CompressEfg(m_efg, *GetSupport());
      efg->WriteEfgFile(file, s_nDecimals);
      delete efg;
      SetFileName(dialog.Filename());
    }
    catch (gFileOutput::OpenFailed &) {
      wxMessageBox((char *) ("Could not open " + dialog.Filename() + " for writing."),
		   "Error", wxOK);
      if (efg)  delete efg;
    }
    catch (gFileOutput::WriteFailed &) {
      wxMessageBox((char *) ("Write error occurred in saving " + dialog.Filename()),
		   "Error", wxOK);
      if (efg)  delete efg;
    }
    catch (Efg::Exception &) {
      wxMessageBox("Internal exception in extensive form", "Error", wxOK);
      if (efg)  delete efg;
    }
  }
}

void EfgShow::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupData();
  }
}

void EfgShow::OnFilePrintPreview(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrintPreview *preview = new wxPrintPreview(new EfgPrintout(m_treeWindow),
					       new EfgPrintout(m_treeWindow),
					       &data);

  if (!preview->Ok()) {
    delete preview;
    return;
  }

  wxPreviewFrame *frame = new wxPreviewFrame(preview, this,
					     "Print Preview",
					     wxPoint(100, 100),
					     wxSize(600, 650));
  frame->Initialize();
  frame->Show(true);
}

void EfgShow::OnFilePrint(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);
  EfgPrintout printout(m_treeWindow);

  if (!printer.Print(this, &printout, true)) {
    wxMessageBox("There was an error in printing", "Error", wxOK);
    return;
  }
  else {
    m_printData = printer.GetPrintDialogData().GetPrintData();
  }
}


void EfgShow::OnEditNodeAdd(wxCommandEvent &)
{
  static int branches = 2; // make this static so it remembers the last entry
  static EFPlayer *player = 0;
  static Infoset *infoset = 0;
  static Efg *last_ef = 0; // need this to make sure player,infoset are valid

  if (last_ef != &m_efg)  {
    player = 0;
    infoset = 0;
    last_ef = &m_efg;
  }
    
  dialogMoveAdd dialog(this, m_efg, "Add Move", player, infoset, branches);

  if (dialog.ShowModal() == wxID_OK)  {
    NodeAddMode mode = dialog.GetAddMode();
    player = dialog.GetPlayer();
    infoset = dialog.GetInfoset();
    branches = dialog.GetActions();
    try {
      if (mode == NodeAddNew) { 
	m_efg.AppendNode(Cursor(), player, branches);
	m_script += "AddMove[RootNode[efg]";
	gArray<int> path = m_efg.PathToNode(Cursor());
	for (int i = 1; i <= path.Length(); i++) { 
	  m_script += "#";
	  m_script += (char *) ToText(path[i]);
	}
	m_script += ", Players[efg]_";
	m_script += (char *) ToText(player->GetNumber());
	m_script += ", ";
	m_script += (char *) ToText(branches);
	m_script += "];\n";
      }
      else {
	m_efg.AppendNode(Cursor(), infoset);
	m_script += "AddMove[RootNode[efg]";
	gArray<int> path = m_efg.PathToNode(Cursor());
	for (int i = 1; i <= path.Length(); i++) {
	  m_script += "#";
	  m_script += (char *) ToText(path[i]);
	}
	m_script += ", Infosets[Players[efg]_";
	m_script += (char *) ToText(infoset->GetPlayer()->GetNumber());
	m_script += "]_";
	m_script += (char *) ToText(infoset->GetNumber());
	m_script += "];\n";
      }
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditNodeDelete(wxCommandEvent &)
{
  try {
    dialogNodeDelete dialog(Cursor(), this);

    if (dialog.ShowModal() == wxID_OK) {
      Node *keep = dialog.KeepNode();
      m_treeWindow->SetCursorPosition(m_efg.DeleteNode(Cursor(), keep));
    }
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditNodeInsert(wxCommandEvent &)
{
  static int branches = 2; // make this static so it remembers the last entry
  static EFPlayer *player = 0;
  static Infoset *infoset = 0;
  static Efg *last_ef = 0; // need this to make sure player,infoset are valid

  if (last_ef != &m_efg)  {
    player = 0;
    infoset = 0;
    last_ef = &m_efg;
  }
    
  dialogMoveAdd dialog(this, m_efg, "Insert Move", player, infoset, branches);

  if (dialog.ShowModal() == wxID_OK)  {
    NodeAddMode mode = dialog.GetAddMode();
    player = dialog.GetPlayer();
    infoset = dialog.GetInfoset();
    branches = dialog.GetActions();

    try {
      if (mode == NodeAddNew) {
	m_efg.InsertNode(Cursor(), player, branches);
      }
      else {
	m_efg.InsertNode(Cursor(), infoset);
      }

      m_treeWindow->SetCursorPosition(Cursor()->GetParent());
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditNodeLabel(wxCommandEvent &)
{
  wxTextEntryDialog dialog(this, "Label node", "Label of node",
			   (char *) Cursor()->GetName());

  if (dialog.ShowModal() == wxID_OK) {
    Cursor()->SetName(dialog.GetValue().c_str());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnEditNodeSetMark(wxCommandEvent &)
{
  m_treeWindow->node_set_mark();
  m_treeWindow->Refresh();
}

void EfgShow::OnEditNodeGotoMark(wxCommandEvent &)
{
  m_treeWindow->node_goto_mark();
}

void EfgShow::OnEditActionDelete(wxCommandEvent &)
{
  Infoset *infoset = Cursor()->GetInfoset();
  dialogActionSelect dialog(infoset, "Delete Action", "Action to delete",
			    this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_efg.DeleteAction(infoset, dialog.GetAction());
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditActionInsert(wxCommandEvent &)
{
  Infoset *infoset = Cursor()->GetInfoset();
  dialogActionSelect dialog(infoset, "Insert action",
			    "Insert new action before", this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_efg.InsertAction(infoset, dialog.GetAction());
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditActionAppend(wxCommandEvent &)
{
  try {
    m_efg.InsertAction(Cursor()->GetInfoset());
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditActionLabel(wxCommandEvent &)
{
  Infoset *infoset = Cursor()->GetInfoset();
  dialogActionLabel dialog(infoset, this);
  
  if (dialog.ShowModal() == wxID_OK) {
    try {
      for (int act = 1; act <= infoset->NumActions(); act++) {
	infoset->Actions()[act]->SetName(dialog.GetActionLabel(act));
      }
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditActionProbs(wxCommandEvent &)
{
  Infoset *infoset = Cursor()->GetInfoset();
  dialogActionProbs dialog(infoset, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      for (int act = 1; act <= infoset->NumActions(); act++) {
	m_efg.SetChanceProb(infoset, act, dialog.GetActionProb(act));
      }
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditOutcomesAttach(wxCommandEvent &)
{
  dialogEfgOutcomeSelect dialog(m_efg, this);
  
  if (dialog.ShowModal() == wxID_OK) {
    Cursor()->SetOutcome(dialog.GetOutcome());
    m_treeWindow->OutcomeChange();
    m_treeWindow->Refresh();
    UpdateMenus();
  }
}

void EfgShow::OnEditOutcomesDetach(wxCommandEvent &)
{
  Cursor()->SetOutcome(0);
  m_treeWindow->OutcomeChange();
  m_treeWindow->Refresh();
  UpdateMenus();
}

void EfgShow::OnEditOutcomesLabel(wxCommandEvent &)
{
  wxTextEntryDialog dialog(this, "New outcome label", "Label outcome",
			   (char *) Cursor()->GetOutcome()->GetName());

  if (dialog.ShowModal() == wxID_OK) {
    Cursor()->GetOutcome()->SetName(dialog.GetValue().c_str());
    m_treeWindow->OutcomeChange();
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnEditOutcomesPayoffs(wxCommandEvent &)
{
  dialogEfgPayoffs dialog(m_efg, Cursor()->GetOutcome(), this);

  if (dialog.ShowModal() == wxID_OK) {
    EFOutcome *outc = Cursor()->GetOutcome();
    gArray<gNumber> payoffs(dialog.Payoffs());

    if (!outc) {
      outc = m_efg.NewOutcome();
      Cursor()->SetOutcome(outc);
    }

    for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
      m_efg.SetPayoff(outc, pl, payoffs[pl]);
    }
    outc->SetName(dialog.Name());

    m_treeWindow->OutcomeChange();
    m_treeWindow->Refresh();
    UpdateMenus();
  }
}

void EfgShow::OnEditOutcomesNew(wxCommandEvent &)
{
  dialogEfgPayoffs dialog(m_efg, 0, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      EFOutcome *outc = m_efg.NewOutcome();
      gArray<gNumber> payoffs(dialog.Payoffs());

      for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
	m_efg.SetPayoff(outc, pl, payoffs[pl]);
      }
      outc->SetName(dialog.Name());
      
      m_treeWindow->OutcomeChange();
      m_treeWindow->Refresh();
      UpdateMenus();
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditOutcomesDelete(wxCommandEvent &)
{
  dialogEfgOutcomeSelect dialog(m_efg, this);
  
  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_efg.DeleteOutcome(dialog.GetOutcome());
      m_treeWindow->OutcomeChange();
      m_treeWindow->Refresh();
      UpdateMenus();
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditInfosetMerge(wxCommandEvent &)
{
  try {
    m_efg.MergeInfoset(m_treeWindow->MarkNode()->GetInfoset(),
		       Cursor()->GetInfoset());
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditInfosetBreak(wxCommandEvent &)
{
  try {
    m_efg.LeaveInfoset(Cursor());
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditInfosetSplit(wxCommandEvent &)
{
  try {
    m_efg.SplitInfoset(Cursor());
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditInfosetJoin(wxCommandEvent &)
{
  try {
    m_efg.JoinInfoset(m_treeWindow->MarkNode()->GetInfoset(), Cursor());
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditInfosetLabel(wxCommandEvent &)
{
  Infoset *infoset = Cursor()->GetInfoset();
  wxTextEntryDialog dialog(this, "New label for information set ",
			   "Label Infoset", (char *) infoset->GetName());

  if (dialog.ShowModal() == wxID_OK) {
    infoset->SetName(dialog.GetValue().c_str());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnEditInfosetPlayer(wxCommandEvent &)
{
  try {
    dialogEfgSelectPlayer dialog(m_efg, false, this);
        
    if (dialog.ShowModal() == wxID_OK) {
      if (dialog.GetPlayer() != Cursor()->GetInfoset()->GetPlayer()) {
	m_efg.SwitchPlayer(Cursor()->GetInfoset(), dialog.GetPlayer());
      }
    }
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditInfosetReveal(wxCommandEvent &)
{
  dialogInfosetReveal dialog(m_efg, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_efg.Reveal(Cursor()->GetInfoset(), dialog.GetPlayers());
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditTreeDelete(wxCommandEvent &)
{
  wxMessageDialog dialog(this, "Delete the whole subtree?", "Confirm");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_efg.DeleteTree(Cursor());
      m_efg.DeleteEmptyInfosets();
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditTreeCopy(wxCommandEvent &)
{
  try {
    m_efg.CopyTree(m_treeWindow->MarkNode(), Cursor());
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditTreeMove(wxCommandEvent &)
{
  try {
    m_efg.MoveTree(m_treeWindow->MarkNode(), Cursor());
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditTreeLabel(wxCommandEvent &)
{
  wxTextEntryDialog dialog(this, "Label game", "Label of game",
			   (char *) m_efg.GetTitle());

  if (dialog.ShowModal() == wxID_OK) {
    m_efg.SetTitle(dialog.GetValue().c_str());
    SetFileName(Filename());
  }
}

void EfgShow::OnEditTreePlayers(wxCommandEvent &)
{
  dialogEfgPlayers dialog(m_efg, this);
  dialog.ShowModal();
}

void EfgShow::OnEditTreeInfosets(wxCommandEvent &)
{
  dialogInfosets dialog(m_efg, this);
  dialog.ShowModal();
}

void EfgShow::OnSubgamesMarkAll(wxCommandEvent &)
{
  m_treeWindow->SubgameMarkAll();
  m_treeWindow->Refresh();
}

void EfgShow::OnSubgamesMark(wxCommandEvent &)
{
  m_treeWindow->SubgameMark();
  m_treeWindow->Refresh();
}

void EfgShow::OnSubgamesUnMarkAll(wxCommandEvent &)
{
  m_treeWindow->SubgameUnmarkAll();
  m_treeWindow->Refresh();
}

void EfgShow::OnSubgamesUnMark(wxCommandEvent &)
{
  m_treeWindow->SubgameUnmark();
  m_treeWindow->Refresh();
}

void EfgShow::OnSubgamesCollapseAll(wxCommandEvent &)
{
  m_treeWindow->SubgameCollapseAll();
  m_treeWindow->Refresh();
}

void EfgShow::OnSubgamesCollapse(wxCommandEvent &)
{
  m_treeWindow->SubgameCollapse();
  m_treeWindow->Refresh();
}

void EfgShow::OnSubgamesExpandAll(wxCommandEvent &)
{
  m_treeWindow->SubgameExpandAll();
  m_treeWindow->Refresh();
}

void EfgShow::OnSubgamesExpand(wxCommandEvent &)
{
  m_treeWindow->SubgameExpand();
  m_treeWindow->Refresh();
}

void EfgShow::OnSubgamesExpandBranch(wxCommandEvent &)
{
  m_treeWindow->SubgameExpandBranch();
  m_treeWindow->Refresh();
}

void EfgShow::OnSupportNew(wxCommandEvent &)
{
  EFSupport newSupport(m_efg);
  newSupport.SetName(UniqueSupportName());
  dialogEfgEditSupport dialog(newSupport, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      EFSupport *support = new EFSupport(dialog.Support());
      m_supports.Append(support);

      m_currentSupport = support;
      m_treeWindow->SupportChanged();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void EfgShow::OnSupportEdit(wxCommandEvent &)
{
  dialogEfgEditSupport dialog(*m_currentSupport, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      *m_currentSupport = dialog.Support();
      m_currentSupport->SetName(dialog.Name());
      m_treeWindow->SupportChanged();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void EfgShow::OnSupportDelete(wxCommandEvent &)
{
  if (m_supports.Length() == 1)  return;

  dialogSupportSelect dialog(this, m_supports, m_currentSupport,
			     "Delete Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      delete m_supports.Remove(dialog.Selected());
      if (!m_supports.Find(m_currentSupport)) {
	m_currentSupport = m_supports[1];
	m_treeWindow->SupportChanged();
      }
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void EfgShow::OnSupportSelectFromList(wxCommandEvent &)
{
  dialogSupportSelect dialog(this, m_supports, m_currentSupport, 
			     "Select Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_currentSupport = m_supports[dialog.Selected()];
      m_treeWindow->SupportChanged();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void EfgShow::OnSupportSelectPrevious(wxCommandEvent &)
{
  int index = m_supports.Find(m_currentSupport);
  if (index == 1) {
    m_currentSupport = m_supports[m_supports.Length()];
  }
  else {
    m_currentSupport = m_supports[index - 1];
  }
  m_treeWindow->SupportChanged();
}

void EfgShow::OnSupportSelectNext(wxCommandEvent &)
{
  int index = m_supports.Find(m_currentSupport);
  if (index == m_supports.Length()) {
    m_currentSupport = m_supports[1];
  }
  else {
    m_currentSupport = m_supports[index + 1];
  }
  m_treeWindow->SupportChanged();
}


void EfgShow::OnSupportUndominated(wxCommandEvent &)
{
  gArray<gText> playerNames(m_efg.NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++)
    playerNames[pl] = m_efg.Players()[pl]->GetName();
  dialogElimBehav dialog(this, playerNames);

  if (dialog.ShowModal() == wxID_OK) {
    EFSupport *sup = m_currentSupport;
    wxStatus status(this, "Dominance Elimination");

    try {
      if (dialog.Iterative()) {
	while ((sup = sup->Undominated(dialog.DomStrong(),
				       dialog.DomConditional(),
				       dialog.Players(), gnull, status)) != 0) {
	  sup->SetName(UniqueSupportName());
	  m_supports.Append(sup);
	}
      }
      else {
	if ((sup = sup->Undominated(dialog.DomStrong(),
				    dialog.DomConditional(),
				    dialog.Players(), gnull, status)) != 0) {
	  sup->SetName(UniqueSupportName());
	  m_supports.Append(sup);
	}
      }
    }
    catch (gSignalBreak &E) { }
    
    if (m_currentSupport != sup) {
      m_currentSupport = m_supports[m_supports.Length()]; 
      m_treeWindow->SupportChanged();
    }
  }
}

void EfgShow::OnSupportReachable(wxCommandEvent &)
{
  m_treeWindow->DrawSettings().SetRootReachable(!m_treeWindow->DrawSettings().RootReachable());
  m_treeWindow->RefreshLayout();
  m_treeWindow->Refresh();
}

void EfgShow::OnSolveStandard(wxCommandEvent &)
{
  // This is a guard against trying to solve the "trivial" game.
  // Most of the GUI code assumes information sets exist.
  if (m_efg.NumPlayerInfosets() == 0)  return;

  bool isPerfectRecall;

  if ((isPerfectRecall = IsPerfectRecall(m_efg)) == false) {
    if (wxMessageBox("This game is not perfect recall\n"
		     "Do you wish to continue?", 
		     "Solve Warning", 
		     wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
      return;
    }
  }

  dialogEfgSolveStandard dialog(this, m_efg);
  if (dialog.ShowModal() != wxID_OK)  {
    return;
  }

  guiEfgSolution *solver = 0;

  wxBeginBusyCursor();

  bool markSubgames = false;
  
  switch (dialog.Type()) {
  case efgSTANDARD_NASH:
    switch (dialog.Number()) {
    case efgSTANDARD_ONE:
      markSubgames = true;
      if (m_efg.NumPlayers() == 2 && isPerfectRecall) {
	if (m_efg.IsConstSum()) 
	  solver = new guiefgLpEfg(this, 1, dialog.Precision());
	else
	  solver = new guiefgLcpEfg(this, 1, dialog.Precision());
      }
      else if (m_efg.NumPlayers() == 2 && !isPerfectRecall)
	solver = new guiefgQreEfg(this, 1);
      else 
	solver = new guiefgSimpdivNfg(this, 1, dialog.Precision(),
				      true);
      break;
    case efgSTANDARD_TWO:
      if (m_efg.NumPlayers() == 2)
	solver = new guiefgEnumMixedNfg(this, 2,
					dialog.Precision(), false);
      else {
	wxMessageBox("Not guaranteed to find two solutions", "Warning");
	solver = new guiefgLiapEfg(this, 2, 10);
      }
      break;
    case efgSTANDARD_ALL:
      if (m_efg.NumPlayers() == 2) {
	solver = new guiefgEnumMixedNfg(this, 0,
					dialog.Precision(), false);
      }
      else  {
	solver = new guiefgPolEnumEfg(this, 0);
      }
      break;
    }
    break;

  case efgSTANDARD_PERFECT:
    markSubgames = true;
    switch (dialog.Number()) {
    case efgSTANDARD_ONE:
      if (m_efg.NumPlayers() == 2 && isPerfectRecall) {
	if (m_efg.IsConstSum()) 
	  solver = new guiefgLpEfg(this, 1, dialog.Precision());
	else
	  solver = new guiefgLcpEfg(this, 1, dialog.Precision());
      }
      else if (m_efg.NumPlayers() == 2 && !isPerfectRecall)
	solver = new guiefgQreEfg(this, 1);
      else 
	solver = new guiefgSimpdivNfg(this, 1, dialog.Precision(),
				      true);
      break;
    case efgSTANDARD_TWO:
      if (m_efg.NumPlayers() == 2)
	solver = new guiefgEnumMixedNfg(this, 2,
					dialog.Precision(), false);
      else {
	wxMessageBox("Not guaranteed to find two solutions", "Warning");
	solver = new guiefgLiapEfg(this, 2, 10);
      }
      break;
    case efgSTANDARD_ALL:
      if (m_efg.NumPlayers() == 2)
	solver = new guiefgEnumMixedNfg(this, 0,
					dialog.Precision(), false);
      else {
	solver = new guiefgPolEnumEfg(this, 0);
      }
      break;
    }
    break;

  case efgSTANDARD_SEQUENTIAL:
    switch (dialog.Number()) {
    case efgSTANDARD_ONE:
      solver = new guiefgQreEfg(this, 1);
      break;
    case efgSTANDARD_TWO:
      wxMessageBox("Not guaranteed to find two solutions", "Warning");
      solver = new guiefgLiapEfg(this, 2, 10);
      break;
    case efgSTANDARD_ALL:
      wxMessageBox("Not guaranteed to find all solutions", "Warning");
      solver = new guiefgLiapEfg(this, 0, 0);
      return;
    }
  }

  try {
    wxConfig config("Gambit");
    config.Write("Solutions/Efg-Interactive-Solutions", 0l);
    if (markSubgames)  
      m_treeWindow->SubgameMarkAll();
    else
      m_treeWindow->SubgameUnmarkAll();

    gList<BehavSolution> solutions = solver->Solve(*m_currentSupport);
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddSolution(solutions[soln], true);
    }
    wxEndBusyCursor();
  }
  catch (gException &E) {
    wxEndBusyCursor();
    guiExceptionDialog(E.Description(), this);
  }

  delete solver;

  ChangeSolution(m_solutionTable->Length());
  UpdateMenus();
}

void EfgShow::OnSolveCustom(wxCommandEvent &p_event)
{
  int algorithm = p_event.GetInt();

  // This is a guard against trying to solve the "trivial" game.
  // Most of the GUI code assumes information sets exist.
  if (m_efg.NumPlayerInfosets() == 0)  return;

  // check that the game is perfect recall, if not give a warning
  if (!IsPerfectRecall(m_efg)) {
    if (wxMessageBox("This game is not perfect recall\n"
		     "Do you wish to continue?", 
		     "Solve Warning", 
		     wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
      return;
    }
  }
    
  // do not want users doing anything while solving
  Enable(false);

  guiEfgSolution *solver = 0;

  switch (algorithm) {
  case efgmenuSOLVE_CUSTOM_EFG_ENUMPURE:
    solver = new guiefgEnumPureEfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_LCP:
    solver = new guiefgLcpEfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_LP:
    solver = new guiefgLpEfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_LIAP:
    solver = new guiefgLiapEfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_POLENUM:
    solver = new guiefgPolEnumEfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_EFG_QRE:
    solver = new guiefgQreEfg(this);
    break;

  case efgmenuSOLVE_CUSTOM_NFG_ENUMPURE: 
    solver = new guiefgEnumPureNfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_ENUMMIXED:
    solver = new guiefgEnumMixedNfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_LCP: 
    solver = new guiefgLcpNfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_LP:
    solver = new guiefgLpNfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_LIAP: 
    solver = new guiefgLiapNfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_SIMPDIV:
    solver = new guiefgSimpdivNfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_POLENUM:
    solver = new guiefgPolEnumNfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_QRE:
    solver = new guiefgQreNfg(this);
    break;
  case efgmenuSOLVE_CUSTOM_NFG_QREGRID: 
    solver = new guiefgQreAllNfg(this);
    break;
  default:
    // internal error, we'll just ignore silently
    return;
  }

  bool go = solver->SolveSetup();
  
  try {
    if (go) {
      if (solver->MarkSubgames())
	m_treeWindow->SubgameMarkAll();
      wxBeginBusyCursor();
      gList<BehavSolution> solutions = solver->Solve(*m_currentSupport);
      for (int soln = 1; soln <= solutions.Length(); soln++) {
	AddSolution(solutions[soln], true);
      }
      wxEndBusyCursor();
    }
  }
  catch (gException &E) {
    wxEndBusyCursor();
    guiExceptionDialog(E.Description(), this);
  }

  delete solver;
 
  ChangeSolution(m_solutionTable->Length());
  UpdateMenus();
  Enable(true);
}

void EfgShow::OnSolveNormalReduced(wxCommandEvent &)
{
  // check that the game is perfect recall, if not give a warning
  if (!IsPerfectRecall(m_efg)) {
    if (wxMessageBox("This game is not perfect recall\n"
		     "Do you wish to continue?", 
		     "Reduced normal form", 
		     wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
      return;
    }
  }
    
  if (m_efg.AssociatedNfg() != 0) {
    return;
  }

  Nfg *nfg = MakeReducedNfg(*m_currentSupport);
  if (nfg) {
    NfgShow *nfgShow = new NfgShow(*nfg, m_parent);
    nfgShow->SetFileName("");
    m_parent->AddGame(&m_efg, nfg, nfgShow);

    for (int i = 1; i <= m_solutionTable->Length(); i++) {
      nfgShow->AddSolution(MixedProfile<gNumber>((*m_solutionTable)[i]), false);
    }
  }
  else {
    wxMessageBox("Could not create normal form game.\n",
		 "Reduced normal form", wxOK);
  }
}

void EfgShow::OnSolveNormalAgent(wxCommandEvent &)
{
  // check that the game is perfect recall, if not give a warning
  if (!IsPerfectRecall(m_efg)) {
    if (wxMessageBox("This game is not perfect recall\n"
		     "Do you wish to continue?", 
		     "Agent normal form", 
		     wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
      return;
    }
  }
    
  Nfg *N = MakeAfg(m_efg);
  if (N) {
    (void) new NfgShow(*N, m_parent);
  }
}

class DominancePage : public wxWizardPageSimple {
private:
  wxRadioBox *m_depthChoice, *m_typeChoice, *m_methodChoice;

public:
  DominancePage(wxWizard *);
};

DominancePage::DominancePage(wxWizard *p_parent)
  : wxWizardPageSimple(p_parent)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString depthChoices[] = { "None", "Once", "Iterative" };
  m_depthChoice = new wxRadioBox(this, idDEPTH_CHOICE, "Depth",
				 wxDefaultPosition, wxDefaultSize,
				 3, depthChoices, 0, wxRA_SPECIFY_COLS);
  topSizer->Add(m_depthChoice, 0, wxALL, 5);

  wxString typeChoices[] = { "Weak", "Strong" };
  m_typeChoice = new wxRadioBox(this, -1, "Type",
				wxDefaultPosition, wxDefaultSize,
				2, typeChoices, 0, wxRA_SPECIFY_COLS);
  if (m_depthChoice->GetSelection() == 0)
    m_typeChoice->Enable(false);
  topSizer->Add(m_typeChoice, 0, wxALL, 5);

  wxString methodChoices[] = { "Pure", "Mixed" };
  m_methodChoice = new wxRadioBox(this, -1, "Method",
				  wxDefaultPosition, wxDefaultSize,
				  2, methodChoices, 0, wxRA_SPECIFY_COLS);
  if (m_depthChoice->GetSelection() == 0)
    m_methodChoice->Enable(false);
  topSizer->Add(m_methodChoice, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

class SubgamesPage : public wxWizardPageSimple {
public:
  SubgamesPage(wxWizard *p_parent);
};

SubgamesPage::SubgamesPage(wxWizard *p_parent)
  : wxWizardPageSimple(p_parent)
{
  new wxStaticText(this, -1, "Subgames options go here");
}

class AlgorithmPage : public wxWizardPageSimple {
public:
  AlgorithmPage(wxWizard *);
};

AlgorithmPage::AlgorithmPage(wxWizard *p_parent)
  : wxWizardPageSimple(p_parent)
{
  new wxStaticText(this, -1, "Algorithm options go here");
}

class FinishPage : public wxWizardPageSimple {
public:
  FinishPage(wxWizard *);
};

FinishPage::FinishPage(wxWizard *p_parent)
  : wxWizardPageSimple(p_parent)
{
  new wxStaticText(this, -1, "All finished... click Finish to run");
}

void EfgShow::OnSolveWizard(wxCommandEvent &)
{
  wxWizard *wizard = wxWizard::Create(this, -1, "Solution wizard");
  wxWizardPageSimple *page1 = new DominancePage(wizard);
  wxWizardPageSimple *page2 = new SubgamesPage(wizard);
  wxWizardPageSimple *page3 = new AlgorithmPage(wizard);
  wxWizardPageSimple *page4 = new FinishPage(wizard);
  wxWizardPageSimple::Chain(page1, page2);
  wxWizardPageSimple::Chain(page2, page3);
  wxWizardPageSimple::Chain(page3, page4);

  if (wizard->RunWizard(page1)) {

  }

  wizard->Destroy();
}


void EfgShow::OnInspectSolutions(wxCommandEvent &)
{
  if (m_solutionSashWindow->IsShown()) {
    m_solutionTable->Show(false);
    m_solutionSashWindow->Show(false);
    GetMenuBar()->Check(efgmenuINSPECT_SOLUTIONS, false);
  }
  else {
    m_solutionTable->Show(true);
    m_solutionSashWindow->Show(true);
    GetMenuBar()->Check(efgmenuINSPECT_SOLUTIONS, true);
  }

  AdjustSizes();
}

void EfgShow::OnInspectCursor(wxCommandEvent &)
{
  if (m_cursorWindow->IsShown()) {
    m_cursorWindow->Show(false);
    m_nodeSashWindow->Show(false);
    GetMenuBar()->Check(efgmenuINSPECT_CURSOR, false);
  }
  else {
    m_cursorWindow->Set(m_treeWindow->Cursor());
    m_cursorWindow->Show(true);
    m_nodeSashWindow->Show(true);
    GetMenuBar()->Check(efgmenuINSPECT_CURSOR, true);
  }

  AdjustSizes();
}

void EfgShow::OnInspectInfosets(wxCommandEvent &)
{
  features.iset_hilight = !features.iset_hilight;
  if (!features.iset_hilight) {
    HilightInfoset(0, 0, 1);
    HilightInfoset(0, 0, 2);
  }
  GetMenuBar()->Check(efgmenuINSPECT_INFOSETS, features.iset_hilight);
}

void EfgShow::OnInspectZoom(wxCommandEvent &)
{
  //  m_treeZoomWindow->Show(!m_treeZoomWindow->IsShown());
  AdjustSizes();
}

void EfgShow::OnInspectGameInfo(wxCommandEvent &)
{
  gText tmp;
  char tempstr[200];
  sprintf(tempstr, "Number of Players: %d", m_efg.NumPlayers());
  tmp += tempstr;
  tmp += "\n";
  sprintf(tempstr, "Is %sconstant sum", ((m_efg.IsConstSum()) ? "" : "NOT "));
  tmp += tempstr;
  tmp += "\n";
  sprintf(tempstr, "Is %sperfect recall", ((IsPerfectRecall(m_efg)) ? "" : "NOT "));
  tmp += tempstr;
  tmp += "\n";

  wxMessageBox((char *) tmp, "Efg Game Info", wxOK, this);
}

void EfgShow::OnInspectScript(wxCommandEvent &)
{
  dialogTextWindow dialog(this, "GCL Script", m_script);
  dialog.ShowModal();
}

const float ZOOM_DELTA = .1;
const float ZOOM_MAX = 1;
const float ZOOM_MIN = .2;

void EfgShow::OnSetZoom(wxCommandEvent &p_event)
{
  double zoom = (double) ToNumber(p_event.GetString().c_str()) / 100.0;

  m_treeWindow->SetZoom(zoom);
  m_treeWindow->Refresh();
}

void EfgShow::OnPrefsZoomIn(wxCommandEvent &)
{
  float zoom = m_treeWindow->GetZoom();
  zoom = gmin(zoom + ZOOM_DELTA, ZOOM_MAX);
  m_treeWindow->SetZoom(zoom);
}

void EfgShow::OnPrefsZoomOut(wxCommandEvent &)
{
  float zoom = m_treeWindow->GetZoom();
  zoom = gmax(zoom - ZOOM_DELTA, ZOOM_MIN);
  m_treeWindow->SetZoom(zoom);
}

void EfgShow::OnPrefsLegend(wxCommandEvent &)
{
  dialogLegends dialog(this, m_treeWindow->DrawSettings());

  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetLabelNodeAbove(dialog.GetNodeAbove());
    m_treeWindow->DrawSettings().SetLabelNodeBelow(dialog.GetNodeBelow());
    m_treeWindow->DrawSettings().SetLabelNodeRight(dialog.GetNodeAfter());
    m_treeWindow->DrawSettings().SetLabelBranchAbove(dialog.GetBranchAbove());
    m_treeWindow->DrawSettings().SetLabelBranchBelow(dialog.GetBranchBelow());
  }
}

void EfgShow::OnPrefsFontsAboveNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeAboveFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnPrefsFontsBelowNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeBelowFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnPrefsFontsAfterNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeRightFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnPrefsFontsAboveBranch(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetBranchAboveFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnPrefsFontsBelowBranch(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetBranchBelowFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnPrefsDisplayLayout(wxCommandEvent &)
{
  TreeDrawSettings &settings = m_treeWindow->DrawSettings();

  dialogLayout dialog(this,
		      settings.BranchLength(), settings.NodeLength(),
		      settings.ForkLength(), settings.YSpacing(),
		      settings.ShowInfosets());

  if (dialog.ShowModal() == wxID_OK) {
    settings.SetBranchLength(dialog.BranchLength());
    settings.SetNodeLength(dialog.NodeLength());
    settings.SetForkLength(dialog.ForkLength());
    settings.SetYSpacing(dialog.YSpacing());
    settings.SetShowInfosets(dialog.InfosetStyle());

    m_treeWindow->RefreshLayout();
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnPrefsDisplayDecimals(wxCommandEvent &)
{
  guiSliderDialog dialog(this, "Decimal places", 0, 25,
			 m_treeWindow->DrawSettings().NumDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNumDecimals(dialog.GetValue());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnPrefsColors(wxCommandEvent &)
{
  wxColourData data;
  wxColourDialog dialog(this, &data);
 
  if (dialog.ShowModal() == wxID_OK) {
    if (Cursor()->GetPlayer()) {
      m_treeWindow->DrawSettings().SetPlayerColor(Cursor()->GetPlayer()->GetNumber(),
						  dialog.GetColourData().GetColour());
    }
    else {
      m_treeWindow->DrawSettings().SetPlayerColor(-1,
						  dialog.GetColourData().GetColour());
    }
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnPrefsSave(wxCommandEvent &)
{
  m_treeWindow->DrawSettings().SaveOptions();
}

void EfgShow::OnPrefsLoad(wxCommandEvent &)
{
  m_treeWindow->DrawSettings().LoadOptions();
}

void EfgShow::OnCloseWindow(wxCloseEvent &p_event)
{
  if (p_event.CanVeto() && m_efg.IsDirty()) {
    if (wxMessageBox("Game has been modified.  Close anyway?", "Warning",
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }

  Show(false);
  Destroy();
}

void EfgShow::OnFocus(wxFocusEvent &)
{
  m_treeWindow->SetFocus();
}

void EfgShow::OnSize(wxSizeEvent &)
{
  AdjustSizes();
}

void EfgShow::OnActivate(wxActivateEvent &p_event)
{
  if (p_event.GetActive()) {
    m_parent->SetActiveWindow(this);
  }
}

void EfgShow::OnSashDrag(wxSashEvent &p_event)
{
  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);

  switch (p_event.GetId()) {
  case idNODEWINDOW:
    m_treeWindow->SetSize(p_event.GetDragRect().width,
			  m_treeWindow->GetRect().y,
			  clientWidth - p_event.GetDragRect().width,
			  m_treeWindow->GetRect().height);
    m_nodeSashWindow->SetSize(m_nodeSashWindow->GetRect().x,
			      m_nodeSashWindow->GetRect().y,
			      p_event.GetDragRect().width,
			      m_nodeSashWindow->GetRect().height);
    break;
  case idSOLUTIONWINDOW:
    m_treeWindow->SetSize(m_treeWindow->GetRect().x, m_treeWindow->GetRect().y,
			  m_treeWindow->GetRect().width,
			  clientHeight - p_event.GetDragRect().height - 40);
    m_nodeSashWindow->SetSize(m_nodeSashWindow->GetRect().x,
			      m_nodeSashWindow->GetRect().y,
			      m_nodeSashWindow->GetRect().width,
			      clientHeight - p_event.GetDragRect().height - 40);
    m_solutionSashWindow->SetSize(0, clientHeight - p_event.GetDragRect().height,
				  clientWidth, p_event.GetDragRect().height);
    break;
  }
}

void EfgShow::AdjustSizes(void)
{
  int width, height;
  GetClientSize(&width, &height);
  if (m_solutionTable && m_solutionSashWindow->IsShown()) {
    m_solutionSashWindow->SetSize(0, height - m_solutionSashWindow->GetRect().height,
				  width, m_solutionSashWindow->GetRect().height);
    height -= m_solutionSashWindow->GetRect().height;
  }

  if ((m_cursorWindow && m_nodeSashWindow->IsShown())) {
    if (m_treeWindow) {
      m_treeWindow->SetSize(250, 0, width - 250, height);
    }
  }
  else if (m_treeWindow) {
    m_treeWindow->SetSize(0, 0, width, height);
  }

  if (m_cursorWindow && m_nodeSashWindow->IsShown()) {
    m_nodeSashWindow->SetSize(0, 0, 250, height);
  }

  if (m_treeWindow) {
    m_treeWindow->SetFocus();
  }
}

void EfgShow::SetFileName(const gText &p_name)
{
  if (p_name != "")
    filename = p_name;
  else
    filename = "untitled.efg";

  SetTitle((char *) ("[" + filename + "] " + m_efg.GetTitle()));
}

const EFSupport *EfgShow::GetSupport(void)
{
  return m_currentSupport;
}

int EfgShow::NumDecimals(void) const
{
  return m_treeWindow->NumDecimals();
}

void EfgShow::OnTreeChanged(bool p_nodesChanged, bool p_infosetsChanged)
{
  if (p_infosetsChanged) {
    while (m_supports.Length()) { 
      delete m_supports.Remove(1);
    }

    m_currentSupport = new EFSupport(m_efg);
    m_supports.Append(m_currentSupport);
    m_currentSupport->SetName("Full Support");
  }

  UpdateMenus();
  if (p_nodesChanged || p_infosetsChanged) {
    m_treeWindow->RefreshLayout();
    m_treeWindow->Refresh();
  }
}

void EfgShow::UpdateMenus(void)
{
  Node *cursor = Cursor(), *markNode = m_treeWindow->MarkNode();
  wxMenuBar *menuBar = GetMenuBar();

  menuBar->Enable(efgmenuEDIT_NODE_ADD,
		  (m_efg.NumChildren(cursor) > 0) ? false : true);
  menuBar->Enable(efgmenuEDIT_NODE_DELETE,
		  (m_efg.NumChildren(cursor) > 0) ? true : false);
  menuBar->Enable(efgmenuEDIT_NODE_GOTO_MARK, (markNode) ? true : false);
  menuBar->Enable(efgmenuEDIT_INFOSET_MERGE,
		  (markNode && markNode->GetInfoset() &&
		   cursor->GetInfoset() &&
		   markNode->GetSubgameRoot() == cursor->GetSubgameRoot() &&
		   markNode->GetPlayer() == cursor->GetPlayer()) ? true : false);
  menuBar->Enable(efgmenuEDIT_INFOSET_BREAK, 
		  (cursor->GetInfoset()) ? true : false);
  menuBar->Enable(efgmenuEDIT_INFOSET_SPLIT,
		  (cursor->GetInfoset()) ? true : false);
  menuBar->Enable(efgmenuEDIT_INFOSET_JOIN, 
		  (markNode && markNode->GetInfoset() &&
		   cursor->GetInfoset() &&
		   markNode->GetSubgameRoot() == cursor->GetSubgameRoot()) ? true : false);
  menuBar->Enable(efgmenuEDIT_INFOSET_LABEL,
		  (cursor->GetInfoset()) ? true : false);
  menuBar->Enable(efgmenuEDIT_INFOSET_PLAYER,
		  (cursor->GetInfoset() &&
		   !cursor->GetPlayer()->IsChance()) ? true : false);
  menuBar->Enable(efgmenuEDIT_INFOSET_REVEAL, 
		  (cursor->GetInfoset()) ? true : false);

  menuBar->Enable(efgmenuEDIT_ACTION_LABEL,
		  (cursor->GetInfoset() &&
		   cursor->GetInfoset()->NumActions() > 0) ? true : false);
  menuBar->Enable(efgmenuEDIT_ACTION_INSERT,
		  (m_efg.NumChildren(cursor) > 0) ? true : false);
  menuBar->Enable(efgmenuEDIT_ACTION_APPEND,
		  (m_efg.NumChildren(cursor) > 0) ? true : false);
  menuBar->Enable(efgmenuEDIT_ACTION_DELETE, 
		  (m_efg.NumChildren(cursor) > 0) ? true : false);
  menuBar->Enable(efgmenuEDIT_ACTION_PROBS,
		  (cursor->GetInfoset() &&
		   cursor->GetPlayer()->IsChance()) ? true : false);

  menuBar->Enable(efgmenuEDIT_TREE_DELETE,
		  (m_efg.NumChildren(cursor) > 0) ? true : false);
  menuBar->Enable(efgmenuEDIT_TREE_COPY,
		  (markNode &&
		   cursor->GetSubgameRoot() == markNode->GetSubgameRoot()) ? true : false);
  menuBar->Enable(efgmenuEDIT_TREE_MOVE,
		  (markNode &&
		   cursor->GetSubgameRoot() == markNode->GetSubgameRoot()) ? true : false);

  menuBar->Enable(efgmenuEDIT_OUTCOMES_ATTACH,
		  (m_efg.NumOutcomes() > 0) ? true : false);
  menuBar->Enable(efgmenuEDIT_OUTCOMES_DETACH,
		  (cursor->GetOutcome()) ? true : false);
  menuBar->Enable(efgmenuEDIT_OUTCOMES_LABEL,
		  (cursor->GetOutcome()) ? true : false);
  menuBar->Enable(efgmenuEDIT_OUTCOMES_DELETE,
		  (m_efg.NumOutcomes() > 0) ? true : false);
  
  if (m_treeWindow) {
    menuBar->Check(efgmenuSUPPORT_REACHABLE, m_treeWindow->DrawSettings().RootReachable());
  }

  menuBar->Enable(efgmenuSOLVE_CUSTOM_EFG_LP,
		  m_efg.NumPlayers() == 2 && m_efg.IsConstSum());
  menuBar->Enable(efgmenuSOLVE_CUSTOM_EFG_LCP, m_efg.NumPlayers() == 2);

  menuBar->Enable(efgmenuSOLVE_CUSTOM_NFG_LP,
		  m_efg.NumPlayers() == 2 && m_efg.IsConstSum());
  menuBar->Enable(efgmenuSOLVE_CUSTOM_NFG_LCP, m_efg.NumPlayers() == 2);
  menuBar->Enable(efgmenuSOLVE_CUSTOM_NFG_ENUMMIXED, m_efg.NumPlayers() == 2);

  m_treeWindow->UpdateMenus();
}

Node *EfgShow::Cursor(void) const
{
  return m_treeWindow->Cursor();
}

#include "base/glist.imp"

template class gList<EFSupport *>;




