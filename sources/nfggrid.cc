//
// FILE: nfggrid.cc -- Implementation of normal form viewer
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
#include "nfggrid.h"

BEGIN_EVENT_TABLE(guiNfgGrid, wxGrid)
  EVT_CHAR(guiNfgGrid::OnChar)
END_EVENT_TABLE()

guiNfgGrid::guiNfgGrid(guiNfgView *p_parent, Nfg &p_nfg)
  : wxGrid(p_parent, -1, wxPoint(-1, -1), wxDefaultSize),
    m_nfg(p_nfg), m_support(p_nfg), m_parent(p_parent),
    m_rowPlayer(1), m_colPlayer(2),
    m_profile(p_nfg.NumPlayers()), m_showProbs(false),
    m_solution(0)
{
  CreateGrid(m_support.NumStrats(m_rowPlayer),
	     m_support.NumStrats(m_colPlayer));
  for (int i = 1; i <= m_profile.Length(); m_profile[i++] = 1);

  UpdateTable();

  UpdateDimensions();
  Refresh();
  Show(TRUE);
}

void guiNfgGrid::UpdateTable(void)
{
  gArray<int> profile(m_profile);

  for (int i = 1; i <= m_support.NumStrats(m_rowPlayer); i++) {
    profile[m_rowPlayer] = m_support.Strategies(m_rowPlayer)[i]->Number();
    for (int j = 1; j <= m_support.NumStrats(m_colPlayer); j++) {
      profile[m_colPlayer] = m_support.Strategies(m_colPlayer)[j]->Number();
      NFOutcome *outcome = m_nfg.GetOutcome(profile);
      if (outcome) {
	SetCellValue((char *) (ToText(outcome->GetNumber()) + ": " + outcome->GetName()), i - 1, j - 1);
      }
      else {
	SetCellValue("(null)", i - 1, j - 1);
      }
    }
  }

  if (m_showProbs && m_solution > 0) {
    for (int i = 1; i <= m_support.NumStrats(m_rowPlayer); i++) {
      Strategy *strategy = m_support.Strategies(m_rowPlayer)[i];
      SetCellValue((char *) ToText((*m_solution)(strategy)),
		   i - 1, m_support.NumStrats(m_colPlayer));
    }

    for (int i = 1; i <= m_support.NumStrats(m_colPlayer); i++) {
      Strategy *strategy = m_support.Strategies(m_colPlayer)[i];
      SetCellValue((char *) ToText((*m_solution)(strategy)),
		   m_support.NumStrats(m_rowPlayer), i - 1);
    }
  }
}

guiNfgGrid::~guiNfgGrid()
{ }

void guiNfgGrid::OnSelectCell(int p_row, int p_col)
{
  if (p_row < m_support.NumStrats(m_rowPlayer) &&
      p_col < m_support.NumStrats(m_colPlayer)) {
    m_profile[m_rowPlayer] =
      m_support.Strategies(m_rowPlayer)[p_row+1]->Number();
    m_profile[m_colPlayer] =
      m_support.Strategies(m_colPlayer)[p_col+1]->Number();
    m_parent->SetProfile(m_profile);
  }
}

void guiNfgGrid::OnChangeLabels(void)
{
  int maxLabelLength = 0;

  for (int st = 1; st <= m_support.NumStrats(m_rowPlayer); st++) {
    wxString label = (char *) m_support.Strategies(m_rowPlayer)[st]->Name();
    int width, height;
    GetTextExtent(label, &width, &height);
    maxLabelLength = gmax(maxLabelLength, width);
    SetLabelValue(wxVERTICAL, label, st - 1); 
  }
  SetLabelSize(wxVERTICAL, maxLabelLength + 20);

  for (int st = 1; st <= m_support.NumStrats(m_colPlayer); st++) {
    SetLabelValue(wxHORIZONTAL,
		  (char *) m_support.Strategies(m_colPlayer)[st]->Name(), st-1);
  }
}

void guiNfgGrid::SetPlayers(int p_rowPlayer, int p_colPlayer)
{
  m_rowPlayer = p_rowPlayer;
  m_colPlayer = p_colPlayer;
  CreateGrid(m_support.NumStrats(p_rowPlayer), 
	     m_support.NumStrats(p_colPlayer));
  UpdateTable();
  SetGridCursor(m_profile[m_rowPlayer] - 1, m_profile[m_colPlayer] - 1);
  Refresh();
}

void guiNfgGrid::SetProfile(const gArray<int> &p_profile)
{
  m_profile = p_profile;
  UpdateTable();
  //  SetGridCursor(m_profile[m_rowPlayer] - 1, m_profile[m_colPlayer] - 1);
  Refresh();
}

void guiNfgGrid::SetShowProbs(bool p_showProbs)
{
  if (p_showProbs != m_showProbs) {
    m_showProbs = p_showProbs;
    if (p_showProbs) {
      AppendCols(1);
      AppendRows(1);
      UpdateTable();
      Refresh();
    }
    else {
      DeleteCols(GetCols() - 1);
      DeleteRows(GetRows() - 1);
      UpdateTable();
      Refresh();
    }
  }
}

void guiNfgGrid::SetSolution(const MixedSolution &p_solution)
{
  m_solution = &p_solution;
  UpdateTable();
  Refresh();
}

void guiNfgGrid::SetOutcome(NFOutcome *p_outcome)
{
  int row = GetCursorRow(), col = GetCursorColumn();

  if (p_outcome) {
    SetCellValue((char *) (ToText(p_outcome->GetNumber()) + ": " + p_outcome->GetName()), row, col);
  }
  else {
    SetCellValue("(null)", row, col);
  }
}

void guiNfgGrid::SetSupport(NFSupport *p_support)
{
  m_support = *p_support;
  CreateGrid(m_support.NumStrats(m_rowPlayer), 
	     m_support.NumStrats(m_colPlayer));
  UpdateTable();
  for (int pl = 1; pl <= m_nfg.NumPlayers(); m_profile[pl++] = 1);
  SetGridCursor(0, 0);
  Refresh();
}

void guiNfgGrid::OnChar(wxKeyEvent &p_event)
{
  int row = GetCursorRow(), col = GetCursorColumn();

  wxMessageBox((char *) ToText(p_event.KeyCode()));

  switch (p_event.KeyCode()) {
  case WXK_HOME:
    SetGridCursor(0, 0);
    break;
  case WXK_LEFT:
    if (col > 0) {
      SetGridCursor(row, col - 1);
    }
    break;
  case WXK_RIGHT:
    if (col < GetCols() - 1) {
      SetGridCursor(row, col + 1);
    }
    break;
  case WXK_UP:
    if (row > 0) {
      SetGridCursor(row - 1, col);
    }
    break;
  case WXK_DOWN:
    if (row < GetRows() - 1) {
      SetGridCursor(row, col + 1);
    }
    break;
  default:
    break;
  }
  Refresh();
}

