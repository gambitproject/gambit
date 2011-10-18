//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/gameframe.cc
// Implementation of frame containing views of a game
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

#include <fstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/fontdlg.h>
#include <wx/printdlg.h>
#if !defined(__WXMSW__) || wxUSE_POSTSCRIPT
#include <wx/dcps.h>
#endif  // !defined(__WXMSW__) || wxUSE_POSTSCRIPT
#include <wx/splitter.h>

#include "libgambit/libgambit.h"

#include "gambit.h"      // for wxGetApp()
#include "gameframe.h"

#include "menuconst.h"

#include "efgpanel.h"
#include "efgprofile.h"

#include "nfgpanel.h"
#include "nfgprofile.h"

#include "dlexcept.h"
#include "dlgameprop.h"
#include "dlnash.h"
#include "dlnashmon.h"
#include "dlefglogit.h"
#include "dlabout.h"

#include "dlinsertmove.h"
#include "dlefgreveal.h"
#include "dleditnode.h"
#include "dleditmove.h"
#include "dlefglayout.h"
#include "dlefglegend.h"


//=====================================================================
//                    class gbtProfileListPanel
//=====================================================================

class gbtProfileListPanel : public wxPanel, public gbtGameView {
private:
  wxWindow *m_behavProfiles, *m_mixedProfiles;

  void OnUpdate(void) { }

public:
  gbtProfileListPanel(wxWindow *p_parent, gbtGameDocument *p_doc);

  void ShowMixed(bool p_show);
};


gbtProfileListPanel::gbtProfileListPanel(wxWindow *p_parent,
					 gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{ 
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  if (p_doc->IsTree()) {
    m_behavProfiles = new gbtBehavProfileList(this, p_doc);
    m_behavProfiles->Show(false);
    topSizer->Add(m_behavProfiles, 1, wxEXPAND, 0);
  }
  else {
    m_behavProfiles = 0;
  }

  m_mixedProfiles = new gbtMixedProfileList(this, p_doc);
  m_mixedProfiles->Show(false);
  topSizer->Add(m_mixedProfiles, 1, wxEXPAND, 0);
  
  SetSizer(topSizer);
  Layout();
}

void gbtProfileListPanel::ShowMixed(bool p_show)
{
  m_mixedProfiles->Show(p_show);
  GetSizer()->Show(m_mixedProfiles, p_show);

  if (m_behavProfiles) {
    m_behavProfiles->Show(!p_show);
    GetSizer()->Show(m_behavProfiles, !p_show);
  }

  Layout();
}

//=====================================================================
//                    class gbtAnalysisNotebook
//=====================================================================

class gbtAnalysisNotebook : public wxPanel, public gbtGameView {
private:
  gbtProfileListPanel *m_profiles;
  wxChoice *m_choices;
  wxStaticText *m_description;

  void OnChoice(wxCommandEvent &);
  void OnUpdate(void);

public:
  gbtAnalysisNotebook(wxWindow *p_parent, gbtGameDocument *p_doc);

  void ShowMixed(bool p_show);

};

gbtAnalysisNotebook::gbtAnalysisNotebook(wxWindow *p_parent,
					 gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{
  m_choices = new wxChoice(this, -1);
  m_choices->Append(wxT("Profiles"));
  m_choices->SetSelection(0);

  Connect(m_choices->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
	  wxCommandEventHandler(gbtAnalysisNotebook::OnChoice));

  m_description = new wxStaticText(this, wxID_STATIC, wxT(""));

  m_profiles = new gbtProfileListPanel(this, p_doc);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *horizSizer = new wxBoxSizer(wxHORIZONTAL);
  horizSizer->Add(m_choices, 0, wxALL | wxALIGN_CENTER, 5);
  horizSizer->Add(m_description, 1, wxALL | wxALIGN_CENTER, 5);
  topSizer->Add(horizSizer, 0, wxEXPAND, 0);

  topSizer->Add(m_profiles, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  Layout();
}

void gbtAnalysisNotebook::ShowMixed(bool p_show)
{
  m_profiles->ShowMixed(p_show);
}

void gbtAnalysisNotebook::OnChoice(wxCommandEvent &p_event)
{
  m_doc->SetProfileList(p_event.GetSelection() + 1);
}

void gbtAnalysisNotebook::OnUpdate(void)
{
  m_choices->Clear();
  for (int i = 1; i <= m_doc->NumProfileLists(); i++) {
    m_choices->Append(wxString::Format(wxT("Profiles %d"), i));
  }
  m_choices->SetSelection(m_doc->GetCurrentProfileList() - 1);

  if (m_doc->GetCurrentProfileList() > 0) {
    m_description->SetLabel(m_doc->GetProfiles().GetDescription());
  }
}


//=====================================================================
//                 Implementation of class gbtGameFrame
//=====================================================================

BEGIN_EVENT_TABLE(gbtGameFrame, wxFrame)
  EVT_MENU(GBT_MENU_FILE_NEW_EFG, gbtGameFrame::OnFileNewEfg)
  EVT_MENU(GBT_MENU_FILE_NEW_NFG, gbtGameFrame::OnFileNewNfg)
  EVT_MENU(wxID_OPEN, gbtGameFrame::OnFileOpen)
  EVT_MENU(wxID_CLOSE, gbtGameFrame::OnFileClose)
  EVT_MENU(wxID_SAVE, gbtGameFrame::OnFileSave)
  EVT_MENU(wxID_SAVEAS, gbtGameFrame::OnFileSave)
  EVT_MENU(GBT_MENU_FILE_EXPORT_EFG, gbtGameFrame::OnFileExportEfg)
  EVT_MENU(GBT_MENU_FILE_EXPORT_NFG, gbtGameFrame::OnFileExportNfg)
  EVT_MENU(GBT_MENU_FILE_EXPORT_BMP, gbtGameFrame::OnFileExportGraphic)
  EVT_MENU(GBT_MENU_FILE_EXPORT_JPEG, gbtGameFrame::OnFileExportGraphic)
  EVT_MENU(GBT_MENU_FILE_EXPORT_PNG, gbtGameFrame::OnFileExportGraphic)
  EVT_MENU(GBT_MENU_FILE_EXPORT_POSTSCRIPT, gbtGameFrame::OnFileExportPS)
  EVT_MENU(GBT_MENU_FILE_EXPORT_SVG, gbtGameFrame::OnFileExportSVG)
  EVT_MENU(wxID_PRINT_SETUP, gbtGameFrame::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, gbtGameFrame::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, gbtGameFrame::OnFilePrint)
  EVT_MENU(wxID_EXIT, gbtGameFrame::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, gbtGameFrame::OnFileMRUFile)
  EVT_MENU(wxID_UNDO, gbtGameFrame::OnEditUndo)
  EVT_MENU(wxID_REDO, gbtGameFrame::OnEditRedo)
  EVT_MENU(GBT_MENU_EDIT_INSERT_MOVE, gbtGameFrame::OnEditInsertMove)
  EVT_MENU(GBT_MENU_EDIT_INSERT_ACTION, gbtGameFrame::OnEditInsertAction)
  EVT_MENU(GBT_MENU_EDIT_DELETE_TREE, gbtGameFrame::OnEditDeleteTree)
  EVT_MENU(GBT_MENU_EDIT_DELETE_PARENT, gbtGameFrame::OnEditDeleteParent)
  EVT_MENU(GBT_MENU_EDIT_REMOVE_OUTCOME, gbtGameFrame::OnEditRemoveOutcome)
  EVT_MENU(GBT_MENU_EDIT_REVEAL, gbtGameFrame::OnEditReveal)
  EVT_MENU(GBT_MENU_EDIT_NODE, gbtGameFrame::OnEditNode)
  EVT_MENU(GBT_MENU_EDIT_MOVE, gbtGameFrame::OnEditMove)
  EVT_MENU(GBT_MENU_EDIT_GAME, gbtGameFrame::OnEditGame)
  EVT_MENU(GBT_MENU_EDIT_NEWPLAYER, gbtGameFrame::OnEditNewPlayer)
  EVT_MENU(GBT_MENU_VIEW_PROFILES, gbtGameFrame::OnViewProfiles)
  EVT_MENU(GBT_MENU_VIEW_ZOOMIN, gbtGameFrame::OnViewZoom)
  EVT_MENU(GBT_MENU_VIEW_ZOOMOUT, gbtGameFrame::OnViewZoom)
  EVT_MENU(GBT_MENU_VIEW_ZOOMFIT, gbtGameFrame::OnViewZoom)
  EVT_MENU(GBT_MENU_VIEW_ZOOM100, gbtGameFrame::OnViewZoom)
  EVT_MENU(GBT_MENU_VIEW_STRATEGIC, gbtGameFrame::OnViewStrategic)
  EVT_MENU(GBT_MENU_FORMAT_FONTS, gbtGameFrame::OnFormatFonts)
  EVT_MENU(GBT_MENU_FORMAT_LAYOUT, gbtGameFrame::OnFormatLayout)
  EVT_MENU(GBT_MENU_FORMAT_LABELS, gbtGameFrame::OnFormatLabels)
  EVT_MENU(GBT_MENU_FORMAT_DECIMALS_ADD, gbtGameFrame::OnFormatDecimalsAdd)
  EVT_MENU(GBT_MENU_FORMAT_DECIMALS_DELETE, 
	   gbtGameFrame::OnFormatDecimalsDelete)
  EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, gbtGameFrame::OnToolsDominance)
  EVT_MENU(GBT_MENU_TOOLS_EQUILIBRIUM, gbtGameFrame::OnToolsEquilibrium)
  EVT_MENU(GBT_MENU_TOOLS_QRE, gbtGameFrame::OnToolsQre)
  EVT_MENU(wxID_ABOUT, gbtGameFrame::OnHelpAbout)
  EVT_CLOSE(gbtGameFrame::OnCloseWindow)
END_EVENT_TABLE()

//---------------------------------------------------------------------
//               gbtGameFrame: Constructor and destructor
//---------------------------------------------------------------------

gbtGameFrame::gbtGameFrame(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxFrame(p_parent, -1, _T(""), wxDefaultPosition, wxSize(800, 600)),
    gbtGameView(p_doc)    
{
#if defined( __WXMSW__)
  SetIcon(wxIcon(wxT("efg_icn")));
#else
#include "bitmaps/gambit.xpm"
  SetIcon(wxIcon(gambit_xpm));
#endif

  CreateStatusBar();
  MakeMenus();
  MakeToolbar();
  
  wxAcceleratorEntry entries[10];
  entries[0].Set(wxACCEL_CTRL, (int) 'o', wxID_OPEN);
  entries[1].Set(wxACCEL_CTRL, (int) 's', wxID_SAVE);
  entries[2].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int) 's', wxID_SAVEAS);
  entries[3].Set(wxACCEL_CTRL, (int) 'p', wxID_PRINT);
  entries[4].Set(wxACCEL_CTRL, (int) 'w', wxID_CLOSE);
  entries[5].Set(wxACCEL_CTRL, (int) 'x', wxID_EXIT);
  entries[6].Set(wxACCEL_CTRL, (int) 'z', wxID_UNDO);
  entries[7].Set(wxACCEL_CTRL, (int) 'y', wxID_REDO);
  //  entries[8].Set(wxACCEL_NORMAL, WXK_DELETE, GBT_MENU_EDIT_DELETE_TREE);
  //entries[9].Set(wxACCEL_NORMAL, WXK_BACK, GBT_MENU_EDIT_DELETE_PARENT);
  entries[8].Set(wxACCEL_CTRL, (int) '+', GBT_MENU_VIEW_ZOOMIN);
  entries[9].Set(wxACCEL_CTRL, (int) '-', GBT_MENU_VIEW_ZOOMOUT);
  wxAcceleratorTable accel(10, entries);
  SetAcceleratorTable(accel);

  m_splitter = new wxSplitterWindow(this, -1);
  if (p_doc->IsTree()) {
    m_efgPanel = new gbtEfgPanel(m_splitter, p_doc);
  }
  else {
    m_efgPanel = 0;
  }

  m_nfgPanel = new gbtNfgPanel(m_splitter, p_doc);
  if (p_doc->IsTree()) {
    m_nfgPanel->Show(false);
  }

  m_analysisPanel = new gbtAnalysisNotebook(m_splitter, p_doc);
  m_analysisPanel->Show(false);

  if (p_doc->IsTree()) {
    m_splitter->Initialize(m_efgPanel);
  }
  else {
    m_splitter->Initialize(m_nfgPanel);
  }
  m_splitter->SetSashGravity(0.5);
  m_splitter->SetMinimumPaneSize(200);

  Connect(m_splitter->GetId(), wxEVT_COMMAND_SPLITTER_UNSPLIT,
	  wxSplitterEventHandler(gbtGameFrame::OnUnsplit));

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_splitter, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  Layout();

  if (p_doc->IsTree()) {
    m_efgPanel->SetFocus();
  }
  else {
    m_nfgPanel->SetFocus();
  }
  
  OnUpdate();
  Show(true);
}

gbtGameFrame::~gbtGameFrame()
{
  wxGetApp().RemoveMenu(GetMenuBar()->GetMenu(0));
}


void gbtGameFrame::OnUpdate(void)
{
  std::string gameTitle;
  gameTitle = m_doc->GetGame()->GetTitle();

  if (m_doc->GetFilename() != wxT("")) {
    SetTitle(wxT("Gambit - [") + m_doc->GetFilename() +
	     wxT("] ") +
	     wxString(gameTitle.c_str(), *wxConvCurrent));
  }
  else {
    SetTitle(wxT("Gambit - ") +
	     wxString(gameTitle.c_str(), *wxConvCurrent));
  }

  if (m_doc->IsModified()) {
    SetTitle(GetTitle() + wxT(" (unsaved changes)"));
  }

  Gambit::GameNode selectNode = m_doc->GetSelectNode();
  wxMenuBar *menuBar = GetMenuBar();

  menuBar->Enable(GBT_MENU_FILE_EXPORT_EFG, m_doc->IsTree());

  menuBar->Enable(wxID_UNDO, m_doc->CanUndo());
  GetToolBar()->EnableTool(wxID_UNDO, m_doc->CanUndo());
  menuBar->Enable(wxID_REDO, m_doc->CanRedo());
  GetToolBar()->EnableTool(wxID_REDO, m_doc->CanRedo());
  menuBar->Enable(GBT_MENU_EDIT_INSERT_MOVE, selectNode != 0);
  menuBar->Enable(GBT_MENU_EDIT_INSERT_ACTION,
		  selectNode && selectNode->GetInfoset());
  menuBar->Enable(GBT_MENU_EDIT_REVEAL,
		  selectNode && selectNode->GetInfoset());
  menuBar->Enable(GBT_MENU_EDIT_DELETE_TREE,
		  selectNode && selectNode->NumChildren() > 0);
  menuBar->Enable(GBT_MENU_EDIT_DELETE_PARENT,
		  selectNode && selectNode->GetParent());
  menuBar->Enable(GBT_MENU_EDIT_REMOVE_OUTCOME,
		  selectNode && selectNode->GetOutcome());
  menuBar->Enable(GBT_MENU_EDIT_NODE, selectNode != 0);
  menuBar->Enable(GBT_MENU_EDIT_MOVE, 
		     selectNode && selectNode->GetInfoset());

  GetToolBar()->EnableTool(GBT_MENU_EDIT_NEWPLAYER,
			   !m_efgPanel || m_efgPanel->IsShown());

  menuBar->Enable(GBT_MENU_VIEW_PROFILES, m_doc->NumProfileLists() > 0);
  GetToolBar()->EnableTool(GBT_MENU_VIEW_PROFILES,
			   m_doc->NumProfileLists() > 0);
  GetToolBar()->EnableTool(GBT_MENU_FORMAT_DECIMALS_DELETE,
			   m_doc->GetStyle().NumDecimals() > 1);

  if (m_doc->NumProfileLists() == 0 && m_splitter->IsSplit()) {
    m_splitter->Unsplit(m_analysisPanel);
  }
  menuBar->Check(GBT_MENU_VIEW_PROFILES, m_splitter->IsSplit());
  GetToolBar()->ToggleTool(GBT_MENU_VIEW_PROFILES, m_splitter->IsSplit());
  menuBar->Enable(GBT_MENU_VIEW_ZOOMIN, m_efgPanel && m_efgPanel->IsShown());
  menuBar->Enable(GBT_MENU_VIEW_ZOOMOUT, m_efgPanel && m_efgPanel->IsShown());
}

//--------------------------------------------------------------------
//          gbtGameFrame: Creating and updating menus and toolbar
//--------------------------------------------------------------------

#include "bitmaps/about.xpm"
#include "bitmaps/adddecimal.xpm"
#include "bitmaps/calc.xpm"
#include "bitmaps/close.xpm"
#include "bitmaps/deldecimal.xpm"
#include "bitmaps/exit.xpm"
#include "bitmaps/font.xpm"
#include "bitmaps/label.xpm"
#include "bitmaps/layout.xpm"
#include "bitmaps/newplayer.xpm"
#include "bitmaps/newtable.xpm"
#include "bitmaps/newtree.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/preview.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/profiles.xpm"
#include "bitmaps/redo.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/saveas.xpm"
#include "bitmaps/table.xpm"
#include "bitmaps/undo.xpm"
#include "bitmaps/zoomfit.xpm"
#include "bitmaps/zoomin.xpm"
#include "bitmaps/zoomout.xpm"
#include "bitmaps/zoom1.xpm"

//
// wxWidgets does not appear to offer a method for easily creating
// a menu item with a bitmap, so we write this convenience function
// to simplify the process.
//
// The bitmaps have currently been disabled, since they really
// don't look so great.
//
static void AppendBitmapItem(wxMenu *p_menu,
			     int p_id, const wxString &p_label,
			     const wxString &p_helpString,
			     const wxBitmap &p_bitmap)
{
  wxMenuItem *item = new wxMenuItem(p_menu, p_id, p_label, p_helpString);
#ifdef UNUSED
  // wxMac does not (apparently) support adding bitmaps to menu items,
  // so we do not set the bitmap in this case.
  item->SetBitmap(p_bitmap);
#endif // !__WXMAC__
  p_menu->Append(item);
}

void gbtGameFrame::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  
  wxMenu *fileNewMenu = new wxMenu;
  AppendBitmapItem(fileNewMenu, GBT_MENU_FILE_NEW_EFG, _("&Extensive game"),
		   _("Create a new extensive (tree) game"),
		   wxBitmap(newtree_xpm));
  AppendBitmapItem(fileNewMenu, GBT_MENU_FILE_NEW_NFG, _("&Strategic game"),
		   _("Create a new strategic (table) game"),
		   wxBitmap(newtable_xpm));
  fileMenu->Append(wxID_NEW, _("&New"), fileNewMenu, _("Create a new game"));

  AppendBitmapItem(fileMenu, wxID_OPEN, _("&Open\tCtrl-O"), 
		   _("Open a saved game"), wxBitmap(open_xpm));
  fileMenu->AppendSeparator();

  AppendBitmapItem(fileMenu, wxID_SAVE, _("&Save\tCtrl-S"), 
		   _("Save this game"), wxBitmap(save_xpm));
  AppendBitmapItem(fileMenu, wxID_SAVEAS, _("Save &as\tShift-Ctrl-S"), 
		   _("Save game to a different file"), wxBitmap(saveas_xpm));

  fileMenu->AppendSeparator();
  wxMenu *fileExportMenu = new wxMenu;
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_EFG, _("Gambit .&efg format"),
			 _("Save the extensive game in .efg format"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_NFG, _("Gambit .&nfg format"),
			 _("Save the strategic game in .nfg format"));
  fileExportMenu->AppendSeparator();
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_BMP, _("&BMP"),
			 _("Save a rendering of the game as a Windows bitmap"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_JPEG, _("&JPEG"),
			 _("Save a rendering of the game as a JPEG image"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_PNG, _("&PNG"),
			 _("Save a rendering of the game as a PNG image"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_POSTSCRIPT, _("Post&Script"),
			 _("Save a printout of the game in PostScript format"));
  fileExportMenu->Enable(GBT_MENU_FILE_EXPORT_POSTSCRIPT, wxUSE_POSTSCRIPT);
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_SVG, _("S&VG"),
			 _("Save a rendering of the game in SVG format"));
  fileMenu->Append(GBT_MENU_FILE_EXPORT, _("&Export"), fileExportMenu,
		   _("Export the game in various formats"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_PRINT_SETUP, _("Page Se&tup"),
		   _("Set up preferences for printing"));
  AppendBitmapItem(fileMenu, wxID_PREVIEW, _("Print Pre&view"),
		   _("View a preview of the game printout"),
		   wxBitmap(preview_xpm));
  AppendBitmapItem(fileMenu, wxID_PRINT, _("&Print\tCtrl-P"), 
		   _("Print this game"), wxBitmap(print_xpm));

  fileMenu->AppendSeparator();
  AppendBitmapItem(fileMenu, wxID_CLOSE, _("&Close\tCtrl-W"), 
		   _("Close this window"), wxBitmap(close_xpm));
  AppendBitmapItem(fileMenu, wxID_EXIT, _("E&xit\tCtrl-Q"), _("Exit Gambit"),
		   wxBitmap(exit_xpm));

  wxMenu *editMenu = new wxMenu;

  AppendBitmapItem(editMenu, wxID_UNDO, _("&Undo\tCtrl-Z"), 
		   _("Undo the last change"), wxBitmap(undo_xpm));
  AppendBitmapItem(editMenu, wxID_REDO, _("&Redo\tCtrl-Y"),
		   _("Redo the last undone change"), wxBitmap(redo_xpm));

  editMenu->AppendSeparator();
  AppendBitmapItem(editMenu, GBT_MENU_EDIT_NEWPLAYER, _("Add p&layer"),
		   _("Add a new player to the game"), wxBitmap(newplayer_xpm));

  editMenu->AppendSeparator();
  editMenu->Append(GBT_MENU_EDIT_INSERT_MOVE, _("&Insert move"), 
		   _("Insert a move"));
  editMenu->Append(GBT_MENU_EDIT_INSERT_ACTION, _("Insert &action"),
		   _("Insert an action at the current move"));
  editMenu->Append(GBT_MENU_EDIT_REVEAL, _("&Reveal"), 
		   _("Reveal choice at node"));
  editMenu->AppendSeparator();

  editMenu->Append(GBT_MENU_EDIT_DELETE_TREE, 
		   _("&Delete subtree"), 
		   _("Delete the subtree starting at the selected node"));
  editMenu->Append(GBT_MENU_EDIT_DELETE_PARENT, 
		   _("Delete &parent"), 
		   _("Delete the node directly before the selected node"));
  editMenu->Append(GBT_MENU_EDIT_REMOVE_OUTCOME,
		   _("Remove &outcome"),
		   _("Remove the outcome from the selected node"));
  editMenu->AppendSeparator();

  editMenu->Append(GBT_MENU_EDIT_NODE, _("&Node"),
		   _("Edit properties of the node"));
  editMenu->Append(GBT_MENU_EDIT_MOVE, _("&Move"),
		   _("Edit properties of the move"));

  editMenu->AppendSeparator();
  editMenu->Append(GBT_MENU_EDIT_GAME, _("&Game"),
		   _("Edit properties of the game"));

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(GBT_MENU_VIEW_PROFILES, _("&Profiles"),
		   _("Display/hide profiles window"), true);
  viewMenu->Check(GBT_MENU_VIEW_PROFILES, false);
  viewMenu->AppendSeparator();

  AppendBitmapItem(viewMenu, GBT_MENU_VIEW_ZOOMIN, _("Zoom &in"),
		   _("Increase display magnification"), wxBitmap(zoomin_xpm));
  AppendBitmapItem(viewMenu, GBT_MENU_VIEW_ZOOMOUT, _("Zoom &out"),
		   _("Decrease display magnification"), wxBitmap(zoomout_xpm));
  AppendBitmapItem(viewMenu, GBT_MENU_VIEW_ZOOM100,
		   _("&Zoom 1:1"), _("Set magnification to 1:1"),
		   wxBitmap(zoom1_xpm));
  AppendBitmapItem(viewMenu, GBT_MENU_VIEW_ZOOMFIT, _("&Fit tree to window"),
		   _("Rescale to show entire tree in window"),
		   wxBitmap(zoomfit_xpm));

  viewMenu->AppendSeparator();
    
  viewMenu->Append(GBT_MENU_VIEW_STRATEGIC, _("&Strategic game"),
		   wxT("Display the reduced strategic representation ")
		     wxT("of the game"), true);
  
  wxMenu *formatMenu = new wxMenu;
  AppendBitmapItem(formatMenu, GBT_MENU_FORMAT_LAYOUT, _("&Layout"),
		   _("Set tree layout parameters"), wxBitmap(layout_xpm));
  AppendBitmapItem(formatMenu, GBT_MENU_FORMAT_LABELS, _("La&bels"),
		   _("Set labels for parts of trees"), wxBitmap(label_xpm));
  AppendBitmapItem(formatMenu, GBT_MENU_FORMAT_FONTS, _("&Font"),
		   _("Set the font for tree labels"), wxBitmap(font_xpm));
  
  wxMenu *toolsMenu = new wxMenu;
  toolsMenu->Append(GBT_MENU_TOOLS_DOMINANCE, _("&Dominance"),
		    _("Find undominated actions"), true);
  AppendBitmapItem(toolsMenu, GBT_MENU_TOOLS_EQUILIBRIUM, _("&Equilibrium"),
		   _("Compute Nash equilibria and refinements"),
		   wxBitmap(calc_xpm));

  toolsMenu->Append(GBT_MENU_TOOLS_QRE, _("&Qre"),
		    _("Compute quantal response equilibria"));


  wxMenu *helpMenu = new wxMenu;
  AppendBitmapItem(helpMenu, wxID_ABOUT, _("&About"), _("About Gambit"),
		   wxBitmap(about_xpm));

  wxMenuBar *menuBar = new wxMenuBar();
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(editMenu, _("&Edit"));
  menuBar->Append(viewMenu, _("&View"));
  menuBar->Append(formatMenu, _("&Format"));
  menuBar->Append(toolsMenu, _("&Tools"));
  menuBar->Append(helpMenu, _("&Help"));

  // Set the menu bar
  SetMenuBar(menuBar);

  wxGetApp().AddMenu(GetMenuBar()->GetMenu(0));
}

void gbtGameFrame::MakeToolbar(void)
{
  wxToolBar *toolBar = CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT);
  toolBar->SetMargins(4, 4);
  toolBar->SetToolBitmapSize(wxSize(24, 24));

  toolBar->AddTool(GBT_MENU_FILE_NEW_EFG, wxBitmap(newtree_xpm),
		   wxNullBitmap, false, -1, -1, 0,
		   _("Create a new extensive (tree) game"),
		   _("Create a new extensive (tree) game"));
  toolBar->AddTool(GBT_MENU_FILE_NEW_NFG, wxBitmap(newtable_xpm),
		   wxNullBitmap, false, -1, -1, 0,
		   _("Create a new strategic (table) game"),
		   _("Create a new strategic (table) game"));
  toolBar->AddTool(wxID_OPEN, wxBitmap(open_xpm), wxNullBitmap, false,
		   -1, -1, 0, _("Open a file"), _("Open a saved game"));
  toolBar->AddTool(wxID_SAVE, wxBitmap(save_xpm), wxNullBitmap, false,
		   -1, -1, 0, _("Save this game"), _("Save this game"));
  toolBar->AddTool(wxID_SAVEAS, wxBitmap(saveas_xpm), wxNullBitmap, false,
		   -1, -1, 0, _("Save to a different file"), 
		   _("Save this game to another file"));
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_PRINT, wxBitmap(print_xpm), wxNullBitmap, false,
		   -1, -1, 0, _("Print this game"), _("Print this game"));
  toolBar->AddTool(wxID_PREVIEW, wxBitmap(preview_xpm), wxNullBitmap,
		   false, -1, -1, 0, _("Print preview"),
		   _("View a preview of the game printout"));

  toolBar->AddSeparator();
  toolBar->AddTool(wxID_UNDO, wxBitmap(undo_xpm), wxNullBitmap,
		   false, -1, -1, 0, _("Undo the last action"),
		   _("Undo the last change to the game"));
  toolBar->AddTool(wxID_REDO, wxBitmap(redo_xpm), wxNullBitmap,
		   false, -1, -1, 0, _("Redo the undone action"),
		   _("Redo the last undone change"));

  toolBar->AddSeparator();
  toolBar->AddTool(GBT_MENU_EDIT_NEWPLAYER, wxBitmap(newplayer_xpm),
		   wxNullBitmap, false, -1, -1, 0,
		   _("Add a new player"), _("Add a new player to the game"));
  if (m_doc->IsTree()) {
    toolBar->AddTool(GBT_MENU_VIEW_ZOOMIN, wxBitmap(zoomin_xpm), wxNullBitmap,
		     false, -1, -1, 0, 
		     _("Zoom in"), _("Increase magnification"));
    toolBar->AddTool(GBT_MENU_VIEW_ZOOMOUT, wxBitmap(zoomout_xpm), wxNullBitmap,
		     false, -1, -1, 0, 
		     _("Zoom out"), _("Decrease magnification"));
    toolBar->AddTool(GBT_MENU_VIEW_ZOOMFIT, wxBitmap(zoomfit_xpm), wxNullBitmap,
		     false, -1, -1, 0, _("Fit to window"), 
		     _("Set magnification to see entire tree"));
  }

  toolBar->AddSeparator();
  toolBar->AddTool(GBT_MENU_FORMAT_DECIMALS_ADD, wxBitmap(adddecimal_xpm),
		   wxNullBitmap, false, -1, -1, 0, 
		   _("Increase the number of decimals displayed"),
		   _("Increase the number of decimal places shown"));

  toolBar->AddTool(GBT_MENU_FORMAT_DECIMALS_DELETE, wxBitmap(deldecimal_xpm),
		   wxNullBitmap, false, -1, -1, 0, 
		   _("Decrease the number of decimals displayed"),
		   _("Decrease the number of decimal places shown"));

  toolBar->AddSeparator();
  if (m_doc->IsTree()) {
    toolBar->AddTool(GBT_MENU_VIEW_STRATEGIC, wxBitmap(table_xpm),
		     wxNullBitmap, true, -1, -1, 0, 
		     wxT("Display the reduced strategic representation ")
		       wxT("of the game"),
		     wxT("Display the reduced strategic representation ")
		       wxT("of the game"));
  }
  toolBar->AddTool(GBT_MENU_VIEW_PROFILES, wxBitmap(profiles_xpm),
		   wxNullBitmap, true, -1, -1, 0, 
		   _("View the list of computed strategy profiles"), 
		   _("Show or hide the list of computed strategy profiles"));
  toolBar->AddTool(GBT_MENU_TOOLS_EQUILIBRIUM, wxBitmap(calc_xpm),
		   wxNullBitmap, false, -1, -1, 0, 
		   _("Compute Nash equilibria of this game"),
		   _("Compute Nash equilibria of this game"));
  
  toolBar->AddSeparator();
  toolBar->AddTool(wxID_ABOUT, wxBitmap(about_xpm), wxNullBitmap, false,
		   -1, -1, 0, _("About Gambit"), _("About Gambit"));

  toolBar->Realize();
  toolBar->SetRows(1);
}

//----------------------------------------------------------------------
//               gbtGameFrame: Menu handlers - File menu
//----------------------------------------------------------------------

void gbtGameFrame::OnFileNewEfg(wxCommandEvent &)
{
  Gambit::Game efg = Gambit::NewTree();
  efg->SetTitle("Untitled Extensive Game");
  efg->NewPlayer()->SetLabel("Player 1");
  efg->NewPlayer()->SetLabel("Player 2");
  gbtGameDocument *doc = new gbtGameDocument(efg);
  (void) new gbtGameFrame(0, doc);
}

void gbtGameFrame::OnFileNewNfg(wxCommandEvent &)
{
  Gambit::Array<int> dim(2);
  dim[1] = 2;
  dim[2] = 2;
  Gambit::Game nfg = Gambit::NewTable(dim);
  nfg->SetTitle("Untitled Strategic Game");
  nfg->GetPlayer(1)->SetLabel("Player 1");
  nfg->GetPlayer(2)->SetLabel("Player 2");
  gbtGameDocument *doc = new gbtGameDocument(nfg);
  (void) new gbtGameFrame(0, doc);
}

void gbtGameFrame::OnFileOpen(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file to open"), 
		      wxGetApp().GetCurrentDir(), _T(""), 
		      wxT("Gambit workbooks (*.gbt)|*.gbt|")
			wxT("Gambit extensive games (*.efg)|*.efg|")
			wxT("Gambit strategic games (*.nfg)|*.nfg|")
			wxT("All files (*.*)|*.*"));

  if (dialog.ShowModal() == wxID_OK) {
    wxString filename = dialog.GetPath();

    wxGetApp().SetCurrentDir(wxPathOnly(filename));
    wxConfig config(_T("Gambit"));
    config.Write(_T("/General/CurrentDirectory"), wxPathOnly(filename));

    gbtAppLoadResult result = wxGetApp().LoadFile(filename);
    if (result == GBT_APP_OPEN_FAILED) {
      wxMessageDialog dialog(this,
			     wxT("Gambit could not open file '") + 
			     filename + wxT("' for reading."), 
			     wxT("Unable to open file"),
			     wxOK | wxICON_ERROR);
      dialog.ShowModal();
    }
    else if (result == GBT_APP_PARSE_FAILED) {
      wxMessageDialog dialog(this,
			     wxT("File '") + filename +
			     wxT("' is not in a format Gambit recognizes."),
			     wxT("Unable to read file"),
			     wxOK | wxICON_ERROR);
      dialog.ShowModal();
    }
  }

}

void gbtGameFrame::OnFileClose(wxCommandEvent &)
{
  Close();
}

void gbtGameFrame::OnFileSave(wxCommandEvent &p_event)
{
  if (p_event.GetId() == wxID_SAVEAS || m_doc->GetFilename() == wxT("")) {
    wxFileDialog dialog(this, _("Choose file"),
			wxPathOnly(m_doc->GetFilename()),
			wxFileNameFromPath(m_doc->GetFilename()),
			wxT("Gambit workbooks (*.gbt)|*.gbt|")
			wxT("All files (*.*)|*.*"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK) {
      try {
	m_doc->DoSave(dialog.GetPath());
      }
      catch (std::exception &ex) {
	gbtExceptionDialog(this, ex.what()).ShowModal();
      }
    }
  }
  else {
    try {
      m_doc->DoSave(m_doc->GetFilename());
    }
    catch (std::exception &ex) {
      gbtExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}

void gbtGameFrame::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  m_printData.SetOrientation(wxLANDSCAPE);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupDialogData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupDialogData();
  }
}

void gbtGameFrame::OnFilePrintPreview(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);

  wxPrintPreview *preview = 0;
  if (m_efgPanel && m_splitter->GetWindow1() == m_efgPanel) {
    preview = new wxPrintPreview(m_efgPanel->GetPrintout(), 
				 m_efgPanel->GetPrintout(),
				 &data);
  }
  else {
    preview = new wxPrintPreview(m_nfgPanel->GetPrintout(), 
				 m_nfgPanel->GetPrintout(),
				 &data);
  }

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

void gbtGameFrame::OnFilePrint(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);

  wxPrintout *printout;
  if (m_efgPanel && m_splitter->GetWindow1() == m_efgPanel) {
    printout = m_efgPanel->GetPrintout();
  }
  else {
    printout = m_nfgPanel->GetPrintout();
  }

  if (!printer.Print(this, printout, true)) {
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

void gbtGameFrame::OnFileExportEfg(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"), 
		      wxGetApp().GetCurrentDir(), _T(""),
		      wxT("Gambit extensive games (*.efg)|*.efg|")
			 wxT("All files (*.*)|*.*"), 
		      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->DoExportEfg(dialog.GetPath());
  }
}

void gbtGameFrame::OnFileExportNfg(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"), 
		      wxGetApp().GetCurrentDir(), _T(""),
		      wxT("Gambit strategic games (*.nfg)|*.nfg|")
			 wxT("All files (*.*)|*.*"),
		      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->DoExportNfg(dialog.GetPath());
  }
}

void gbtGameFrame::OnFileExportGraphic(wxCommandEvent &p_event)
{
  wxBitmap bitmap = wxNullBitmap;
  bool bitmapOK = false;

  if (m_efgPanel && m_efgPanel->IsShown()) {
    bitmapOK = m_efgPanel->GetBitmap(bitmap, 50, 50);
  }
  else {
    bitmapOK = m_nfgPanel->GetBitmap(bitmap, 50, 50);
  }

  if (!bitmapOK) {
    wxMessageBox(_("Game image too large to export to graphics file"),
		 _("Error"), wxOK, this);
    return;
  }

  wxString filter = wxT("|All files (*.*)|*.*");
  switch (p_event.GetId()) {
  case GBT_MENU_FILE_EXPORT_BMP:
    filter = wxT("Windows bitmap files (*.bmp)|*.bmp") + filter;
    break;
  case GBT_MENU_FILE_EXPORT_JPEG:
    filter = wxT("JPEG files (*.jpeg)|*.jpeg|")
		 wxT("JPG files (*.jpg)|*.jpg") + filter;
    break;
  case GBT_MENU_FILE_EXPORT_PNG:
    filter = wxT("PNG files (*.png)|*.png") + filter;
    break;
  default:
    break;
  }

  wxFileDialog dialog(this, _("Choose output file"), 
		      wxGetApp().GetCurrentDir(), _T(""), filter,
		      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxBitmapType code;
    switch (p_event.GetId()) {
    case GBT_MENU_FILE_EXPORT_BMP:  code = wxBITMAP_TYPE_BMP;  break;
    case GBT_MENU_FILE_EXPORT_JPEG: code = wxBITMAP_TYPE_JPEG; break;
    case GBT_MENU_FILE_EXPORT_PNG:  code = wxBITMAP_TYPE_PNG;  break;
    default: break;
    }

    if (!bitmap.SaveFile(dialog.GetPath(), code)) {
      wxMessageBox(_("An error occurred in writing ") + dialog.GetPath() + 
		   wxT("."),  _("Error"), wxOK, this);
    }
  }
}

void gbtGameFrame::OnFileExportPS(wxCommandEvent &)
{
#if wxUSE_POSTSCRIPT
  wxPrintData printData(m_printData);

  wxFileDialog dialog(this, _("Choose output file"),
		      wxGetApp().GetCurrentDir(), _T(""),
		      _T("PostScript files (*.ps)|*.ps"), 
		      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    printData.SetFilename(dialog.GetPath());
  }
  else {
    return;
  }
  printData.SetPrintMode(wxPRINT_MODE_FILE);

  wxPostScriptDC dc(printData);
  dc.SetBackgroundMode(wxTRANSPARENT);
  if (m_efgPanel && m_efgPanel->IsShown()) {
    dc.StartDoc(_T("Gambit extensive game"));
  }
  else {
    dc.StartDoc(_T("Gambit strategic game"));
  }
  dc.StartPage();
  if (m_efgPanel && m_efgPanel->IsShown()) {
    m_efgPanel->RenderGame(dc, 50, 50);
  }
  else {
    m_nfgPanel->RenderGame(dc, 50, 50);
  }
  dc.EndPage();
  dc.EndDoc();
#endif  // wxUSE_POSTSCRIPT
}

void gbtGameFrame::OnFileExportSVG(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"), 
		      wxGetApp().GetCurrentDir(), _T(""),
		      wxT("SVG files (*.svg)|*.svg|")
			  wxT("All files (*.*)|*.*"),
		      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    if (m_efgPanel && m_efgPanel->IsShown()) {
      m_efgPanel->GetSVG(dialog.GetPath(), 50, 50);
    }
    else {
      m_nfgPanel->GetSVG(dialog.GetPath(), 50, 50);
    }
  }
}

void gbtGameFrame::OnFileExit(wxCommandEvent &p_event)
{
  if (wxGetApp().AreDocumentsModified()) {
    if (wxMessageBox(wxT("There are modified games.\n")
		       wxT("Any unsaved changes will be lost!\n")
		       wxT("Close anyway?"), 
		     _("Warning"),
		     wxOK | wxCANCEL) == wxCANCEL) {
      return;
    }
  }

  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
}

void gbtGameFrame::OnFileMRUFile(wxCommandEvent &p_event)
{
  wxString filename = wxGetApp().GetHistoryFile(p_event.GetId() - wxID_FILE1);
  gbtAppLoadResult result = wxGetApp().LoadFile(filename);

  if (result == GBT_APP_OPEN_FAILED) {
    wxMessageDialog dialog(this,
			   wxT("Gambit could not open file '") + 
			   filename + wxT("' for reading."), 
			   wxT("Unable to open file"),
			   wxOK | wxICON_ERROR);
    dialog.ShowModal();
  }
  else if (result == GBT_APP_PARSE_FAILED) {
    wxMessageDialog dialog(this,
			   wxT("File '") + filename +
			   wxT("' is not in a format Gambit recognizes."),
			   wxT("Unable to read file"),
			   wxOK | wxICON_ERROR);
    dialog.ShowModal();
  }
}

//----------------------------------------------------------------------
//                gbtGameFrame: Menu handlers - Edit menu
//----------------------------------------------------------------------

void gbtGameFrame::OnEditUndo(wxCommandEvent &)
{
  if (m_doc->CanUndo()) m_doc->Undo();
}

void gbtGameFrame::OnEditRedo(wxCommandEvent &)
{
  if (m_doc->CanRedo()) m_doc->Redo();
}

void gbtGameFrame::OnEditInsertMove(wxCommandEvent &)
{ 
  gbtInsertMoveDialog dialog(this, m_doc);
  if (dialog.ShowModal() == wxID_OK)  {
    try {
      if (dialog.GetInfoset()) {
	m_doc->DoInsertMove(m_doc->GetSelectNode(), dialog.GetInfoset());
      }
      else {
	m_doc->DoInsertMove(m_doc->GetSelectNode(), 
			    dialog.GetPlayer(), dialog.GetActions());
      }
    }
    catch (std::exception &ex) {
      gbtExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}

void gbtGameFrame::OnEditInsertAction(wxCommandEvent &)
{
  try {
    m_doc->DoInsertAction(m_doc->GetSelectNode());
  }
  catch (std::exception &ex) {
    gbtExceptionDialog(this, ex.what()).ShowModal();
  }
}

void gbtGameFrame::OnEditDeleteTree(wxCommandEvent &)
{
  try {
    m_doc->DoDeleteTree(m_doc->GetSelectNode());
  }
  catch (std::exception &ex) {
    gbtExceptionDialog(this, ex.what()).ShowModal();
  }
}

void gbtGameFrame::OnEditDeleteParent(wxCommandEvent &)
{
  try {
    m_doc->DoDeleteParent(m_doc->GetSelectNode());
  }
  catch (std::exception &ex) {
    gbtExceptionDialog(this, ex.what()).ShowModal();
  }
}

void gbtGameFrame::OnEditRemoveOutcome(wxCommandEvent &)
{
  try {
    m_doc->DoRemoveOutcome(m_doc->GetSelectNode());
  }
  catch (std::exception &ex) {
    gbtExceptionDialog(this, ex.what()).ShowModal();
  }
}

void gbtGameFrame::OnEditReveal(wxCommandEvent &)
{
  gbtRevealMoveDialog dialog(this, m_doc);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      for (int pl = 1; pl <= dialog.GetPlayers().Length(); pl++) {
	m_doc->DoRevealAction(m_doc->GetSelectNode()->GetInfoset(),
			      dialog.GetPlayers()[pl]);
      }
    }
    catch (std::exception &ex) {
      gbtExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}

void gbtGameFrame::OnEditNode(wxCommandEvent &)
{
  dialogEditNode dialog(this, m_doc->GetSelectNode());
  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_doc->DoSetNodeLabel(m_doc->GetSelectNode(), dialog.GetNodeName());
      if (dialog.GetOutcome() > 0) {
	m_doc->DoSetOutcome(m_doc->GetSelectNode(),
			    m_doc->GetGame()->GetOutcome(dialog.GetOutcome()));
      }
      else {
	m_doc->DoSetOutcome(m_doc->GetSelectNode(), 0);
      }

      if (m_doc->GetSelectNode()->NumChildren() > 0 &&
	  dialog.GetInfoset() != m_doc->GetSelectNode()->GetInfoset()) {
	if (dialog.GetInfoset() == 0) {
	  m_doc->DoLeaveInfoset(m_doc->GetSelectNode());
	}
	else {
	  m_doc->DoSetInfoset(m_doc->GetSelectNode(), dialog.GetInfoset());
	}
      }
    }
    catch (std::exception &ex) {
      gbtExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}

void gbtGameFrame::OnEditMove(wxCommandEvent &)
{
  Gambit::GameInfoset infoset = m_doc->GetSelectNode()->GetInfoset();
  if (!infoset)  return;

  gbtEditMoveDialog dialog(this, infoset);
  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_doc->DoSetInfosetLabel(infoset, dialog.GetInfosetName());

      if (!infoset->IsChanceInfoset() && 
	  dialog.GetPlayer() != infoset->GetPlayer()->GetNumber()) {
	m_doc->DoSetPlayer(infoset, 
			   m_doc->GetGame()->GetPlayer(dialog.GetPlayer()));
      }

      for (int act = 1; act <= infoset->NumActions(); act++) {
	m_doc->DoSetActionLabel(infoset->GetAction(act), 
				dialog.GetActionName(act));
	if (infoset->IsChanceInfoset()) {
	  m_doc->DoSetActionProb(infoset, infoset->GetAction(act)->GetNumber(),
				 dialog.GetActionProb(act));
	}
      }
    }
    catch (std::exception &ex) {
      gbtExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}

void gbtGameFrame::OnEditGame(wxCommandEvent &)
{
  gbtGamePropertiesDialog dialog(this, m_doc);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->DoSetTitle(dialog.GetTitle(), dialog.GetComment());
  }
}

void gbtGameFrame::OnEditNewPlayer(wxCommandEvent &)
{
  try {
    m_doc->DoNewPlayer();
  }
  catch (std::exception &ex) {
    gbtExceptionDialog(this, ex.what()).ShowModal();
  }
}

//----------------------------------------------------------------------
//                gbtGameFrame: Menu handlers - View menu
//----------------------------------------------------------------------

void gbtGameFrame::OnViewProfiles(wxCommandEvent &p_event)
{
  if (m_splitter->IsSplit()) {
    m_splitter->Unsplit(m_analysisPanel);
  }
  else if (m_efgPanel && m_efgPanel->IsShown()) {
    m_analysisPanel->ShowMixed(false);
    m_splitter->SplitHorizontally(m_efgPanel, m_analysisPanel);
  }
  else {
    m_analysisPanel->ShowMixed(true);
    m_splitter->SplitHorizontally(m_nfgPanel, m_analysisPanel);
  }

  GetMenuBar()->Check(GBT_MENU_VIEW_PROFILES, p_event.IsChecked());
  GetToolBar()->ToggleTool(GBT_MENU_VIEW_PROFILES, p_event.IsChecked());
}

void gbtGameFrame::OnViewZoom(wxCommandEvent &p_event)
{
  // All zoom events get passed along to the panel
  wxPostEvent(m_efgPanel, p_event);
}

void gbtGameFrame::OnViewStrategic(wxCommandEvent &p_event)
{
  if (m_efgPanel->IsShown()) {
    // We are switching to strategic view

    if (!m_doc->GetGame()->IsPerfectRecall()) {
      if (wxMessageBox(wxT("This is not a game of perfect recall\n")
			 wxT("Do you wish to continue?"), 
		       _("Strategic game"), 
		       wxOK | wxCANCEL | wxALIGN_CENTER, this) != wxOK) {
	return;
      }
    }
    
    m_doc->BuildNfg();

    m_splitter->ReplaceWindow(m_efgPanel, m_nfgPanel);
    m_efgPanel->Show(false);
    m_nfgPanel->Show(true);
    if (m_splitter->IsSplit()) {
      m_analysisPanel->ShowMixed(true);
    }
    m_nfgPanel->SetFocus();
    m_nfgPanel->OnUpdate();
  }
  else {
    m_splitter->ReplaceWindow(m_nfgPanel, m_efgPanel);
    m_nfgPanel->Show(false);
    m_efgPanel->Show(true);
    if (m_splitter->IsSplit()) {
      m_analysisPanel->ShowMixed(false);
    }
    m_efgPanel->SetFocus();
  }

  GetMenuBar()->Check(GBT_MENU_VIEW_STRATEGIC, m_nfgPanel->IsShown());
  GetMenuBar()->Enable(GBT_MENU_VIEW_ZOOMIN, !p_event.IsChecked());
  GetMenuBar()->Enable(GBT_MENU_VIEW_ZOOMOUT, !p_event.IsChecked());

  GetToolBar()->ToggleTool(GBT_MENU_VIEW_STRATEGIC, p_event.IsChecked());
  GetToolBar()->EnableTool(GBT_MENU_VIEW_ZOOMIN, !p_event.IsChecked());
  GetToolBar()->EnableTool(GBT_MENU_VIEW_ZOOMOUT, !p_event.IsChecked());
  GetToolBar()->EnableTool(GBT_MENU_VIEW_ZOOMFIT, !p_event.IsChecked());
}

//----------------------------------------------------------------------
//               gbtGameFrame: Menu handlers - Format menu
//----------------------------------------------------------------------

void gbtGameFrame::OnFormatLayout(wxCommandEvent &)
{
  gbtStyle style = m_doc->GetStyle();
  gbtLayoutDialog dialog(this, style);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.GetSettings(style);
    m_doc->SetStyle(style);
  }
}

void gbtGameFrame::OnFormatLabels(wxCommandEvent &)
{
  gbtLegendDialog dialog(this, m_doc->GetStyle());

  if (dialog.ShowModal() == wxID_OK) {
    gbtStyle style = m_doc->GetStyle();
    style.SetNodeAboveLabel(dialog.GetNodeAbove());
    style.SetNodeBelowLabel(dialog.GetNodeBelow());
    style.SetBranchAboveLabel(dialog.GetBranchAbove());
    style.SetBranchBelowLabel(dialog.GetBranchBelow());
    m_doc->SetStyle(style);
  }
}

void gbtGameFrame::OnFormatFonts(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_doc->GetStyle().GetFont());
  wxFontDialog dialog(this, data);
  
  if (dialog.ShowModal() == wxID_OK) {
    gbtStyle style = m_doc->GetStyle();
    style.SetFont(dialog.GetFontData().GetChosenFont());
    m_doc->SetStyle(style);
  }
}

void gbtGameFrame::OnFormatDecimalsAdd(wxCommandEvent &)
{
  gbtStyle style = m_doc->GetStyle();
  style.SetNumDecimals(style.NumDecimals()+1);
  m_doc->SetStyle(style);
}

void gbtGameFrame::OnFormatDecimalsDelete(wxCommandEvent &)
{
  gbtStyle style = m_doc->GetStyle();
  style.SetNumDecimals(style.NumDecimals()-1);
  m_doc->SetStyle(style);
}

//----------------------------------------------------------------------
//               gbtGameFrame: Menu handlers - Tools menu
//----------------------------------------------------------------------

void gbtGameFrame::OnToolsDominance(wxCommandEvent &p_event)
{
  if (m_efgPanel)  wxPostEvent(m_efgPanel, p_event);
  if (m_nfgPanel)  wxPostEvent(m_nfgPanel, p_event);
  if (!p_event.IsChecked()) {
    m_doc->TopBehavElimLevel();
    m_doc->TopStrategyElimLevel();
  }
}

void gbtGameFrame::OnToolsEquilibrium(wxCommandEvent &)
{
  gbtNashChoiceDialog dialog(this, m_doc);

  if (dialog.ShowModal() == wxID_OK) {
    gbtAnalysisOutput *command = dialog.GetCommand();

    gbtNashMonitorDialog dialog(this, m_doc, command);

    dialog.ShowModal();

    if (!m_splitter->IsSplit()) {
      if (m_efgPanel && m_efgPanel->IsShown()) {
	m_analysisPanel->ShowMixed(false);
    	m_splitter->SplitHorizontally(m_efgPanel, m_analysisPanel);
      }
      else {
	m_analysisPanel->ShowMixed(true);
	m_splitter->SplitHorizontally(m_nfgPanel, m_analysisPanel);
      }
    }
  }
}

extern void LogitStrategic(wxWindow *, gbtGameDocument *);

void gbtGameFrame::OnToolsQre(wxCommandEvent &)
{
  if (m_efgPanel && m_splitter->GetWindow1() == m_efgPanel) {
    gbtLogitBehavDialog(this, m_doc).ShowModal();
  }
  else {
    LogitStrategic(this, m_doc);
  }
}

//----------------------------------------------------------------------
//                 gbtGameFrame: Menu handlers - Help menu
//----------------------------------------------------------------------

void gbtGameFrame::OnHelpAbout(wxCommandEvent &)
{
  gbtAboutDialog(this).ShowModal();
}

//----------------------------------------------------------------------
//                  gbtGameFrame: Non-menu event handlers
//----------------------------------------------------------------------

void gbtGameFrame::OnUnsplit(wxSplitterEvent &)
{
  GetMenuBar()->Check(GBT_MENU_VIEW_PROFILES, false);
  GetToolBar()->ToggleTool(GBT_MENU_VIEW_PROFILES, false);
}

void gbtGameFrame::OnCloseWindow(wxCloseEvent &p_event)
{
  if (p_event.CanVeto() && m_doc->IsModified()) {
    if (wxMessageBox(wxT("Game has been modified.\n")
		       wxT("Unsaved changes will be lost!\n")
		       wxT("Close anyway?"), 
		     _("Warning"),
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }
  p_event.Skip();
}

bool gbtGameFrame::ProcessEvent(wxEvent &p_event)
{
  if (p_event.GetEventType() == wxEVT_COMMAND_MENU_SELECTED) {
    m_doc->PostPendingChanges();
  }
  return wxFrame::ProcessEvent(p_event);
}
