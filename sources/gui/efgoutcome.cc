//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of extensive form outcome palette window
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
#include "efgoutcome.h"
#include "numberedit.h"

const int idPOPUP_NEW = 2001;
const int idPOPUP_DELETE = 2002;
const int idPOPUP_ATTACH = 2003;
const int idPOPUP_DETACH = 2004;

BEGIN_EVENT_TABLE(EfgOutcomeWindow, wxPanel)
  EVT_KEY_DOWN(EfgOutcomeWindow::OnChar)
  EVT_GRID_CELL_CHANGE(EfgOutcomeWindow::OnCellChanged)
  EVT_GRID_CELL_RIGHT_CLICK(EfgOutcomeWindow::OnCellRightClick)
  EVT_GRID_LABEL_RIGHT_CLICK(EfgOutcomeWindow::OnLabelRightClick)
  EVT_MENU(idPOPUP_NEW, EfgOutcomeWindow::OnPopupOutcomeNew)
  EVT_MENU(idPOPUP_DELETE, EfgOutcomeWindow::OnPopupOutcomeDelete)
  EVT_MENU(idPOPUP_ATTACH, EfgOutcomeWindow::OnPopupOutcomeAttach)
  EVT_MENU(idPOPUP_DETACH, EfgOutcomeWindow::OnPopupOutcomeDetach)
END_EVENT_TABLE()

EfgOutcomeWindow::EfgOutcomeWindow(gbtGameDocument *p_game,
				   EfgShow *p_efgShow, wxWindow *p_parent)
  : wxPanel(p_parent, -1), gbtGameView(p_game),
    m_parent(p_efgShow)
{
  SetAutoLayout(true);

  m_grid = new wxGrid(this, -1, wxDefaultPosition, wxSize(200, 200));

  m_grid->CreateGrid(m_game->m_efg->NumOutcomes(),
		     m_game->m_efg->NumPlayers() + 1);
  for (int row = 0; row < m_grid->GetRows(); row++) {
    for (int col = 1; col < m_grid->GetCols(); col++) {
      m_grid->SetCellEditor(row, col, new NumberEditor);
    }
  }
      
  m_grid->EnableEditing(true);
  m_grid->SetSelectionMode(m_grid->wxGridSelectRows);
  m_grid->SetLabelSize(wxVERTICAL, 0);
  m_grid->SetLabelValue(wxHORIZONTAL, "Name", 0);
  m_grid->SetDefaultCellAlignment(wxCENTER, wxCENTER);
  m_grid->EnableDragRowSize(false);
  m_grid->AdjustScrollbars();

  m_menu = new wxMenu("Outcomes");
  m_menu->Append(idPOPUP_NEW, "New outcome", "Create a new outcome");
  m_menu->Append(idPOPUP_DELETE, "Delete outcome", "Delete this outcome");
  m_menu->AppendSeparator();
  m_menu->Append(idPOPUP_ATTACH, "Attach outcome",
		 "Attach this outcome at the cursor");
  m_menu->Append(idPOPUP_DETACH, "Detach outcome",
		 "Detach the outcome at the cursor");

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_grid, 1, wxALL | wxEXPAND, 0);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(true);
}

void EfgOutcomeWindow::UpdateValues(void)
{
  const efgGame &efg = *m_game->m_efg;

  if (m_grid->GetCols() < efg.NumPlayers() + 1) {
    m_grid->AppendCols(efg.NumPlayers() + 1 - m_grid->GetCols());

    for (int row = 0; row < m_grid->GetRows(); row++) {
      for (int col = 1; col < m_grid->GetCols(); col++) {
	m_grid->SetCellEditor(row, col, new NumberEditor);
      }
    }
  }

  if (m_grid->GetRows() != efg.NumOutcomes()) {
    m_grid->DeleteRows(0, m_grid->GetRows());
    m_grid->AppendRows(efg.NumOutcomes());

    for (int row = 0; row < m_grid->GetRows(); row++) {
      for (int col = 1; col < m_grid->GetCols(); col++) {
	m_grid->SetCellEditor(row, col, new NumberEditor);
      }
    }
  }

  for (int outc = 1; outc <= efg.NumOutcomes(); outc++) {
    gbtEfgOutcome outcome = efg.GetOutcome(outc);

    m_grid->SetCellValue((char *) outcome.GetLabel(), outc - 1, 0);

    for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
      m_grid->SetCellValue((char *) ToText(efg.Payoff(outcome,
						      efg.Players()[pl])),
			   outc - 1, pl);
    }
  }

  for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
    if (efg.Players()[pl]->GetName() != "") {
      m_grid->SetLabelValue(wxHORIZONTAL,
			    (char *) efg.Players()[pl]->GetName(), pl);
    }
    else {
      m_grid->SetLabelValue(wxHORIZONTAL,
			    wxString::Format("Player %d", pl), pl);
    }
  }

  m_grid->AdjustScrollbars();
}

//
// This implements the automatic creation of a new outcome via
// pressing return or down-arrow while in the last row
//
void EfgOutcomeWindow::OnChar(wxKeyEvent &p_event)
{
  if (m_grid->GetCursorRow() == m_grid->GetRows() - 1 &&
      (p_event.GetKeyCode() == WXK_DOWN ||
       p_event.GetKeyCode() == WXK_RETURN)) {
    if (m_grid->IsCellEditControlEnabled()) {
      m_grid->SaveEditControlValue();
      m_grid->HideCellEditControl();
    }
    gText outcomeName = m_game->UniqueEfgOutcomeName();
    gbtEfgOutcome outcome = m_game->m_efg->NewOutcome();
    m_game->m_efg->SetLabel(outcome, outcomeName);
    for (int pl = 1; pl <= m_game->m_efg->NumPlayers(); pl++) {
      m_game->m_efg->SetPayoff(outcome, pl, gNumber(0));
    }
    m_grid->AppendRows();
    for (int pl = 1; pl <= m_game->m_efg->NumPlayers(); pl++) {
      m_grid->SetCellEditor(m_grid->GetRows() - 1, pl, new NumberEditor);
    }
    m_parent->OnOutcomesEdited();
    UpdateValues();
    m_grid->SetGridCursor(m_grid->GetRows() - 1, 0);
  }
  else {
    p_event.Skip();
  }
}

void EfgOutcomeWindow::OnCellChanged(wxGridEvent &p_event)
{
  int row = p_event.GetRow();
  int col = p_event.GetCol();

  if (col == 0) { 
    // Edited cell label
    gbtEfgOutcome outcome = m_game->m_efg->GetOutcome(row + 1);
    m_game->m_efg->SetLabel(outcome, m_grid->GetCellValue(row, col).c_str());
  }
  else {
    // Edited payoff
    m_game->m_efg->SetPayoff(m_game->m_efg->GetOutcome(row + 1), col,
			     ToNumber(m_grid->GetCellValue(row, col).c_str()));
  }

  m_parent->OnOutcomesEdited();
}

void EfgOutcomeWindow::OnCellRightClick(wxGridEvent &p_event)
{
  m_menu->Enable(idPOPUP_ATTACH, m_game->Cursor());
  m_menu->Enable(idPOPUP_DETACH, m_game->Cursor());
  PopupMenu(m_menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void EfgOutcomeWindow::OnLabelRightClick(wxGridEvent &p_event)
{
  m_menu->Enable(idPOPUP_ATTACH, m_game->Cursor());
  m_menu->Enable(idPOPUP_DETACH, m_game->Cursor());
  PopupMenu(m_menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void EfgOutcomeWindow::OnPopupOutcomeNew(wxCommandEvent &)
{
  gText outcomeName = m_game->UniqueEfgOutcomeName();
  gbtEfgOutcome outcome = m_game->m_efg->NewOutcome();
  m_game->m_efg->SetLabel(outcome, outcomeName);
  // Appending the row here keeps currently selected row selected
  m_grid->AppendRows();
  for (int pl = 1; pl <= m_game->m_efg->NumPlayers(); pl++) {
    m_game->m_efg->SetPayoff(outcome, pl, gNumber(0));
    m_grid->SetCellEditor(m_grid->GetRows() - 1, pl, new NumberEditor);
  }
  m_parent->OnOutcomesEdited();
  UpdateValues();
}

void EfgOutcomeWindow::OnPopupOutcomeDelete(wxCommandEvent &)
{
  if (m_grid->GetGridCursorRow() >= 0 && 
      m_grid->GetGridCursorRow() < m_grid->GetRows()) {
    gbtEfgOutcome outcome = m_game->m_efg->GetOutcome(m_grid->GetGridCursorRow() + 1);
    m_game->m_efg->DeleteOutcome(outcome);
    m_parent->OnOutcomesEdited();
  }
  UpdateValues();
}

void EfgOutcomeWindow::OnPopupOutcomeAttach(wxCommandEvent &)
{
  if (m_grid->GetGridCursorRow() >= 0 && 
      m_grid->GetGridCursorRow() < m_grid->GetRows()) {
    m_game->m_efg->SetOutcome(m_game->Cursor(),
			      m_game->m_efg->GetOutcome(m_grid->GetGridCursorRow() + 1));
    m_parent->OnOutcomesEdited();
  }
}

void EfgOutcomeWindow::OnPopupOutcomeDetach(wxCommandEvent &)
{
  m_game->m_efg->SetOutcome(m_game->Cursor(), 0);
  m_parent->OnOutcomesEdited();
}






