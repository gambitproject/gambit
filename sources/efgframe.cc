//
// FILE: efgframe.cc -- Extensive form frame class
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "wx/splitter.h"
#include "wx/colordlg.h"

#include "efg.h"
#include "behavsol.h"

#include "guiapp.h"
#include "efgview.h"
#include "nfgview.h"
#include "efgframe.h"
#include "efgtree.h"
#include "efginfopanel.h"
#include "efgsolutions.h"

#include "guistatus.h"
#include "dialogenumpure.h"
#include "dialogenummixed.h"

#include "dialogefgsave.h"

#include "psnesub.h"
#include "enumsub.h"


const int EFG_SUPPORTS_UNDOMINATED = 2150;
const int EFG_SUPPORTS_NEW = 2151;
const int EFG_SUPPORTS_EDIT = 2152;
const int EFG_SUPPORTS_DELETE = 2153;
const int EFG_SUPPORTS_SELECT = 2154;
const int EFG_SOLVE_STANDARD = 2400;
const int EFG_SOLVE_CUSTOM = 2401;
const int EFG_SOLVE_CUSTOM_EFG = 2410;
const int EFG_SOLVE_CUSTOM_EFG_ENUMPURE = 2411;
const int EFG_SOLVE_CUSTOM_EFG_LP = 2412;
const int EFG_SOLVE_CUSTOM_EFG_LCP = 2413;
const int EFG_SOLVE_CUSTOM_EFG_LIAP = 2414;
const int EFG_SOLVE_CUSTOM_EFG_POLENUM = 2415;
const int EFG_SOLVE_CUSTOM_EFG_QRE = 2416;
const int EFG_SOLVE_CUSTOM_NFG = 2420;
const int EFG_SOLVE_CUSTOM_NFG_ENUMPURE = 2421;
const int EFG_SOLVE_CUSTOM_NFG_ENUMMIXED = 2422;
const int EFG_SOLVE_CUSTOM_NFG_LP = 2423;
const int EFG_SOLVE_CUSTOM_NFG_LCP = 2424;
const int EFG_SOLVE_CUSTOM_NFG_LIAP = 2425;
const int EFG_SOLVE_CUSTOM_NFG_POLENUM = 2426;
const int EFG_SOLVE_CUSTOM_NFG_SIMPDIV = 2427;
const int EFG_SOLVE_CUSTOM_NFG_QRE = 2428;
const int EFG_SOLVE_CUSTOM_NFG_QREGRID = 2429;
const int EFG_VIEW_SOLUTIONS = 2429;
const int EFG_VIEW_NFG = 2430;
const int EFG_VIEW_COLOR = 2431;
const int EFG_EDIT_DELETE = 2500;
const int EFG_EDIT_COPY = 2501;
const int EFG_EDIT_PASTE = 2502;
const int EFG_EDIT_PROPERTIES = 2503;
const int EFG_VIEW_ZOOM_IN = 2600;
const int EFG_VIEW_ZOOM_OUT = 2601;
const int EFG_FILE_SAVE = 2700;

BEGIN_EVENT_TABLE(guiEfgFrame, gambitGameView)
  EVT_MENU(EFG_FILE_SAVE, OnFileSave)
  EVT_MENU(EFG_EDIT_COPY, OnEditCopy)
  EVT_MENU(EFG_EDIT_PASTE, OnEditPaste)
  EVT_MENU(EFG_SOLVE_CUSTOM_NFG_ENUMPURE, OnSolveCustomNfgEnumPure)
  EVT_MENU(EFG_SOLVE_CUSTOM_NFG_ENUMMIXED, OnSolveCustomNfgEnumMixed)
  EVT_MENU(EFG_VIEW_SOLUTIONS, OnViewSolutions)
  EVT_MENU(EFG_VIEW_ZOOM_IN, OnViewZoomIn)
  EVT_MENU(EFG_VIEW_ZOOM_OUT, OnViewZoomOut)
  EVT_MENU(EFG_VIEW_COLOR, OnViewColor)
  EVT_MENU(EFG_VIEW_NFG, OnSolveEfgNfg)
END_EVENT_TABLE()

guiEfgFrame::guiEfgFrame(wxMDIParentFrame *p_parent, FullEfg *p_efg,
			 const wxPoint &p_position, const wxSize &p_size)
  : gambitGameView(p_parent, p_efg, p_position, p_size),
    m_nfgView(0)
{
  m_efgView = new guiEfgView(this, p_efg, m_solutionSplitter);
  m_solutionView = new guiEfgSolutions(this, m_solutionSplitter, *p_efg);
  m_solutionView->Show(FALSE);

  m_solutionSplitter->Initialize(m_efgView);
  int width, height;
  GetClientSize(&width, &height);
  m_efgView->SetSize(width, height);
  m_efgView->Show(TRUE);

  wxMenu *fileMenu = new wxMenu;
  wxMenu *fileNewMenu = new wxMenu;
  fileNewMenu->Append(GAMBIT_NEW_EFG, "&Extensive form");
  fileNewMenu->Append(GAMBIT_NEW_NFG, "&Normal form");
  fileMenu->Append(GAMBIT_NEW, "&New", fileNewMenu);
  wxMenu *fileOpenMenu = new wxMenu;
  fileOpenMenu->Append(GAMBIT_OPEN_EFG, "&Extensive form");
  fileOpenMenu->Append(GAMBIT_OPEN_NFG, "&Normal form");
  fileMenu->Append(GAMBIT_OPEN, "&Open", fileOpenMenu);
  fileMenu->Append(EFG_FILE_SAVE, "&Save");
  fileMenu->Append(GAMBIT_CLOSE, "&Close");
  fileMenu->AppendSeparator();
  fileMenu->Append(GAMBIT_QUIT, "&Exit");

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(EFG_EDIT_COPY, "&Copy");
  editMenu->Append(EFG_EDIT_PASTE, "&Paste");
  editMenu->Append(EFG_EDIT_DELETE, "&Delete");
  editMenu->AppendSeparator();
  editMenu->Append(EFG_EDIT_PROPERTIES, "P&roperties...");
  editMenu->Append(GAME_EDIT_LABEL, "Game &Label");

  wxMenu *supportsMenu = new wxMenu;
  supportsMenu->Append(EFG_SUPPORTS_UNDOMINATED, "&Undominated");
  supportsMenu->Append(EFG_SUPPORTS_NEW, "&New");
  supportsMenu->Append(EFG_SUPPORTS_EDIT, "&Edit");
  supportsMenu->Append(EFG_SUPPORTS_DELETE, "&Delete");
  supportsMenu->Append(EFG_SUPPORTS_SELECT, "&Select");

  wxMenu *solveMenu = new wxMenu;
  solveMenu->Append(EFG_SOLVE_STANDARD, "&Standard...");
  wxMenu *solveCustomMenu = new wxMenu;
  wxMenu *solveCustomEfgMenu = new wxMenu;
  solveCustomEfgMenu->Append(EFG_SOLVE_CUSTOM_EFG_ENUMPURE, "Enum&Pure");
  solveCustomEfgMenu->Append(EFG_SOLVE_CUSTOM_EFG_LP, "&Lp");
  solveCustomEfgMenu->Append(EFG_SOLVE_CUSTOM_EFG_LCP, "L&cp");
  solveCustomEfgMenu->Append(EFG_SOLVE_CUSTOM_EFG_LIAP, "L&iap");
  solveCustomEfgMenu->Append(EFG_SOLVE_CUSTOM_EFG_POLENUM, "Pol&Enum");
  solveCustomEfgMenu->Append(EFG_SOLVE_CUSTOM_EFG_QRE, "&Qre");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_EFG, "Extensive form",
			  solveCustomEfgMenu);

  wxMenu *solveCustomNfgMenu = new wxMenu;
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_ENUMPURE, "Enum&Pure");
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_ENUMMIXED, "Enum&Mixed");
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_LP, "&Lp");
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_LCP, "L&cp");
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_LIAP, "L&iap");
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_POLENUM, "Pol&Enum");
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_SIMPDIV, "&Simpdiv");
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_QRE, "&Qre");
  solveCustomNfgMenu->Append(EFG_SOLVE_CUSTOM_NFG_QREGRID, "Qre&Grid");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG, "Normal form",
			  solveCustomNfgMenu);
  solveMenu->Append(EFG_SOLVE_CUSTOM, "Custom", solveCustomMenu);
  solveMenu->Append(EFG_VIEW_NFG, "&Nfg", "", TRUE);

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(EFG_VIEW_SOLUTIONS, "&Solutions", "", TRUE);
  viewMenu->AppendSeparator();
  viewMenu->Append(EFG_VIEW_ZOOM_IN, "Zoom &In");
  viewMenu->Append(EFG_VIEW_ZOOM_OUT, "Zoom &Out");
  viewMenu->Append(EFG_VIEW_COLOR, "&Color");

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(GAMBIT_ABOUT, "&About");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(supportsMenu, "Su&pports");
  menuBar->Append(solveMenu, "&Solve");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(helpMenu, "&Help");
  SetMenuBar(menuBar);
}

void guiEfgFrame::OnFileSave(wxCommandEvent &)
{
  dialogEfgSave dialog(this, "", "", 2);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      gFileOutput out(dialog.Filename());
      m_efgView->GetEfg()->WriteEfgFile(out, dialog.NumDecimals());
    }
    catch (...) { }
  }
}

void guiEfgFrame::OnEditDeleteEfg(wxCommandEvent &)
{
  m_efgView->OnDelete();
}

void guiEfgFrame::OnEditCopy(wxCommandEvent &)
{
  m_efgView->OnCopy();
}

void guiEfgFrame::OnEditPaste(wxCommandEvent &)
{
  m_efgView->OnPaste();
}

void guiEfgFrame::OnEditPropertiesEfg(wxCommandEvent &)
{
  /*
  if (m_tree->SelectedOutcome()) {
    dialogEfgOutcome dialog(this, *GetEfg(), m_tree->SelectedOutcome());

    if (dialog.ShowModal() == wxID_OK) {

    }
  }
  */
}

void guiEfgFrame::OnViewSolutions(wxCommandEvent &)
{
  if (m_solutionSplitter->IsSplit()) {
    m_solutionSplitter->Unsplit();
    GetMenuBar()->Check(EFG_VIEW_SOLUTIONS, FALSE);
    m_solutionView->Show(FALSE);
  }
  else {
    m_solutionSplitter->SplitHorizontally(m_solutionSplitter->GetWindow1(),
					  m_solutionView);
    m_solutionView->Show(TRUE);
    GetMenuBar()->Check(EFG_VIEW_SOLUTIONS, TRUE);
  }
}

void guiEfgFrame::OnViewZoomIn(wxCommandEvent &)
{
  m_efgView->SetZoom(m_efgView->GetZoom() / 0.75);
}

void guiEfgFrame::OnViewZoomOut(wxCommandEvent &)
{
  m_efgView->SetZoom(m_efgView->GetZoom() * 0.75);
}

void guiEfgFrame::OnViewColor(wxCommandEvent &)
{
  wxColourDialog dialog(this);

  if (dialog.ShowModal() == wxID_OK) {

  }
}

void guiEfgFrame::OnSolveEfgStandard(wxCommandEvent &)
{
  /*
  dialogEfgStandard dialog(this, *GetEfg());
  if (dialog.ShowModal() == wxID_OK) {

  }

  */
}

void guiEfgFrame::OnSolveCustomNfgEnumPure(wxCommandEvent &)
{
  dialogEnumPure dialog(this, true, true);

  if (dialog.ShowModal() == wxID_OK) {
    guiStatus status(this, "EnumPureSolve Progress");
    efgEnumPureNfgSolve module(EFSupport(*m_efgView->GetEfg()), 0, status);
    gList<BehavSolution> solutions = module.Solve(EFSupport(*m_efgView->GetEfg()));
    m_solutionView->AddSolutions(solutions);
  }

}

void guiEfgFrame::OnSolveCustomNfgEnumMixed(wxCommandEvent &)
{
  dialogEnumMixed dialog(this, true);

  if (dialog.ShowModal() == wxID_OK) {
    EnumParams params;
    EnumBySubgame module(EFSupport(*m_efgView->GetEfg()), params);
    gList<BehavSolution> solutions = module.Solve(EFSupport(*m_efgView->GetEfg()));
    m_solutionView->AddSolutions(solutions);
  }
}

void guiEfgFrame::OnSolveEfgNfg(wxCommandEvent &)
{
  if (!GetMenuBar()->IsChecked(EFG_VIEW_NFG)) {
    m_nfgView->Show(false);
    m_solutionSplitter->ReplaceWindow(m_solutionSplitter->GetWindow1(),
				      m_efgView);
    m_efgView->Show(true);
  }
  else {
    m_efgView->Show(false);
    if (!m_nfgView) {
      Nfg *nfg = MakeReducedNfg(EFSupport(*m_efgView->GetEfg()));
      m_nfgView = new guiNfgView(nfg, m_solutionSplitter);
    }

    m_solutionSplitter->ReplaceWindow(m_solutionSplitter->GetWindow1(),
				      m_nfgView);
    m_nfgView->Show(true);
  }
}

