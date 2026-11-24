//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#include <optional>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/filename.h>
#include <wx/fontdlg.h>
#include <wx/printdlg.h>
#if !defined(__WXMSW__) || wxUSE_POSTSCRIPT
#include <wx/dcps.h>
#endif // !defined(__WXMSW__) || wxUSE_POSTSCRIPT
#include <wx/splitter.h>
#include <wx/spinctrl.h>
#include <wx/artprov.h>

#include "games.h"
#include "games/gametree.h"

#include "app.h" // for wxGetApp()
#include "gameframe.h"

#include "menuconst.h"

#include "efgpanel.h"
#include "efgprofile.h"

#include "nfgpanel.h"
#include "nfgprofile.h"

#include "dialogstyle.h"
#include "dlcatalog.h"
#include "dlgameprop.h"
#include "dlnash.h"
#include "dllogit.h"
#include "dlabout.h"

#include "dlefglayout.h"
#include "dlefglegend.h"
#include "dlnewtable.h"

namespace Gambit::GUI {

//=====================================================================
//                    class ProfileListPanel
//=====================================================================

class ProfileListPanel final : public wxPanel, public GameView {
  wxWindow *m_behavProfiles, *m_mixedProfiles;

  void OnUpdate() override {}

public:
  ProfileListPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc);

  void ShowMixed(bool p_show);
};

ProfileListPanel::ProfileListPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
  : wxPanel(p_parent, wxID_ANY), GameView(p_doc)
{
  auto *topSizer = new wxBoxSizer(wxHORIZONTAL);

  if (As<GameTreeRep>(p_doc->GetGame())) {
    m_behavProfiles = new MixedBehaviorProfileList(this, p_doc);
    m_behavProfiles->Show(false);
    topSizer->Add(m_behavProfiles, 1, wxEXPAND, 0);
  }
  else {
    m_behavProfiles = nullptr;
  }

  m_mixedProfiles = new MixedStrategyProfileList(this, p_doc);
  m_mixedProfiles->Show(false);
  topSizer->Add(m_mixedProfiles, 1, wxEXPAND, 0);

  SetSizer(topSizer);
  wxWindowBase::Layout();
}

void ProfileListPanel::ShowMixed(bool p_show)
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
//                    class AnalysisNotebook
//=====================================================================

class AnalysisNotebook final : public wxPanel, public GameView {
  ProfileListPanel *m_profiles;
  wxChoice *m_choices;
  wxStaticText *m_description;
  wxStaticText *m_decimalsLabel;
  wxSpinCtrl *m_decimals;

  void OnChoice(wxCommandEvent &);
  void OnDecimals(wxSpinEvent &);
  void OnUpdate() override;

public:
  AnalysisNotebook(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc);

  void ShowMixed(bool p_show);
};

AnalysisNotebook::AnalysisNotebook(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
  : wxPanel(p_parent, wxID_ANY), GameView(p_doc), m_profiles(new ProfileListPanel(this, p_doc))
{
  m_choices = new wxChoice(this, wxID_ANY);
  m_choices->Append(_("Profiles"));
  m_choices->SetSelection(0);

  Connect(m_choices->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
          wxCommandEventHandler(AnalysisNotebook::OnChoice));

  m_description = new wxStaticText(this, wxID_STATIC, wxT(""));

  m_decimalsLabel = new wxStaticText(this, wxID_STATIC, _("Decimals"));
  m_decimals = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                              wxSP_ARROW_KEYS, 1, 15, p_doc->GetStyle().NumDecimals());
  m_decimals->SetToolTip(_("Number of decimal places to display for floating-point profiles"));

  Connect(m_decimals->GetId(), wxEVT_SPINCTRL, wxSpinEventHandler(AnalysisNotebook::OnDecimals));

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *horizSizer = new wxBoxSizer(wxHORIZONTAL);
  horizSizer->Add(m_choices, 0, wxALL | wxALIGN_CENTER, 5);
  horizSizer->Add(m_description, 1, wxALL | wxALIGN_CENTER, 5);
  horizSizer->Add(m_decimalsLabel, 0, wxALL | wxALIGN_CENTER, 5);
  horizSizer->Add(m_decimals, 0, wxALL | wxALIGN_CENTER, 5);
  topSizer->Add(horizSizer, 0, wxEXPAND, 0);

  topSizer->Add(m_profiles, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  wxWindowBase::Layout();
}

void AnalysisNotebook::ShowMixed(bool p_show) { m_profiles->ShowMixed(p_show); }

void AnalysisNotebook::OnChoice(wxCommandEvent &p_event)
{
  m_doc->DoSelectEquilibriumOutput(p_event.GetSelection() + 1);
}

void AnalysisNotebook::OnDecimals(wxSpinEvent &p_event)
{
  TreeStyle style = m_doc->GetStyle();
  style.SetNumDecimals(p_event.GetPosition());
  m_doc->Apply(SetTreeStyle{.style = style});
}

void AnalysisNotebook::OnUpdate()
{
  m_choices->Clear();
  for (int i = 1; i <= m_doc->GetWorkspace().NumProfileLists(); i++) {
    wxString label;
    label << _("Profiles ") << i;
    m_choices->Append(label);
  }
  m_choices->SetSelection(m_doc->GetWorkspace().GetCurrentProfileList() - 1);

  const bool haveProfiles = m_doc->GetWorkspace().GetCurrentProfileList() > 0;
  if (haveProfiles) {
    m_description->SetLabel(
        wxString::FromUTF8(m_doc->GetWorkspace().GetProfiles().GetDescription()));
  }

  const bool showDecimals = haveProfiles && m_doc->GetWorkspace().GetProfiles().IsFloatingPoint();
  m_decimalsLabel->Show(showDecimals);
  m_decimals->Show(showDecimals);
  if (showDecimals) {
    m_decimals->SetValue(m_doc->GetStyle().NumDecimals());
  }
}

//=====================================================================
//                 Implementation of class GameFrame
//=====================================================================

BEGIN_EVENT_TABLE(GameFrame, wxFrame)
EVT_MENU(GBT_MENU_FILE_NEW_EFG, GameFrame::OnFileNewEfg)
EVT_MENU(GBT_MENU_FILE_NEW_NFG, GameFrame::OnFileNewNfg)
EVT_MENU(wxID_OPEN, GameFrame::OnFileOpen)
EVT_MENU(GBT_MENU_FILE_OPEN_CATALOG, GameFrame::OnFileOpenCatalog)
EVT_MENU(wxID_CLOSE, GameFrame::OnFileClose)
EVT_MENU(wxID_SAVE, GameFrame::OnFileSave)
EVT_MENU(wxID_SAVEAS, GameFrame::OnFileSave)
EVT_MENU(GBT_MENU_FILE_EXPORT_BMP, GameFrame::OnFileExportGraphic)
EVT_MENU(GBT_MENU_FILE_EXPORT_JPEG, GameFrame::OnFileExportGraphic)
EVT_MENU(GBT_MENU_FILE_EXPORT_PNG, GameFrame::OnFileExportGraphic)
EVT_MENU(GBT_MENU_FILE_EXPORT_POSTSCRIPT, GameFrame::OnFileExportPS)
EVT_MENU(GBT_MENU_FILE_EXPORT_SVG, GameFrame::OnFileExportSVG)
EVT_MENU(wxID_PRINT_SETUP, GameFrame::OnFilePageSetup)
EVT_MENU(wxID_PREVIEW, GameFrame::OnFilePrintPreview)
EVT_MENU(wxID_PRINT, GameFrame::OnFilePrint)
EVT_MENU(wxID_EXIT, GameFrame::OnFileExit)
EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, GameFrame::OnFileMRUFile)
EVT_MENU(wxID_UNDO, GameFrame::OnEditUndo)
EVT_MENU(wxID_REDO, GameFrame::OnEditRedo)
EVT_MENU(GBT_MENU_EDIT_GAME, GameFrame::OnEditGame)
EVT_MENU(GBT_MENU_VIEW_PROFILES, GameFrame::OnViewProfiles)
EVT_CHAR_HOOK(GameFrame::OnCharHook)
EVT_MENU(GBT_MENU_VIEW_ZOOMIN, GameFrame::OnViewZoom)
EVT_MENU(GBT_MENU_VIEW_ZOOMOUT, GameFrame::OnViewZoom)
EVT_MENU(GBT_MENU_VIEW_ZOOMFIT, GameFrame::OnViewZoom)
EVT_MENU(GBT_MENU_VIEW_ZOOM100, GameFrame::OnViewZoom)
EVT_MENU(GBT_MENU_VIEW_STRATEGIC, GameFrame::OnViewStrategic)
EVT_MENU(GBT_MENU_FORMAT_FONTS, GameFrame::OnFormatFonts)
EVT_MENU(GBT_MENU_FORMAT_LAYOUT, GameFrame::OnFormatLayout)
EVT_MENU(GBT_MENU_FORMAT_LABELS, GameFrame::OnFormatLabels)
EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, GameFrame::OnToolsDominance)
EVT_MENU(GBT_MENU_TOOLS_EQUILIBRIUM, GameFrame::OnToolsEquilibrium)
EVT_MENU(GBT_MENU_TOOLS_QRE, GameFrame::OnToolsQre)
EVT_MENU(wxID_ABOUT, GameFrame::OnHelpAbout)
EVT_CLOSE(GameFrame::OnCloseWindow)
END_EVENT_TABLE()

//---------------------------------------------------------------------
//               GameFrame: Constructor and destructor
//---------------------------------------------------------------------

GameFrame::GameFrame(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
  : wxFrame(p_parent, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize), GameView(p_doc)
{
  const wxRect area = wxGetClientDisplayRect();
  SetClientSize(area.GetWidth() * 4 / 5, area.GetHeight() * 4 / 5);
  Centre();

#if defined(__WXMSW__)
  SetIcon(wxIcon(wxT("gambit_icn")));
#else
#include "bitmaps/gambit.xpm"
  SetIcon(wxIcon(gambit_xpm));
#endif

  wxFrameBase::CreateStatusBar();
  MakeMenus();
  MakeToolbar();

  wxAcceleratorEntry entries[12];
  entries[0].Set(wxACCEL_CTRL, 'o', wxID_OPEN);
  entries[1].Set(wxACCEL_CTRL, 's', wxID_SAVE);
  entries[2].Set(wxACCEL_CTRL | wxACCEL_SHIFT, 's', wxID_SAVEAS);
  entries[3].Set(wxACCEL_CTRL, 'p', wxID_PRINT);
  entries[4].Set(wxACCEL_CTRL, 'w', wxID_CLOSE);
  entries[5].Set(wxACCEL_CTRL, 'q', wxID_EXIT);
  entries[6].Set(wxACCEL_CTRL, '+', GBT_MENU_VIEW_ZOOMIN);
  entries[7].Set(wxACCEL_CTRL, '=', GBT_MENU_VIEW_ZOOMIN);
  entries[8].Set(wxACCEL_CTRL, '-', GBT_MENU_VIEW_ZOOMOUT);
  entries[9].Set(wxACCEL_CTRL, '0', GBT_MENU_VIEW_ZOOM100);
  entries[10].Set(wxACCEL_CTRL, 'z', wxID_UNDO);
  entries[11].Set(wxACCEL_CTRL | wxACCEL_SHIFT, 'z', wxID_REDO);
  const wxAcceleratorTable accel(12, entries);
  wxWindowBase::SetAcceleratorTable(accel);

  m_splitter = new wxSplitterWindow(this, wxID_ANY);
  if (As<GameTreeRep>(p_doc->GetGame())) {
    m_efgPanel = new EfgPanel(m_splitter, p_doc);
    m_efgPanel->Show(true);
    m_splitter->Initialize(m_efgPanel);
    m_nfgPanel = nullptr;
  }
  else {
    m_efgPanel = nullptr;
    m_nfgPanel = new NfgPanel(m_splitter, p_doc, false);
    m_nfgPanel->Show(true);
    m_splitter->Initialize(m_nfgPanel);
  }

  m_analysisPanel = new AnalysisNotebook(m_splitter, p_doc);
  m_analysisPanel->Show(false);

  m_splitter->SetSashGravity(0.5);
  m_splitter->SetMinimumPaneSize(200);

  Connect(m_splitter->GetId(), wxEVT_COMMAND_SPLITTER_UNSPLIT,
          wxSplitterEventHandler(GameFrame::OnUnsplit));

  auto *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_splitter, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  wxTopLevelWindowBase::Layout();

  if (As<GameTreeRep>(p_doc->GetGame())) {
    m_efgPanel->SetFocus();
  }
  else {
    m_nfgPanel->SetFocus();
  }

  OnUpdate();
  wxFrame::Show(true);
}

GameFrame::~GameFrame() { wxGetApp().RemoveMenu(wxFrameBase::GetMenuBar()->GetMenu(0)); }

void GameFrame::OnUpdate()
{
  std::string gameTitle;
  gameTitle = m_doc->GetGame()->GetTitle();

  if (m_doc->IsCatalogGame()) {
    SetTitle(wxT("Gambit - [") + wxString::Format(_("catalog: %s"), m_doc->GetCatalogSlug()) +
             wxT("] ") + wxString::FromUTF8(gameTitle));
  }
  else if (!m_doc->GetFilename().empty()) {
    SetTitle(wxT("Gambit - [") + m_doc->GetFilename() + wxT("] ") + wxString::FromUTF8(gameTitle));
  }
  else {
    SetTitle(wxT("Gambit - ") + wxString::FromUTF8(gameTitle));
  }

  if (m_doc->IsModified()) {
    SetTitle(GetTitle() + _(" (unsaved changes)"));
  }

  wxMenuBar *menuBar = GetMenuBar();

  menuBar->Enable(wxID_UNDO, m_doc->CanUndo());
  menuBar->Enable(wxID_REDO, m_doc->CanRedo());
  GetToolBar()->EnableTool(wxID_UNDO, m_doc->CanUndo());
  GetToolBar()->EnableTool(wxID_REDO, m_doc->CanRedo());

  menuBar->Enable(GBT_MENU_VIEW_PROFILES, m_doc->GetWorkspace().NumProfileLists() > 0);
  GetToolBar()->EnableTool(GBT_MENU_VIEW_PROFILES, m_doc->GetWorkspace().NumProfileLists() > 0);

  if (m_doc->GetWorkspace().NumProfileLists() == 0 && m_splitter->IsSplit()) {
    m_splitter->Unsplit(m_analysisPanel);
  }
  menuBar->Check(GBT_MENU_VIEW_PROFILES, m_splitter->IsSplit());
  GetToolBar()->ToggleTool(GBT_MENU_VIEW_PROFILES, m_splitter->IsSplit());

  const bool treeShown = m_efgPanel && m_efgPanel->IsShown();
  menuBar->Enable(GBT_MENU_VIEW_ZOOMIN, treeShown);
  menuBar->Enable(GBT_MENU_VIEW_ZOOMOUT, treeShown);
  menuBar->Enable(GBT_MENU_VIEW_ZOOMFIT, treeShown);
  menuBar->Enable(GBT_MENU_VIEW_ZOOM100, treeShown);
  // The Format menu sets how the tree is drawn; the strategic view's table is not affected by it
  if (const int formatMenu = menuBar->FindMenu(_("&Format")); formatMenu != wxNOT_FOUND) {
    menuBar->EnableTop(formatMenu, treeShown);
  }
}

//--------------------------------------------------------------------
//          GameFrame: Creating and updating menus and toolbar
//--------------------------------------------------------------------

#include "bitmaps/calc.h"
#include "bitmaps/newtable.h"
#include "bitmaps/newtree.h"
#include "bitmaps/open.h"
#include "bitmaps/profiles.h"
#include "bitmaps/redo.h"
#include "bitmaps/save.h"
#include "bitmaps/table.h"
#include "bitmaps/undo.h"

namespace {
const wxSize TOOLBAR_ICON_SIZE(24, 24);
} // end anonymous namespace

void GameFrame::MakeMenus()
{
  auto *fileMenu = new wxMenu;

  auto *fileNewMenu = new wxMenu;
  fileNewMenu->Append(GBT_MENU_FILE_NEW_EFG, _("&Extensive game"),
                      _("Create a new extensive (tree) game"));
  fileNewMenu->Append(GBT_MENU_FILE_NEW_NFG, _("&Strategic game..."),
                      _("Create a new strategic (table) game"));
  fileMenu->Append(wxID_NEW, _("&New"), fileNewMenu, _("Create a new game"));

  fileMenu->Append(wxID_OPEN, _("&Open...\tCtrl-O"), _("Open a saved game"));
  fileMenu->Append(GBT_MENU_FILE_OPEN_CATALOG, _("Open from &catalog..."),
                   _("Browse and open a game from Gambit's catalog of games"));
  fileMenu->AppendSeparator();

  fileMenu->Append(wxID_SAVE, _("&Save\tCtrl-S"), _("Save this game"));
  fileMenu->Append(wxID_SAVEAS, _("Save &as...\tShift-Ctrl-S"),
                   _("Save game to a different file"));

  fileMenu->AppendSeparator();
  auto *fileExportMenu = new wxMenu;
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_BMP, _("&BMP..."),
                         _("Save a rendering of the game as a Windows bitmap"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_JPEG, _("&JPEG..."),
                         _("Save a rendering of the game as a JPEG image"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_PNG, _("&PNG..."),
                         _("Save a rendering of the game as a PNG image"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_POSTSCRIPT, _("Post&Script..."),
                         _("Save a printout of the game in PostScript format"));
  fileExportMenu->Enable(GBT_MENU_FILE_EXPORT_POSTSCRIPT, wxUSE_POSTSCRIPT);
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_SVG, _("S&VG..."),
                         _("Save a rendering of the game in SVG format"));
  fileMenu->Append(GBT_MENU_FILE_EXPORT, _("&Export"), fileExportMenu,
                   _("Export the game in various formats"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_PRINT_SETUP, _("Page se&tup..."), _("Set up preferences for printing"));
  fileMenu->Append(wxID_PREVIEW, _("Print pre&view"), _("View a preview of the game printout"));
  fileMenu->Append(wxID_PRINT, _("&Print...\tCtrl-P"), _("Print this game"));

  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_CLOSE, _("&Close\tCtrl-W"), _("Close this window"));
  fileMenu->Append(wxID_EXIT, _("E&xit\tCtrl-Q"), _("Exit Gambit"));

  auto *editMenu = new wxMenu;
  editMenu->Append(wxID_UNDO, _("&Undo\tCtrl-Z"), _("Undo the last change"));
  editMenu->Append(wxID_REDO, _("&Redo\tShift-Ctrl-Z"), _("Redo the last undone change"));
  editMenu->AppendSeparator();
  editMenu->Append(GBT_MENU_EDIT_GAME, _("&Game..."), _("Edit properties of the game"));

  auto *viewMenu = new wxMenu;
  viewMenu->Append(GBT_MENU_VIEW_PROFILES, _("&Profiles"), _("Display/hide profiles window"),
                   true);
  viewMenu->Check(GBT_MENU_VIEW_PROFILES, false);
  viewMenu->AppendSeparator();

  viewMenu->Append(GBT_MENU_VIEW_ZOOMIN, _("Zoom &in\tCtrl-+"),
                   _("Increase display magnification"));
  viewMenu->Append(GBT_MENU_VIEW_ZOOMOUT, _("Zoom &out\tCtrl--"),
                   _("Decrease display magnification"));
  viewMenu->Append(GBT_MENU_VIEW_ZOOM100, _("&Actual size\tCtrl-0"),
                   _("Set magnification to 1:1"));
  viewMenu->Append(GBT_MENU_VIEW_ZOOMFIT, _("Zoom to &fit"),
                   _("Rescale to show entire tree in window"));

  viewMenu->AppendSeparator();

  viewMenu->Append(GBT_MENU_VIEW_STRATEGIC, _("&Strategic game"),
                   _("Display the reduced strategic representation of the game"), true);
  if (!As<GameTreeRep>(m_doc->GetGame())) {
    viewMenu->Check(GBT_MENU_VIEW_STRATEGIC, true);
    viewMenu->Enable(GBT_MENU_VIEW_STRATEGIC, false);
  }

  auto *formatMenu = new wxMenu;
  formatMenu->Append(GBT_MENU_FORMAT_LAYOUT, _("&Layout..."), _("Set tree layout parameters"));
  formatMenu->Append(GBT_MENU_FORMAT_LABELS, _("La&bels..."), _("Set labels for parts of trees"));
  formatMenu->Append(GBT_MENU_FORMAT_FONTS, _("&Font..."), _("Set the font for tree labels"));

  auto *toolsMenu = new wxMenu;
  toolsMenu->Append(GBT_MENU_TOOLS_DOMINANCE, _("&Dominance"),
                    _("Show and eliminate dominated strategies"), true);
  if (As<GameTreeRep>(m_doc->GetGame())) {
    toolsMenu->Enable(GBT_MENU_TOOLS_DOMINANCE, false);
  }
  toolsMenu->Append(GBT_MENU_TOOLS_EQUILIBRIUM, _("&Equilibrium..."),
                    _("Compute Nash equilibria and refinements"));

  toolsMenu->Append(GBT_MENU_TOOLS_QRE, _("&QRE..."), _("Compute quantal response equilibria"));

  auto *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, _("&About Gambit"), _("About Gambit"));

  auto *menuBar = new wxMenuBar();
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

void GameFrame::MakeToolbar()
{
  wxToolBar *toolBar = CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT);
  toolBar->SetMargins(2, 2);
  toolBar->SetToolBitmapSize(wxSize(24, 24));

  toolBar->AddTool(GBT_MENU_FILE_NEW_EFG, wxEmptyString,
                   wxBitmapBundle::FromSVG(newtree_svg, TOOLBAR_ICON_SIZE), wxBitmapBundle(),
                   wxITEM_NORMAL, _("Create a new extensive (tree) game"),
                   _("Create a new extensive (tree) game"), nullptr);
  toolBar->AddTool(GBT_MENU_FILE_NEW_NFG, wxEmptyString,
                   wxBitmapBundle::FromSVG(newtable_svg, TOOLBAR_ICON_SIZE), wxBitmapBundle(),
                   wxITEM_NORMAL, _("Create a new strategic (table) game"),
                   _("Create a new strategic (table) game"), nullptr);
  toolBar->AddTool(wxID_OPEN, wxEmptyString, wxBitmapBundle::FromSVG(open_svg, TOOLBAR_ICON_SIZE),
                   wxBitmapBundle(), wxITEM_NORMAL, _("Open a file"), _("Open a file"), nullptr);
  toolBar->AddTool(wxID_SAVE, wxEmptyString, wxBitmapBundle::FromSVG(save_svg, TOOLBAR_ICON_SIZE),
                   wxBitmapBundle(), wxITEM_NORMAL, _("Save this game"), _("Save this game"),
                   nullptr);

  toolBar->AddSeparator();

  toolBar->AddTool(wxID_UNDO, wxEmptyString, wxBitmapBundle::FromSVG(undo_svg, TOOLBAR_ICON_SIZE),
                   wxBitmapBundle(), wxITEM_NORMAL, _("Undo the last change"),
                   _("Undo the last change"), nullptr);
  toolBar->AddTool(wxID_REDO, wxEmptyString, wxBitmapBundle::FromSVG(redo_svg, TOOLBAR_ICON_SIZE),
                   wxBitmapBundle(), wxITEM_NORMAL, _("Redo the last undone change"),
                   _("Redo the last undone change"), nullptr);

  toolBar->AddSeparator();

  if (As<GameTreeRep>(m_doc->GetGame())) {
    toolBar->AddTool(GBT_MENU_VIEW_STRATEGIC, wxEmptyString,
                     wxBitmapBundle::FromSVG(table_svg, TOOLBAR_ICON_SIZE), wxBitmapBundle(),
                     wxITEM_CHECK, _("Display the reduced strategic representation of the game"),
                     _("Display the reduced strategic representation of the game"), nullptr);
  }
  toolBar->AddTool(GBT_MENU_VIEW_PROFILES, wxEmptyString,
                   wxBitmapBundle::FromSVG(profiles_svg, TOOLBAR_ICON_SIZE), wxBitmapBundle(),
                   wxITEM_CHECK, _("View the list of computed strategy profiles"),
                   _("Show or hide the list of computed strategy profiles"), nullptr);
  toolBar->AddTool(GBT_MENU_TOOLS_EQUILIBRIUM, wxEmptyString,
                   wxBitmapBundle::FromSVG(calc_svg, TOOLBAR_ICON_SIZE), wxBitmapBundle(),
                   wxITEM_NORMAL, _("Compute Nash equilibria of this game"),
                   _("Compute Nash equilibria of this game"), nullptr);

  toolBar->Realize();
  toolBar->SetRows(1);
}

//----------------------------------------------------------------------
//               GameFrame: Menu handlers - File menu
//----------------------------------------------------------------------

void GameFrame::OnFileNewEfg(wxCommandEvent &) { new GameFrame(nullptr, NewTreeDocument()); }

void GameFrame::OnFileNewNfg(wxCommandEvent &)
{
  NewTableDialog dialog(this);
  if (dialog.ShowModal() != wxID_OK) {
    return;
  }
  new GameFrame(nullptr, NewTableDocument(dialog.GetDimensions()));
}

void GameFrame::OnFileOpen(wxCommandEvent &) { wxGetApp().ChooseAndLoadFile(this); }

void GameFrame::OnFileOpenCatalog(wxCommandEvent &)
{
  if (const auto choice = ChooseCatalogGame(this)) {
    wxGetApp().LoadCatalogGame(choice->path, choice->slug, choice->version, this);
  }
}

void GameFrame::OnFileClose(wxCommandEvent &) { Close(); }

void GameFrame::OnFileSave(wxCommandEvent &p_event)
{
  SaveDocument(p_event.GetId() == wxID_SAVEAS);
}

bool GameFrame::SaveDocument(bool p_saveAs)
{
  const bool saveAs = p_saveAs || m_doc->GetFilename().empty();

  const bool isExtensiveGame = As<GameTreeRep>(m_doc->GetGame()) != nullptr;

  const auto doSave = [this](const wxString &path, GameDocument::GameSaveFormat format) {
    try {
      m_doc->DoSave(path, format);
      return true;
    }
    catch (const std::exception &ex) {
      ShowOperationError(this, _("Unable to save file"), _("Gambit could not write this file."),
                         path + "\n\n" + wxString::FromUTF8(ex.what()));
      return false;
    }
  };

  if (!saveAs) {
    return doSave(m_doc->GetFilename(), m_doc->GetCurrentSaveFormat());
  }

  const auto nativeFormat =
      isExtensiveGame ? GameDocument::GameSaveFormat::Efg : GameDocument::GameSaveFormat::Nfg;

  const wxString nativeExt = isExtensiveGame ? wxT("efg") : wxT("nfg");

  const wxString nativeWildcard = isExtensiveGame ? _("Gambit extensive games (*.efg)|*.efg|")
                                                  : _("Gambit strategic games (*.nfg)|*.nfg|");

  const wxString &currentFilename = m_doc->GetFilename();
  const wxString catalogSlug = m_doc->GetCatalogSlug();
  const wxString proposedName = !catalogSlug.empty()
                                    ? catalogSlug.AfterLast('/') + wxT(".") + nativeExt
                                    : wxFileNameFromPath(currentFilename);

  wxFileDialog dialog(this, _("Save game as"), wxPathOnly(currentFilename), proposedName,
                      nativeWildcard + _("Gambit workspaces (*.gbt)|*.gbt"),
                      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() != wxID_OK) {
    return false;
  }

  wxFileName filename(dialog.GetPath());

  GameDocument::GameSaveFormat format = GameDocument::GameSaveFormat::Workspace;

  if (!filename.HasExt()) {
    switch (dialog.GetFilterIndex()) {
    case 0:
      filename.SetExt(nativeExt);
      format = nativeFormat;
      break;

    case 1:
      filename.SetExt(wxT("gbt"));
      format = GameDocument::GameSaveFormat::Workspace;
      break;

    default:
      break;
    }
  }
  else {
    const wxString ext = filename.GetExt().Lower();

    if (ext == wxT("gbt")) {
      format = GameDocument::GameSaveFormat::Workspace;
    }
    else if (ext == nativeExt) {
      format = nativeFormat;
    }
    else {
      ShowOperationError(this, _("Unsupported file type"),
                         _("This game cannot be saved with that file extension."),
                         wxString::Format(_("Use .%s or .gbt."), nativeExt));
      return false;
    }
  }

  return doSave(filename.GetFullPath(), format);
}

void GameFrame::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  m_printData.SetOrientation(wxLANDSCAPE);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupDialogData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupDialogData();
  }
}

void GameFrame::OnFilePrintPreview(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);

  wxPrintPreview *preview = nullptr;
  if (m_efgPanel && m_splitter->GetWindow1() == m_efgPanel) {
    preview = new wxPrintPreview(m_efgPanel->GetPrintout(), m_efgPanel->GetPrintout(), &data);
  }
  else {
    preview = new wxPrintPreview(m_nfgPanel->GetPrintout(), m_nfgPanel->GetPrintout(), &data);
  }

  if (!preview->Ok()) {
    delete preview;
    return;
  }

  auto *frame =
      new wxPreviewFrame(preview, this, _("Print preview"), wxPoint(100, 100), wxSize(600, 650));
  frame->Initialize();
  frame->Show(true);
}

void GameFrame::OnFilePrint(wxCommandEvent &)
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
      ShowOperationError(this, _("Unable to print"), _("The game could not be printed."),
                         _("The printer reported an error."));
    }
    // Otherwise, user hit "cancel"; just be quiet and return.
    return;
  }
  else {
    m_printData = printer.GetPrintDialogData().GetPrintData();
  }
}

void GameFrame::OnFileExportGraphic(wxCommandEvent &p_event)
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
    ShowOperationError(this, _("Unable to export image"),
                       _("The game is too large to export as an image."),
                       _("Export it as SVG or PostScript instead."));
    return;
  }

  wxString filter = _("|All files (*.*)|*.*");
  switch (p_event.GetId()) {
  case GBT_MENU_FILE_EXPORT_BMP:
    filter = _("Windows bitmap files (*.bmp)|*.bmp") + filter;
    break;
  case GBT_MENU_FILE_EXPORT_JPEG:
    filter = _("JPEG files (*.jpeg)|*.jpeg|JPG files (*.jpg)|*.jpg") + filter;
    break;
  case GBT_MENU_FILE_EXPORT_PNG:
    filter = _("PNG files (*.png)|*.png") + filter;
    break;
  default:
    break;
  }

  wxFileDialog dialog(this, _("Choose output file"), wxGetApp().GetCurrentDir(), _T(""), filter,
                      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxBitmapType code;
    switch (p_event.GetId()) {
    case GBT_MENU_FILE_EXPORT_BMP:
      code = wxBITMAP_TYPE_BMP;
      break;
    case GBT_MENU_FILE_EXPORT_JPEG:
      code = wxBITMAP_TYPE_JPEG;
      break;
    case GBT_MENU_FILE_EXPORT_PNG:
      code = wxBITMAP_TYPE_PNG;
      break;
    default:
      return;
    }

    if (!bitmap.SaveFile(dialog.GetPath(), code)) {
      ShowOperationError(this, _("Unable to export image"), _("Gambit could not write this file."),
                         dialog.GetPath());
    }
  }
}

void GameFrame::OnFileExportPS(wxCommandEvent &)
{
#if wxUSE_POSTSCRIPT
  wxPrintData printData(m_printData);

  wxFileDialog dialog(this, _("Choose output file"), wxGetApp().GetCurrentDir(), _T(""),
                      _("PostScript files (*.ps)|*.ps"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

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
#endif // wxUSE_POSTSCRIPT
}

void GameFrame::OnFileExportSVG(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"), wxGetApp().GetCurrentDir(), _T(""),
                      _("SVG files (*.svg)|*.svg|All files (*.*)|*.*"),
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

void GameFrame::OnFileExit(wxCommandEvent &)
{
  while (auto *topWindow = wxGetApp().GetTopWindow()) {
    topWindow->Raise();
    const auto before = wxGetApp().GetTopWindow();
    topWindow->Close();

    // The close was vetoed, or otherwise did not destroy/advance the top window.
    // In that case, abort application exit.
    if (wxGetApp().GetTopWindow() == before) {
      return;
    }
  }
}

void GameFrame::OnFileMRUFile(wxCommandEvent &p_event)
{
  const wxString filename = wxGetApp().GetHistoryFile(p_event.GetId() - wxID_FILE1);
  wxGetApp().LoadFile(filename, this);
}

//----------------------------------------------------------------------
//                GameFrame: Menu handlers - Edit menu
//----------------------------------------------------------------------

void GameFrame::OnEditUndo(wxCommandEvent &) { m_doc->Undo(); }

void GameFrame::OnEditRedo(wxCommandEvent &) { m_doc->Redo(); }

void GameFrame::OnEditGame(wxCommandEvent &)
{
  GamePropertiesDialog dialog(this, m_doc);
  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_doc->Apply(dialog.GetEdit());
    }
    catch (std::exception &ex) {
      ShowInternalError(this, ex.what());
    }
  }
}

//----------------------------------------------------------------------
//                GameFrame: Menu handlers - View menu
//----------------------------------------------------------------------

void GameFrame::OnViewProfiles(wxCommandEvent &p_event)
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

void GameFrame::OnViewZoom(wxCommandEvent &p_event)
{
  // All zoom events get passed along to the panel
  if (m_efgPanel && m_efgPanel->IsShown()) {
    wxPostEvent(m_efgPanel, p_event);
  }
}

void GameFrame::OnCharHook(wxKeyEvent &p_event)
{
  // Windows does not reliably deliver Ctrl+'+', Ctrl+'-' or Ctrl+'0' (main row or numpad)
  // through accelerator tables, so the zoom shortcuts are matched on the key itself.
  if (p_event.ControlDown() && !p_event.AltDown()) {
    int id = 0;
    switch (p_event.GetKeyCode()) {
    case '+':
    case '=':
    case WXK_ADD:
    case WXK_NUMPAD_ADD:
      id = GBT_MENU_VIEW_ZOOMIN;
      break;
    case '-':
    case WXK_SUBTRACT:
    case WXK_NUMPAD_SUBTRACT:
      id = GBT_MENU_VIEW_ZOOMOUT;
      break;
    case '0':
    case WXK_NUMPAD0:
      id = GBT_MENU_VIEW_ZOOM100;
      break;
    default:
      break;
    }
    if (id != 0) {
      wxCommandEvent event(wxEVT_MENU, id);
      OnViewZoom(event);
      return;
    }
  }
  p_event.Skip();
}

void GameFrame::OnViewStrategic(wxCommandEvent &p_event)
{
  if (m_efgPanel->IsShown()) {
    // We are switching to strategic view
    if (!m_doc->GetGame()->IsPerfectRecall()) {
      ShowOperationError(this, _("Unable to show strategic game"),
                         _("This game does not have perfect recall."),
                         _("The reduced strategic form can be computed only for games of "
                           "perfect recall."));
      return;
    }

    if (const size_t contingencies = m_doc->GetGame()->GetStrategies().extent_product();
        !m_nfgPanel && contingencies >= 50000) {
      if (!ConfirmOperation(
              this, _("Large strategic game"),
              wxString::Format(_("This game has %zu contingencies in strategic form."),
                               contingencies),
              _("Browsing the strategic form will be slow, and Gambit may stop responding."),
              _("Show anyway"))) {
        return;
      }
    }

    if (!m_nfgPanel) {
      m_nfgPanel =
          new NfgPanel(m_splitter, m_doc, GetMenuBar()->IsChecked(GBT_MENU_TOOLS_DOMINANCE));
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
  GetMenuBar()->Enable(GBT_MENU_TOOLS_DOMINANCE, m_nfgPanel->IsShown());

  GetToolBar()->ToggleTool(GBT_MENU_VIEW_STRATEGIC, p_event.IsChecked());

  OnUpdate();
}

//----------------------------------------------------------------------
//               GameFrame: Menu handlers - Format menu
//----------------------------------------------------------------------

void GameFrame::OnFormatLayout(wxCommandEvent &)
{
  TreeStyle style = m_doc->GetStyle();
  LayoutDialog dialog(this, style);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.GetSettings(style);
    m_doc->Apply(SetTreeStyle{.style = style});
  }
}

void GameFrame::OnFormatLabels(wxCommandEvent &)
{
  LegendDialog dialog(this, m_doc->GetStyle());

  if (dialog.ShowModal() == wxID_OK) {
    TreeStyle style = m_doc->GetStyle();
    style.SetNodeAboveLabel(dialog.GetNodeAbove());
    style.SetNodeBelowLabel(dialog.GetNodeBelow());
    style.SetBranchAboveLabel(dialog.GetBranchAbove());
    style.SetBranchBelowLabel(dialog.GetBranchBelow());
    m_doc->Apply(SetTreeStyle{.style = style});
  }
}

void GameFrame::OnFormatFonts(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_doc->GetLabelFont());
  wxFontDialog dialog(this, data);

  if (dialog.ShowModal() == wxID_OK) {
    TreeStyle style = m_doc->GetStyle();
    style.SetFont(ToTreeFont(dialog.GetFontData().GetChosenFont()));
    m_doc->Apply(SetTreeStyle{.style = style});
  }
}

//----------------------------------------------------------------------
//               GameFrame: Menu handlers - Tools menu
//----------------------------------------------------------------------

void GameFrame::OnToolsDominance(wxCommandEvent &p_event)
{
  if (m_nfgPanel) {
    wxPostEvent(m_nfgPanel, p_event);
  }
  if (!p_event.IsChecked()) {
    m_doc->DoTopDominanceLevel();
  }
}

extern void ShowNashMonitorDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                                  const std::shared_ptr<AnalysisOutput> &p_command);

void GameFrame::OnToolsEquilibrium(wxCommandEvent &)
{
  if (!m_doc->GetGame()->IsPerfectRecall()) {
    ShowOperationError(this, _("Unable to compute equilibria"),
                       _("This game does not have perfect recall."),
                       _("Nash equilibria can be computed only for games of perfect recall."));
    return;
  }

  NashChoiceDialog dialog(this, m_doc);

  if (dialog.ShowModal() == wxID_OK) {
    if (dialog.UseStrategic()) {
      if (const size_t contingencies = m_doc->GetGame()->GetStrategies().extent_product();
          contingencies >= 50000) {
        if (!ConfirmOperation(
                this, _("Large strategic game"),
                wxString::Format(_("This game has %zu contingencies in strategic form."),
                                 contingencies),
                _("Solving the strategic form will be slow, and Gambit may stop responding."),
                _("Compute anyway"))) {
          return;
        }
      }
    }

    m_doc->SetNashChoice(dialog.GetChoice());
    ShowNashMonitorDialog(this, m_doc, dialog.CreateOutput());

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

void GameFrame::OnToolsQre(wxCommandEvent &)
{
  if (!m_doc->GetGame()->IsPerfectRecall()) {
    ShowOperationError(this, _("Unable to compute quantal response equilibria"),
                       _("This game does not have perfect recall."),
                       _("Quantal response equilibria can be computed only for games of "
                         "perfect recall."));
    return;
  }

  if (m_efgPanel && m_splitter->GetWindow1() == m_efgPanel) {
    LogitBehavDialog(this, m_doc).ShowModal();
  }
  else {
    LogitStrategic(this, m_doc);
  }
}

//----------------------------------------------------------------------
//                 GameFrame: Menu handlers - Help menu
//----------------------------------------------------------------------

void GameFrame::OnHelpAbout(wxCommandEvent &) { AboutDialog(this).ShowModal(); }

//----------------------------------------------------------------------
//                  GameFrame: Non-menu event handlers
//----------------------------------------------------------------------

void GameFrame::OnUnsplit(wxSplitterEvent &)
{
  GetMenuBar()->Check(GBT_MENU_VIEW_PROFILES, false);
  GetToolBar()->ToggleTool(GBT_MENU_VIEW_PROFILES, false);
}

namespace {

struct CloseWarningText {
  wxString primary;
  wxString secondary;
};

CloseWarningText CloseWarningMessage(const std::shared_ptr<GameDocument> &p_doc)
{
  const bool gameModified = p_doc->IsGameModified();
  const bool workspaceModified = p_doc->IsWorkspaceModified();

  if (gameModified && !workspaceModified) {
    return {.primary = _("Do you want to save the changes to this game?"),
            .secondary = _("Your changes will be lost if you don't save them.")};
  }

  if (!gameModified && workspaceModified) {
    return {.primary = _("Do you want to save the computed results and drawing settings?"),
            .secondary = _("These are kept only in a Gambit workspace file (.gbt). They will be "
                           "lost if you don't save them.")};
  }

  if (gameModified && workspaceModified) {
    return {.primary = _("Do you want to save the changes to this game and its workspace?"),
            .secondary = _("Computed results and drawing settings are kept only in a Gambit "
                           "workspace file (.gbt). Your changes will be lost if you don't save "
                           "them.")};
  }

  return {.primary = wxEmptyString, .secondary = wxEmptyString};
}

} // namespace

void GameFrame::OnCloseWindow(wxCloseEvent &p_event)
{
  if (!p_event.CanVeto()) {
    p_event.Skip();
    return;
  }

  if (m_doc->IsModified()) {
    const auto warning = CloseWarningMessage(m_doc);

    AlertDialog dialog(this, _("Unsaved changes"), wxART_WARNING, warning.primary,
                       warning.secondary, wxYES | wxNO | wxCANCEL);
    dialog.SetButtonLabel(wxID_YES, _("Save"));
    dialog.SetButtonLabel(wxID_NO, _("Don't save"));
    dialog.SetDefaultButton(wxID_YES);
    const int answer = dialog.ShowModal();
    // Results and drawing settings survive only in a workspace file, so saving them from a game
    // file asks where to save
    const bool needsWorkspace =
        m_doc->IsWorkspaceModified() &&
        m_doc->GetCurrentSaveFormat() != GameDocument::GameSaveFormat::Workspace;
    if (answer == wxID_CANCEL || (answer == wxID_YES && !SaveDocument(needsWorkspace))) {
      p_event.Veto();
      return;
    }
  }

  p_event.Skip();
}

bool GameFrame::ProcessEvent(wxEvent &p_event)
{
  if (p_event.GetEventType() == wxEVT_COMMAND_MENU_SELECTED) {
    m_doc->PostPendingChanges();
  }
  return wxFrame::ProcessEvent(p_event);
}

} // namespace Gambit::GUI
