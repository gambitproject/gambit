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

class gbtNfgOutcomeWindow : public wxGrid {
private:
  gbtGameDocument *m_doc;

  void OnChar(wxKeyEvent &);
  void OnCellChanged(wxGridEvent &);

public:
  gbtNfgOutcomeWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~gbtNfgOutcomeWindow() { }

  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtNfgOutcomeWindow, wxGrid)
  EVT_KEY_DOWN(gbtNfgOutcomeWindow::OnChar)
  EVT_GRID_CELL_CHANGE(gbtNfgOutcomeWindow::OnCellChanged)
END_EVENT_TABLE()

gbtNfgOutcomeWindow::gbtNfgOutcomeWindow(gbtGameDocument *p_doc,
					 wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_doc(p_doc)
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
  EnableDragRowSize(false);

  Show(true);
}

void gbtNfgOutcomeWindow::OnUpdate(void)
{
  SetDefaultCellFont(m_doc->GetPreferences().GetDataFont());
  SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  SetLabelFont(m_doc->GetPreferences().GetLabelFont());
  SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  
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
      SetCellTextColour(m_doc->GetPreferences().PlayerColor(pl),
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
void gbtNfgOutcomeWindow::OnChar(wxKeyEvent &p_event)
{
  if (GetCursorRow() == GetRows() - 1 &&
      (p_event.GetKeyCode() == WXK_DOWN ||
       p_event.GetKeyCode() == WXK_RETURN)) {
    if (IsCellEditControlEnabled()) {
      SaveEditControlValue();
      HideCellEditControl();
    }
    gText outcomeName = m_doc->UniqueNfgOutcomeName();
    gbtNfgOutcome outcome = m_doc->GetNfg().NewOutcome();
    outcome.SetLabel(outcomeName);
    for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
      outcome.SetPayoff(m_doc->GetNfg().GetPlayer(pl), gNumber(0));
    }
    AppendRows();
    for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
      SetCellEditor(GetRows() - 1, pl, new NumberEditor);
    }
    m_doc->UpdateViews(0, true, true);
    SetGridCursor(GetRows() - 1, 0);
  }
  else {
    p_event.Skip();
  }
}

void gbtNfgOutcomeWindow::OnCellChanged(wxGridEvent &p_event)
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
}

//-------------------------------------------------------------------------
//                      class gbtOutcomeFrame
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtOutcomeFrame, wxFrame)
  EVT_MENU(wxID_CLOSE, gbtOutcomeFrame::Close)
  EVT_MENU(GBT_NFG_MENU_OUTCOMES_NEW, gbtOutcomeFrame::OnOutcomeNew)
  EVT_MENU(GBT_NFG_MENU_OUTCOMES_DELETE, gbtOutcomeFrame::OnOutcomeDelete)
  EVT_MENU(GBT_NFG_MENU_OUTCOMES_ATTACH, gbtOutcomeFrame::OnOutcomeAttach)
  EVT_MENU(GBT_NFG_MENU_OUTCOMES_DETACH, gbtOutcomeFrame::OnOutcomeDetach)
  EVT_CLOSE(gbtOutcomeFrame::OnClose)
END_EVENT_TABLE()

gbtOutcomeFrame::gbtOutcomeFrame(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(300, 200)),
    gbtGameView(p_doc)
{
  m_grid = new gbtNfgOutcomeWindow(p_doc, this);

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_CLOSE, "&Close", "Close this window");

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(GBT_NFG_MENU_OUTCOMES_NEW, "New",
		   "Create a new outcome");
  editMenu->Append(GBT_NFG_MENU_OUTCOMES_DELETE, "Delete",
		   "Delete this outcome");
  editMenu->AppendSeparator();
  editMenu->Append(GBT_NFG_MENU_OUTCOMES_ATTACH, "Attach",
		   "Attach this outcome at the cursor");
  editMenu->Append(GBT_NFG_MENU_OUTCOMES_DETACH, "Detach",
		   "Detach the outcome at the cursor");

  wxMenu *viewMenu = new wxMenu;

  wxMenu *formatMenu = new wxMenu;

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(formatMenu, "&Format");
  SetMenuBar(menuBar);

  Show(false);
}

gbtOutcomeFrame::~gbtOutcomeFrame()
{ }

void gbtOutcomeFrame::OnClose(wxCloseEvent &p_event)
{
  m_doc->SetShowOutcomes(false);
  // Frame is now hidden; leave it that way, don't actually close
  p_event.Veto();
}

void gbtOutcomeFrame::OnUpdate(gbtGameView *p_sender)
{
  if (m_doc->ShowOutcomes()) {
    m_grid->OnUpdate();
    wxSize size = m_grid->GetBestSize();
    SetClientSize(size);
    m_grid->SetSize(size.GetWidth() + 1, size.GetHeight() + 1);
    m_grid->SetScrollRate(0, 0);

    if (m_doc->GetFilename() != "") {
      SetTitle(wxString::Format("Gambit - Outcomes: [%s] %s", 
				m_doc->GetFilename().c_str(), 
				(char *) m_doc->GetNfg().GetTitle()));
    }
    else {
      SetTitle(wxString::Format("Gambit - Outcomes: %s",
				(char *) m_doc->GetNfg().GetTitle()));
    }
  }
  Show(m_doc->ShowOutcomes());
}

void gbtOutcomeFrame::OnOutcomeNew(wxCommandEvent &)
{
  gText outcomeName = m_doc->UniqueNfgOutcomeName();
  gbtNfgOutcome outcome = m_doc->GetNfg().NewOutcome();
  outcome.SetLabel(outcomeName);
  // Appending the row here keeps currently selected row selected
  m_grid->AppendRows();
  for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
    outcome.SetPayoff(m_doc->GetNfg().GetPlayer(pl), gNumber(0));
    m_grid->SetCellEditor(m_grid->GetRows() - 1, pl, new NumberEditor);
  }
  m_doc->UpdateViews(0, true, true);
}

void gbtOutcomeFrame::OnOutcomeDelete(wxCommandEvent &)
{
  if (m_grid->GetGridCursorRow() >= 0 && 
      m_grid->GetGridCursorRow() < m_grid->GetRows()) {
    m_doc->GetNfg().DeleteOutcome(m_doc->GetNfg().GetOutcomeId(m_grid->GetGridCursorRow() + 1));
  }
  m_doc->UpdateViews(0, true, true);
}

void gbtOutcomeFrame::OnOutcomeAttach(wxCommandEvent &)
{
  if (m_grid->GetGridCursorRow() >= 0 && 
      m_grid->GetGridCursorRow() < m_grid->GetRows()) {
    StrategyProfile profile(m_doc->GetNfg());
    for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
      profile.Set(pl, m_doc->GetNfg().GetPlayer(pl).GetStrategy(m_doc->GetContingency()[pl]));
    }
    profile.SetOutcome(m_doc->GetNfg().GetOutcomeId(m_grid->GetGridCursorRow() + 1));
  }
  m_doc->UpdateViews(0, true, true);
}

void gbtOutcomeFrame::OnOutcomeDetach(wxCommandEvent &)
{
  StrategyProfile profile(m_doc->GetNfg());
  for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
    profile.Set(pl, m_doc->GetNfg().GetPlayer(pl).GetStrategy(m_doc->GetContingency()[pl]));
  }
  profile.SetOutcome(0);
  m_doc->UpdateViews(0, true, true);
}

