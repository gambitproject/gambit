//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of extensive form viewing frame
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/fontdlg.h"
#include "wx/printdlg.h"
#if !defined(__WXMSW__) || wxUSE_POSTSCRIPT
#include "wx/dcps.h"
#endif  // !defined(__WXMSW__) || wxUSE_POSTSCRIPT
#include "dlspinctrl.h"
#include "wxstatus.h"

#include "math/gmath.h"
#include "game/efg.h"
#include "game/infoset.h"
#include "game/node.h"
#include "game/efplayer.h"
#include "game/efgutils.h"
#include "nash/behavsol.h"
#include "game/nfg.h"
#include "nash/efglogit.h"

#include "efgconst.h"
#include "treewin.h"
#include "efgprint.h"
#include "efgshow.h"
#include "efgprofile.h"
#include "efgnavigate.h"
#include "efgoutcome.h"
#include "efgsupport.h"
#include "nfgshow.h"

#include "dlinsertmove.h"
#include "dlefgdelete.h"
#include "dlefgreveal.h"
#include "dleditnode.h"
#include "dleditmove.h"
#include "dleditefg.h"
#include "dlefglayout.h"
#include "dlefglegend.h"
#include "dlefgcolor.h"
#include "dlelimbehav.h"
#include "dlefgnash.h"
#include "dlqrefile.h"
#include "dlreport.h"
#include "dleditbehav.h"


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
  EVT_MENU(efgmenuFILE_IMPORT_COMLAB, EfgShow::OnFileImportComLab)
  EVT_MENU(efgmenuFILE_EXPORT_BMP, EfgShow::OnFileExportBMP)
  EVT_MENU(efgmenuFILE_EXPORT_JPEG, EfgShow::OnFileExportJPEG)
  EVT_MENU(efgmenuFILE_EXPORT_PNG, EfgShow::OnFileExportPNG)
  EVT_MENU(efgmenuFILE_EXPORT_POSTSCRIPT, EfgShow::OnFileExportPS)
  EVT_MENU(wxID_PRINT_SETUP, EfgShow::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, EfgShow::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, EfgShow::OnFilePrint)
  EVT_MENU(wxID_EXIT, EfgShow::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, EfgShow::OnFileMRUFile)
  EVT_MENU(wxID_CUT, EfgShow::OnEditCut)
  EVT_MENU(wxID_COPY, EfgShow::OnEditCopy)
  EVT_MENU(wxID_PASTE, EfgShow::OnEditPaste)
  EVT_MENU(efgmenuEDIT_INSERT, EfgShow::OnEditInsert)
  EVT_MENU(efgmenuEDIT_DELETE, EfgShow::OnEditDelete)
  EVT_MENU(efgmenuEDIT_REVEAL, EfgShow::OnEditReveal)
  EVT_MENU(efgmenuEDIT_TOGGLE_SUBGAME, EfgShow::OnEditToggleSubgame)
  EVT_MENU(efgmenuEDIT_MARK_SUBGAME_TREE, EfgShow::OnEditMarkSubgameTree)
  EVT_MENU(efgmenuEDIT_UNMARK_SUBGAME_TREE, EfgShow::OnEditUnmarkSubgameTree)
  EVT_MENU(efgmenuEDIT_NODE, EfgShow::OnEditNode)
  EVT_MENU(efgmenuEDIT_MOVE, EfgShow::OnEditMove)
  EVT_MENU(efgmenuEDIT_GAME, EfgShow::OnEditGame)
  EVT_MENU(efgmenuVIEW_PROFILES, EfgShow::OnViewProfiles)
  EVT_MENU(efgmenuVIEW_NAVIGATION, EfgShow::OnViewCursor)
  EVT_MENU(efgmenuVIEW_OUTCOMES, EfgShow::OnViewOutcomes)
  EVT_MENU(efgmenuVIEW_SUPPORTS, EfgShow::OnViewSupports)
  EVT_MENU(efgmenuVIEW_ZOOMIN, EfgShow::OnViewZoomIn)
  EVT_MENU(efgmenuVIEW_ZOOMOUT, EfgShow::OnViewZoomOut)
  EVT_MENU(efgmenuVIEW_SUPPORT_REACHABLE, EfgShow::OnViewSupportReachable)
  EVT_MENU(efgmenuFORMAT_FONTS_ABOVENODE, EfgShow::OnFormatFontsAboveNode)
  EVT_MENU(efgmenuFORMAT_FONTS_BELOWNODE, EfgShow::OnFormatFontsBelowNode)
  EVT_MENU(efgmenuFORMAT_FONTS_AFTERNODE, EfgShow::OnFormatFontsAfterNode)
  EVT_MENU(efgmenuFORMAT_FONTS_ABOVEBRANCH, EfgShow::OnFormatFontsAboveBranch)
  EVT_MENU(efgmenuFORMAT_FONTS_BELOWBRANCH, EfgShow::OnFormatFontsBelowBranch)
  EVT_MENU(efgmenuFORMAT_DISPLAY_LAYOUT, EfgShow::OnFormatDisplayLayout)
  EVT_MENU(efgmenuFORMAT_DISPLAY_LEGEND, EfgShow::OnFormatDisplayLegend)
  EVT_MENU(efgmenuFORMAT_DISPLAY_COLORS, EfgShow::OnFormatDisplayColors)
  EVT_MENU(efgmenuFORMAT_DISPLAY_DECIMALS, EfgShow::OnFormatDisplayDecimals)
  EVT_MENU(efgmenuTOOLS_DOMINANCE, EfgShow::OnToolsDominance)
  EVT_MENU(efgmenuTOOLS_EQUILIBRIUM, EfgShow::OnToolsEquilibrium)
  EVT_MENU(efgmenuTOOLS_QRE, EfgShow::OnToolsQre)
  EVT_MENU(efgmenuTOOLS_NFG_REDUCED, EfgShow::OnToolsNormalReduced)
  EVT_MENU(efgmenuTOOLS_NFG_AGENT, EfgShow::OnToolsNormalAgent)
  EVT_MENU(wxID_ABOUT, EfgShow::OnHelpAbout)
  EVT_MENU(efgmenuSUPPORT_DUPLICATE, EfgShow::OnSupportDuplicate)
  EVT_MENU(efgmenuSUPPORT_DELETE, EfgShow::OnSupportDelete)
  EVT_MENU(efgmenuPROFILES_NEW, EfgShow::OnProfilesNew)
  EVT_MENU(efgmenuPROFILES_DUPLICATE, EfgShow::OnProfilesDuplicate)
  EVT_MENU(efgmenuPROFILES_DELETE, EfgShow::OnProfilesDelete)
  EVT_MENU(efgmenuPROFILES_PROPERTIES, EfgShow::OnProfilesProperties)
  EVT_MENU(efgmenuPROFILES_REPORT, EfgShow::OnProfilesReport)
  EVT_LIST_ITEM_ACTIVATED(idEFG_SOLUTION_LIST, EfgShow::OnProfilesProperties)
  EVT_LIST_ITEM_SELECTED(idEFG_SOLUTION_LIST, EfgShow::OnProfileSelected)
  EVT_SET_FOCUS(EfgShow::OnFocus)
  EVT_SIZE(EfgShow::OnSize)
  EVT_CLOSE(EfgShow::OnCloseWindow)
  EVT_SASH_DRAGGED_RANGE(idSOLUTIONWINDOW, idTREEWINDOW, EfgShow::OnSashDrag)
  EVT_NOTEBOOK_PAGE_CHANGED(idINFONOTEBOOK, EfgShow::OnInfoNotebookPage)
END_EVENT_TABLE()

//---------------------------------------------------------------------
//               EfgShow: Constructor and destructor
//---------------------------------------------------------------------

EfgShow::EfgShow(gbtGameDocument *p_game, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxPoint(0, 0), wxSize(600, 400)),
    gbtGameView(p_game),
    m_treeWindow(0),
    m_profileTable(0), m_solutionSashWindow(0),
    m_navigateWindow(0), m_outcomeWindow(0), m_supportWindow(0)
{
  SetSizeHints(300, 300);

  // Give the frame an icon
#ifdef __WXMSW__
  SetIcon(wxIcon("efg_icn"));
#else
#include "bitmaps/efg.xbm"
  SetIcon(wxIcon(efg_bits, efg_width, efg_height));
#endif

  CreateStatusBar();

  wxAcceleratorEntry entries[7];
  entries[0].Set(wxACCEL_CTRL, (int) 'N', wxID_NEW);
  entries[1].Set(wxACCEL_CTRL, (int) 'O', wxID_OPEN);
  entries[2].Set(wxACCEL_CTRL, (int) 'S', wxID_SAVE);
  entries[3].Set(wxACCEL_CTRL, (int) 'P', wxID_PRINT);
  entries[4].Set(wxACCEL_CTRL, (int) 'C', wxID_COPY);
  entries[5].Set(wxACCEL_CTRL, (int) 'V', wxID_PASTE);
  entries[6].Set(wxACCEL_CTRL, (int) 'X', wxID_EXIT);
  wxAcceleratorTable accel(7, entries);
  SetAcceleratorTable(accel);

  MakeMenus();
  MakeToolbar();
  
  m_game->m_currentEfgSupport = new EFSupport(*m_game->m_efg);
  m_game->m_currentEfgSupport->SetName("Full Support");
  m_game->m_efgSupports.Append(m_game->m_currentEfgSupport);

  m_nodeSashWindow = new wxSashWindow(this, idNODEWINDOW,
				      wxPoint(0, 40), wxSize(200, 200),
				      wxNO_BORDER | wxSW_3D);
  m_nodeSashWindow->SetSashVisible(wxSASH_RIGHT, true);

  m_treeWindow = new TreeWindow(m_game, this, this);
  m_treeWindow->SetSize(200, 40, 200, 200);
  m_treeWindow->RefreshTree();
  m_treeWindow->RefreshLayout();

  m_infoNotebook = new wxNotebook(m_nodeSashWindow, idINFONOTEBOOK);

  m_navigateWindow = new EfgNavigateWindow(m_game, m_infoNotebook);
  m_navigateWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_navigateWindow, "Navigation");

  m_outcomeWindow = new EfgOutcomeWindow(m_game, this, m_infoNotebook);
  m_outcomeWindow->UpdateValues();
  m_navigateWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_outcomeWindow, "Outcomes");

  m_supportWindow = new EfgSupportWindow(m_game, this, m_infoNotebook);
  m_supportWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_supportWindow, "Supports");
  m_infoNotebook->SetSelection(0);

  m_nodeSashWindow->Show(false);
  m_nodeSashWindow->SetSashVisible(wxSASH_LEFT, false);
  
  m_solutionSashWindow = new wxSashWindow(this, idSOLUTIONWINDOW,
					  wxDefaultPosition,
					  wxSize(600, 100));
  m_solutionSashWindow->SetSashVisible(wxSASH_TOP, true);

  m_profileTable = new EfgProfileList(m_game, m_solutionSashWindow);
  m_profileTable->Show(false);
  m_solutionSashWindow->Show(false);

  m_game->m_efg->SetIsDirty(false);

  AdjustSizes();
  m_treeWindow->FitZoom();
  m_game->UpdateAllViews();

  Show(true);
  // Force this at end to make sure item is unchecked; under MSW,
  // the ordering of events in creating the window leaves this checked
  GetMenuBar()->Check(efgmenuVIEW_NAVIGATION, false);
  UpdateMenus();
}

EfgShow::~EfgShow()
{
  wxGetApp().RemoveGame(m_game->m_efg);
}


//---------------------------------------------------------------------
//               EfgShow: Manipulation of profile list
//---------------------------------------------------------------------

void EfgShow::ChangeProfile(int sol)
{
  m_game->m_currentProfile = sol;
  m_game->UpdateAllViews();
}

void EfgShow::RemoveProfile(int p_profile)
{
  m_game->m_behavProfiles.Remove(p_profile);
  if (m_game->m_currentProfile == p_profile) {
    m_game->m_currentProfile = (m_game->m_behavProfiles.Length() > 0) ? 1 : 0;
  }
  else if (m_game->m_currentProfile > p_profile) {
    m_game->m_currentProfile--;
  }

  m_game->UpdateAllViews();
}

void EfgShow::RemoveProfiles(void)
{
  m_game->m_currentProfile = 0;
  m_game->m_behavProfiles.Flush();
  m_game->UpdateAllViews();
}

void EfgShow::AddProfile(const BehavSolution &p_profile, bool p_map)
{
  if (p_profile.GetName() == "") {
    BehavSolution tmp(p_profile);
    tmp.SetName(UniqueProfileName());
    m_game->m_behavProfiles.Append(tmp);
  }
  else {
    m_game->m_behavProfiles.Append(p_profile);
  }

  m_game->UpdateAllViews();
}

gText EfgShow::UniqueProfileName(void) const
{
  int number = m_game->m_behavProfiles.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_game->m_behavProfiles.Length(); i++) {
      if (m_game->m_behavProfiles[i].GetName() == "Profile" + ToText(number)) {
	break;
      }
    }

    if (i > m_game->m_behavProfiles.Length())
      return "Profile" + ToText(number);
    
    number++;
  }
}

//---------------------------------------------------------------------
//            EfgShow: Coordinating updates of child windows
//---------------------------------------------------------------------

void EfgShow::OnOutcomesEdited(void)
{
  for (int i = 1; i <= m_game->m_behavProfiles.Length(); i++) {
    m_game->m_behavProfiles[i].Invalidate();
  }
}

int EfgShow::NumDecimals(void) const
{
  return m_treeWindow->DrawSettings().NumDecimals();
}

void EfgShow::OnTreeChanged(bool p_nodesChanged, bool p_infosetsChanged)
{
  if (p_infosetsChanged) {
    m_game->FlushEfgSupports();
  }

  if (p_infosetsChanged || p_nodesChanged) {
    // It would be nice to relax this, but be conservative for now
    m_game->m_copyNode = 0;
    if (m_game->m_cutNode) {
      m_treeWindow->SetCutNode(m_game->m_cutNode, false);
      m_game->m_cutNode = 0;
    }
  }

  m_game->UpdateAllViews(this);
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
  wxMenu *fileImportMenu = new wxMenu;
  fileImportMenu->Append(efgmenuFILE_IMPORT_COMLAB, "&ComLabGames",
			 "Import a game saved in ComLabGames format");
  fileMenu->Append(efgmenuFILE_IMPORT, "&Import", fileImportMenu,
		   "Import a game from various formats");
  wxMenu *fileExportMenu = new wxMenu;
  fileExportMenu->Append(efgmenuFILE_EXPORT_BMP, "&BMP",
			 "Save a rendering of the game as a Windows bitmap");
  fileExportMenu->Append(efgmenuFILE_EXPORT_JPEG, "&JPEG",
			 "Save a rendering of the game as a JPEG image");
  fileExportMenu->Append(efgmenuFILE_EXPORT_PNG, "&PNG",
			 "Save a rendering of the game as a PNG image");
  fileExportMenu->Append(efgmenuFILE_EXPORT_POSTSCRIPT, "Post&Script",
			 "Save a printout of the game in PostScript format");
  fileExportMenu->Enable(efgmenuFILE_EXPORT_POSTSCRIPT, wxUSE_POSTSCRIPT);
  fileMenu->Append(efgmenuFILE_EXPORT, "&Export", fileExportMenu,
		   "Export the game in various formats");
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
  editMenu->AppendSeparator();
  editMenu->Append(efgmenuEDIT_INSERT, "&Insert", "Insert a move");
  editMenu->Append(efgmenuEDIT_DELETE, "&Delete...", "Delete an object");
  editMenu->Append(efgmenuEDIT_REVEAL, "&Reveal", 
		   "Reveal choice at node");
  editMenu->AppendSeparator();
  editMenu->Append(efgmenuEDIT_TOGGLE_SUBGAME, "Mark &subgame",
		   "Mark or unmark the subgame at this node");
  editMenu->Append(efgmenuEDIT_MARK_SUBGAME_TREE, "Mar&k subgame tree",
		   "Mark all subgames in this subtree");
  editMenu->Append(efgmenuEDIT_UNMARK_SUBGAME_TREE, "&Unmark subgame tree",
		   "Unmark all subgames in this subtree");
  editMenu->AppendSeparator();
  editMenu->Append(efgmenuEDIT_NODE, "&Node",
		   "Edit properties of the node");
  editMenu->Append(efgmenuEDIT_MOVE, "&Move",
		   "Edit properties of the move");
  editMenu->Append(efgmenuEDIT_GAME, "&Game",
		   "Edit properties of the game");

  wxMenu *toolsMenu = new wxMenu;

  toolsMenu->Append(efgmenuTOOLS_DOMINANCE, "&Dominance",
		    "Find undominated actions");
  toolsMenu->Append(efgmenuTOOLS_EQUILIBRIUM, "&Equilibrium",
		    "Compute Nash equilibria and refinements");
  toolsMenu->Append(efgmenuTOOLS_QRE, "&Qre",
		    "Compute quantal response equilibria");

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
  viewMenu->AppendSeparator();
  viewMenu->Append(efgmenuVIEW_ZOOMIN, "Zoom &in",
		   "Increase display magnification");
  viewMenu->Append(efgmenuVIEW_ZOOMOUT, "Zoom &out",
		   "Decrease display magnification");
  viewMenu->AppendSeparator();
  viewMenu->Append(efgmenuVIEW_SUPPORT_REACHABLE, "&Root Reachable",
		   "Display only nodes that are support-reachable",
		   true);
  
  wxMenu *formatMenu = new wxMenu;
  wxMenu *formatDisplayMenu = new wxMenu;
  formatDisplayMenu->Append(efgmenuFORMAT_DISPLAY_LAYOUT, "&Layout",
			    "Set tree layout parameters");
  formatDisplayMenu->Append(efgmenuFORMAT_DISPLAY_LEGEND, "Le&gends",
			    "Set legends");
  formatDisplayMenu->Append(efgmenuFORMAT_DISPLAY_COLORS, "&Colors",
			    "Set colors");
  formatDisplayMenu->Append(efgmenuFORMAT_DISPLAY_DECIMALS, "&Decimal Places",
			   "Set number of decimal places to display");
  formatMenu->Append(efgmenuFORMAT_DISPLAY, "&Display", formatDisplayMenu,
		     "Set display options");
  
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
  
  wxMenu *helpMenu = new wxMenu;
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
  Node *cursor = m_game->Cursor();
  wxMenuBar *menuBar = GetMenuBar();

  menuBar->Enable(wxID_COPY, (cursor) ? true : false);
  menuBar->Enable(wxID_CUT, (cursor) ? true : false);
  menuBar->Enable(wxID_PASTE, (m_game->m_cutNode || m_game->m_copyNode) ? true : false);

  menuBar->Enable(efgmenuEDIT_INSERT, (cursor) ? true : false);
  menuBar->Enable(efgmenuEDIT_DELETE,
		  (cursor && m_game->m_efg->NumChildren(cursor) > 0) ? true : false);
  menuBar->Enable(efgmenuEDIT_REVEAL, 
		  (cursor && cursor->GetInfoset()) ? true : false);

  menuBar->Enable(efgmenuEDIT_TOGGLE_SUBGAME,
		  (cursor && m_game->m_efg->IsLegalSubgame(cursor) &&
		   cursor->GetParent()));
  menuBar->Enable(efgmenuEDIT_MARK_SUBGAME_TREE,
		  (cursor && m_game->m_efg->IsLegalSubgame(cursor)));
  menuBar->Enable(efgmenuEDIT_UNMARK_SUBGAME_TREE,
		  (cursor && m_game->m_efg->IsLegalSubgame(cursor)));
  menuBar->SetLabel(efgmenuEDIT_TOGGLE_SUBGAME,
		    (cursor && cursor->GetParent() &&
		     m_game->m_efg->IsLegalSubgame(cursor) &&
		     cursor->GetSubgameRoot() == cursor) ?
		    "Unmark &subgame" : "Mark &subgame");

  menuBar->Enable(efgmenuEDIT_NODE, (cursor) ? true : false);
  menuBar->Enable(efgmenuEDIT_MOVE,
		  (cursor && cursor->GetInfoset()) ? true : false);

  if (m_treeWindow) {
    menuBar->Check(efgmenuVIEW_SUPPORT_REACHABLE,
		   m_treeWindow->DrawSettings().RootReachable());
  }

  m_treeWindow->UpdateMenus();
}

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/preview.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/zoomin.xpm"
#include "bitmaps/zoomout.xpm"
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

  toolBar->AddTool(wxID_ABOUT, wxBITMAP(help), wxNullBitmap, false,
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
  if (p_event.GetId() == wxID_SAVEAS || m_game->m_filename == "") {
    wxFileDialog dialog(this, "Choose file", wxPathOnly((char *) m_game->m_filename),
			wxFileNameFromPath((char *) m_game->m_filename), "*.efg",
			wxSAVE | wxOVERWRITE_PROMPT);

    switch (dialog.ShowModal()) {
    case wxID_OK:
      m_game->m_filename = dialog.GetPath().c_str();
      break;
    case wxID_CANCEL:
    default:
      return;
    }
  }

  efgGame *efg = 0;
  try {
    gFileOutput file(m_game->m_filename);
    m_game->m_efg->WriteEfgFile(file, 6);
    m_game->m_efg->SetIsDirty(false);
  }
  catch (gFileOutput::OpenFailed &) {
    wxMessageBox(wxString::Format("Could not open %s for writing.",
				  (const char *) m_game->m_filename),
		 "Error", wxOK, this);
    if (efg)  delete efg;
  }
  catch (gFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format("Write error occurred in saving %s.\n",
				  (const char *) m_game->m_filename),
		 "Error", wxOK, this);
    if (efg)  delete efg;
  }
  catch (efgGame::Exception &) {
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
  wxPrintPreview *preview = 
    new wxPrintPreview(new EfgPrintout(m_treeWindow,(char *) m_game->m_efg->GetTitle()),
		       new EfgPrintout(m_treeWindow,(char *) m_game->m_efg->GetTitle()),
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
  EfgPrintout printout(m_treeWindow, (char *) m_game->m_efg->GetTitle());

  if (!printer.Print(this, &printout, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      wxMessageBox("There was an error in printing", "Error", wxOK);
    }
    // Otherwise, user hit "cancel"; just be quiet and return.
    return;
  }
  else {
    m_printData = printer.GetPrintDialogData().GetPrintData();
  }
}

void EfgShow::OnFileImportComLab(wxCommandEvent &)
{
  wxGetApp().OnFileImportComLab(this);
}

void EfgShow::OnFileExportBMP(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Choose output file", wxGetApp().CurrentDir(), "",
		      "Windows bitmap files (*.bmp)|*.bmp", wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeWindow->m_layout.MaxX(),
		    m_treeWindow->m_layout.MaxY());
    dc.SelectObject(bitmap);
    m_treeWindow->OnDraw(dc, 1.0);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_BMP)) {
      wxMessageBox(wxString::Format("An error occurred in writing '%s'.\n",
				    dialog.GetPath().c_str()),
		   "Error", wxOK, this);
    }
  }
}

void EfgShow::OnFileExportJPEG(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Choose output file", wxGetApp().CurrentDir(), "",
		      "JPEG files (*.jpeg)|*.jpeg|"
		      "JPEG files (*.jpg)|*.jpg", wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeWindow->m_layout.MaxX(),
		    m_treeWindow->m_layout.MaxY());
    dc.SelectObject(bitmap);
    m_treeWindow->OnDraw(dc, 1.0);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_JPEG)) {
      wxMessageBox(wxString::Format("An error occurred in writing '%s'.\n",
				    dialog.GetPath().c_str()),
		   "Error", wxOK, this);
    }
  }
}

void EfgShow::OnFileExportPNG(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Choose output file", wxGetApp().CurrentDir(), "",
		      "PNG files (*.png)|*.png", wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeWindow->m_layout.MaxX(),
		    m_treeWindow->m_layout.MaxY());
    dc.SelectObject(bitmap);
    m_treeWindow->OnDraw(dc, 1.0);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_PNG)) {
      wxMessageBox(wxString::Format("An error occurred in writing '%s'.\n",
				    dialog.GetPath().c_str()),
		   "Error", wxOK, this);
    }
  }
}

void EfgShow::OnFileExportPS(wxCommandEvent &)
{
#if wxUSE_POSTSCRIPT
  wxPrintData printData(m_printData);

  wxFileDialog dialog(this, "Choose output file", wxGetApp().CurrentDir(), "",
		      "PostScript files (*.ps)|*.ps", wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    printData.SetFilename(dialog.GetPath());
  }
  else {
    return;
  }
  printData.SetPrintMode(wxPRINT_MODE_FILE);

  // This code is borrowed from the extensive form printout class.
  // Seems like it would be nice to consolidate it in one place.
  wxPostScriptDC dc(printData);
  dc.StartDoc("Extensive form game");
  dc.SetBackgroundMode(wxTRANSPARENT);
  dc.StartPage();

  // The actual size of the tree, in pixels
  int maxX = m_treeWindow->m_layout.MaxX();
  int maxY = m_treeWindow->m_layout.MaxY();

  // Margins
  int marginX = 50;
  int marginY = 50;

  maxX += 2 * marginX;
  maxY += 2 * marginY;

  // Get the size of the DC in pixels
  wxCoord w, h;
  dc.GetSize(&w, &h);

  // Calculate a scaling factor
  float scaleX = (float) w / (float) maxX;
  float scaleY = (float) h / (float) maxY;

  float actualScale = (scaleX < scaleY) ? scaleX : scaleY;

  // Calculate the position on the DC to center the tree
  float posX = (float) ((w - (m_treeWindow->m_layout.MaxX() * actualScale)) / 2.0);
  float posY = (float) ((h - (m_treeWindow->m_layout.MaxY() * actualScale)) / 2.0);

  // Set the scale and origin
  dc.SetUserScale(actualScale, actualScale);
  dc.SetDeviceOrigin((long) posX, (long) posY);

  // Draw!
  m_treeWindow->OnDraw(dc, actualScale);
  dc.EndPage();
  dc.EndDoc();
#endif  // wxUSE_POSTSCRIPT
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
//                EfgShow: Menu handlers - Edit menu
//----------------------------------------------------------------------

void EfgShow::OnEditCut(wxCommandEvent &)
{
  if (m_game->m_cutNode) {
    m_treeWindow->SetCutNode(m_game->m_cutNode, false);
  }
  m_game->m_cutNode = m_game->Cursor();
  m_treeWindow->SetCutNode(m_game->m_cutNode, true);
  m_game->m_copyNode = 0;
  m_treeWindow->Refresh();
}

void EfgShow::OnEditCopy(wxCommandEvent &)
{
  m_game->m_copyNode = m_game->Cursor();
  if (m_game->m_cutNode) {
    m_treeWindow->SetCutNode(m_game->m_cutNode, false);
    m_game->m_cutNode = 0;
  }
  m_treeWindow->Refresh();
}

void EfgShow::OnEditPaste(wxCommandEvent &)
{
  try {
    if (m_game->m_copyNode) {
      m_game->m_efg->CopyTree(m_game->m_copyNode, m_game->Cursor());
    }
    else {
      m_game->m_efg->MoveTree(m_game->m_cutNode, m_game->Cursor());
    }
    OnTreeChanged(true, true);
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditInsert(wxCommandEvent &)
{ 
  dialogInsertMove dialog(this, *m_game->m_efg);

  if (dialog.ShowModal() == wxID_OK)  {
    try {
      if (!dialog.GetInfoset()) {
	m_game->m_efg->InsertNode(m_game->Cursor(), dialog.GetPlayer(), dialog.GetActions());
	OnTreeChanged(true, true);
      }
      else {
	m_game->m_efg->InsertNode(m_game->Cursor(), dialog.GetInfoset());
	OnTreeChanged(true, false);
      }
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditDelete(wxCommandEvent &)
{
  try {
    dialogEfgDelete dialog(this, m_game->Cursor());

    if (dialog.ShowModal() == wxID_OK) {
      if (dialog.DeleteTree()) {
	m_game->m_efg->DeleteTree(m_game->Cursor());
      }
      else {
	Node *keep = dialog.KeepNode();
	m_treeWindow->SetCursorPosition(m_game->m_efg->DeleteNode(m_game->Cursor(), keep));
      }
      m_game->m_efg->DeleteEmptyInfosets();
      OnTreeChanged(true, true);
    }
  }
  catch (gException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void EfgShow::OnEditReveal(wxCommandEvent &)
{
  dialogInfosetReveal dialog(this, *m_game->m_efg);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_game->m_efg->Reveal(m_game->Cursor()->GetInfoset(), dialog.GetPlayers());
      OnTreeChanged(true, true);
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void EfgShow::OnEditToggleSubgame(wxCommandEvent &)
{
  if (m_game->Cursor()->GetSubgameRoot() == m_game->Cursor()) {
    m_game->m_efg->UnmarkSubgame(m_game->Cursor());
  }
  else {
    m_game->m_efg->MarkSubgame(m_game->Cursor());
  }
  m_treeWindow->RefreshLayout();
  m_treeWindow->Refresh();
}

void EfgShow::OnEditMarkSubgameTree(wxCommandEvent &)
{
  gList<Node *> subgames;
  LegalSubgameRoots(*m_game->m_efg, m_game->Cursor(), subgames);
  for (int i = 1; i <= subgames.Length(); i++) {
    m_game->m_efg->MarkSubgame(subgames[i]);
  }
  m_treeWindow->RefreshLayout();
  m_treeWindow->Refresh();
}

void EfgShow::OnEditUnmarkSubgameTree(wxCommandEvent &)
{
  gList<Node *> subgames;
  LegalSubgameRoots(*m_game->m_efg, m_game->Cursor(), subgames);
  for (int i = 1; i <= subgames.Length(); i++) {
    m_game->m_efg->UnmarkSubgame(subgames[i]);
  }
  m_treeWindow->RefreshLayout();
  m_treeWindow->Refresh();
}

void EfgShow::OnEditNode(wxCommandEvent &)
{
  dialogEditNode dialog(this, m_game->Cursor());
  if (dialog.ShowModal() == wxID_OK) {
    m_game->Cursor()->SetName(dialog.GetNodeName().c_str());
    if (dialog.GetOutcome() > 0) {
      m_game->m_efg->SetOutcome(m_game->Cursor(), m_game->m_efg->GetOutcome(dialog.GetOutcome()));
    }
    else {
      m_game->m_efg->SetOutcome(m_game->Cursor(), 0);
    }

    if (m_game->m_efg->IsLegalSubgame(m_game->Cursor()) && m_game->Cursor()->GetParent()) {
      if (dialog.MarkedSubgame()) {
	m_game->m_efg->MarkSubgame(m_game->Cursor());
      }
      else {
	m_game->m_efg->UnmarkSubgame(m_game->Cursor());
      }
    }

    if (m_game->Cursor()->NumChildren() > 0 &&
	dialog.GetInfoset() != m_game->Cursor()->GetInfoset()) {
      if (dialog.GetInfoset() == 0) {
	m_game->m_efg->LeaveInfoset(m_game->Cursor());
      }
      else {
	m_game->m_efg->JoinInfoset(dialog.GetInfoset(), m_game->Cursor());
      }
      OnTreeChanged(true, true);
    }
    m_treeWindow->RefreshTree();
    m_treeWindow->Refresh();
    UpdateMenus();
  }
}

void EfgShow::OnEditMove(wxCommandEvent &)
{
  Infoset *infoset = m_game->Cursor()->GetInfoset();

  dialogEditMove dialog(this, infoset);
  if (dialog.ShowModal() == wxID_OK) {
    infoset->SetName(dialog.GetInfosetName().c_str());
    
    if (!infoset->IsChanceInfoset() && 
	dialog.GetPlayer() != infoset->GetPlayer()->GetNumber()) {
      m_game->m_efg->SwitchPlayer(infoset, m_game->m_efg->Players()[dialog.GetPlayer()]);
    }

    for (int act = 1; act <= infoset->NumActions(); act++) {
      if (!dialog.GetActions().Find(infoset->Actions()[act])) {
	m_game->m_efg->DeleteAction(infoset, infoset->Actions()[act]);
	act--;
      }
    }

    int insertAt = 1;
    for (int act = 1; act <= dialog.NumActions(); act++) {
      Action *action = dialog.GetActions()[act];
      if (action) {
	action->SetName(dialog.GetActionName(act));
	if (infoset->IsChanceInfoset()) {
	  m_game->m_efg->SetChanceProb(infoset, action->GetNumber(),
			      dialog.GetActionProb(act));
	}
	insertAt = dialog.GetActions()[act]->GetNumber() + 1;
      }
      else if (insertAt > infoset->NumActions()) {
	Action *newAction = m_game->m_efg->InsertAction(infoset);
	insertAt++;
	newAction->SetName(dialog.GetActionName(act));
	if (infoset->IsChanceInfoset()) {
	  m_game->m_efg->SetChanceProb(infoset, newAction->GetNumber(), 
			      dialog.GetActionProb(act));
	}
      }
      else {
	Action *newAction = m_game->m_efg->InsertAction(infoset,
					       infoset->GetAction(insertAt++));
	newAction->SetName(dialog.GetActionName(act));
	if (infoset->IsChanceInfoset()) {
	  m_game->m_efg->SetChanceProb(infoset, newAction->GetNumber(), 
			      dialog.GetActionProb(act));
	}
      }
    }
    OnTreeChanged(true, true);
    m_treeWindow->Refresh();
    UpdateMenus();
  }
}

void EfgShow::OnEditGame(wxCommandEvent &)
{
  dialogEditEfg dialog(this, *m_game->m_efg, (char *) m_game->m_filename);
  if (dialog.ShowModal() == wxID_OK) {
    m_game->m_efg->SetTitle(dialog.GetGameTitle().c_str());
    m_game->m_efg->SetComment(dialog.GetComment().c_str());
    for (int pl = 1; pl <= dialog.NumPlayers(); pl++) {
      if (pl > m_game->m_efg->NumPlayers()) {
	m_game->m_efg->NewPlayer()->SetName(dialog.GetPlayerName(pl).c_str());
      }
      else {
	m_game->m_efg->Players()[pl]->SetName(dialog.GetPlayerName(pl).c_str());
      }
    }
    m_game->UpdateAllViews();
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

const double ZOOM_DELTA = .1;
const double ZOOM_MAX = 1;
const double ZOOM_MIN = .2;

void EfgShow::OnViewZoomIn(wxCommandEvent &)
{
  double zoom = m_treeWindow->GetZoom();
  zoom = gmin(zoom + ZOOM_DELTA, ZOOM_MAX);
  m_treeWindow->SetZoom(zoom);
}

void EfgShow::OnViewZoomOut(wxCommandEvent &)
{
  double zoom = m_treeWindow->GetZoom();
  zoom = gmax(zoom - ZOOM_DELTA, ZOOM_MIN);
  m_treeWindow->SetZoom(zoom);
}

void EfgShow::OnViewSupportReachable(wxCommandEvent &)
{
  m_treeWindow->DrawSettings().SetRootReachable(!m_treeWindow->DrawSettings().RootReachable());
  m_treeWindow->RefreshLayout();
  m_treeWindow->Refresh();
}

//----------------------------------------------------------------------
//               EfgShow: Menu handlers - Format menu
//----------------------------------------------------------------------

void EfgShow::OnFormatFontsAboveNode(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_treeWindow->DrawSettings().NodeAboveFont());
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeAboveFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsBelowNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeBelowFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsAfterNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeRightFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsAboveBranch(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetBranchAboveFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatFontsBelowBranch(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetBranchBelowFont(dialog.GetFontData().GetChosenFont());
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatDisplayLayout(wxCommandEvent &)
{
  TreeDrawSettings &settings = m_treeWindow->DrawSettings();

  dialogLayout dialog(this, settings);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.GetSettings(settings);
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->RefreshLayout();
    m_treeWindow->Refresh();
  }
}

void EfgShow::OnFormatDisplayLegend(wxCommandEvent &)
{
  dialogLegend dialog(this, m_treeWindow->DrawSettings());

  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNodeAboveLabel(dialog.GetNodeAbove());
    m_treeWindow->DrawSettings().SetNodeBelowLabel(dialog.GetNodeBelow());
    m_treeWindow->DrawSettings().SetNodeRightLabel(dialog.GetNodeAfter());
    m_treeWindow->DrawSettings().SetBranchAboveLabel(dialog.GetBranchAbove());
    m_treeWindow->DrawSettings().SetBranchBelowLabel(dialog.GetBranchBelow());
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->RefreshLabels();
  }
}

void EfgShow::OnFormatDisplayColors(wxCommandEvent &)
{
  dialogEfgColor dialog(this, m_treeWindow->DrawSettings());

  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetChanceColor(dialog.GetChanceColor());
    m_treeWindow->DrawSettings().SetTerminalColor(dialog.GetTerminalColor());
    for (int pl = 1; pl <= 8; pl++) {
      m_treeWindow->DrawSettings().SetPlayerColor(pl,
						  dialog.GetPlayerColor(pl));
    }
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->RefreshTree();
  }
}

void EfgShow::OnFormatDisplayDecimals(wxCommandEvent &)
{
  dialogSpinCtrl dialog(this, "Decimal places", 0, 25,
			m_treeWindow->DrawSettings().NumDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_treeWindow->DrawSettings().SetNumDecimals(dialog.GetValue());
    m_treeWindow->DrawSettings().SaveOptions();
    m_treeWindow->Refresh();
  }
}

//----------------------------------------------------------------------
//             EfgShow: Menu handler - Tools->Dominance
//----------------------------------------------------------------------

void EfgShow::OnToolsDominance(wxCommandEvent &)
{
  gArray<gText> playerNames(m_game->m_efg->NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++) {
    playerNames[pl] = m_game->m_efg->Players()[pl]->GetName();
  }
  dialogElimBehav dialog(this, playerNames);

  if (dialog.ShowModal() == wxID_OK) {
    EFSupport support(*m_game->m_currentEfgSupport);
    wxStatus status(this, "Dominance Elimination");

    try {
      EFSupport newSupport(support);

      while (true) {
	gNullOutput gnull;
	newSupport = support.Undominated(dialog.DomStrong(),
					 dialog.DomConditional(),
					 dialog.Players(), gnull, status);

	if (newSupport == support) {
	  break;
	}
	else {
	  newSupport.SetName(m_game->UniqueEfgSupportName());
	  m_game->m_efgSupports.Append(new EFSupport(newSupport));
	  support = newSupport;
	}

	if (!dialog.Iterative()) {
	  // Bit of a kludge; short-circuit loop if iterative not requested
	  break;
	}
      }
    }
    catch (gSignalBreak &) { }
    
    if (*m_game->m_currentEfgSupport != support) {
      m_game->m_currentEfgSupport = m_game->m_efgSupports[m_game->m_efgSupports.Length()]; 
    }
  }
}

//----------------------------------------------------------------------
//            EfgShow: Menu handlers - Tools->Equilibrium
//----------------------------------------------------------------------

void EfgShow::OnToolsEquilibrium(wxCommandEvent &)
{
  dialogEfgNash dialog(this, *m_game->m_currentEfgSupport);

  if (dialog.ShowModal() == wxID_OK) {
    gbtEfgNashAlgorithm *algorithm = dialog.GetAlgorithm();

    if (!algorithm) {
      return;
    }

    try {
      wxStatus status(this, algorithm->GetAlgorithm() + "Solve Progress");
      gList<BehavSolution> solutions;
      solutions = algorithm->Solve(*m_game->m_currentEfgSupport, status);

      for (int soln = 1; soln <= solutions.Length(); soln++) {
	AddProfile(solutions[soln], true);
      }
      ChangeProfile(m_game->m_behavProfiles.Length());
   
      if (!m_solutionSashWindow->IsShown()) {
	m_profileTable->Show(true);
	m_solutionSashWindow->Show(true);
	GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
	AdjustSizes();
      }
      
      UpdateMenus();
    }
    catch (gException &ex) {
      wxMessageDialog msgDialog(this, (char *) ex.Description(),
				"Gambit exception", wxOK);
      msgDialog.ShowModal();
    }
    catch (...) {
      wxMessageDialog msgDialog(this,
				"An internal exception occurred while solving",
				"Gambit exception", wxOK);
      msgDialog.ShowModal();
    }

    delete algorithm;
  }
}

void EfgShow::OnToolsQre(wxCommandEvent &)
{
  try {
    gbtEfgNashLogit algorithm;
    algorithm.SetFullGraph(true);
    algorithm.SetMaxLambda(10000000);

    wxStatus status(this, "QreSolve Progress");
    gList<BehavSolution> solutions = algorithm.Solve(*m_game->m_currentEfgSupport,
						     status);

    if (solutions.Length() > 0) {
      dialogQreFile fileDialog(this, solutions);
      if (fileDialog.ShowModal() == wxID_OK) {

      }
    }
  }
  catch (...) {
  }

}

void EfgShow::OnToolsNormalReduced(wxCommandEvent &)
{
  // check that the game is perfect recall, if not give a warning
  if (!IsPerfectRecall(*m_game->m_efg)) {
    if (wxMessageBox("This game is not perfect recall\n"
		     "Do you wish to continue?", 
		     "Reduced normal form", 
		     wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
      return;
    }
  }
    
  if (m_game->m_efg->AssociatedNfg() != 0) {
    return;
  }

  Nfg *nfg = 0;
  try {
    nfg = MakeReducedNfg(*m_game->m_currentEfgSupport);
  }
  catch (...) {
    wxMessageDialog msgDialog(this,
			      "An internal exception occurred while converting",
			      "Gambit exception", wxOK);
    msgDialog.ShowModal();
    return;
  }

  if (nfg) {
    wxGetApp().AddGame(m_game->m_efg, nfg);
  }
  else {
    wxMessageBox("Could not create normal form game.\n",
		 "Reduced normal form", wxOK);
  }
}

void EfgShow::OnToolsNormalAgent(wxCommandEvent &)
{
#ifdef UNUSED
  // check that the game is perfect recall, if not give a warning
  if (!IsPerfectRecall(*m_game->m_efg)) {
    if (wxMessageBox("This game is not perfect recall\n"
		     "Do you wish to continue?", 
		     "Agent normal form", 
		     wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
      return;
    }
  }

  Nfg *N = 0;
  try {
    N = MakeAfg(*m_game->m_efg);
  }
  catch (...) {
    wxMessageDialog msgDialog(this,
			      "An internal exception occurred while converting",
			      "Gambit exception", wxOK);
    msgDialog.ShowModal();
    return;
  }
  if (N) {
    (void) new NfgShow(*N, m_parent);
  }
#endif  // UNUSED
}

//----------------------------------------------------------------------
//                 EfgShow: Menu handlers - Help menu
//----------------------------------------------------------------------

void EfgShow::OnHelpAbout(wxCommandEvent &)
{
  wxGetApp().OnHelpAbout(this);
}

//----------------------------------------------------------------------
//               EfgShow: Menu handlers - Support menu
//----------------------------------------------------------------------

void EfgShow::OnSupportDuplicate(wxCommandEvent &)
{
  gbtEfgDuplicateSupportCommand *command =
    new gbtEfgDuplicateSupportCommand(m_game,
				      m_game->m_efgSupports.Find(m_game->m_currentEfgSupport));
  m_game->Submit(command);
}

void EfgShow::OnSupportDelete(wxCommandEvent &)
{
  gbtEfgDeleteSupportCommand *command = new gbtEfgDeleteSupportCommand(m_game);
  m_game->Submit(command);
}

//----------------------------------------------------------------------
//              EfgShow: Menu handlers - Profiles menu
//----------------------------------------------------------------------

void EfgShow::OnProfilesNew(wxCommandEvent &)
{
  BehavSolution profile = BehavProfile<gNumber>(EFSupport(*m_game->m_efg));

  dialogEditBehav dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddProfile(dialog.GetProfile(), true);
    ChangeProfile(m_game->m_behavProfiles.Length());
  }
}

void EfgShow::OnProfilesDuplicate(wxCommandEvent &)
{
  BehavSolution profile(m_game->m_behavProfiles[m_game->m_currentProfile]);

  dialogEditBehav dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddProfile(dialog.GetProfile(), true);
    ChangeProfile(m_game->m_behavProfiles.Length());
  }
}

void EfgShow::OnProfilesDelete(wxCommandEvent &)
{
  m_game->m_behavProfiles.Remove(m_game->m_currentProfile);
  if (m_game->m_efg->AssociatedNfg()) {
    wxLogWarning("Profile list out of synch; problems will occur.");
  }
  m_game->m_currentProfile = (m_game->m_behavProfiles.Length() > 0) ? 1 : 0;
  ChangeProfile(m_game->m_currentProfile);
}

void EfgShow::OnProfilesProperties(wxCommandEvent &)
{
  if (m_game->m_currentProfile > 0) {
    dialogEditBehav dialog(this, m_game->m_behavProfiles[m_game->m_currentProfile]);

    if (dialog.ShowModal() == wxID_OK) {
      m_game->m_behavProfiles[m_game->m_currentProfile] = dialog.GetProfile();
      ChangeProfile(m_game->m_currentProfile);
    }
  }
}

void EfgShow::OnProfilesReport(wxCommandEvent &)
{
  dialogReport dialog(this, m_profileTable->GetReport());
  dialog.ShowModal();
}

void EfgShow::OnProfileSelected(wxListEvent &p_event)
{
  m_game->m_currentProfile = p_event.GetIndex() + 1;
  m_treeWindow->RefreshLabels();
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
  if (p_event.CanVeto() && m_game->m_efg->IsDirty()) {
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
			  m_treeWindow->wxWindowBase::GetRect().y,
			  clientWidth - p_event.GetDragRect().width,
			  m_treeWindow->wxWindowBase::GetRect().height);
    m_nodeSashWindow->SetSize(m_nodeSashWindow->GetRect().x,
			      m_nodeSashWindow->GetRect().y,
			      p_event.GetDragRect().width,
			      m_nodeSashWindow->GetRect().height);
    break;
  case idSOLUTIONWINDOW:
    m_treeWindow->SetSize(m_treeWindow->wxWindowBase::GetRect().x,
			  m_treeWindow->wxWindowBase::GetRect().y,
			  m_treeWindow->wxWindowBase::GetRect().width,
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
      m_treeWindow->SetSize(m_nodeSashWindow->GetRect().width, 0,
			    width - m_nodeSashWindow->GetRect().width, height);
    }
  }
  else if (m_treeWindow) {
    m_treeWindow->SetSize(0, 0, width, height);
  }

  if (m_navigateWindow && m_nodeSashWindow->IsShown()) {
    m_nodeSashWindow->SetSize(0, 0, m_nodeSashWindow->GetRect().width, height);
  }

  if (m_treeWindow) {
    m_treeWindow->SetFocus();
  }
}

#include "base/glist.imp"

template class gList<EFSupport *>;




