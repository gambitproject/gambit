//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of extensive form viewing frame
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/fontdlg.h"
#include "wx/printdlg.h"
#include "guishare/wxmisc.h"
#include "guishare/wxstatus.h"

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
#include "efgprint.h"
#include "efgshow.h"
#include "efgprofile.h"
#include "efgnavigate.h"
#include "efgoutcome.h"
#include "efgsupport.h"
#include "efgsoln.h"
#include "nfgshow.h"
#include "efgsolvd.h"

#include "dlefgplayer.h"
#include "dlmoveadd.h"
#include "dlnodedelete.h"
#include "dlefgoutcome.h"
#include "dlefgpayoff.h"
#include "dlefgreveal.h"
#include "dlactionselect.h"
#include "dlactionlabel.h"
#include "dlactionprobs.h"
#include "dlinfosets.h"
#include "dlsubgames.h"
#include "dlefgproperties.h"

#include "dllayout.h"
#include "dllegends.h"

#include "dlelim.h"
#include "dlsupportselect.h"
#include "dlefgeditsupport.h"

#include "algenumpure.h"
#include "algenummixed.h"
#include "alglcp.h"
#include "algliap.h"
#include "alglp.h"
#include "algpolenum.h"
#include "algqre.h"
#include "algqregrid.h"
#include "algsimpdiv.h"

#include "behavedit.h"

//=====================================================================
//                 Implementation of class EfgShow
//=====================================================================

const int idTREEWINDOW = 999;
const int idNODEWINDOW = 998;
const int idTOOLWINDOW = 997;
const int idSOLUTIONWINDOW = 996;
const int idINFONOTEBOOK = 995;

BEGIN_EVENT_TABLE(EfgShow, wxFrame)
  EVT_MENU(wxID_NEW, EfgShow::OnFileNew)
  EVT_MENU(wxID_OPEN, EfgShow::OnFileOpen)
  EVT_MENU(wxID_CLOSE, EfgShow::Close)
  EVT_MENU(wxID_SAVE, EfgShow::OnFileSave)
  EVT_MENU(wxID_SAVEAS, EfgShow::OnFileSave)
  EVT_MENU(wxID_PRINT_SETUP, EfgShow::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, EfgShow::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, EfgShow::OnFilePrint)
  EVT_MENU(wxID_EXIT, EfgShow::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, EfgShow::OnFileMRUFile)
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
  EVT_MENU(efgmenuEDIT_TREE_INFOSETS, EfgShow::OnEditTreeInfosets)
  EVT_MENU(efgmenuEDIT_PROPERTIES, EfgShow::OnEditProperties)
  EVT_MENU(efgmenuVIEW_PROFILES, EfgShow::OnViewProfiles)
  EVT_MENU(efgmenuVIEW_NAVIGATION, EfgShow::OnViewCursor)
  EVT_MENU(efgmenuVIEW_OUTCOMES, EfgShow::OnViewOutcomes)
  EVT_MENU(efgmenuVIEW_SUPPORTS, EfgShow::OnViewSupports)
  EVT_MENU(efgmenuVIEW_INFOSETS, EfgShow::OnViewInfosets)
  EVT_MENU(efgmenuVIEW_ZOOMIN, EfgShow::OnViewZoomIn)
  EVT_MENU(efgmenuVIEW_ZOOMOUT, EfgShow::OnViewZoomOut)
  EVT_MENU(efgmenuFORMAT_LEGEND, EfgShow::OnFormatLegend)
  EVT_MENU(efgmenuFORMAT_FONTS_ABOVENODE, EfgShow::OnFormatFontsAboveNode)
  EVT_MENU(efgmenuFORMAT_FONTS_BELOWNODE, EfgShow::OnFormatFontsBelowNode)
  EVT_MENU(efgmenuFORMAT_FONTS_AFTERNODE, EfgShow::OnFormatFontsAfterNode)
  EVT_MENU(efgmenuFORMAT_FONTS_ABOVEBRANCH, EfgShow::OnFormatFontsAboveBranch)
  EVT_MENU(efgmenuFORMAT_FONTS_BELOWBRANCH, EfgShow::OnFormatFontsBelowBranch)
  EVT_MENU(efgmenuFORMAT_DISPLAY_LAYOUT, EfgShow::OnFormatDisplayLayout)
  EVT_MENU(efgmenuFORMAT_DISPLAY_DECIMALS, EfgShow::OnFormatDisplayDecimals)
  EVT_MENU(efgmenuFORMAT_SAVE, EfgShow::OnFormatSave)
  EVT_MENU(efgmenuFORMAT_LOAD, EfgShow::OnFormatLoad)
  EVT_MENU(efgmenuTOOLS_SUBGAME_MARKALL, EfgShow::OnToolsSubgamesMarkAll)
  EVT_MENU(efgmenuTOOLS_SUBGAME_MARK, EfgShow::OnToolsSubgamesMark)
  EVT_MENU(efgmenuTOOLS_SUBGAME_UNMARKALL, EfgShow::OnToolsSubgamesUnMarkAll)
  EVT_MENU(efgmenuTOOLS_SUBGAME_UNMARK, EfgShow::OnToolsSubgamesUnMark)
  EVT_MENU(efgmenuTOOLS_SUBGAME_VIEW, EfgShow::OnToolsSubgamesView)
  EVT_MENU(efgmenuTOOLS_SUPPORT_UNDOMINATED,
	   EfgShow::OnToolsSupportUndominated)
  EVT_MENU(efgmenuTOOLS_SUPPORT_NEW, EfgShow::OnToolsSupportNew)
  EVT_MENU(efgmenuTOOLS_SUPPORT_EDIT, EfgShow::OnToolsSupportEdit)
  EVT_MENU(efgmenuTOOLS_SUPPORT_DELETE, EfgShow::OnToolsSupportDelete)
  EVT_MENU(efgmenuTOOLS_SUPPORT_SELECT_FROMLIST,
	   EfgShow::OnToolsSupportSelectFromList)
  EVT_MENU(efgmenuTOOLS_SUPPORT_SELECT_PREVIOUS, 
	   EfgShow::OnToolsSupportSelectPrevious)
  EVT_MENU(efgmenuTOOLS_SUPPORT_SELECT_NEXT, EfgShow::OnToolsSupportSelectNext)
  EVT_MENU(efgmenuTOOLS_SUPPORT_REACHABLE, EfgShow::OnToolsSupportReachable)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_STANDARD, 
	   EfgShow::OnToolsEquilibriumStandard)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_ENUMPURE, 
	   EfgShow::OnToolsEquilibriumCustomEfgEnumPure)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_LCP,
	   EfgShow::OnToolsEquilibriumCustomEfgLcp)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_LIAP,
	   EfgShow::OnToolsEquilibriumCustomEfgLiap)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_LP,
	   EfgShow::OnToolsEquilibriumCustomEfgLp)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_POLENUM,
	   EfgShow::OnToolsEquilibriumCustomEfgPolEnum)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_QRE,
	   EfgShow::OnToolsEquilibriumCustomEfgQre)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_ENUMPURE,
	   EfgShow::OnToolsEquilibriumCustomNfgEnumPure)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_ENUMMIXED, 
	   EfgShow::OnToolsEquilibriumCustomNfgEnumMixed)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_LCP,
	   EfgShow::OnToolsEquilibriumCustomNfgLcp)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_LIAP,
	   EfgShow::OnToolsEquilibriumCustomNfgLiap)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_LP, 
	   EfgShow::OnToolsEquilibriumCustomNfgLp)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_POLENUM, 
	   EfgShow::OnToolsEquilibriumCustomNfgPolEnum)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_QRE,
	   EfgShow::OnToolsEquilibriumCustomNfgQre)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_QREGRID,
	   EfgShow::OnToolsEquilibriumCustomNfgQreGrid)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_SIMPDIV, 
	   EfgShow::OnToolsEquilibriumCustomNfgSimpdiv)
  EVT_MENU(efgmenuTOOLS_NFG_REDUCED, EfgShow::OnToolsNormalReduced)
  EVT_MENU(efgmenuTOOLS_NFG_AGENT, EfgShow::OnToolsNormalAgent)
  EVT_MENU(wxID_HELP_CONTENTS, EfgShow::OnHelpContents)
  EVT_MENU(wxID_HELP_INDEX, EfgShow::OnHelpIndex)
  EVT_MENU(wxID_ABOUT, EfgShow::OnHelpAbout)
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
  EVT_NOTEBOOK_PAGE_CHANGED(idINFONOTEBOOK, EfgShow::OnInfoNotebookPage)
END_EVENT_TABLE()

//---------------------------------------------------------------------
//               EfgShow: Constructor and destructor
//---------------------------------------------------------------------

EfgShow::EfgShow(FullEfg &p_efg, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxPoint(0, 0), wxSize(600, 400)),
    EfgClient(&p_efg),
    m_efg(p_efg), m_treeWindow(0), 
    m_treeZoomWindow(0), m_currentProfile(0),
    m_profileTable(0), m_solutionSashWindow(0),
    m_navigateWindow(0), m_outcomeWindow(0), m_supportWindow(0)
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
  entries[2].Set(wxACCEL_CTRL, (int) 'S', wxID_SAVE);
  entries[3].Set(wxACCEL_CTRL, (int) 'P', wxID_PRINT);
  entries[4].Set(wxACCEL_CTRL, (int) 'X', wxID_EXIT);
  entries[5].Set(wxACCEL_NORMAL, WXK_F1, wxID_HELP_CONTENTS);
  entries[6].Set(wxACCEL_NORMAL, (int) '+', efgmenuVIEW_ZOOMIN);
  entries[7].Set(wxACCEL_NORMAL, (int) '-', efgmenuVIEW_ZOOMOUT);
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
  m_treeWindow->RefreshTree();
  m_treeWindow->RefreshLayout();

  m_infoNotebook = new wxNotebook(m_nodeSashWindow, idINFONOTEBOOK);

  m_navigateWindow = new EfgNavigateWindow(this, m_infoNotebook);
  m_navigateWindow->Set(m_treeWindow->Cursor());
  m_navigateWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_navigateWindow, "Navigation");

  m_outcomeWindow = new EfgOutcomeWindow(this, m_infoNotebook);
  m_outcomeWindow->UpdateValues();
  m_navigateWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_outcomeWindow, "Outcomes");

  m_supportWindow = new EfgSupportWindow(this, m_infoNotebook);
  m_supportWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_supportWindow, "Supports");
  m_infoNotebook->SetSelection(0);

  m_nodeSashWindow->Show(false);
  m_nodeSashWindow->SetSashVisible(wxSASH_LEFT, false);
  
  m_solutionSashWindow = new wxSashWindow(this, idSOLUTIONWINDOW,
					  wxDefaultPosition,
					  wxSize(600, 100));
  m_solutionSashWindow->SetSashVisible(wxSASH_TOP, true);

  m_profileTable = new EfgProfileList(this, m_solutionSashWindow);
  m_profileTable->Show(true);
  m_solutionSashWindow->Show(false);

  m_efg.SetIsDirty(false);

  AdjustSizes();
  m_treeWindow->FitZoom();

  Show(true);
}

EfgShow::~EfgShow()
{
  wxGetApp().RemoveGame(&m_efg);
}


//---------------------------------------------------------------------
//               EfgShow: Manipulation of profile list
//---------------------------------------------------------------------

void EfgShow::ChangeProfile(int sol)
{
  m_currentProfile = sol;
  m_treeWindow->RefreshLabels();
  if (m_navigateWindow) {
    m_navigateWindow->Set(m_treeWindow->Cursor());
  }
  if (m_profileTable) {
    m_profileTable->UpdateValues();
  }
}


void EfgShow::RemoveProfiles(void)
{
  m_currentProfile = 0;
  m_profileTable->Flush();
  OnSelectedMoved(0); // update the node inspect window if any
}

const BehavSolution &EfgShow::GetCurrentProfile(void) const
{
  return (*m_profileTable)[m_currentProfile];
}

void EfgShow::AddProfile(const BehavSolution &p_profile, bool p_map)
{
  m_profileTable->Append(p_profile);

  if (m_efg.AssociatedNfg() && p_map) {
    wxGetApp().GetWindow(m_efg.AssociatedNfg())->AddSolution(MixedProfile<gNumber>(p_profile), false);
  }

  m_profileTable->UpdateValues();
  UpdateMenus();
}


gText EfgShow::GetRealizProb(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node) {
    return "";
  }
  return ToText((*m_profileTable)[m_currentProfile].RealizProb(p_node),
		NumDecimals());
}

gText EfgShow::GetBeliefProb(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node || !p_node->GetPlayer()) {
    return "";
  }
  return ToText((*m_profileTable)[m_currentProfile].BeliefProb(p_node),
		NumDecimals());
}

gText EfgShow::GetNodeValue(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node) {
    return "";
  }
  gText tmp = "(";
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    tmp += ToText((*m_profileTable)[m_currentProfile].NodeValue(p_node)[pl], 
		  NumDecimals());
    if (pl < m_efg.NumPlayers()) {
      tmp += ",";
    }
    else {
      tmp += ")";
    }
  }
  return tmp;
}

gText EfgShow::GetInfosetProb(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node || !p_node->GetPlayer()) {
    return "";
  }
  return ToText((*m_profileTable)[m_currentProfile].IsetProb(p_node->GetInfoset()),
		NumDecimals());
}

gText EfgShow::GetInfosetValue(const Node *p_node) const
{
  if (m_currentProfile == 0 || !p_node || !p_node->GetPlayer() ||
      p_node->GetPlayer()->IsChance()) {
    return "";
  }
  if (GetCurrentProfile().IsetProb(p_node->GetInfoset()) > gNumber(0)) {
    return ToText(GetCurrentProfile().IsetValue(p_node->GetInfoset()),
		  NumDecimals());
  }
  else {
    // this is due to a bug in the value computation
    return "";
  }
}

gText EfgShow::GetActionProb(const Node *p_node, int p_act) const
{
  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    return ToText(m_efg.GetChanceProb(p_node->GetInfoset(), p_act),
		  NumDecimals());
  }

  if (m_currentProfile == 0 || !p_node->GetPlayer()) {
    return "";
  }

  return ToText(GetCurrentProfile().ActionProb(p_node->GetInfoset()->Actions()[p_act]),
		NumDecimals());
}

gText EfgShow::GetActionValue(const Node *p_node, int p_act) const
{
  if (m_currentProfile == 0 || !p_node || !p_node->GetPlayer() ||
      p_node->GetPlayer()->IsChance()) {
    return "";
  }

  if (GetCurrentProfile().IsetProb(p_node->GetInfoset()) > gNumber(0)) {
    return ToText(GetCurrentProfile().ActionValue(p_node->GetInfoset()->Actions()[p_act]),
		  NumDecimals());
  }
  else  {
    // this is due to a bug in the value computation
    return "";
  }
}

gNumber EfgShow::ActionProb(const Node *p_node, int p_action) const
{
  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    return m_efg.GetChanceProb(p_node->GetInfoset(), p_action);
  }

  if (m_currentProfile && p_node->GetInfoset()) {
    return (*m_profileTable)[m_currentProfile](p_node->GetInfoset()->Actions()[p_action]);
  }
  return -1;
}

//---------------------------------------------------------------------
//            EfgShow: Coordinating updates of child windows
//---------------------------------------------------------------------

void EfgShow::OnOutcomesEdited(void)
{
  m_treeWindow->OutcomeChange();
  m_treeWindow->Refresh();
  m_outcomeWindow->UpdateValues();
}

void EfgShow::OnSupportsEdited(void)
{
  m_treeWindow->SupportChanged();
  m_supportWindow->UpdateValues();
}


// if who == 2, hilight in the tree display
// if who == 1, hilight in the solution window display
void EfgShow::HilightInfoset(int pl, int iset, int who)
{
  if (!features.iset_hilight) 
    return;

#ifdef NOT_PORTED_YET
  if (who == 1)
    m_profileTable->HilightInfoset(pl, iset);
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

void EfgShow::OnSelectedMoved(const Node *n)
{
  // The only time the inspection window won't be around is on construction
  if (m_navigateWindow) {
    m_navigateWindow->Set(n);
  }
  UpdateMenus();
}

void EfgShow::SetFilename(const wxString &p_name)
{
  m_filename = p_name;
  if (m_filename != "") {
    SetTitle(wxString::Format("Gambit - [%s] %s", m_filename.c_str(), 
			      (char *) m_efg.GetTitle()));
  }
  else {
    SetTitle(wxString::Format("Gambit - %s", (char *) m_efg.GetTitle()));
  }
  wxGetApp().SetFilename(this, p_name.c_str());
}

EFSupport *EfgShow::GetSupport(void)
{
  return m_currentSupport;
}

void EfgShow::SetSupportNumber(int p_number)
{
  if (p_number >= 1 && p_number <= m_supports.Length()) {
    m_currentSupport = m_supports[p_number];
    OnSupportsEdited();
  }
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
    OnSupportsEdited();
  }
  else if (p_nodesChanged) {
    m_treeWindow->RefreshTree();
    m_treeWindow->RefreshLayout();
    m_treeWindow->Refresh();
  }
  
  UpdateMenus();
}

Node *EfgShow::Cursor(void) const
{
  return m_treeWindow->Cursor();
}

//--------------------------------------------------------------------
//          EfgShow: Creating and updating menus and toolbar
//--------------------------------------------------------------------

void EfgShow::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new game");
  fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a saved game");
  fileMenu->Append(wxID_CLOSE, "&Close", "Close this window");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save this game");
  fileMenu->Append(wxID_SAVEAS, "Save &as", "Save game to a different file");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_PRINT_SETUP, "Page Se&tup",
		   "Set up preferences for printing");
  fileMenu->Append(wxID_PREVIEW, "Print Pre&view",
		   "View a preview of the game printout");
  fileMenu->Append(wxID_PRINT, "&Print\tCtrl-P", "Print this game");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, "E&xit\tCtrl-X", "Exit Gambit");

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(wxID_CUT, "Cu&t", "Cut the current selection");
  editMenu->Append(wxID_COPY, "&Copy", "Copy the current selection");
  editMenu->Append(wxID_PASTE, "&Paste", "Paste from clipboard");
  // For the moment, these are not implemented -- leave disabled
  editMenu->Enable(wxID_CUT, false);
  editMenu->Enable(wxID_COPY, false);
  editMenu->Enable(wxID_PASTE, false);
  editMenu->AppendSeparator();

  wxMenu *nodeMenu = new wxMenu;
  nodeMenu->Append(efgmenuEDIT_NODE_ADD, "&Add Move", "Add a move");
  nodeMenu->Append(efgmenuEDIT_NODE_DELETE, "&Delete Move",
		   "Remove move at cursor");
  nodeMenu->Append(efgmenuEDIT_NODE_INSERT, "&Insert Move",
		   "Insert move at cursor");
  nodeMenu->Append(efgmenuEDIT_NODE_LABEL, "&Label", "Label cursor node");
  nodeMenu->AppendSeparator();
  nodeMenu->Append(efgmenuEDIT_NODE_SET_MARK, "Set &Mark", "Mark cursor node");
  nodeMenu->Append(efgmenuEDIT_NODE_GOTO_MARK, "Go&to Mark", 
		   "Goto marked node");

  wxMenu *actionMenu = new wxMenu;
  actionMenu->Append(efgmenuEDIT_ACTION_DELETE, "&Delete",
		     "Delete an action from cursor information set");
  actionMenu->Append(efgmenuEDIT_ACTION_INSERT, "&Insert",
		     "Insert an action in the cursor's information set");
  actionMenu->Append(efgmenuEDIT_ACTION_APPEND, "&Append",
		     "Append an action to the cursor's information set");
  actionMenu->Append(efgmenuEDIT_ACTION_LABEL, "&Label",
		     "Label the actions of the cursor's information set");
  actionMenu->Append(efgmenuEDIT_ACTION_PROBS, "&Probabilities",
		     "Set chance probabilities for the cursor's information set");

  wxMenu *infosetMenu = new wxMenu;
  infosetMenu->Append(efgmenuEDIT_INFOSET_MERGE, "&Merge", 
		      "Merge cursor information set w/ marked");
  infosetMenu->Append(efgmenuEDIT_INFOSET_BREAK, "&Break", 
		      "Make cursor a new information set");
  infosetMenu->Append(efgmenuEDIT_INFOSET_SPLIT, "&Split", 
		      "Split information set at cursor");
  infosetMenu->Append(efgmenuEDIT_INFOSET_JOIN, "&Join",
		      "Join cursor to marked information set");
  infosetMenu->Append(efgmenuEDIT_INFOSET_LABEL, "&Label",
		      "Label cursor information set");
  infosetMenu->Append(efgmenuEDIT_INFOSET_PLAYER, "&Player",
		      "Change player at cursor information set");
  infosetMenu->Append(efgmenuEDIT_INFOSET_REVEAL, "&Reveal", 
		      "Reveal choice at information set to players");

  wxMenu *outcomeMenu = new wxMenu;
  outcomeMenu->Append(efgmenuEDIT_OUTCOMES_NEW, "&New",
		      "Create a new outcome");
  outcomeMenu->Append(efgmenuEDIT_OUTCOMES_DELETE, "Dele&te",
		      "Delete an outcome");
  outcomeMenu->Append(efgmenuEDIT_OUTCOMES_ATTACH, "&Attach",
		      "Attach an outcome to the node at cursor");
  outcomeMenu->Append(efgmenuEDIT_OUTCOMES_DETACH, "&Detach",
		      "Detach the outcome from the node at cursor");
  outcomeMenu->Append(efgmenuEDIT_OUTCOMES_LABEL, "&Label",
		      "Label the outcome at the node at cursor");
  outcomeMenu->Append(efgmenuEDIT_OUTCOMES_PAYOFFS, "&Payoffs",
		      "Set the payoffs for the outcome at the cursor");

  wxMenu *treeMenu = new wxMenu;
  treeMenu->Append(efgmenuEDIT_TREE_COPY, "&Copy",
		   "Copy tree from marked node");
  treeMenu->Append(efgmenuEDIT_TREE_MOVE, "&Move",
		   "Move tree from marked node");
  treeMenu->Append(efgmenuEDIT_TREE_DELETE, "&Delete",
		   "Delete recursively from cursor");
  treeMenu->Append(efgmenuEDIT_TREE_INFOSETS, "&Infosets",
		   "Edit/View infosets");

  editMenu->Append(efgmenuEDIT_NODE, "&Node", nodeMenu, "Edit the node");
  editMenu->Append(efgmenuEDIT_ACTIONS, "&Actions", actionMenu, 
		   "Edit actions");
  editMenu->Append(efgmenuEDIT_INFOSET, "&Infoset", infosetMenu,
		   "Edit infosets");
  editMenu->Append(efgmenuEDIT_OUTCOMES, "&Outcomes", outcomeMenu,
		   "Edit outcomes and payoffs");
  editMenu->Append(efgmenuEDIT_TREE, "&Tree", treeMenu,
		   "Edit the tree");
  editMenu->AppendSeparator();
  editMenu->Append(efgmenuEDIT_PROPERTIES, "Pr&operties",
		   "View and change properties of current selection");

  wxMenu *toolsMenu = new wxMenu;

  wxMenu *toolsSubgameMenu = new wxMenu;
  toolsSubgameMenu->Append(efgmenuTOOLS_SUBGAME_MARKALL, "Mark &All",
			   "Scan tree for subgames");
  toolsSubgameMenu->Append(efgmenuTOOLS_SUBGAME_MARK, "&Mark",
			   "Set node subgame root");
  toolsSubgameMenu->Append(efgmenuTOOLS_SUBGAME_UNMARKALL, "UnMark &All",
			   "Clear all subgame info");
  toolsSubgameMenu->Append(efgmenuTOOLS_SUBGAME_UNMARK, "&UnMark",
			   "Unmark node subgame");
  toolsSubgameMenu->AppendSeparator();
  toolsSubgameMenu->Append(efgmenuTOOLS_SUBGAME_VIEW, "&View",
			   "View all subgames");
  toolsMenu->Append(efgmenuTOOLS_SUBGAME, "&Subgame", toolsSubgameMenu,
		    "Manipulate subgames");

  wxMenu *toolsSupportsMenu = new wxMenu;
  toolsSupportsMenu->Append(efgmenuTOOLS_SUPPORT_UNDOMINATED, "&Undominated",
			    "Find undominated strategies");
  toolsSupportsMenu->Append(efgmenuTOOLS_SUPPORT_NEW, "&New",
			    "Create a new support");
  toolsSupportsMenu->Append(efgmenuTOOLS_SUPPORT_EDIT, "&Edit",
			    "Edit the currently displayed support");
  toolsSupportsMenu->Append(efgmenuTOOLS_SUPPORT_DELETE, "&Delete",
			    "Delete a support");
  wxMenu *supportsSelectMenu = new wxMenu;
  supportsSelectMenu->Append(efgmenuTOOLS_SUPPORT_SELECT_FROMLIST, 
			     "From &List...",
			     "Select a support from the list of defined supports");
  supportsSelectMenu->Append(efgmenuTOOLS_SUPPORT_SELECT_PREVIOUS, "&Previous",
			     "Select the previous support from the list");
  supportsSelectMenu->Append(efgmenuTOOLS_SUPPORT_SELECT_NEXT, "&Next",
			     "Select the next support from the list");
  toolsSupportsMenu->Append(efgmenuTOOLS_SUPPORT_SELECT, "&Select",
			    supportsSelectMenu,
			    "Change the current support");
  toolsSupportsMenu->AppendSeparator();
  toolsSupportsMenu->Append(efgmenuTOOLS_SUPPORT_REACHABLE, "&Root Reachable",
			    "Display only nodes that are support-reachable",
			    true);
  toolsMenu->Append(efgmenuTOOLS_SUPPORT, "S&upport", toolsSupportsMenu,
		    "Manipulate supports");

  wxMenu *toolsEquilibriumMenu = new wxMenu;
  toolsEquilibriumMenu->Append(efgmenuTOOLS_EQUILIBRIUM_STANDARD,
			       "&Standard...", "Standard solutions");

  wxMenu *solveCustomMenu = new wxMenu;
  wxMenu *solveCustomEfgMenu = new wxMenu;
  solveCustomEfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_ENUMPURE, 
			     "EnumPure",
			     "Enumerate pure strategy equilibria");
  solveCustomEfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_LCP, "LCP",
			     "Solve by linear complementarity program");
  solveCustomEfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_LP, "LP",
			     "Solve by linear program");
  solveCustomEfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_LIAP,
			     "Liapunov",
			     "Liapunov function minimization");
  solveCustomEfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_POLENUM, 
			     "PolEnum",
			     "Enumeration by systems of polynomials");
  solveCustomEfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_QRE, "QRE",
			     "Compute quantal response equilibrium"
			     " correspondence");
  solveCustomMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG,
			  "Extensive form", solveCustomEfgMenu,
			  "Solve using extensive form based algorithms");

  wxMenu *solveCustomNfgMenu = new wxMenu;
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_ENUMPURE,
			     "EnumPure",
			     "Enumerate pure strategy equilibria");
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_ENUMMIXED,
			     "EnumMixed",
			     "Enumerate all equilibria");
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_LCP, "LCP",
			     "Solve by linear complementarity program");
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_LP, "LP",
			     "Solve by linear program");
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_LIAP, 
			     "Liapunov",
			     "Liapunov function minimization");
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_SIMPDIV, 
			     "Simpdiv",
			     "Simplicial subdivision");
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_POLENUM,
			     "PolEnum",
			     "Enumeration by systems of polynomials");
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_QRE, "QRE",
			     "Compute quantal response equilibria");
  solveCustomNfgMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_QREGRID, "QRE Grid",
			     "Compute quantal response equilibria");
  solveCustomMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG, "Normal form",
			  solveCustomNfgMenu,
			  "Solve using normal form based algorithms");

  toolsEquilibriumMenu->Append(efgmenuTOOLS_EQUILIBRIUM_CUSTOM, "Custom", 
			       solveCustomMenu, "Select a specific algorithm");

  toolsMenu->Append(efgmenuTOOLS_EQUILIBRIUM, "&Equilibrium",
		    toolsEquilibriumMenu, "Compute (Nash) equilibria");

  wxMenu *toolsNfgMenu = new wxMenu;
  toolsNfgMenu->Append(efgmenuTOOLS_NFG_REDUCED, "Reduced",
		       "Generate reduced normal form");
  toolsNfgMenu->Append(efgmenuTOOLS_NFG_AGENT, "Agent",
		       "Generate agent normal form");
  toolsMenu->Append(efgmenuTOOLS_NFG, "Normal form", toolsNfgMenu,
		    "Create a normal form representation of this game");

  
  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(efgmenuVIEW_PROFILES, "&Profiles",
		   "Display/hide profiles window", true);
  viewMenu->Check(efgmenuVIEW_PROFILES, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(efgmenuVIEW_NAVIGATION, "&Navigation",
		   "Display navigation window", true);
  viewMenu->Check(efgmenuVIEW_NAVIGATION, false);
  viewMenu->Append(efgmenuVIEW_OUTCOMES, "&Outcomes",
		   "Display and edit outcomes", true);
  viewMenu->Check(efgmenuVIEW_OUTCOMES, false);
  viewMenu->Append(efgmenuVIEW_SUPPORTS, "&Supports",
		   "Display and edit supports", true);
  viewMenu->Check(efgmenuVIEW_SUPPORTS, false);
  viewMenu->Append(efgmenuVIEW_INFOSETS, "&Infosets",
		   "Toggle information set highlighting", true);
  viewMenu->Check(efgmenuVIEW_INFOSETS, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(efgmenuVIEW_ZOOMIN, "Zoom &in\t+",
		   "Increase display magnification");
  viewMenu->Append(efgmenuVIEW_ZOOMOUT, "Zoom &out\t-",
		   "Decrease display magnification");
  
  wxMenu *formatMenu = new wxMenu;
  wxMenu *formatDisplayMenu = new wxMenu;
  formatDisplayMenu->Append(efgmenuFORMAT_DISPLAY_DECIMALS, "&Decimal Places",
			   "Set number of decimal places to display");
  formatDisplayMenu->Append(efgmenuFORMAT_DISPLAY_LAYOUT, "&Layout",
			   "Set tree layout parameters");
  formatMenu->Append(efgmenuFORMAT_DISPLAY, "&Display", formatDisplayMenu,
		     "Set display options");
  formatMenu->Append(efgmenuFORMAT_LEGEND, "&Legends...",
		     "Set legends");

  wxMenu *formatFontsMenu = new wxMenu;
  formatFontsMenu->Append(efgmenuFORMAT_FONTS_ABOVENODE, "Above Node",
			 "Font for label above nodes");
  formatFontsMenu->Append(efgmenuFORMAT_FONTS_BELOWNODE, "Below Node",
			 "Font for label below nodes");
  formatFontsMenu->Append(efgmenuFORMAT_FONTS_AFTERNODE, "After Node",
			 "Font for label to right of nodes");
  formatFontsMenu->Append(efgmenuFORMAT_FONTS_ABOVEBRANCH, "Above Branch",
			 "Font for label above branches");
  formatFontsMenu->Append(efgmenuFORMAT_FONTS_BELOWBRANCH, "Below Branch",
			 "Font for label below branches");
  formatMenu->Append(efgmenuFORMAT_FONTS, "&Fonts", formatFontsMenu,
		     "Set display fonts");
  formatMenu->AppendSeparator();
  formatMenu->Append(efgmenuFORMAT_SAVE, "&Save");
  formatMenu->Append(efgmenuFORMAT_LOAD, "&Load");
  
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");
  helpMenu->Append(wxID_HELP_INDEX, "&Index", "Index of help file");
  helpMenu->AppendSeparator();
  helpMenu->Append(wxID_ABOUT, "&About", "About Gambit");

  wxMenuBar *menuBar = new wxMenuBar(wxMB_DOCKABLE);
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(formatMenu, "&Format");
  menuBar->Append(toolsMenu, "&Tools");
  menuBar->Append(helpMenu, "&Help");

  // Set the menu bar
  SetMenuBar(menuBar);
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
		  (!m_efg.GetOutcome(cursor).IsNull()) ? true : false);
  menuBar->Enable(efgmenuEDIT_OUTCOMES_LABEL,
		  (!m_efg.GetOutcome(cursor).IsNull()) ? true : false);
  menuBar->Enable(efgmenuEDIT_OUTCOMES_DELETE,
		  (m_efg.NumOutcomes() > 0) ? true : false);
  
  if (m_treeWindow) {
    menuBar->Check(efgmenuTOOLS_SUPPORT_REACHABLE,
		   m_treeWindow->DrawSettings().RootReachable());
  }

  menuBar->Enable(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_LP,
		  m_efg.NumPlayers() == 2 && m_efg.IsConstSum());
  menuBar->Enable(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_EFG_LCP,
		  m_efg.NumPlayers() == 2);

  menuBar->Enable(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_LP,
		  m_efg.NumPlayers() == 2 && m_efg.IsConstSum());
  menuBar->Enable(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_LCP,
		  m_efg.NumPlayers() == 2);
  menuBar->Enable(efgmenuTOOLS_EQUILIBRIUM_CUSTOM_NFG_ENUMMIXED,
		  m_efg.NumPlayers() == 2);

  m_treeWindow->UpdateMenus();
}

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/preview.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/zoomin.xpm"
#include "bitmaps/zoomout.xpm"
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
  toolBar->AddTool(wxID_SAVE, wxBITMAP(save), wxNullBitmap, false,
		   -1, -1, 0, "Save game", "Save this game");
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_PREVIEW, wxBITMAP(preview), wxNullBitmap,
		   false, -1, -1, 0, "Print Preview",
		   "View a preview of the game printout");
  toolBar->AddTool(wxID_PRINT, wxBITMAP(print), wxNullBitmap, false,
		   -1, -1, 0, "Print", "Print this game");
  toolBar->AddSeparator();

  toolBar->AddTool(efgmenuVIEW_ZOOMIN, wxBITMAP(zoomin), wxNullBitmap,
		   false, -1, -1, 0, "Zoom in", "Increase magnification");
  toolBar->AddTool(efgmenuVIEW_ZOOMOUT, wxBITMAP(zoomout), wxNullBitmap,
		   false, -1, -1, 0, "Zoom out", "Decrease magnification");
  toolBar->AddSeparator();

  toolBar->AddTool(efgmenuTOOLS_NFG_REDUCED, wxBITMAP(table), wxNullBitmap,
		   false, -1, -1, 0, "Normal form",
		   "Generate reduced normal form");
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_HELP_CONTENTS, wxBITMAP(help), wxNullBitmap, false,
		   -1, -1, 0, "Help", "Table of contents");

  toolBar->Realize();
  toolBar->SetRows(1);
}

//----------------------------------------------------------------------
//               EfgShow: Menu handlers - File menu
//----------------------------------------------------------------------

void EfgShow::OnFileNew(wxCommandEvent &)
{
  wxGetApp().OnFileNew(this);
}

void EfgShow::OnFileOpen(wxCommandEvent &)
{
  wxGetApp().OnFileOpen(this);
}

void EfgShow::OnFileSave(wxCommandEvent &p_event)
{
  if (p_event.GetId() == wxID_SAVEAS || m_filename == "") {
    wxFileDialog dialog(this, "Choose file", wxPathOnly(m_filename),
			wxFileNameFromPath(m_filename), "*.efg",
			wxSAVE | wxOVERWRITE_PROMPT);

    switch (dialog.ShowModal()) {
    case wxID_OK:
      SetFilename(dialog.GetPath());
      break;
    case wxID_CANCEL:
    default:
      return;
    }
  }

  FullEfg *efg = 0;
  try {
    gFileOutput file(m_filename);
    efg = CompressEfg(m_efg, *GetSupport());
    efg->WriteEfgFile(file, 6);
    delete efg;
  }
  catch (gFileOutput::OpenFailed &) {
    wxMessageBox(wxString::Format("Could not open %s for writing.",
				  m_filename.c_str()),
		 "Error", wxOK, this);
    if (efg)  delete efg;
  }
  catch (gFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format("Write error occurred in saving %s.\n",
				  m_filename.c_str()),
		 "Error", wxOK, this);
    if (efg)  delete efg;
  }
  catch (Efg::Game::Exception &) {
    wxMessageBox("Internal exception in extensive form", "Error",
		 wxOK, this);
    if (efg)  delete efg;
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

void EfgShow::OnFileExit(wxCommandEvent &)
{
  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
}

void EfgShow::OnFileMRUFile(wxCommandEvent &p_event)
{
  wxGetApp().OnFileMRUFile(p_event);
}

//----------------------------------------------------------------------
//             EfgShow: Menu handlers - Edit->Node menu
//----------------------------------------------------------------------

void EfgShow::OnEditNodeAdd(wxCommandEvent &)
{
  static int branches = 2; // make this static so it remembers the last entry
  static EFPlayer *player = 0;
  static Infoset *infoset = 0;
  static Efg::Game *last_ef = 0; // need this to make sure player,infoset are valid

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
      }
      else {
	m_efg.AppendNode(Cursor(), infoset);
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
  static Efg::Game *last_ef = 0; // need this to make sure player,infoset are valid

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

//----------------------------------------------------------------------
//            EfgShow: Menu handlers - Edit->Action menu
//----------------------------------------------------------------------

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
      OnSupportsEdited();
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

//----------------------------------------------------------------------
//           EfgShow: Menu handlers - Edit->Outcomes menu
//----------------------------------------------------------------------

void EfgShow::OnEditOutcomesAttach(wxCommandEvent &)
{
  dialogEfgOutcomeSelect dialog(m_efg, this);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_efg.SetOutcome(Cursor(), dialog.GetOutcome());
    m_treeWindow->OutcomeChange();
    m_treeWindow->Refresh();
    UpdateMenus();
  }
}

void EfgShow::OnEditOutcomesDetach(wxCommandEvent &)
{
  m_efg.SetOutcome(Cursor(), m_efg.GetNullOutcome());
  m_treeWindow->OutcomeChange();
  m_treeWindow->Refresh();
  UpdateMenus();
}

void EfgShow::OnEditOutcomesLabel(wxCommandEvent &)
{
  Efg::Outcome outcome = m_efg.GetOutcome(Cursor());

  wxTextEntryDialog dialog(this, "New outcome label", "Label outcome",
			   (char *) m_efg.GetOutcomeName(outcome));

  if (dialog.ShowModal() == wxID_OK) {
    m_efg.SetOutcomeName(outcome, dialog.GetValue().c_str());
    OnOutcomesEdited();
  }
}

void EfgShow::OnEditOutcomesPayoffs(wxCommandEvent &)
{
  dialogEfgPayoffs dialog(m_efg, m_efg.GetOutcome(Cursor()), this);

  if (dialog.ShowModal() == wxID_OK) {
    Efg::Outcome outcome = m_efg.GetOutcome(Cursor());
    gArray<gNumber> payoffs(dialog.Payoffs());

    if (!outcome.IsNull()) {
      outcome = m_efg.NewOutcome();
      m_efg.SetOutcome(Cursor(), outcome);
    }

    for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
      m_efg.SetPayoff(outcome, pl, payoffs[pl]);
    }
    m_efg.SetOutcomeName(outcome, dialog.Name());

    m_treeWindow->OutcomeChange();
    m_treeWindow->Refresh();
    m_outcomeWindow->UpdateValues();
    UpdateMenus();
  }
}

void EfgShow::OnEditOutcomesNew(wxCommandEvent &)
{
  dialogEfgPayoffs dialog(m_efg, m_efg.GetNullOutcome(), this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      Efg::Outcome outcome = m_efg.NewOutcome();
      gArray<gNumber> payoffs(dialog.Payoffs());

      for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
	m_efg.SetPayoff(outcome, pl, payoffs[pl]);
      }
      m_efg.SetOutcomeName(outcome, dialog.Name());
      
      m_treeWindow->OutcomeChange();
      m_treeWindow->Refresh();
      m_outcomeWindow->UpdateValues();
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

//----------------------------------------------------------------------
//           EfgShow: Menu handlers - Edit->Infoset menu
//----------------------------------------------------------------------

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
    OnSupportsEdited();
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

//----------------------------------------------------------------------
//           EfgShow: Menu handlers - Edit->Tree menu
//----------------------------------------------------------------------

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

void EfgShow::OnEditTreeInfosets(wxCommandEvent &)
{
  dialogInfosets dialog(m_efg, this);
  dialog.ShowModal();
}

void EfgShow::OnEditProperties(wxCommandEvent &)
{
  dialogEfgProperties dialog(this, m_efg, m_filename);
  if (dialog.ShowModal() == wxID_OK) {
    m_efg.SetTitle(dialog.GetGameTitle().c_str());
    SetFilename(Filename());
    m_efg.SetComment(dialog.GetComment().c_str());
    for (int pl = 1; pl <= dialog.NumPlayers(); pl++) {
      if (pl > m_efg.NumPlayers()) {
	m_efg.NewPlayer()->SetName(dialog.GetPlayerName(pl).c_str());
      }
      else {
	m_efg.Players()[pl]->SetName(dialog.GetPlayerName(pl).c_str());
      }
    }
  }
}

//----------------------------------------------------------------------
//                EfgShow: Menu handlers - View menu
//----------------------------------------------------------------------

void EfgShow::OnViewProfiles(wxCommandEvent &)
{
  if (m_solutionSashWindow->IsShown()) {
    m_profileTable->Show(false);
    m_solutionSashWindow->Show(false);
    GetMenuBar()->Check(efgmenuVIEW_PROFILES, false);
  }
  else {
    m_profileTable->Show(true);
    m_solutionSashWindow->Show(true);
    GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
  }

  AdjustSizes();
}

void EfgShow::OnViewCursor(wxCommandEvent &)
{
  if (m_nodeSashWindow->IsShown() && m_infoNotebook->GetSelection() != 0) {
    m_infoNotebook->SetSelection(0);
    m_navigateWindow->Show(true);
    GetMenuBar()->Check(efgmenuVIEW_NAVIGATION, true);
    GetMenuBar()->Check(efgmenuVIEW_OUTCOMES, false);
    GetMenuBar()->Check(efgmenuVIEW_SUPPORTS, false);
  }
  else if (m_nodeSashWindow->IsShown()) {
    m_nodeSashWindow->Show(false);
    GetMenuBar()->Check(efgmenuVIEW_NAVIGATION, false);
  }
  else {
    m_nodeSashWindow->Show(true);
    m_infoNotebook->SetSelection(0);
    GetMenuBar()->Check(efgmenuVIEW_NAVIGATION, true);
  }

  AdjustSizes();
}

void EfgShow::OnViewOutcomes(wxCommandEvent &)
{
  if (m_nodeSashWindow->IsShown() && m_infoNotebook->GetSelection() != 1) {
    m_infoNotebook->SetSelection(1);
    m_navigateWindow->Show(true);
    GetMenuBar()->Check(efgmenuVIEW_OUTCOMES, true);
    GetMenuBar()->Check(efgmenuVIEW_NAVIGATION, false);
    GetMenuBar()->Check(efgmenuVIEW_SUPPORTS, false);
  }
  else if (m_nodeSashWindow->IsShown()) {
    m_nodeSashWindow->Show(false);
    GetMenuBar()->Check(efgmenuVIEW_OUTCOMES, false);
  }
  else {
    m_nodeSashWindow->Show(true);
    m_infoNotebook->SetSelection(1);
    GetMenuBar()->Check(efgmenuVIEW_OUTCOMES, true);
  }

  AdjustSizes();
}

void EfgShow::OnViewSupports(wxCommandEvent &)
{
  if (m_nodeSashWindow->IsShown() && m_infoNotebook->GetSelection() != 2) {
    m_infoNotebook->SetSelection(2);
    GetMenuBar()->Check(efgmenuVIEW_NAVIGATION, false);
    GetMenuBar()->Check(efgmenuVIEW_OUTCOMES, false);
    GetMenuBar()->Check(efgmenuVIEW_SUPPORTS, true);
  }
  else if (m_nodeSashWindow->IsShown()) {
    m_nodeSashWindow->Show(false);
    GetMenuBar()->Check(efgmenuVIEW_SUPPORTS, false);
  }
  else {
    m_nodeSashWindow->Show(true);
    m_infoNotebook->SetSelection(2);
    GetMenuBar()->Check(efgmenuVIEW_SUPPORTS, true);
  }

  AdjustSizes();
}

void EfgShow::OnViewInfosets(wxCommandEvent &)
{
  features.iset_hilight = !features.iset_hilight;
  if (!features.iset_hilight) {
    HilightInfoset(0, 0, 1);
    HilightInfoset(0, 0, 2);
  }
  GetMenuBar()->Check(efgmenuVIEW_INFOSETS, features.iset_hilight);
}

const float ZOOM_DELTA = .1;
const float ZOOM_MAX = 1;
const float ZOOM_MIN = .2;

void EfgShow::OnViewZoomIn(wxCommandEvent &)
{
  float zoom = m_treeWindow->GetZoom();
  zoom = gmin(zoom + ZOOM_DELTA, ZOOM_MAX);
  m_treeWindow->SetZoom(zoom);
}

void EfgShow::OnViewZoomOut(wxCommandEvent &)
{
  float zoom = m_treeWindow->GetZoom();
  zoom = gmax(zoom - ZOOM_DELTA, ZOOM_MIN);
  m_treeWindow->SetZoom(zoom);
}

//----------------------------------------------------------------------
//               EfgShow: Menu handlers - Format menu
//----------------------------------------------------------------------

void EfgShow::OnFormatLegend(wxCommandEvent &)
{
  dialogLegends dialog(this, m_treeWindow->DrawSettings());

  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeAboveLabel(dialog.GetNodeAbove());
    m_treeWindow->DrawSettings().SetNodeBelowLabel(dialog.GetNodeBelow());
    m_treeWindow->DrawSettings().SetNodeRightLabel(dialog.GetNodeAfter());
    m_treeWindow->DrawSettings().SetBranchAboveLabel(dialog.GetBranchAbove());
    m_treeWindow->DrawSettings().SetBranchBelowLabel(dialog.GetBranchBelow());
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsAboveNode(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_treeWindow->DrawSettings().NodeAboveFont());
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeAboveFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsBelowNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeBelowFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsAfterNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeRightFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsAboveBranch(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetBranchAboveFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsBelowBranch(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetBranchBelowFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatDisplayLayout(wxCommandEvent &)
{
  TreeDrawSettings &settings = m_treeWindow->DrawSettings();

  dialogLayout dialog(this, settings);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.GetSettings(settings);
    m_treeWindow->RefreshLayout();
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnFormatDisplayDecimals(wxCommandEvent &)
{
  guiSliderDialog dialog(this, "Decimal places", 0, 25,
			 m_treeWindow->DrawSettings().NumDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNumDecimals(dialog.GetValue());
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnFormatSave(wxCommandEvent &)
{
  m_treeWindow->DrawSettings().SaveOptions();
}

void EfgShow::OnFormatLoad(wxCommandEvent &)
{
  m_treeWindow->DrawSettings().LoadOptions();
}

//----------------------------------------------------------------------
//           EfgShow: Menu handlers - Tools->Subgames menu
//----------------------------------------------------------------------

void EfgShow::OnToolsSubgamesMarkAll(wxCommandEvent &)
{
  m_treeWindow->SubgameMarkAll();
  m_treeWindow->Refresh();
}

void EfgShow::OnToolsSubgamesMark(wxCommandEvent &)
{
  m_treeWindow->SubgameMark();
  m_treeWindow->Refresh();
}

void EfgShow::OnToolsSubgamesUnMarkAll(wxCommandEvent &)
{
  m_treeWindow->SubgameUnmarkAll();
  m_treeWindow->Refresh();
}

void EfgShow::OnToolsSubgamesUnMark(wxCommandEvent &)
{
  m_treeWindow->SubgameUnmark();
  m_treeWindow->Refresh();
}

void EfgShow::OnToolsSubgamesView(wxCommandEvent &)
{
  dialogSubgames dialog(this, m_efg);
  dialog.ShowModal();
}

//----------------------------------------------------------------------
//          EfgShow: Menu handlers - Tools->Support menu
//----------------------------------------------------------------------

void EfgShow::OnToolsSupportNew(wxCommandEvent &)
{
  EFSupport newSupport(m_efg);
  newSupport.SetName(UniqueSupportName());
  dialogEfgEditSupport dialog(newSupport, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      EFSupport *support = new EFSupport(dialog.Support());
      m_supports.Append(support);

      m_currentSupport = support;
      OnSupportsEdited();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void EfgShow::OnToolsSupportEdit(wxCommandEvent &)
{
  dialogEfgEditSupport dialog(*m_currentSupport, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      *m_currentSupport = dialog.Support();
      m_currentSupport->SetName(dialog.Name());
      OnSupportsEdited();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void EfgShow::OnToolsSupportDelete(wxCommandEvent &)
{
  if (m_supports.Length() == 1)  return;

  dialogSupportSelect dialog(this, m_supports, m_currentSupport,
			     "Delete Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      delete m_supports.Remove(dialog.Selected());
      if (!m_supports.Find(m_currentSupport)) {
	m_currentSupport = m_supports[1];
	OnSupportsEdited();
      }
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void EfgShow::OnToolsSupportSelectFromList(wxCommandEvent &)
{
  dialogSupportSelect dialog(this, m_supports, m_currentSupport, 
			     "Select Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_currentSupport = m_supports[dialog.Selected()];
      OnSupportsEdited();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void EfgShow::OnToolsSupportSelectPrevious(wxCommandEvent &)
{
  int index = m_supports.Find(m_currentSupport);
  if (index == 1) {
    m_currentSupport = m_supports[m_supports.Length()];
  }
  else {
    m_currentSupport = m_supports[index - 1];
  }
  OnSupportsEdited();
}

void EfgShow::OnToolsSupportSelectNext(wxCommandEvent &)
{
  int index = m_supports.Find(m_currentSupport);
  if (index == m_supports.Length()) {
    m_currentSupport = m_supports[1];
  }
  else {
    m_currentSupport = m_supports[index + 1];
  }
  OnSupportsEdited();
}


void EfgShow::OnToolsSupportUndominated(wxCommandEvent &)
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
      OnSupportsEdited();
    }
  }
}

void EfgShow::OnToolsSupportReachable(wxCommandEvent &)
{
  m_treeWindow->DrawSettings().SetRootReachable(!m_treeWindow->DrawSettings().RootReachable());
  m_treeWindow->RefreshLayout();
  m_treeWindow->Refresh();
}

//----------------------------------------------------------------------
//            EfgShow: Menu handlers - Tools->Equilibrium
//----------------------------------------------------------------------

void EfgShow::OnToolsEquilibriumStandard(wxCommandEvent &)
{
  // This is a guard against trying to solve the "trivial" game.
  // Most of the GUI code assumes information sets exist.
  if (m_efg.NumPlayerInfosets() == 0)  return;

  bool isPerfectRecall = IsPerfectRecall(m_efg);

  if (!isPerfectRecall &&
      wxGetApp().GetPreferences().WarnOnSolveImperfectRecall()) {
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

#ifdef COMMENTED_OUT
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
      AddProfile(solutions[soln], true);
    }
    wxEndBusyCursor();
  }
  catch (gException &E) {
    wxEndBusyCursor();
    guiExceptionDialog(E.Description(), this);
  }

  delete solver;

  ChangeProfile(m_profileTable->Length());
  UpdateMenus();
  if (!m_solutionSashWindow->IsShown())  {
    m_profileTable->Show(true);
    m_solutionSashWindow->Show(true);
    GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
    AdjustSizes();
  }
#endif  // COMMENTED_OUT
}

void EfgShow::OnToolsEquilibriumCustomEfgEnumPure(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (EnumPureEfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomEfgLcp(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (LcpEfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomEfgLiap(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (LiapEfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomEfgLp(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (LpEfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomEfgPolEnum(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (PolEnumEfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomEfgQre(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (QreEfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgEnumPure(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (EnumPureNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgEnumMixed(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (EnumMixedNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgLcp(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (LcpNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgLiap(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (LiapNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgLp(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (LpNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgPolEnum(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (PolEnumNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgQre(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (QreNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgQreGrid(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (QreGridNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsEquilibriumCustomNfgSimpdiv(wxCommandEvent &)
{
  gList<BehavSolution> solutions;
  if (SimpdivNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddProfile(solutions[soln], true);
    }
    
    ChangeProfile(m_profileTable->Length());
    UpdateMenus();
    if (!m_solutionSashWindow->IsShown())  {
      m_profileTable->Show(true);
      m_solutionSashWindow->Show(true);
      GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
      AdjustSizes();
    }
  }
}

void EfgShow::OnToolsNormalReduced(wxCommandEvent &)
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
    nfgShow->SetFilename("");
    wxGetApp().AddGame(&m_efg, nfg, nfgShow);

    for (int i = 1; i <= m_profileTable->Length(); i++) {
      nfgShow->AddSolution(MixedProfile<gNumber>((*m_profileTable)[i]), false);
    }
  }
  else {
    wxMessageBox("Could not create normal form game.\n",
		 "Reduced normal form", wxOK);
  }
}

void EfgShow::OnToolsNormalAgent(wxCommandEvent &)
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

//----------------------------------------------------------------------
//                 EfgShow: Menu handlers - Help menu
//----------------------------------------------------------------------

void EfgShow::OnHelpContents(wxCommandEvent &)
{
  wxGetApp().OnHelpContents();
}

void EfgShow::OnHelpIndex(wxCommandEvent &)
{
  wxGetApp().OnHelpIndex();
}

void EfgShow::OnHelpAbout(wxCommandEvent &)
{
  wxGetApp().OnHelpAbout(this);
}

//----------------------------------------------------------------------
//              EfgShow: Menu handlers - Profiles menu
//----------------------------------------------------------------------

void EfgShow::OnProfilesNew(wxCommandEvent &)
{
  BehavSolution profile = BehavProfile<gNumber>(EFSupport(m_efg));

  dialogBehavEditor dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddProfile(dialog.GetProfile(), true);
    ChangeProfile(m_profileTable->Length());
  }
}

void EfgShow::OnProfilesClone(wxCommandEvent &)
{
  BehavSolution profile((*m_profileTable)[m_currentProfile]);

  dialogBehavEditor dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddProfile(dialog.GetProfile(), true);
    ChangeProfile(m_profileTable->Length());
  }
}

void EfgShow::OnProfilesRename(wxCommandEvent &)
{
  if (m_currentProfile > 0) {
    wxTextEntryDialog dialog(this, "Enter new name for profile",
			     "Rename profile",
			     (char *) (*m_profileTable)[m_currentProfile].GetName());

    if (dialog.ShowModal() == wxID_OK) {
      (*m_profileTable)[m_currentProfile].SetName(dialog.GetValue().c_str());
      m_profileTable->UpdateValues();
    }
  }
}

void EfgShow::OnProfilesEdit(wxCommandEvent &)
{
  if (m_currentProfile > 0) {
    dialogBehavEditor dialog(this, (*m_profileTable)[m_currentProfile]);

    if (dialog.ShowModal() == wxID_OK) {
      (*m_profileTable)[m_currentProfile] = dialog.GetProfile();
      ChangeProfile(m_currentProfile);
    }
  }
}

void EfgShow::OnProfilesDelete(wxCommandEvent &)
{
  m_profileTable->Remove(m_currentProfile);
  m_currentProfile = (m_profileTable->Length() > 0) ? 1 : 0;
  ChangeProfile(m_currentProfile);
}


//----------------------------------------------------------------------
//                  EfgShow: Non-menu event handlers
//----------------------------------------------------------------------

void EfgShow::OnInfoNotebookPage(wxNotebookEvent &p_event)
{
  if (!m_nodeSashWindow->IsShown()) {
    return;
  }

  GetMenuBar()->Check(efgmenuVIEW_OUTCOMES, false);
  GetMenuBar()->Check(efgmenuVIEW_NAVIGATION, false);
  GetMenuBar()->Check(efgmenuVIEW_SUPPORTS, false);

  switch (p_event.GetSelection()) {
  case 0:
    GetMenuBar()->Check(efgmenuVIEW_NAVIGATION, true);
    break;
  case 1:
    GetMenuBar()->Check(efgmenuVIEW_OUTCOMES, true);
    break;
  case 2:
    GetMenuBar()->Check(efgmenuVIEW_SUPPORTS, true);
    break;
  default:
    break;
  }
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
			  clientHeight - p_event.GetDragRect().height);
    m_nodeSashWindow->SetSize(m_nodeSashWindow->GetRect().x,
			      m_nodeSashWindow->GetRect().y,
			      m_nodeSashWindow->GetRect().width,
			      clientHeight - p_event.GetDragRect().height);
    m_solutionSashWindow->SetSize(0, clientHeight - p_event.GetDragRect().height,
				  clientWidth, p_event.GetDragRect().height);
    break;
  }
}

void EfgShow::AdjustSizes(void)
{
  int width, height;
  GetClientSize(&width, &height);
  if (m_profileTable && m_solutionSashWindow->IsShown()) {
    m_solutionSashWindow->SetSize(0, height - m_solutionSashWindow->GetRect().height,
				  width, m_solutionSashWindow->GetRect().height);
    height -= m_solutionSashWindow->GetRect().height;
  }

  if ((m_navigateWindow && m_nodeSashWindow->IsShown())) {
    if (m_treeWindow) {
      m_treeWindow->SetSize(250, 0, width - 250, height);
    }
  }
  else if (m_treeWindow) {
    m_treeWindow->SetSize(0, 0, width, height);
  }

  if (m_navigateWindow && m_nodeSashWindow->IsShown()) {
    m_nodeSashWindow->SetSize(0, 0, 250, height);
  }

  if (m_treeWindow) {
    m_treeWindow->SetFocus();
  }
}

#include "base/glist.imp"

template class gList<EFSupport *>;




