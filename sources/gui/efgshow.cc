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
#include "game/game.h"
#include "game/efgutils.h"
#include "nash/behavsol.h"
#include "nash/efglogit.h"

#include "id.h"
#include "treewin.h"
#include "efgprint.h"
#include "efgshow.h"
#include "profile.h"
#include "efgnavigate.h"
#include "outcomes.h"
#include "efgsupport.h"
#include "nfgshow.h"

#include "dlinsertmove.h"
#include "dlefgdelete.h"
#include "dlefgreveal.h"
#include "dleditnode.h"
#include "dleditmove.h"
#include "dleditgame.h"
#include "dlefglayout.h"
#include "dlefglegend.h"
#include "dlefgcolor.h"
#include "dlelimbehav.h"
#include "dlefgnash.h"
#include "dlqrefile.h"
#include "dlreport.h"
#include "dleditbehav.h"


//=====================================================================
//                 Implementation of command classes
//=====================================================================


//---------------------------------------------------------------------
//                    class gbtCmdDeleteTree
//---------------------------------------------------------------------

//
// Delete the subtree rooted at a node
//
class gbtCmdDeleteTree : public gbtGameCommand {
private:
  gbtGameNode m_node;

public:
  gbtCmdDeleteTree(gbtGameNode p_node)
    : m_node(p_node) { }
  virtual ~gbtCmdDeleteTree() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return true; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdDeleteTree::Do(gbtGameDocument *p_doc)  
{
  m_node->DeleteTree();
  p_doc->GetGame()->DeleteEmptyInfosets();
}

//---------------------------------------------------------------------
//                    class gbtCmdDeleteMove
//---------------------------------------------------------------------

//
// Deletes the incoming move
//
class gbtCmdDeleteMove : public gbtGameCommand {
private:
  gbtGameNode m_node;

public:
  gbtCmdDeleteMove(gbtGameNode p_node)
    : m_node(p_node) { }
  virtual ~gbtCmdDeleteMove() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return true; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdDeleteMove::Do(gbtGameDocument *p_doc)  
{
  m_node->DeleteMove();
  p_doc->GetGame()->DeleteEmptyInfosets();
}

//---------------------------------------------------------------------
//                        class gbtCmdReveal
//---------------------------------------------------------------------

//
// Reveal the actions at an information set to a player
//
class gbtCmdReveal : public gbtGameCommand {
private:
  gbtGameInfoset m_infoset;
  gbtGamePlayer m_player;

public:
  gbtCmdReveal(gbtGameInfoset p_infoset, gbtGamePlayer p_player)
    : m_infoset(p_infoset), m_player(p_player) { }
  virtual ~gbtCmdReveal() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return true; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdReveal::Do(gbtGameDocument *p_doc)  
{
  m_infoset->Reveal(m_player);
}

//---------------------------------------------------------------------
//                    class gbtCmdLeaveInfoset
//---------------------------------------------------------------------

//
// Cause a node to become a member of its own information set
//
class gbtCmdLeaveInfoset : public gbtGameCommand {
private:
  gbtGameNode m_node;

public:
  gbtCmdLeaveInfoset(gbtGameNode p_node)
    : m_node(p_node) { }
  virtual ~gbtCmdLeaveInfoset() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return true; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdLeaveInfoset::Do(gbtGameDocument *p_doc)  
{
  m_node->LeaveInfoset();
}

//---------------------------------------------------------------------
//                    class gbtCmdJoinInfoset
//---------------------------------------------------------------------

//
// Cause a node to join an information set
//
class gbtCmdJoinInfoset : public gbtGameCommand {
private:
  gbtGameNode m_node;
  gbtGameInfoset m_infoset;

public:
  gbtCmdJoinInfoset(gbtGameNode p_node, gbtGameInfoset p_infoset)
    : m_node(p_node), m_infoset(p_infoset) { }
  virtual ~gbtCmdJoinInfoset() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return true; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdJoinInfoset::Do(gbtGameDocument *p_doc)  
{
  m_node->JoinInfoset(m_infoset);
  p_doc->GetGame()->DeleteEmptyInfosets();
}

//---------------------------------------------------------------------
//                    class gbtCmdMarkSubgame
//---------------------------------------------------------------------

//
// Marks or unmarks a node as a marked subgame.
//
class gbtCmdMarkSubgame : public gbtGameCommand {
private:
  gbtGameNode m_node;
  bool m_mark; 

public:
  gbtCmdMarkSubgame(gbtGameNode p_node, bool p_mark)
    : m_node(p_node), m_mark(p_mark) { }
  virtual ~gbtCmdMarkSubgame() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdMarkSubgame::Do(gbtGameDocument *p_doc)  
{
  if (m_mark) {
    p_doc->GetGame()->MarkSubgame(m_node);
  }
  else {
    p_doc->GetGame()->UnmarkSubgame(m_node);
  }
}

//---------------------------------------------------------------------
//                   class gbtCmdAddEfgSupport
//---------------------------------------------------------------------

//
// Adds a support to the current support list
//
class gbtCmdAddEfgSupport : public gbtGameCommand {
private:
  gbtEfgSupport m_support;

public:
  gbtCmdAddEfgSupport(const gbtEfgSupport &p_support) 
    : m_support(p_support) { }
  virtual ~gbtCmdAddEfgSupport() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdAddEfgSupport::Do(gbtGameDocument *p_doc)
{
  p_doc->GetEfgSupportList().Append(m_support);
}

//---------------------------------------------------------------------
//                   class gbtCmdRemoveEfgSupport
//---------------------------------------------------------------------

//
// Removes the current support from the support list
//
class gbtCmdRemoveEfgSupport : public gbtGameCommand {
public:
  gbtCmdRemoveEfgSupport(void) { }
  virtual ~gbtCmdRemoveEfgSupport() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdRemoveEfgSupport::Do(gbtGameDocument *p_doc)
{
  p_doc->GetEfgSupportList().Remove();
}

//=====================================================================
//                 Implementation of class gbtEfgFrame
//=====================================================================

BEGIN_EVENT_TABLE(gbtEfgFrame, wxFrame)
  EVT_MENU(wxID_NEW, gbtEfgFrame::OnFileNew)
  EVT_MENU(wxID_OPEN, gbtEfgFrame::OnFileOpen)
  EVT_MENU(wxID_CLOSE, gbtEfgFrame::Close)
  EVT_MENU(wxID_SAVE, gbtEfgFrame::OnFileSave)
  EVT_MENU(wxID_SAVEAS, gbtEfgFrame::OnFileSave)
  EVT_MENU(GBT_MENU_FILE_EXPORT_BMP, gbtEfgFrame::OnFileExportBMP)
  EVT_MENU(GBT_MENU_FILE_EXPORT_JPEG, gbtEfgFrame::OnFileExportJPEG)
  EVT_MENU(GBT_MENU_FILE_EXPORT_PNG, gbtEfgFrame::OnFileExportPNG)
  EVT_MENU(GBT_MENU_FILE_EXPORT_POSTSCRIPT, gbtEfgFrame::OnFileExportPS)
  EVT_MENU(wxID_PRINT_SETUP, gbtEfgFrame::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, gbtEfgFrame::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, gbtEfgFrame::OnFilePrint)
  EVT_MENU(wxID_EXIT, gbtEfgFrame::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, gbtEfgFrame::OnFileMRUFile)
  EVT_MENU(wxID_CUT, gbtEfgFrame::OnEditCut)
  EVT_MENU(wxID_COPY, gbtEfgFrame::OnEditCopy)
  EVT_MENU(wxID_PASTE, gbtEfgFrame::OnEditPaste)
  EVT_MENU(GBT_MENU_EDIT_INSERT, gbtEfgFrame::OnEditInsert)
  EVT_MENU(GBT_MENU_EDIT_DELETE, gbtEfgFrame::OnEditDelete)
  EVT_MENU(GBT_MENU_EDIT_REVEAL, gbtEfgFrame::OnEditReveal)
  EVT_MENU(GBT_MENU_EDIT_TOGGLE_SUBGAME, gbtEfgFrame::OnEditToggleSubgame)
  EVT_MENU(GBT_MENU_EDIT_MARK_SUBGAME_TREE, gbtEfgFrame::OnEditMarkSubgameTree)
  EVT_MENU(GBT_MENU_EDIT_UNMARK_SUBGAME_TREE, gbtEfgFrame::OnEditUnmarkSubgameTree)
  EVT_MENU(GBT_MENU_EDIT_NODE, gbtEfgFrame::OnEditNode)
  EVT_MENU(GBT_MENU_EDIT_MOVE, gbtEfgFrame::OnEditMove)
  EVT_MENU(GBT_MENU_EDIT_GAME, gbtEfgFrame::OnEditGame)
  EVT_MENU(GBT_MENU_VIEW_NFG_REDUCED, gbtEfgFrame::OnViewNfgReduced)
  EVT_MENU(GBT_MENU_VIEW_PROFILES, gbtEfgFrame::OnViewProfiles)
  EVT_MENU(GBT_MENU_VIEW_NAVIGATION, gbtEfgFrame::OnViewCursor)
  EVT_MENU(GBT_MENU_VIEW_OUTCOMES, gbtEfgFrame::OnViewOutcomes)
  EVT_MENU(GBT_MENU_VIEW_SUPPORTS, gbtEfgFrame::OnViewSupports)
  EVT_MENU(GBT_MENU_VIEW_ZOOMIN, gbtEfgFrame::OnViewZoomIn)
  EVT_MENU(GBT_MENU_VIEW_ZOOMOUT, gbtEfgFrame::OnViewZoomOut)
  EVT_MENU(GBT_MENU_VIEW_SUPPORT_REACHABLE, gbtEfgFrame::OnViewSupportReachable)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_ABOVENODE, gbtEfgFrame::OnFormatFontsAboveNode)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_BELOWNODE, gbtEfgFrame::OnFormatFontsBelowNode)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_AFTERNODE, gbtEfgFrame::OnFormatFontsAfterNode)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_ABOVEBRANCH, gbtEfgFrame::OnFormatFontsAboveBranch)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_BELOWBRANCH, gbtEfgFrame::OnFormatFontsBelowBranch)
  EVT_MENU(GBT_MENU_FORMAT_DISPLAY_LAYOUT, gbtEfgFrame::OnFormatDisplayLayout)
  EVT_MENU(GBT_MENU_FORMAT_DISPLAY_LEGEND, gbtEfgFrame::OnFormatDisplayLegend)
  EVT_MENU(GBT_MENU_FORMAT_DISPLAY_COLORS, gbtEfgFrame::OnFormatDisplayColors)
  EVT_MENU(GBT_MENU_FORMAT_DISPLAY_DECIMALS, gbtEfgFrame::OnFormatDisplayDecimals)
  EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, gbtEfgFrame::OnToolsDominance)
  EVT_MENU(GBT_MENU_TOOLS_EQUILIBRIUM, gbtEfgFrame::OnToolsEquilibrium)
  EVT_MENU(GBT_MENU_TOOLS_QRE, gbtEfgFrame::OnToolsQre)
  EVT_MENU(wxID_ABOUT, gbtEfgFrame::OnHelpAbout)
  EVT_MENU(GBT_MENU_SUPPORTS_DUPLICATE, gbtEfgFrame::OnSupportDuplicate)
  EVT_MENU(GBT_MENU_SUPPORTS_DELETE, gbtEfgFrame::OnSupportDelete)
  EVT_MENU(GBT_MENU_PROFILES_NEW, gbtEfgFrame::OnProfilesNew)
  EVT_MENU(GBT_MENU_PROFILES_DUPLICATE, gbtEfgFrame::OnProfilesDuplicate)
  EVT_MENU(GBT_MENU_PROFILES_DELETE, gbtEfgFrame::OnProfilesDelete)
  EVT_MENU(GBT_MENU_PROFILES_PROPERTIES, gbtEfgFrame::OnProfilesProperties)
  EVT_MENU(GBT_MENU_PROFILES_REPORT, gbtEfgFrame::OnProfilesReport)
  EVT_SET_FOCUS(gbtEfgFrame::OnFocus)
  EVT_CLOSE(gbtEfgFrame::OnCloseWindow)
END_EVENT_TABLE()

//---------------------------------------------------------------------
//               gbtEfgFrame: Constructor and destructor
//---------------------------------------------------------------------

gbtEfgFrame::gbtEfgFrame(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxFrame(p_parent, -1, wxT(""), wxPoint(0, 0), wxSize(600, 400)),
    gbtGameView(p_doc),
    m_treeWindow(0)
{
  SetSizeHints(600, 400);

  // Give the frame an icon
#ifdef __WXMSW__
  SetIcon(wxIcon("efg_icn"));
#else
#include "bitmaps/efg.xbm"
  SetIcon(wxIcon(wxString::Format(wxT("%s"), efg_bits),
		 efg_width, efg_height));
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

  m_treeWindow = new gbtTreeView(m_doc, this);
  m_treeWindow->SetSize(GetClientSize());
  m_treeWindow->FitZoom();

  (void) new gbtEfgNavigateFrame(m_doc, this);
  (void) new gbtOutcomeFrame(m_doc, this);
  (void) new gbtEfgSupportFrame(m_doc, this);
  (void) new gbtProfileFrame(m_doc, this);

  (void) new gbtNfgFrame(m_doc, this);

  Show(true);
  m_doc->UpdateViews();
}

gbtEfgFrame::~gbtEfgFrame()
{
  wxGetApp().GetFileHistory().RemoveMenu(GetMenuBar()->GetMenu(0));
}

//---------------------------------------------------------------------
//            gbtEfgFrame: Coordinating updates of child windows
//---------------------------------------------------------------------

void gbtEfgFrame::OnUpdate(gbtGameView *)
{
  gbtGameNode cursor = m_doc->GetCursor();
  wxMenuBar *menuBar = GetMenuBar();

  menuBar->Enable(wxID_COPY, !cursor.IsNull());
  menuBar->Enable(wxID_CUT, !cursor.IsNull());
  menuBar->Enable(wxID_PASTE, (!m_doc->GetCutNode().IsNull() || 
			       !m_doc->GetCopyNode().IsNull()));

  menuBar->Enable(GBT_MENU_EDIT_INSERT, !cursor.IsNull());
  menuBar->Enable(GBT_MENU_EDIT_DELETE, 
		  !cursor.IsNull() && cursor->NumChildren() > 0);
  menuBar->Enable(GBT_MENU_EDIT_REVEAL, 
		  !cursor.IsNull() && !cursor->GetInfoset().IsNull());

  menuBar->Enable(GBT_MENU_EDIT_TOGGLE_SUBGAME,
		  (!cursor.IsNull() && cursor->IsSubgameRoot() &&
		   !cursor->GetParent().IsNull()));
  menuBar->Enable(GBT_MENU_EDIT_MARK_SUBGAME_TREE,
		  (!cursor.IsNull() && cursor->IsSubgameRoot()));
  menuBar->Enable(GBT_MENU_EDIT_UNMARK_SUBGAME_TREE,
		  (!cursor.IsNull() && cursor->IsSubgameRoot()));
  menuBar->SetLabel(GBT_MENU_EDIT_TOGGLE_SUBGAME,
		    (!cursor.IsNull() && !cursor->GetParent().IsNull() &&
		     cursor->IsSubgameRoot() &&
		     cursor->GetSubgameRoot() == cursor) ?
		    _("Unmark &subgame") : _("Mark &subgame"));

  menuBar->Enable(GBT_MENU_EDIT_NODE, !cursor.IsNull());
  menuBar->Enable(GBT_MENU_EDIT_MOVE,
		  !cursor.IsNull() && !cursor->GetInfoset().IsNull());

  menuBar->Check(GBT_MENU_VIEW_NFG_REDUCED, m_doc->ShowNfg());
  menuBar->Check(GBT_MENU_VIEW_PROFILES, m_doc->ShowProfiles());
  menuBar->Check(GBT_MENU_VIEW_NAVIGATION, m_doc->ShowEfgNavigate());
  menuBar->Check(GBT_MENU_VIEW_OUTCOMES, m_doc->ShowOutcomes());
  menuBar->Check(GBT_MENU_VIEW_SUPPORT_REACHABLE,
		 m_doc->GetPreferences().RootReachable());

  if (m_doc->GetFilename() != wxT("")) {
    SetTitle(wxString::Format(_("Gambit - [%s] %s"), 
			      m_doc->GetFilename().c_str(), 
			      (char *) m_doc->GetGame()->GetLabel()));
  }
  else {
    SetTitle(wxString::Format(_("Gambit - %s"), 
			      (char *) m_doc->GetGame()->GetLabel()));
  }
}

//--------------------------------------------------------------------
//          gbtEfgFrame: Creating and updating menus and toolbar
//--------------------------------------------------------------------

void gbtEfgFrame::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_NEW, _("&New\tCtrl-N"), _("Create a new game"));
  fileMenu->Append(wxID_OPEN, _("&Open\tCtrl-O"), _("Open a saved game"));
  fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_SAVE, _("&Save\tCtrl-S"), _("Save this game"));
  fileMenu->Append(wxID_SAVEAS, _("Save &as"), 
		   _("Save game to a different file"));
  fileMenu->AppendSeparator();
  wxMenu *fileExportMenu = new wxMenu;
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_BMP, wxT("&BMP"),
			 _("Save a rendering of the game as a Windows bitmap"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_JPEG, wxT("&JPEG"),
			 _("Save a rendering of the game as a JPEG image"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_PNG, wxT("&PNG"),
			 _("Save a rendering of the game as a PNG image"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_POSTSCRIPT, wxT("Post&Script"),
			 _("Save a printout of the game in PostScript format"));
  fileExportMenu->Enable(GBT_MENU_FILE_EXPORT_POSTSCRIPT, wxUSE_POSTSCRIPT);
  fileMenu->Append(GBT_MENU_FILE_EXPORT, _("&Export"), fileExportMenu,
		   _("Export the game in various formats"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_PRINT_SETUP, _("Page Se&tup"),
		   _("Set up preferences for printing"));
  fileMenu->Append(wxID_PREVIEW, _("Print Pre&view"),
		   _("View a preview of the game printout"));
  fileMenu->Append(wxID_PRINT, _("&Print\tCtrl-P"), _("Print this game"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, _("E&xit\tCtrl-X"), _("Exit Gambit"));

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(wxID_CUT, _("Cu&t"), _("Cut the current selection"));
  editMenu->Append(wxID_COPY, _("&Copy"), _("Copy the current selection"));
  editMenu->Append(wxID_PASTE, _("&Paste"), _("Paste from clipboard"));
  editMenu->AppendSeparator();
  editMenu->Append(GBT_MENU_EDIT_INSERT, _("&Insert"), _("Insert a move"));
  editMenu->Append(GBT_MENU_EDIT_DELETE, _("&Delete..."), 
		   _("Delete an object"));
  editMenu->Append(GBT_MENU_EDIT_REVEAL, _("&Reveal"), 
		   _("Reveal choice at node"));
  editMenu->AppendSeparator();
  editMenu->Append(GBT_MENU_EDIT_TOGGLE_SUBGAME, _("Mark &subgame"),
		   _("Mark or unmark the subgame at this node"));
  editMenu->Append(GBT_MENU_EDIT_MARK_SUBGAME_TREE, _("Mar&k subgame tree"),
		   _("Mark all subgames in this subtree"));
  editMenu->Append(GBT_MENU_EDIT_UNMARK_SUBGAME_TREE,
		   _("&Unmark subgame tree"),
		   _("Unmark all subgames in this subtree"));
  editMenu->AppendSeparator();
  editMenu->Append(GBT_MENU_EDIT_NODE, _("&Node"),
		   _("Edit properties of the node"));
  editMenu->Append(GBT_MENU_EDIT_MOVE, _("&Move"),
		   _("Edit properties of the move"));
  editMenu->Append(GBT_MENU_EDIT_GAME, _("&Game"),
		   _("Edit properties of the game"));

  wxMenu *toolsMenu = new wxMenu;

  toolsMenu->Append(GBT_MENU_TOOLS_DOMINANCE, _("&Dominance"),
		    _("Find undominated actions"));
  toolsMenu->Append(GBT_MENU_TOOLS_EQUILIBRIUM, _("&Equilibrium"),
		    _("Compute Nash equilibria and refinements"));
  toolsMenu->Append(GBT_MENU_TOOLS_QRE, _("&Qre"),
		    _("Compute quantal response equilibria"));
  
  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(GBT_MENU_VIEW_NFG_REDUCED, _("Normal form"),
		   _("Display reduced normal form"), true);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_PROFILES, _("&Profiles"),
		   _("Display/hide profiles window"), true);
  viewMenu->Check(GBT_MENU_VIEW_PROFILES, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_NAVIGATION, _("&Navigation"),
		   _("Display navigation window"), true);
  viewMenu->Append(GBT_MENU_VIEW_OUTCOMES, _("&Outcomes"),
		   _("Display and edit outcomes"), true);
  viewMenu->Check(GBT_MENU_VIEW_OUTCOMES, false);
  viewMenu->Append(GBT_MENU_VIEW_SUPPORTS, _("&Supports"),
		   _("Display and edit supports"), true);
  viewMenu->Check(GBT_MENU_VIEW_SUPPORTS, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_ZOOMIN, _("Zoom &in"),
		   _("Increase display magnification"));
  viewMenu->Append(GBT_MENU_VIEW_ZOOMOUT, _("Zoom &out"),
		   _("Decrease display magnification"));
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_SUPPORT_REACHABLE, _("&Root Reachable"),
		   _("Display only nodes that are support-reachable"),
		   true);
  
  wxMenu *formatMenu = new wxMenu;
  wxMenu *formatDisplayMenu = new wxMenu;
  formatDisplayMenu->Append(GBT_MENU_FORMAT_DISPLAY_LAYOUT, _("&Layout"),
			    _("Set tree layout parameters"));
  formatDisplayMenu->Append(GBT_MENU_FORMAT_DISPLAY_LEGEND, _("Le&gends"),
			    _("Set legends"));
  formatDisplayMenu->Append(GBT_MENU_FORMAT_DISPLAY_COLORS, _("&Colors"),
			    _("Set colors"));
  formatDisplayMenu->Append(GBT_MENU_FORMAT_DISPLAY_DECIMALS, 
			    _("&Decimal Places"),
			    _("Set number of decimal places to display"));
  formatMenu->Append(GBT_MENU_FORMAT_DISPLAY, _("&Display"), formatDisplayMenu,
		     _("Set display options"));
  
  wxMenu *formatFontsMenu = new wxMenu;
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_ABOVENODE, _("Above Node"),
			 _("Font for label above nodes"));
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_BELOWNODE, _("Below Node"),
			 _("Font for label below nodes"));
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_AFTERNODE, _("After Node"),
			 _("Font for label to right of nodes"));
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_ABOVEBRANCH, _("Above Branch"),
			 _("Font for label above branches"));
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_BELOWBRANCH, _("Below Branch"),
			 _("Font for label below branches"));
  formatMenu->Append(GBT_MENU_FORMAT_FONTS, _("&Fonts"), formatFontsMenu,
		     _("Set display fonts"));
  
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, _("&About"), _("About Gambit"));

  wxMenuBar *menuBar = new wxMenuBar(wxMB_DOCKABLE);
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(editMenu, _("&Edit"));
  menuBar->Append(viewMenu, _("&View"));
  menuBar->Append(formatMenu, _("&Format"));
  menuBar->Append(toolsMenu, _("&Tools"));
  menuBar->Append(helpMenu, _("&Help"));

  SetMenuBar(menuBar);
  wxGetApp().GetFileHistory().UseMenu(menuBar->GetMenu(0));
  wxGetApp().GetFileHistory().AddFilesToMenu(menuBar->GetMenu(0));
}

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/preview.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/zoomin.xpm"
#include "bitmaps/zoomout.xpm"
#include "bitmaps/help.xpm"

void gbtEfgFrame::MakeToolbar(void)
{
  wxToolBar *toolBar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE |
				     wxTB_HORIZONTAL);
  toolBar->SetMargins(4, 4);

  toolBar->AddTool(wxID_NEW, wxBITMAP(new), wxNullBitmap, false,
		   -1, -1, 0, _("New game"), _("Create a new game"));
  toolBar->AddTool(wxID_OPEN, wxBITMAP(open), wxNullBitmap, false,
		   -1, -1, 0, _("Open file"), _("Open a saved game"));
  toolBar->AddTool(wxID_SAVE, wxBITMAP(save), wxNullBitmap, false,
		   -1, -1, 0, _("Save game"), _("Save this game"));
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_PREVIEW, wxBITMAP(preview), wxNullBitmap,
		   false, -1, -1, 0, _("Print Preview"),
		   _("View a preview of the game printout"));
  toolBar->AddTool(wxID_PRINT, wxBITMAP(print), wxNullBitmap, false,
		   -1, -1, 0, _("Print"), _("Print this game"));
  toolBar->AddSeparator();

  toolBar->AddTool(GBT_MENU_VIEW_ZOOMIN, wxBITMAP(zoomin), wxNullBitmap,
		   false, -1, -1, 0,
		   _("Zoom in"), _("Increase magnification"));
  toolBar->AddTool(GBT_MENU_VIEW_ZOOMOUT, wxBITMAP(zoomout), wxNullBitmap,
		   false, -1, -1, 0, 
		   _("Zoom out"), _("Decrease magnification"));
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_ABOUT, wxBITMAP(help), wxNullBitmap, false,
		   -1, -1, 0, _("Help"), _("Table of contents"));

  toolBar->Realize();
  toolBar->SetRows(1);
}

//----------------------------------------------------------------------
//               gbtEfgFrame: Menu handlers - File menu
//----------------------------------------------------------------------

void gbtEfgFrame::OnFileNew(wxCommandEvent &)
{
  wxGetApp().OnFileNew(this);
}

void gbtEfgFrame::OnFileOpen(wxCommandEvent &)
{
  wxGetApp().OnFileOpen(this);
}

void gbtEfgFrame::OnFileSave(wxCommandEvent &p_event)
{
  if (p_event.GetId() == wxID_SAVEAS || m_doc->GetFilename() == wxT("")) {
    wxFileDialog dialog(this, _("Choose file"), 
			wxPathOnly(m_doc->GetFilename()),
			wxFileNameFromPath(m_doc->GetFilename()),
			wxT("*.efg"), wxSAVE | wxOVERWRITE_PROMPT);

    switch (dialog.ShowModal()) {
    case wxID_OK:
      m_doc->SetFilename(dialog.GetPath());
      break;
    case wxID_CANCEL:
    default:
      return;
    }
  }

  try {
    gbtFileOutput file(m_doc->GetFilename().mb_str());
    gbtGame efg = CompressEfg(m_doc->GetGame(), 
				 m_doc->GetEfgSupportList().GetCurrent());
    efg->WriteEfg(file);
    m_doc->SetIsModified(false);
  }
  catch (gbtFileOutput::OpenFailed &) {
    wxMessageBox(wxString::Format(_("Could not open %s for writing."),
				  (const char *) m_doc->GetFilename().mb_str()),
		 _("Error"), wxOK, this);
  }
  catch (gbtFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format(_("Write error occurred in saving %s."),
				  (const char *) m_doc->GetFilename().mb_str()),
		 _("Error"), wxOK, this);
  }
  catch (gbtGameException &) {
    wxMessageBox(_("Internal exception in extensive form"), _("Error"),
		 wxOK, this);
  }
}

void gbtEfgFrame::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupData();
  }
}

void gbtEfgFrame::OnFilePrintPreview(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrintPreview *preview = 
    new wxPrintPreview(new gbtEfgPrintout(m_treeWindow,
				       wxString::Format(wxT("%s"),
							(char *) m_doc->GetGame()->GetLabel())),
		       new gbtEfgPrintout(m_treeWindow,
				       wxString::Format(wxT("%s"),
							(char *) m_doc->GetGame()->GetLabel())),
		       &data);

  if (!preview->Ok()) {
    delete preview;
    return;
  }

  wxPreviewFrame *frame = new wxPreviewFrame(preview, this,
					     _("Print Preview"),
					     wxPoint(100, 100),
					     wxSize(600, 650));
  frame->Initialize();
  frame->Show(true);
}

void gbtEfgFrame::OnFilePrint(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);
  gbtEfgPrintout printout(m_treeWindow, 
		       wxString::Format(wxT("%s"),
					(char *) m_doc->GetGame()->GetLabel()));

  if (!printer.Print(this, &printout, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      wxMessageBox(_("There was an error in printing"), _("Error"), wxOK);
    }
    // Otherwise, user hit "cancel"; just be quiet and return.
    return;
  }
  else {
    m_printData = printer.GetPrintDialogData().GetPrintData();
  }
}

void gbtEfgFrame::OnFileExportBMP(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"),
		      wxGetApp().CurrentDir(), wxT(""),
		      _("Windows bitmap files (*.bmp)|*.bmp"), wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeWindow->m_layout.MaxX(),
		    m_treeWindow->m_layout.MaxY());
    dc.SelectObject(bitmap);
    m_treeWindow->OnDraw(dc, 1.0);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_BMP)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtEfgFrame::OnFileExportJPEG(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"),
		      wxGetApp().CurrentDir(), wxT(""),
		      _("JPEG files (*.jpeg)|*.jpeg|JPEG files (*.jpg)|*.jpg"),
		      wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeWindow->m_layout.MaxX(),
		    m_treeWindow->m_layout.MaxY());
    dc.SelectObject(bitmap);
    m_treeWindow->OnDraw(dc, 1.0);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_JPEG)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtEfgFrame::OnFileExportPNG(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"),
		      wxGetApp().CurrentDir(), wxT(""),
		      _("PNG files (*.png)|*.png"), wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treeWindow->m_layout.MaxX(),
		    m_treeWindow->m_layout.MaxY());
    dc.SelectObject(bitmap);
    m_treeWindow->OnDraw(dc, 1.0);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_PNG)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtEfgFrame::OnFileExportPS(wxCommandEvent &)
{
#if wxUSE_POSTSCRIPT
  wxPrintData printData(m_printData);

  wxFileDialog dialog(this, _("Choose output file"),
		      wxGetApp().CurrentDir(), wxT(""),
		      _("PostScript files (*.ps)|*.ps"), wxSAVE);

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
  dc.StartDoc(_("Extensive form game"));
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

void gbtEfgFrame::OnFileExit(wxCommandEvent &)
{
  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
}

void gbtEfgFrame::OnFileMRUFile(wxCommandEvent &p_event)
{
  wxGetApp().OnFileMRUFile(p_event);
}

//----------------------------------------------------------------------
//                gbtEfgFrame: Menu handlers - Edit menu
//----------------------------------------------------------------------

void gbtEfgFrame::OnEditCut(wxCommandEvent &)
{
  m_doc->SetCutNode(m_doc->GetCursor());
}

void gbtEfgFrame::OnEditCopy(wxCommandEvent &)
{
  m_doc->SetCopyNode(m_doc->GetCursor());
}

void gbtEfgFrame::OnEditPaste(wxCommandEvent &)
{
  if (!m_doc->GetCopyNode().IsNull()) {
    m_doc->Submit(new gbtCmdCopyTree(m_doc->GetCopyNode(),
				     m_doc->GetCursor()));
  }
  else {
    m_doc->Submit(new gbtCmdMoveTree(m_doc->GetCutNode(), 
				     m_doc->GetCursor()));
  }
}

void gbtEfgFrame::OnEditInsert(wxCommandEvent &)
{ 
  dialogInsertMove dialog(this, m_doc);

  if (dialog.ShowModal() == wxID_OK)  {
    m_doc->Submit(dialog.GetCommand());
  }
}

void gbtEfgFrame::OnEditDelete(wxCommandEvent &)
{
  try {
    dialogEfgDelete dialog(this, m_doc->GetCursor());

    if (dialog.ShowModal() == wxID_OK) {
      if (dialog.DeleteTree()) {
	m_doc->Submit(new gbtCmdDeleteTree(m_doc->GetCursor()));
      }
      else {
	m_doc->Submit(new gbtCmdDeleteMove(m_doc->GetCursor()));
      }
    }
  }
  catch (gbtException &ex) {
    guiExceptionDialog(ex.Description(), this);
  }
}

void gbtEfgFrame::OnEditReveal(wxCommandEvent &)
{
  dialogInfosetReveal dialog(this, m_doc->GetGame());

  if (dialog.ShowModal() == wxID_OK) {
    try {
      for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
	gbtGamePlayer player = m_doc->GetGame()->GetPlayer(pl);
	m_doc->Submit(new gbtCmdReveal(m_doc->GetCursor()->GetInfoset(),
				       player));
      }
    }
    catch (gbtException &ex) {
      guiExceptionDialog(ex.Description(), this);
    }
  }
}

void gbtEfgFrame::OnEditToggleSubgame(wxCommandEvent &)
{
  if (m_doc->GetCursor()->GetSubgameRoot() == m_doc->GetCursor()) {
    m_doc->Submit(new gbtCmdMarkSubgame(m_doc->GetCursor(), false));
  }
  else {
    m_doc->Submit(new gbtCmdMarkSubgame(m_doc->GetCursor(), true));
  }
}

void gbtEfgFrame::OnEditMarkSubgameTree(wxCommandEvent &)
{
  gbtList<gbtGameNode> subgames;
  LegalSubgameRoots(m_doc->GetGame(), m_doc->GetCursor(), subgames);
  for (int i = 1; i <= subgames.Length(); i++) {
    m_doc->Submit(new gbtCmdMarkSubgame(subgames[i], true));
  }
}

void gbtEfgFrame::OnEditUnmarkSubgameTree(wxCommandEvent &)
{
  gbtList<gbtGameNode> subgames;
  LegalSubgameRoots(m_doc->GetGame(), m_doc->GetCursor(), subgames);
  for (int i = 1; i <= subgames.Length(); i++) {
    m_doc->Submit(new gbtCmdMarkSubgame(subgames[i], false));
  }
}

void gbtEfgFrame::OnEditNode(wxCommandEvent &)
{
  dialogEditNode dialog(this, m_doc->GetCursor());
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetCursor()->SetLabel(gbtText(dialog.GetNodeName().mb_str()));
    if (dialog.GetOutcome() > 0) {
      gbtGameOutcome outcome = m_doc->GetGame()->GetOutcome(dialog.GetOutcome());
      m_doc->Submit(new gbtCmdSetOutcome(m_doc->GetCursor(), outcome));
    }
    else {
      m_doc->Submit(new gbtCmdSetOutcome(m_doc->GetCursor(), 0));
    }

    if (m_doc->GetCursor()->IsSubgameRoot() &&
	!m_doc->GetCursor()->GetParent().IsNull()) {
      m_doc->Submit(new gbtCmdMarkSubgame(m_doc->GetCursor(),
					  dialog.MarkedSubgame()));
    }

    if (m_doc->GetCursor()->NumChildren() > 0 &&
	dialog.GetInfoset() != m_doc->GetCursor()->GetInfoset()) {
      if (dialog.GetInfoset() == 0) {
	m_doc->Submit(new gbtCmdLeaveInfoset(m_doc->GetCursor()));
      }
      else {
	m_doc->Submit(new gbtCmdJoinInfoset(m_doc->GetCursor(),
					    dialog.GetInfoset()));
      }
    }
  }
}

void gbtEfgFrame::OnEditMove(wxCommandEvent &)
{
  gbtGameInfoset infoset = m_doc->GetCursor()->GetInfoset();

  dialogEditMove dialog(this, infoset);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->Submit(dialog.GetCommand());
  }
}

void gbtEfgFrame::OnEditGame(wxCommandEvent &)
{
  gbtDialogEditGame dialog(this, m_doc);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->Submit(dialog.GetCommand());
  }
}

//----------------------------------------------------------------------
//                gbtEfgFrame: Menu handlers - View menu
//----------------------------------------------------------------------

void gbtEfgFrame::OnViewNfgReduced(wxCommandEvent &)
{
  m_doc->SetShowNfg(!m_doc->ShowNfg());
}

void gbtEfgFrame::OnViewProfiles(wxCommandEvent &)
{
  m_doc->SetShowProfiles(!m_doc->ShowProfiles());
}

void gbtEfgFrame::OnViewCursor(wxCommandEvent &)
{
  m_doc->SetShowEfgNavigate(!m_doc->ShowEfgNavigate());
}

void gbtEfgFrame::OnViewOutcomes(wxCommandEvent &)
{
  m_doc->SetShowOutcomes(!m_doc->ShowOutcomes());
}

void gbtEfgFrame::OnViewSupports(wxCommandEvent &)
{
  m_doc->SetShowEfgSupports(!m_doc->ShowEfgSupports());
}

const double ZOOM_DELTA = .1;
const double ZOOM_MAX = 1;
const double ZOOM_MIN = .2;

void gbtEfgFrame::OnViewZoomIn(wxCommandEvent &)
{
  double zoom = m_treeWindow->GetZoom();
  zoom = gmin(zoom + ZOOM_DELTA, ZOOM_MAX);
  m_treeWindow->SetZoom(zoom);
}

void gbtEfgFrame::OnViewZoomOut(wxCommandEvent &)
{
  double zoom = m_treeWindow->GetZoom();
  zoom = gmax(zoom - ZOOM_DELTA, ZOOM_MIN);
  m_treeWindow->SetZoom(zoom);
}

void gbtEfgFrame::OnViewSupportReachable(wxCommandEvent &)
{
  m_doc->GetPreferences().SetRootReachable(!m_doc->GetPreferences().RootReachable());
  m_doc->UpdateViews();
}

//----------------------------------------------------------------------
//               gbtEfgFrame: Menu handlers - Format menu
//----------------------------------------------------------------------

void gbtEfgFrame::OnFormatFontsAboveNode(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_doc->GetPreferences().NodeAboveFont());
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetNodeAboveFont(dialog.GetFontData().GetChosenFont());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtEfgFrame::OnFormatFontsBelowNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetNodeBelowFont(dialog.GetFontData().GetChosenFont());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtEfgFrame::OnFormatFontsAfterNode(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetNodeRightFont(dialog.GetFontData().GetChosenFont());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtEfgFrame::OnFormatFontsAboveBranch(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetBranchAboveFont(dialog.GetFontData().GetChosenFont());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtEfgFrame::OnFormatFontsBelowBranch(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetBranchBelowFont(dialog.GetFontData().GetChosenFont());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtEfgFrame::OnFormatDisplayLayout(wxCommandEvent &)
{
  gbtPreferences &prefs = m_doc->GetPreferences();

  dialogLayout dialog(this, prefs);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.GetPreferences(prefs);
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtEfgFrame::OnFormatDisplayLegend(wxCommandEvent &)
{
  dialogLegend dialog(this, m_doc->GetPreferences());

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetNodeAboveLabel(dialog.GetNodeAbove());
    m_doc->GetPreferences().SetNodeBelowLabel(dialog.GetNodeBelow());
    m_doc->GetPreferences().SetOutcomeLabel(dialog.GetOutcome());
    m_doc->GetPreferences().SetBranchAboveLabel(dialog.GetBranchAbove());
    m_doc->GetPreferences().SetBranchBelowLabel(dialog.GetBranchBelow());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtEfgFrame::OnFormatDisplayColors(wxCommandEvent &)
{
  dialogEfgColor dialog(this, m_doc->GetPreferences());

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetChanceColor(dialog.GetChanceColor());
    m_doc->GetPreferences().SetTerminalColor(dialog.GetTerminalColor());
    for (int pl = 1; pl <= 8; pl++) {
      m_doc->GetPreferences().SetPlayerColor(pl, dialog.GetPlayerColor(pl));
    }
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews(this);
  }
}

void gbtEfgFrame::OnFormatDisplayDecimals(wxCommandEvent &)
{
  dialogSpinCtrl dialog(this, _("Decimal places"), 0, 25,
			m_doc->GetPreferences().NumDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetNumDecimals(dialog.GetValue());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

//----------------------------------------------------------------------
//             gbtEfgFrame: Menu handler - Tools->Dominance
//----------------------------------------------------------------------

void gbtEfgFrame::OnToolsDominance(wxCommandEvent &)
{
  gbtArray<gbtText> playerNames(m_doc->GetGame()->NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++) {
    playerNames[pl] = m_doc->GetGame()->GetPlayer(pl)->GetLabel();
  }
  dialogElimBehav dialog(this, playerNames);

  if (dialog.ShowModal() == wxID_OK) {
    gbtEfgSupport support(m_doc->GetEfgSupportList().GetCurrent());
    gbtProgressDialog status(this, "Dominance Elimination");

    try {
      gbtEfgSupport newSupport(support);

      while (true) {
	gbtNullOutput gnull;
	newSupport = support->Undominated(dialog.DomStrong(),
					  dialog.DomConditional(),
					  dialog.Players(), gnull, status);

	if (newSupport == support) {
	  break;
	}
	else {
	  newSupport->SetLabel(m_doc->GetEfgSupportList().GenerateUniqueLabel());
	  m_doc->Submit(new gbtCmdAddEfgSupport(newSupport));
	  support = newSupport;
	}

	if (!dialog.Iterative()) {
	  // Bit of a kludge; short-circuit loop if iterative not requested
	  break;
	}
      }
    }
    catch (gbtSignalBreak &) { }
  }
}

//----------------------------------------------------------------------
//            gbtEfgFrame: Menu handlers - Tools->Equilibrium
//----------------------------------------------------------------------

void gbtEfgFrame::OnToolsEquilibrium(wxCommandEvent &)
{
  dialogEfgNash dialog(this, m_doc->GetEfgSupportList().GetCurrent());

  if (dialog.ShowModal() == wxID_OK) {
    gbtEfgNashAlgorithm *algorithm = dialog.GetAlgorithm();

    if (!algorithm) {
      return;
    }

    try {
      gbtProgressDialog status(this, 
			       algorithm->GetAlgorithm() + "Solve Progress");
      gbtList<BehavSolution> solutions;
      solutions = algorithm->Solve(m_doc->GetEfgSupportList().GetCurrent(),
				   status);

      for (int soln = 1; soln <= solutions.Length(); soln++) {
	m_doc->AddProfile(solutions[soln]);
      }
      m_doc->SetCurrentProfile(m_doc->AllBehavProfiles().Length());
      m_doc->SetShowProfiles(true);
    }
    catch (gbtException &ex) {
      wxMessageDialog msgDialog(this,
				wxString::Format(wxT("%s"),
						 (char *) ex.Description()),
				_("Gambit exception"), wxOK);
      msgDialog.ShowModal();
    }
    catch (...) {
      wxMessageDialog msgDialog(this,
				_("An internal exception occurred while solving"),
				_("Gambit exception"), wxOK);
      msgDialog.ShowModal();
    }

    delete algorithm;
  }
}

void gbtEfgFrame::OnToolsQre(wxCommandEvent &)
{
  try {
    gbtEfgNashLogit algorithm;
    algorithm.SetFullGraph(true);
    algorithm.SetMaxLambda(10000000);

    gbtProgressDialog status(this, "QreSolve Progress");
    gbtList<BehavSolution> solutions =
      algorithm.Solve(m_doc->GetEfgSupportList().GetCurrent(), status);

    if (solutions.Length() > 0) {
      (void) new dialogQreFile(this, m_doc, solutions);
    }
  }
  catch (...) {
  }

}

void gbtEfgFrame::OnToolsNormalAgent(wxCommandEvent &)
{
#ifdef UNUSED
  // check that the game is perfect recall, if not give a warning
  if (!IsPerfectRecall(m_doc->GetGame())) {
    if (wxMessageBox("This game is not perfect recall\n"
		     "Do you wish to continue?", 
		     "Agent normal form", 
		     wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
      return;
    }
  }

  try {
    gbtGame nfg = MakeAfg(m_doc->GetGame());
    (void) new gbtNfgFrame(m_doc, m_parent);
  }
  catch (...) {
    wxMessageDialog msgDialog(this,
			      "An internal exception occurred while converting",
			      "Gambit exception", wxOK);
    msgDialog.ShowModal();
    return;
  }
#endif  // UNUSED
}

//----------------------------------------------------------------------
//                 gbtEfgFrame: Menu handlers - Help menu
//----------------------------------------------------------------------

void gbtEfgFrame::OnHelpAbout(wxCommandEvent &)
{
  wxGetApp().OnHelpAbout(this);
}

//----------------------------------------------------------------------
//               gbtEfgFrame: Menu handlers - Support menu
//----------------------------------------------------------------------

void gbtEfgFrame::OnSupportDuplicate(wxCommandEvent &)
{
  gbtEfgSupport newSupport(m_doc->GetEfgSupportList().GetCurrent());
  newSupport->SetLabel(m_doc->GetEfgSupportList().GenerateUniqueLabel());
  m_doc->Submit(new gbtCmdAddEfgSupport(newSupport));
}

void gbtEfgFrame::OnSupportDelete(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdRemoveEfgSupport());
}

//----------------------------------------------------------------------
//              gbtEfgFrame: Menu handlers - Profiles menu
//----------------------------------------------------------------------

void gbtEfgFrame::OnProfilesNew(wxCommandEvent &)
{
  BehavSolution profile = m_doc->GetGame()->NewBehavProfile(gbtNumber(0));

  dialogEditBehav dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->AddProfile(dialog.GetProfile());
    m_doc->SetCurrentProfile(m_doc->AllBehavProfiles().Length());
  }
}

void gbtEfgFrame::OnProfilesDuplicate(wxCommandEvent &)
{
  BehavSolution profile(m_doc->GetBehavProfile());

  dialogEditBehav dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->AddProfile(dialog.GetProfile());
    m_doc->SetCurrentProfile(m_doc->AllBehavProfiles().Length());
  }
}

void gbtEfgFrame::OnProfilesDelete(wxCommandEvent &)
{
  m_doc->RemoveProfile(m_doc->AllBehavProfiles().Find(m_doc->GetBehavProfile()));
}

void gbtEfgFrame::OnProfilesProperties(wxCommandEvent &)
{
  if (m_doc->IsProfileSelected()) {
    dialogEditBehav dialog(this, m_doc->GetBehavProfile());

    if (dialog.ShowModal() == wxID_OK) {
      m_doc->SetCurrentProfile(dialog.GetProfile());
    }
  }
}

void gbtEfgFrame::OnProfilesReport(wxCommandEvent &)
{
  //  dialogReport dialog(this, m_profileTable->GetReport());
  // dialog.ShowModal();
}

//----------------------------------------------------------------------
//                  gbtEfgFrame: Non-menu event handlers
//----------------------------------------------------------------------

void gbtEfgFrame::OnCloseWindow(wxCloseEvent &p_event)
{
  if (p_event.CanVeto() && m_doc->IsModified()) {
    if (wxMessageBox(_("Game has been modified.  Close anyway?"), _("Warning"),
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }

  Show(false);
  Destroy();
}

void gbtEfgFrame::OnFocus(wxFocusEvent &)
{
  if (m_treeWindow) {
    m_treeWindow->SetFocus();
  }
}
