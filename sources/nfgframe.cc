//
// FILE: nfgframe.cc -- Normal form frame class
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

#include "guiapp.h"
#include "nfgview.h"
#include "nfgframe.h"
#include "nfgsolutions.h"

#include "dialogenumpure.h"
#include "dialogenummixed.h"
#include "dialoglp.h"
#include "dialoglcp.h"
#include "dialogliap.h"

#include "dialogstrategies.h"
#include "dialogelim.h"
#include "dialogsupportselect.h"
#include "dialognfgeditsupport.h"
#include "dialognfgstandard.h"

#include "nfgpure.h"
#include "enum.h"
#include "nfgcsum.h"
#include "lemke.h"
#include "nliap.h"

#include "guistatus.h"

const int NFG_EDIT_PLAYERS = 2001;
const int NFG_SUPPORTS_UNDOMINATED = 2100;
const int NFG_SUPPORTS_NEW = 2101;
const int NFG_SUPPORTS_EDIT = 2102;
const int NFG_SUPPORTS_DELETE = 2103;
const int NFG_SUPPORTS_SELECT = 2104;
const int NFG_SOLVE_STANDARD = 2200;
const int NFG_SOLVE_CUSTOM = 2201;
const int NFG_SOLVE_CUSTOM_ENUMPURE = 2202;
const int NFG_SOLVE_CUSTOM_ENUMMIXED = 2203;
const int NFG_SOLVE_CUSTOM_LP = 2204;
const int NFG_SOLVE_CUSTOM_LCP = 2205;
const int NFG_SOLVE_CUSTOM_LIAP = 2206;
const int NFG_SOLVE_CUSTOM_POLENUM = 2207;
const int NFG_SOLVE_CUSTOM_SIMPDIV = 2208;
const int NFG_SOLVE_CUSTOM_QRE = 2209;
const int NFG_SOLVE_CUSTOM_QREGRID = 2210;
const int NFG_VIEW_SOLUTIONS = 2300;

BEGIN_EVENT_TABLE(guiNfgFrame, gambitGameView)
  EVT_MENU(NFG_EDIT_PLAYERS, gambitGameView::OnEditPlayers)
  EVT_MENU(NFG_SUPPORTS_UNDOMINATED, OnSupportsNfgUndominated)
  EVT_MENU(NFG_SUPPORTS_NEW, OnSupportsNfgNew)
  EVT_MENU(NFG_SUPPORTS_EDIT, OnSupportsNfgEdit)
  EVT_MENU(NFG_SUPPORTS_DELETE, OnSupportsNfgDelete)
  EVT_MENU(NFG_SUPPORTS_SELECT, OnSupportsNfgSelect)
  EVT_MENU(NFG_SOLVE_STANDARD,OnSolveNfgStandard)
  EVT_MENU(NFG_SOLVE_CUSTOM_ENUMPURE, OnSolveNfgCustomEnumPure)
  EVT_MENU(NFG_SOLVE_CUSTOM_ENUMMIXED, OnSolveNfgCustomEnumMixed)
  EVT_MENU(NFG_SOLVE_CUSTOM_LP, OnSolveNfgCustomLp)
  EVT_MENU(NFG_SOLVE_CUSTOM_LCP, OnSolveNfgCustomLcp)
  EVT_MENU(NFG_SOLVE_CUSTOM_LIAP, OnSolveNfgCustomLiap)
  EVT_MENU(NFG_SOLVE_CUSTOM_POLENUM, OnSolveNfgCustomPolEnum)
  EVT_MENU(NFG_SOLVE_CUSTOM_SIMPDIV, OnSolveNfgCustomSimpdiv)
  EVT_MENU(NFG_SOLVE_CUSTOM_QRE, OnSolveNfgCustomQre)
  EVT_MENU(NFG_SOLVE_CUSTOM_QREGRID, OnSolveNfgCustomQreGrid)
  EVT_MENU(NFG_VIEW_SOLUTIONS, OnViewSolutions)
END_EVENT_TABLE()

guiNfgFrame::guiNfgFrame(wxMDIParentFrame *p_parent, Nfg *p_nfg,
		       const wxPoint &p_position, const wxSize &p_size)
  : gambitGameView(p_parent, p_nfg, p_position, p_size),
    m_solutionView(0)
{
  m_nfgView = new guiNfgView(p_nfg, m_solutionSplitter);
  m_solutionView = new guiNfgSolutions(this, m_solutionSplitter, *p_nfg);
  m_solutionView->Show(FALSE);

  m_solutionSplitter->Initialize(m_nfgView);
  int width, height;
  GetClientSize(&width, &height);
  m_nfgView->SetSize(width, height);
  m_nfgView->Show(TRUE);

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
  editMenu->Append(GAME_EDIT_LABEL, "&Label");
  editMenu->Append(NFG_EDIT_PLAYERS, "&Players");

  wxMenu *supportsMenu = new wxMenu;
  supportsMenu->Append(NFG_SUPPORTS_UNDOMINATED, "&Undominated");
  supportsMenu->Append(NFG_SUPPORTS_NEW, "&New");
  supportsMenu->Append(NFG_SUPPORTS_EDIT, "&Edit");
  supportsMenu->Append(NFG_SUPPORTS_DELETE, "&Delete");
  supportsMenu->Append(NFG_SUPPORTS_SELECT, "&Select");

  wxMenu *solveMenu = new wxMenu;
  solveMenu->Append(NFG_SOLVE_STANDARD, "&Standard...");
  wxMenu *solveCustomMenu = new wxMenu;
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_ENUMPURE, "Enum&Pure");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_ENUMMIXED, "Enum&Mixed");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LP, "&Lp");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LCP, "L&cp");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LIAP, "L&iap");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_POLENUM, "Pol&Enum");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_SIMPDIV, "&Simpdiv");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_QRE, "&Qre");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_QREGRID, "Qre&Grid");
  solveMenu->Append(NFG_SOLVE_CUSTOM, "Custom", solveCustomMenu);

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(NFG_VIEW_SOLUTIONS, "&Solutions", "View solutions", TRUE);
  
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

  Show(TRUE);
}

guiNfgFrame::~guiNfgFrame()
{ }

void guiNfgFrame::OnSolveNfgCustomEnumPure(wxCommandEvent &)
{
  dialogEnumPure dialog(this, true, false);
  
  if (dialog.ShowModal() == wxID_OK) {
    gList<MixedSolution> solutions;
    guiStatus status(this, "EnumPureSolve Progress");
    try {
      FindPureNash(*m_nfgView->CurrentSupport(), 0, status, solutions);
    }
    catch (gSignalBreak &) { }
    catch (gException &) { }
    m_solutionView->AddSolutions(solutions);
  }
}

void guiNfgFrame::OnSolveNfgCustomEnumMixed(wxCommandEvent &)
{
  dialogEnumMixed dialog(this, false);

  if (dialog.ShowModal() == wxID_OK) {
    gList<MixedSolution> solutions;
    try {
      guiStatus status(this, "EnumMixedSolve Progress");
      EnumParams params(status);
      params.stopAfter = dialog.StopAfter();
      long npivots;
      double time;

      Enum(*m_nfgView->CurrentSupport(), params, solutions, npivots, time);
    }
    catch (gSignalBreak &) { }
    catch (gException &) { }
    m_solutionView->AddSolutions(solutions);
  }
}

void guiNfgFrame::OnSolveNfgCustomLp(wxCommandEvent &)
{
  dialogLp dialog(this, false, true);

  if (dialog.ShowModal() == wxID_OK) {
    gList<MixedSolution> solutions;
    try {
      guiStatus status(this, "LpSolve Progress");
      ZSumParams params(status);
      params.stopAfter = 1;
      int npivots;
      double time;

      ZSum(*m_nfgView->CurrentSupport(), params, solutions, npivots, time);
    }
    catch (gSignalBreak &) { }
    catch (gException &) { }
    m_solutionView->AddSolutions(solutions);
  }
}

void guiNfgFrame::OnSolveNfgCustomLcp(wxCommandEvent &)
{
  dialogLcp dialog(this, false, true);

  if (dialog.ShowModal() == wxID_OK) {
    gList<MixedSolution> solutions;
    try {
      guiStatus status(this, "LcpSolve Progress");
      LemkeParams params(status);
      params.stopAfter = dialog.StopAfter();
      int npivots;
      double time;
    
      Lemke(*m_nfgView->CurrentSupport(), params, solutions, npivots, time);
    }
    catch (gSignalBreak &) { }
    catch (gException &) { }
    m_solutionView->AddSolutions(solutions);
  }
}

void guiNfgFrame::OnSolveNfgCustomLiap(wxCommandEvent &)
{
  dialogLiap dialog(this, false, true);

  if (dialog.ShowModal() == wxID_OK) {
    gList<MixedSolution> solutions;
    try {
      guiStatus status(this, "LiapSolve Progress");
      NFLiapParams params(status);
      params.stopAfter = dialog.StopAfter();
      long nevals, niters;
      Liap(*m_nfgView->GetNfg(), params, 
	   MixedProfile<gNumber>(*m_nfgView->CurrentSupport()),
	   solutions, nevals, niters);
    }
    catch (gSignalBreak &) { }
    catch (gException &) { }
    m_solutionView->AddSolutions(solutions);
  }
}

void guiNfgFrame::OnSolveNfgCustomPolEnum(wxCommandEvent &)
{

}

void guiNfgFrame::OnSolveNfgCustomSimpdiv(wxCommandEvent &)
{

}

void guiNfgFrame::OnSolveNfgCustomQre(wxCommandEvent &)
{

}

void guiNfgFrame::OnSolveNfgCustomQreGrid(wxCommandEvent &)
{

}

void guiNfgFrame::OnViewSolutions(wxCommandEvent &)
{
  if (m_solutionSplitter->IsSplit()) {
    m_solutionSplitter->Unsplit();
    GetMenuBar()->Check(NFG_VIEW_SOLUTIONS, FALSE);
    m_solutionView->Show(FALSE);
    m_nfgView->SetShowProbs(false);
  }
  else {
    m_solutionSplitter->SplitHorizontally(m_nfgView, m_solutionView);
    m_solutionView->Show(TRUE);
    GetMenuBar()->Check(NFG_VIEW_SOLUTIONS, TRUE);
    m_nfgView->SetShowProbs(true);
  }
}

void guiNfgFrame::SetSolution(const MixedSolution &p_solution)
{
  m_nfgView->SetSolution(p_solution);
}

void guiNfgFrame::OnSupportsNfgUndominated(wxCommandEvent &)
{
  gArray<gText> players(m_nfgView->GetNfg()->NumPlayers());
  for (int pl = 1; pl <= m_nfgView->GetNfg()->NumPlayers(); pl++) {
    players[pl] = m_nfgView->GetNfg()->Players()[pl]->GetName();
  }
  dialogElim dialog(this, players, true);

  if (dialog.ShowModal() == wxID_OK) {
    NFSupport *support = m_nfgView->CurrentSupport();
    guiStatus status(this, "Dominance Elimination");

    try {
      if (!dialog.DomMixed()) {
	if (dialog.FindAll()) {
	  while ((support = support->Undominated(dialog.DomStrong(), 
						 dialog.Players(),
						 gnull, gstatus)) != 0) {
	    support->SetName(m_nfgView->UniqueSupportName());
	    m_nfgView->AddSupport(support);
	  }
	}
	else {
	  if ((support = support->Undominated(dialog.DomStrong(), 
					      dialog.Players(), 
					      gnull, gstatus)) != 0) {
	    support->SetName(m_nfgView->UniqueSupportName());
	    m_nfgView->AddSupport(support);
	  }
	}
      }
      else {
	if (dialog.FindAll()) {
	  while ((support = support->MixedUndominated(dialog.DomStrong(),
						      precRATIONAL,
						      dialog.Players(),
						      gnull, gstatus)) != 0) {
	    support->SetName(m_nfgView->UniqueSupportName());
	    m_nfgView->AddSupport(support);
	  }
	}
	else {
	  if ((support = support->MixedUndominated(dialog.DomStrong(),
						   precRATIONAL,
						   dialog.Players(),
						   gnull, gstatus)) != 0) {
	    support->SetName(m_nfgView->UniqueSupportName());
	    m_nfgView->AddSupport(support);
	  }
	}
      }
    }
    catch (gSignalBreak &) { }
  }
}

void guiNfgFrame::OnSupportsNfgNew(wxCommandEvent &)
{
  NFSupport newSupport(*m_nfgView->GetNfg());
  newSupport.SetName(m_nfgView->UniqueSupportName());
  dialogNfgEditSupport dialog(this, newSupport);

  if (dialog.ShowModal() == wxID_OK) {
    NFSupport *support = new NFSupport(dialog.Support());
    support->SetName(dialog.Name());
    m_nfgView->AddSupport(support);
  }
}

void guiNfgFrame::OnSupportsNfgEdit(wxCommandEvent &)
{
  NFSupport *support = m_nfgView->CurrentSupport();
  dialogNfgEditSupport dialog(this, *support);

  if (dialog.ShowModal() == wxID_OK) {
    m_nfgView->EditCurrentSupport(dialog.Support());
  }
}

void guiNfgFrame::OnSupportsNfgDelete(wxCommandEvent &)
{
  if (m_nfgView->NumSupports() == 1) {
    return;
  }

  dialogSupportSelect dialog(this, m_nfgView->Supports(), 
			     m_nfgView->CurrentSupportIndex(),
			     "Delete support");

  if (dialog.ShowModal() == wxID_OK) {
    m_nfgView->DeleteSupport(dialog.Selected());
  }
}

void guiNfgFrame::OnSupportsNfgSelect(wxCommandEvent &)
{
  dialogSupportSelect dialog(this, m_nfgView->Supports(),
			     m_nfgView->CurrentSupportIndex(),
			     "Select support");

  if (dialog.ShowModal() == wxID_OK) {
    m_nfgView->SetCurrentSupport(dialog.Selected());
  }
}

void guiNfgFrame::OnSolveNfgStandard(wxCommandEvent &)
{
  /*
  dialogNfgStandard dialog(this, *GetNfg());
  if (dialog.ShowModal() == wxID_OK) {
    

  }
  */
}

