//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of extensive form outcome palette window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgoutcome.h"
#include "guishare/numberedit.h"

const int idPOPUP_NEW = 2001;
const int idPOPUP_DELETE = 2002;
const int idPOPUP_ATTACH = 2003;
const int idPOPUP_DETACH = 2004;

BEGIN_EVENT_TABLE(EfgOutcomeWindow, wxGrid)
  EVT_KEY_DOWN(EfgOutcomeWindow::OnChar)
  EVT_GRID_CELL_CHANGE(EfgOutcomeWindow::OnCellChanged)
  EVT_GRID_CELL_RIGHT_CLICK(EfgOutcomeWindow::OnCellRightClick)
  EVT_GRID_LABEL_RIGHT_CLICK(EfgOutcomeWindow::OnLabelRightClick)
  EVT_MENU(idPOPUP_NEW, EfgOutcomeWindow::OnPopupOutcomeNew)
  EVT_MENU(idPOPUP_DELETE, EfgOutcomeWindow::OnPopupOutcomeDelete)
  EVT_MENU(idPOPUP_ATTACH, EfgOutcomeWindow::OnPopupOutcomeAttach)
  EVT_MENU(idPOPUP_DETACH, EfgOutcomeWindow::OnPopupOutcomeDetach)
END_EVENT_TABLE()

EfgOutcomeWindow::EfgOutcomeWindow(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_efgShow)
{
  CreateGrid(p_efgShow->Game()->NumOutcomes(),
	     p_efgShow->Game()->NumPlayers() + 1);
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
  m_menu->Append(idPOPUP_NEW, "New", "Create a new outcome");
  m_menu->Append(idPOPUP_DELETE, "Delete", "Delete this outcome");
  m_menu->AppendSeparator();
  m_menu->Append(idPOPUP_ATTACH, "Attach",
		 "Attach this outcome at the cursor");
  m_menu->Append(idPOPUP_DETACH, "Detach", "Detach the outcome at the cursor");

  Show(true);
}

void EfgOutcomeWindow::UpdateValues(void)
{
  const FullEfg &efg = *m_parent->Game();

  if (GetCols() < efg.NumPlayers() + 1) {
    AppendCols(efg.NumPlayers() + 1 - GetCols());

    for (int row = 0; row < GetRows(); row++) {
      for (int col = 1; col < GetCols(); col++) {
	SetCellEditor(row, col, new NumberEditor);
      }
    }
  }

  if (GetRows() != efg.NumOutcomes()) {
    DeleteRows(0, GetRows());
    AppendRows(efg.NumOutcomes());

    for (int row = 0; row < GetRows(); row++) {
      for (int col = 1; col < GetCols(); col++) {
	SetCellEditor(row, col, new NumberEditor);
      }
    }
  }

  for (int outc = 1; outc <= efg.NumOutcomes(); outc++) {
    efgOutcome *outcome = efg.GetOutcome(outc);

    SetCellValue((char *) efg.GetOutcomeName(outcome), outc - 1, 0);

    for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
      SetCellValue((char *) ToText(efg.Payoff(outcome, efg.Players()[pl])),
		   outc - 1, pl);
    }
  }

  for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
    if (efg.Players()[pl]->GetName() != "") {
      SetLabelValue(wxHORIZONTAL, (char *) efg.Players()[pl]->GetName(), pl);
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
void EfgOutcomeWindow::OnChar(wxKeyEvent &p_event)
{
  if (GetCursorRow() == GetRows() - 1 &&
      (p_event.GetKeyCode() == WXK_DOWN ||
       p_event.GetKeyCode() == WXK_RETURN)) {
    if (IsCellEditControlEnabled()) {
      SaveEditControlValue();
      HideCellEditControl();
    }
    m_parent->Game()->NewOutcome();
    AppendRows();
    m_parent->OnOutcomesEdited();
    UpdateValues();
    SetGridCursor(GetRows() - 1, 0);
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
    m_parent->Game()->SetOutcomeName(m_parent->Game()->GetOutcome(row + 1),
				     GetCellValue(row, col).c_str());
  }
  else {
    // Edited payoff
    m_parent->Game()->SetPayoff(m_parent->Game()->GetOutcome(row + 1), col,
				ToNumber(GetCellValue(row, col).c_str()));
  }

  m_parent->OnOutcomesEdited();
}

void EfgOutcomeWindow::OnCellRightClick(wxGridEvent &p_event)
{
  m_menu->Enable(idPOPUP_ATTACH, m_parent->Cursor());
  m_menu->Enable(idPOPUP_DETACH, m_parent->Cursor());
  PopupMenu(m_menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void EfgOutcomeWindow::OnLabelRightClick(wxGridEvent &p_event)
{
  m_menu->Enable(idPOPUP_ATTACH, m_parent->Cursor());
  m_menu->Enable(idPOPUP_DETACH, m_parent->Cursor());
  PopupMenu(m_menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void EfgOutcomeWindow::OnPopupOutcomeNew(wxCommandEvent &)
{
  m_parent->Game()->NewOutcome();
  // Appending the row here keeps currently selected row selected
  AppendRows();
  m_parent->OnOutcomesEdited();
}

void EfgOutcomeWindow::OnPopupOutcomeDelete(wxCommandEvent &)
{
  if (GetGridCursorRow() >= 0 && GetGridCursorRow() < GetRows()) {
    m_parent->Game()->DeleteOutcome(m_parent->Game()->GetOutcome(GetGridCursorRow() + 1));
    m_parent->OnOutcomesEdited();
  }
}

void EfgOutcomeWindow::OnPopupOutcomeAttach(wxCommandEvent &)
{
  if (GetGridCursorRow() >= 0 && GetGridCursorRow() < GetRows()) {
    m_parent->Game()->SetOutcome(m_parent->Cursor(),
				 m_parent->Game()->GetOutcome(GetGridCursorRow() + 1));
    m_parent->OnOutcomesEdited();
  }
}

void EfgOutcomeWindow::OnPopupOutcomeDetach(wxCommandEvent &)
{
  m_parent->Game()->SetOutcome(m_parent->Cursor(), 0);
  m_parent->OnOutcomesEdited();
}






