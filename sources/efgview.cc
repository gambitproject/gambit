//
// FILE: efgview.cc -- Extensive form frame class
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

#include "efg.h"
#include "behavsol.h"

#include "guiapp.h"
#include "efgview.h"
#include "efgframe.h"
#include "efgtree.h"
#include "efginfopanel.h"
#include "efgsolutions.h"

#include "dialogenummixed.h"

#include "enumsub.h"

const int EFG_SUPPORTS_UNDOMINATED = 2150;
const int EFG_SUPPORTS_NEW = 2151;
const int EFG_SUPPORTS_EDIT = 2152;
const int EFG_SUPPORTS_DELETE = 2153;
const int EFG_SUPPORTS_SELECT = 2154;
const int EFG_SOLVE_STANDARD = 2400;
const int EFG_SOLVE_CUSTOM = 2401;
const int EFG_SOLVE_CUSTOM_NFG_ENUMPURE = 2402;
const int EFG_SOLVE_CUSTOM_NFG_ENUMMIXED = 2403;
const int EFG_SOLVE_CUSTOM_NFG_LP = 2404;
const int EFG_SOLVE_CUSTOM_NFG_LCP = 2405;
const int EFG_SOLVE_CUSTOM_NFG_LIAP = 2406;
const int EFG_SOLVE_CUSTOM_NFG_POLENUM = 2407;
const int EFG_SOLVE_CUSTOM_NFG_SIMPDIV = 2408;
const int EFG_SOLVE_CUSTOM_NFG_QRE = 2409;
const int EFG_SOLVE_CUSTOM_NFG_QREGRID = 2410;
const int EFG_VIEW_SOLUTIONS = 2429;
const int EFG_VIEW_NFG = 2430;
const int EFG_EDIT_DELETE = 2500;
const int EFG_EDIT_COPY = 2501;
const int EFG_EDIT_PASTE = 2502;
const int EFG_EDIT_PROPERTIES = 2503;
const int EFG_VIEW_ZOOM_IN = 2600;
const int EFG_VIEW_ZOOM_OUT = 2601;

BEGIN_EVENT_TABLE(guiEfgFrame, gambitGameView)
  EVT_MENU(EFG_SOLVE_CUSTOM_NFG_ENUMMIXED, OnSolveCustomNfgEnumMixed)
  EVT_MENU(EFG_VIEW_SOLUTIONS, OnViewSolutions)
END_EVENT_TABLE()

guiEfgView::guiEfgView(guiEfgFrame *p_parent, Efg *p_efg,
		       wxSplitterWindow *p_solutionSplitter,
		       wxSplitterWindow *p_infoSplitter)
  : m_parent(p_parent), m_efg(p_efg) 
{
  m_tree = new guiEfgTree(this, p_solutionSplitter, *p_efg);
  m_efgInfoPanel = new guiEfgInfoPanel(this, p_infoSplitter, *p_efg);

  p_infoSplitter->SplitVertically(m_efgInfoPanel,
				  p_solutionSplitter, 300);
  p_solutionSplitter->Initialize(m_tree);
  m_tree->Show(TRUE);
}

void guiEfgView::OnDelete(void)
{
}

void guiEfgView::OnProperties(void)
{
}

guiEfgFrame::guiEfgFrame(wxMDIParentFrame *p_parent, Efg *p_efg,
			 const wxPoint &p_position, const wxSize &p_size)
  : gambitGameView(p_parent, p_efg, p_position, p_size)
{
  m_efgView = new guiEfgView(this, p_efg, m_solutionSplitter, m_infoSplitter);
  m_solutionView = new guiEfgSolutions(this, m_solutionSplitter, *p_efg);
  m_solutionView->Show(FALSE);

  wxMenu *fileMenu = new wxMenu;
  wxMenu *fileNewMenu = new wxMenu;
  fileNewMenu->Append(GAMBIT_NEW_EFG, "&Extensive form");
  fileNewMenu->Append(GAMBIT_NEW_NFG, "&Normal form");
  fileMenu->Append(GAMBIT_NEW, "&New", fileNewMenu);
  wxMenu *fileOpenMenu = new wxMenu;
  fileOpenMenu->Append(GAMBIT_OPEN_EFG, "&Extensive form");
  fileOpenMenu->Append(GAMBIT_OPEN_NFG, "&Normal form");
  fileMenu->Append(GAMBIT_OPEN, "&Open", fileOpenMenu);
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
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_ENUMPURE, "Enum&Pure");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_ENUMMIXED, "Enum&Mixed");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_LP, "&Lp");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_LCP, "L&cp");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_LIAP, "L&iap");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_POLENUM, "Pol&Enum");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_SIMPDIV, "&Simpdiv");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_QRE, "&Qre");
  solveCustomMenu->Append(EFG_SOLVE_CUSTOM_NFG_QREGRID, "Qre&Grid");
  solveMenu->Append(EFG_SOLVE_CUSTOM, "Custom", solveCustomMenu);
  solveMenu->Append(EFG_VIEW_NFG, "&Nfg", "", TRUE);

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(EFG_VIEW_SOLUTIONS, "&Solutions", "", TRUE);
  viewMenu->AppendSeparator();
  viewMenu->Append(EFG_VIEW_ZOOM_IN, "Zoom &In");
  viewMenu->Append(EFG_VIEW_ZOOM_OUT, "Zoom &Out");

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

void guiEfgFrame::OnEditDeleteEfg(wxCommandEvent &)
{
  m_efgView->OnDelete();
}

void guiEfgFrame::OnEditCopyEfg(wxCommandEvent &)
{
  /*
  if (m_tree->SelectedNode()) {
    m_copyNode = m_tree->SelectedNode();
    m_copyOutcome = 0;
  }
  else if (m_tree->SelectedOutcome()) {
    m_copyNode = 0;
    m_copyOutcome = m_tree->SelectedOutcome();
  }
  */
}

void guiEfgFrame::OnEditPasteEfg(wxCommandEvent &)
{
  /*
  if (m_copyNode) {
    Node *currentNode = m_tree->SelectedNode();

    if (!currentNode) {
      return;
    }
    
    if (currentNode->NumChildren() > 0) {
      return;
    }
    
    GetEfg()->CopyTree(m_copyNode, currentNode);
    m_tree->OnTreeChanged();
  }
  else if (m_copyOutcome) {
    Node *currentNode = m_tree->SelectedNode();

    if (!currentNode) {
      return;
    }

    currentNode->SetOutcome(m_copyOutcome);
    m_tree->OnTreeChanged();
  }
  */
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
    m_solutionSplitter->SplitHorizontally(m_efgView->TreeWindow(),
					  m_solutionView);
    m_solutionView->Show(TRUE);
    GetMenuBar()->Check(EFG_VIEW_SOLUTIONS, TRUE);
  }
}

void guiEfgFrame::OnViewZoomIn(wxCommandEvent &)
{
  //  m_tree->SetZoom(m_tree->GetZoom() / 0.75);
}

void guiEfgFrame::OnViewZoomOut(wxCommandEvent &)
{
  //  m_tree->SetZoom(m_tree->GetZoom() * 0.75);
}

void guiEfgFrame::OnSolveEfgStandard(wxCommandEvent &)
{
  /*
  dialogEfgStandard dialog(this, *GetEfg());
  if (dialog.ShowModal() == wxID_OK) {

  }

  */
}

void guiEfgFrame::OnSolveEfgCustom(wxCommandEvent &)
{

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
  /*
  m_grid = new guiNfgGrid(this, m_solutionSplitter, *GetNfg());
  m_infoPanel = new guiNfgInfoPanel(this, m_infoSplitter, *GetNfg());
  
  m_efgInfoPanel->Show(FALSE);
  m_tree->Show(FALSE);
  m_infoSplitter->ReplaceWindow(m_efgInfoPanel, m_infoPanel);
  m_solutionSplitter->ReplaceWindow(m_tree, m_grid);

  m_grid->Show(TRUE);
  m_infoPanel->Show(TRUE);
  m_grid->SetEditable(FALSE);
  m_grid->Refresh();
  */
}

