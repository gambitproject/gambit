//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of normal form outcome palette window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "nfgoutcome.h"
#include "guishare/numberedit.h"

const int idPOPUP_NEW = 2001;
const int idPOPUP_DELETE = 2002;
const int idPOPUP_ATTACH = 2003;
const int idPOPUP_DETACH = 2004;

BEGIN_EVENT_TABLE(NfgOutcomeWindow, wxGrid)
  EVT_GRID_CELL_CHANGE(NfgOutcomeWindow::OnCellChanged)
  EVT_GRID_CELL_RIGHT_CLICK(NfgOutcomeWindow::OnCellRightClick)
  EVT_GRID_LABEL_RIGHT_CLICK(NfgOutcomeWindow::OnLabelRightClick)
  EVT_MENU(idPOPUP_NEW, NfgOutcomeWindow::OnPopupOutcomeNew)
  EVT_MENU(idPOPUP_DELETE, NfgOutcomeWindow::OnPopupOutcomeDelete)
  EVT_MENU(idPOPUP_ATTACH, NfgOutcomeWindow::OnPopupOutcomeAttach)
  EVT_MENU(idPOPUP_DETACH, NfgOutcomeWindow::OnPopupOutcomeDetach)
END_EVENT_TABLE()

NfgOutcomeWindow::NfgOutcomeWindow(NfgShow *p_nfgShow, wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_nfgShow)
{
  CreateGrid(p_nfgShow->Game().NumOutcomes(),
	     p_nfgShow->Game().NumPlayers() + 1);
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

  for (int i = 1; i <= p_nfgShow->Game().NumPlayers(); i++) {
    SetLabelValue(wxHORIZONTAL, 
		  (char *) p_nfgShow->Game().Players()[i]->GetName(), i);
  }
  
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

void NfgOutcomeWindow::UpdateValues(void)
{
  const Nfg &nfg = m_parent->Game();

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
    NFOutcome *outcome = nfg.GetOutcome(outc);

    SetCellValue((char *) outcome->GetName(), outc - 1, 0);

    for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
      SetCellValue((char *) ToText(nfg.Payoff(outcome, nfg.Players()[pl])),
		   outc - 1, pl);
    }
  }
}

void NfgOutcomeWindow::OnCellChanged(wxGridEvent &p_event)
{
  int row = p_event.GetRow();
  int col = p_event.GetCol();

  if (col == 0) { 
    // Edited cell label
    m_parent->Game().Outcomes()[row+1]->SetName(GetCellValue(row, col).c_str());
  }
  else {
    // Edited payoff
    m_parent->Game().SetPayoff(m_parent->Game().Outcomes()[row+1], col,
			       ToNumber(GetCellValue(row, col).c_str()));
  }

  m_parent->OnOutcomesEdited();
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
  m_parent->Game().NewOutcome();
  // Appending the row here keeps currently selected row selected
  AppendRows();
  m_parent->OnOutcomesEdited();
}

void NfgOutcomeWindow::OnPopupOutcomeDelete(wxCommandEvent &)
{
  if (GetGridCursorRow() >= 0 && GetGridCursorRow() < GetRows()) {
    m_parent->Game().DeleteOutcome(m_parent->Game().GetOutcome(GetGridCursorRow() + 1));
    m_parent->OnOutcomesEdited();
  }
}

void NfgOutcomeWindow::OnPopupOutcomeAttach(wxCommandEvent &)
{
  if (GetGridCursorRow() >= 0 && GetGridCursorRow() < GetRows()) {
    m_parent->Game().SetOutcome(m_parent->GetProfile(),
				m_parent->Game().GetOutcome(GetGridCursorRow() + 1));
    m_parent->OnOutcomesEdited();
  }
}

void NfgOutcomeWindow::OnPopupOutcomeDetach(wxCommandEvent &)
{
  m_parent->Game().SetOutcome(m_parent->GetProfile(), 0);
  m_parent->OnOutcomesEdited();
}


