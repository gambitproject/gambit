//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of normal form outcome palette window
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
#include "nfgoutcome.h"
#include "numberedit.h"

const int idPOPUP_NEW = 2001;
const int idPOPUP_DELETE = 2002;
const int idPOPUP_ATTACH = 2003;
const int idPOPUP_DETACH = 2004;

BEGIN_EVENT_TABLE(NfgOutcomeWindow, wxGrid)
  EVT_KEY_DOWN(NfgOutcomeWindow::OnChar)
  EVT_GRID_CELL_CHANGE(NfgOutcomeWindow::OnCellChanged)
  EVT_GRID_CELL_RIGHT_CLICK(NfgOutcomeWindow::OnCellRightClick)
  EVT_GRID_LABEL_RIGHT_CLICK(NfgOutcomeWindow::OnLabelRightClick)
  EVT_MENU(idPOPUP_NEW, NfgOutcomeWindow::OnPopupOutcomeNew)
  EVT_MENU(idPOPUP_DELETE, NfgOutcomeWindow::OnPopupOutcomeDelete)
  EVT_MENU(idPOPUP_ATTACH, NfgOutcomeWindow::OnPopupOutcomeAttach)
  EVT_MENU(idPOPUP_DETACH, NfgOutcomeWindow::OnPopupOutcomeDetach)
END_EVENT_TABLE()

NfgOutcomeWindow::NfgOutcomeWindow(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    gbtGameView(p_doc)
{
  CreateGrid(m_doc->GetNfg().NumOutcomes(),
	     m_doc->GetNfg().NumPlayers() + 1);
  for (int row = 0; row < GetRows(); row++) {
    for (int col = 1; col < GetCols(); col++) {
      SetCellEditor(row, col, new NumberEditor);
    }
  }
      
  EnableEditing(true);
  SetSelectionMode(wxGridSelectRows);
  SetLabelSize(wxVERTICAL, 0);
  SetLabelValue(wxHORIZONTAL, "Name", 0);
  SetDefaultCellAlignment(wxCENTER, wxCENTER);
  EnableDragRowSize(false);

  AdjustScrollbars();

  m_menu = new wxMenu("Outcomes");
  m_menu->Append(idPOPUP_NEW, "New outcome", "Create a new outcome");
  m_menu->Append(idPOPUP_DELETE, "Delete outcome", "Delete this outcome");
  m_menu->AppendSeparator();
  m_menu->Append(idPOPUP_ATTACH, "Attach outcome",
		 "Attach this outcome at the cursor");
  m_menu->Append(idPOPUP_DETACH, "Detach outcome",
		 "Detach the outcome at the cursor");

  Show(true);
}

void NfgOutcomeWindow::UpdateValues(void)
{
  gbtNfgGame nfg = m_doc->GetNfg();

  if (GetRows() != nfg.NumOutcomes()) {
    DeleteRows(0, GetRows());
    AppendRows(nfg.NumOutcomes());

    for (int row = 0; row < GetRows(); row++) {
      for (int col = 1; col < GetCols(); col++) {
	SetCellEditor(row, col, new NumberEditor);
      }
    }
  }

  for (int outc = 1; outc <= nfg.NumOutcomes(); outc++) {
    gbtNfgOutcome outcome = nfg.GetOutcomeId(outc);

    SetCellValue((char *) outcome.GetLabel(), outc - 1, 0);

    for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
      SetCellValue((char *) ToText(outcome.GetPayoff(nfg.GetPlayer(pl))),
		   outc - 1, pl);
    }
  }

  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    if (nfg.GetPlayer(pl).GetLabel() != "") {
      SetLabelValue(wxHORIZONTAL, (char *) nfg.GetPlayer(pl).GetLabel(), pl);
    }
    else {
      SetLabelValue(wxHORIZONTAL, wxString::Format("Player %d", pl), pl);
    }
  }
  
  AdjustScrollbars();
}

//
// This implements the automatic creation of a new outcome via
// pressing return or down-arrow while in the last row
//
void NfgOutcomeWindow::OnChar(wxKeyEvent &p_event)
{
  if (GetCursorRow() == GetRows() - 1 &&
      (p_event.GetKeyCode() == WXK_DOWN ||
       p_event.GetKeyCode() == WXK_RETURN)) {
    if (IsCellEditControlEnabled()) {
      SaveEditControlValue();
      HideCellEditControl();
    }
    gText outcomeName = m_doc->m_nfgShow->UniqueOutcomeName();
    gbtNfgOutcome outcome = m_doc->GetNfg().NewOutcome();
    outcome.SetLabel(outcomeName);
    for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
      outcome.SetPayoff(m_doc->GetNfg().GetPlayer(pl), gNumber(0));
    }
    AppendRows();
    for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
      SetCellEditor(GetRows() - 1, pl, new NumberEditor);
    }
    m_doc->m_nfgShow->OnOutcomesEdited();
    UpdateValues();
    SetGridCursor(GetRows() - 1, 0);
  }
  else {
    p_event.Skip();
  }
}

void NfgOutcomeWindow::OnCellChanged(wxGridEvent &p_event)
{
  int row = p_event.GetRow();
  int col = p_event.GetCol();

  gbtNfgOutcome outcome = m_doc->GetNfg().GetOutcomeId(row+1);
  if (col == 0) { 
    // Edited cell label
    outcome.SetLabel(GetCellValue(row, col).c_str());
  }
  else {
    // Edited payoff
    outcome.SetPayoff(m_doc->GetNfg().GetPlayer(col),
		      ToNumber(GetCellValue(row, col).c_str()));
  }

  m_doc->m_nfgShow->OnOutcomesEdited();
}

void NfgOutcomeWindow::OnCellRightClick(wxGridEvent &p_event)
{
  PopupMenu(m_menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void NfgOutcomeWindow::OnLabelRightClick(wxGridEvent &p_event)
{
  PopupMenu(m_menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void NfgOutcomeWindow::OnPopupOutcomeNew(wxCommandEvent &)
{
  gText outcomeName = m_doc->m_nfgShow->UniqueOutcomeName();
  gbtNfgOutcome outcome = m_doc->GetNfg().NewOutcome();
  outcome.SetLabel(outcomeName);
  // Appending the row here keeps currently selected row selected
  AppendRows();
  for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
    outcome.SetPayoff(m_doc->GetNfg().GetPlayer(pl), gNumber(0));
    SetCellEditor(GetRows() - 1, pl, new NumberEditor);
  }
  m_doc->m_nfgShow->OnOutcomesEdited();
  UpdateValues();
}

void NfgOutcomeWindow::OnPopupOutcomeDelete(wxCommandEvent &)
{
  if (GetGridCursorRow() >= 0 && GetGridCursorRow() < GetRows()) {
    m_doc->GetNfg().DeleteOutcome(m_doc->GetNfg().GetOutcomeId(GetGridCursorRow() + 1));
    m_doc->m_nfgShow->OnOutcomesEdited();
  }
  UpdateValues();
}

void NfgOutcomeWindow::OnPopupOutcomeAttach(wxCommandEvent &)
{
  if (GetGridCursorRow() >= 0 && GetGridCursorRow() < GetRows()) {
    m_doc->GetNfg().SetOutcome(m_doc->m_nfgShow->GetContingency(),
			       m_doc->GetNfg().GetOutcomeId(GetGridCursorRow() + 1));
    m_doc->m_nfgShow->OnOutcomesEdited();
  }
}

void NfgOutcomeWindow::OnPopupOutcomeDetach(wxCommandEvent &)
{
  m_doc->GetNfg().SetOutcome(m_doc->m_nfgShow->GetContingency(), 0);
  m_doc->m_nfgShow->OnOutcomesEdited();
}


