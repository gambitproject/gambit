//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of outcome palette window
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
#include "outcomes.h"
#include "numberedit.h"
#include "id.h"

//=========================================================================
//                       Outcome-editing commands
//=========================================================================

//-------------------------------------------------------------------------
//                       class gbtCmdNewOutcome
//-------------------------------------------------------------------------

class gbtCmdNewOutcome : public gbtGameCommand {
public:
  gbtCmdNewOutcome(void) { }
  virtual ~gbtCmdNewOutcome() { }

  void Do(gbtGameDocument *);
};

void gbtCmdNewOutcome::Do(gbtGameDocument *p_doc)
{
  if (p_doc->HasEfg()) {
    gText outcomeName = p_doc->UniqueEfgOutcomeName();
    gbtEfgOutcome outcome = p_doc->GetEfg().NewOutcome();
    outcome.SetLabel(outcomeName);
    for (int pl = 1; pl <= p_doc->GetEfg().NumPlayers(); pl++) {
      outcome.SetPayoff(p_doc->GetEfg().GetPlayer(pl), gNumber(0));
    }
  }
  else {
    gText outcomeName = p_doc->UniqueNfgOutcomeName();
    gbtNfgOutcome outcome = p_doc->GetNfg().NewOutcome();
    outcome.SetLabel(outcomeName);
    for (int pl = 1; pl <= p_doc->GetNfg().NumPlayers(); pl++) {
      outcome.SetPayoff(p_doc->GetNfg().GetPlayer(pl), gNumber(0));
    }
  }
}

//-------------------------------------------------------------------------
//                   class gbtCmdDeleteOutcome
//-------------------------------------------------------------------------

class gbtCmdDeleteOutcome : public gbtGameCommand {
private:
  int m_id;

public:
  gbtCmdDeleteOutcome(int p_id) : m_id(p_id) { }
  virtual ~gbtCmdDeleteOutcome() { }

  void Do(gbtGameDocument *);
};

void gbtCmdDeleteOutcome::Do(gbtGameDocument *p_doc)
{
  if (p_doc->HasEfg()) {
    p_doc->GetEfg().GetOutcome(m_id).DeleteOutcome();
  }
  else {
    p_doc->GetNfg().GetOutcome(m_id).DeleteOutcome();
  }
}

//-------------------------------------------------------------------------
//                   class gbtCmdAttachOutcome
//-------------------------------------------------------------------------

class gbtCmdAttachOutcome : public gbtGameCommand {
private:
  int m_id;

public:
  gbtCmdAttachOutcome(int p_id) : m_id(p_id) { }
  virtual ~gbtCmdAttachOutcome() { }

  void Do(gbtGameDocument *);
};

void gbtCmdAttachOutcome::Do(gbtGameDocument *p_doc)
{
  if (p_doc->HasEfg()) {
    if (m_id > 0) {
      p_doc->GetCursor().SetOutcome(p_doc->GetEfg().GetOutcome(m_id));
    }
    else {
      p_doc->GetCursor().SetOutcome(0);
    }
  }
  else {
    StrategyProfile profile(p_doc->GetNfg());
    for (int pl = 1; pl <= p_doc->GetNfg().NumPlayers(); pl++) {
      profile.Set(pl, p_doc->GetNfg().GetPlayer(pl).GetStrategy(p_doc->GetContingency()[pl]));
    }
    if (m_id > 0) {
      profile.SetOutcome(p_doc->GetNfg().GetOutcome(m_id));
    }
    else {
      profile.SetOutcome(0);
    }
  }
}

//=========================================================================
//                       class gbtOutcomeWindow
//=========================================================================

class gbtOutcomeWindow : public wxGrid {
private:
  gbtGameDocument *m_doc;

  void OnChar(wxKeyEvent &);
  void OnCellChanged(wxGridEvent &);

public:
  gbtOutcomeWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~gbtOutcomeWindow() { }

  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtOutcomeWindow, wxGrid)
  EVT_KEY_DOWN(gbtOutcomeWindow::OnChar)
  EVT_GRID_CELL_CHANGE(gbtOutcomeWindow::OnCellChanged)
END_EVENT_TABLE()

gbtOutcomeWindow::gbtOutcomeWindow(gbtGameDocument *p_doc,
				   wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_doc(p_doc)
{
  CreateGrid((m_doc->HasEfg()) ? m_doc->GetEfg().NumOutcomes() :
	      m_doc->GetNfg().NumOutcomes(),
	      m_doc->GetNfg().NumPlayers() + 1);
  EnableEditing(true);
  SetSelectionMode(wxGridSelectRows);
  SetLabelSize(wxVERTICAL, 0);
  SetLabelValue(wxHORIZONTAL, "Name", 0);
  EnableDragRowSize(false);
  EnableGridLines(false);

  Show(true);
}

void gbtOutcomeWindow::OnUpdate(void)
{
  SetDefaultCellFont(m_doc->GetPreferences().GetDataFont());
  SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  SetLabelFont(m_doc->GetPreferences().GetLabelFont());
  SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  
  if (m_doc->HasEfg()) {
    gbtEfgGame efg = m_doc->GetEfg();

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
      gbtEfgOutcome outcome = efg.GetOutcome(outc);

      SetCellValue((char *) outcome.GetLabel(), outc - 1, 0);

      for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
	SetCellValue((char *) ToText(outcome.GetPayoff(efg.GetPlayer(pl))),
		     outc - 1, pl);
	SetCellTextColour(m_doc->GetPreferences().PlayerColor(pl),
			  outc - 1, pl);
      }

      for (int col = 0; col < GetNumberCols(); col++) {
	if ((outc - 1) % 2 == 0) {
	  SetCellBackgroundColour(outc - 1, col, wxColour(200, 200, 200));
	}
	else {
	  SetCellBackgroundColour(outc - 1, col, wxColour(225, 225, 225));
	}
      }
    }

    for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
      if (efg.GetPlayer(pl).GetLabel() != "") {
	SetLabelValue(wxHORIZONTAL, (char *) efg.GetPlayer(pl).GetLabel(), pl);
      }
      else {
	SetLabelValue(wxHORIZONTAL, wxString::Format("Player %d", pl), pl);
      }
    }
  }
  else {
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
      gbtNfgOutcome outcome = nfg.GetOutcome(outc);

      SetCellValue((char *) outcome.GetLabel(), outc - 1, 0);

      for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
	SetCellValue((char *) ToText(outcome.GetPayoff(nfg.GetPlayer(pl))),
		     outc - 1, pl);
	SetCellTextColour(m_doc->GetPreferences().PlayerColor(pl),
			  outc - 1, pl);
      }

      for (int col = 0; col < GetNumberCols(); col++) {
	if ((outc - 1) % 2 == 0) {
	  SetCellBackgroundColour(outc - 1, col, wxColour(200, 200, 200));
	}
	else {
	  SetCellBackgroundColour(outc - 1, col, wxColour(225, 225, 225));
	}
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
  }
}

//
// This implements the automatic creation of a new outcome via
// pressing return or down-arrow while in the last row
//
void gbtOutcomeWindow::OnChar(wxKeyEvent &p_event)
{
  if (GetCursorRow() == GetRows() - 1 &&
      (p_event.GetKeyCode() == WXK_DOWN ||
       p_event.GetKeyCode() == WXK_RETURN)) {
    if (IsCellEditControlEnabled()) {
      SaveEditControlValue();
      HideCellEditControl();
    }
    m_doc->Submit(new gbtCmdNewOutcome());
  }
  else {
    p_event.Skip();
  }
}

void gbtOutcomeWindow::OnCellChanged(wxGridEvent &p_event)
{
  int row = p_event.GetRow();
  int col = p_event.GetCol();

  if (m_doc->HasEfg()) {
    gbtEfgOutcome outcome = m_doc->GetEfg().GetOutcome(row+1);
    if (col == 0) { 
      // Edited cell label
      outcome.SetLabel(GetCellValue(row, col).c_str());
    }
    else {
      // Edited payoff
      outcome.SetPayoff(m_doc->GetEfg().GetPlayer(col),
			ToNumber(GetCellValue(row, col).c_str()));
    }
  }
  else {
    gbtNfgOutcome outcome = m_doc->GetNfg().GetOutcome(row+1);
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
}

//=========================================================================
//                       class gbtOutcomeFrame
//=========================================================================

BEGIN_EVENT_TABLE(gbtOutcomeFrame, wxFrame)
  EVT_MENU(wxID_CLOSE, gbtOutcomeFrame::Close)
  EVT_MENU(GBT_MENU_OUTCOMES_NEW, gbtOutcomeFrame::OnOutcomeNew)
  EVT_MENU(GBT_MENU_OUTCOMES_DELETE, gbtOutcomeFrame::OnOutcomeDelete)
  EVT_MENU(GBT_MENU_OUTCOMES_ATTACH, gbtOutcomeFrame::OnOutcomeAttach)
  EVT_MENU(GBT_MENU_OUTCOMES_DETACH, gbtOutcomeFrame::OnOutcomeDetach)
  EVT_CLOSE(gbtOutcomeFrame::OnClose)
END_EVENT_TABLE()

gbtOutcomeFrame::gbtOutcomeFrame(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(300, 200)),
    gbtGameView(p_doc)
{
  m_grid = new gbtOutcomeWindow(p_doc, this);

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_CLOSE, "&Close", "Close this window");

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(GBT_MENU_OUTCOMES_NEW, "New",
		   "Create a new outcome");
  editMenu->Append(GBT_MENU_OUTCOMES_DELETE, "Delete",
		   "Delete this outcome");
  editMenu->AppendSeparator();
  editMenu->Append(GBT_MENU_OUTCOMES_ATTACH, "Attach",
		   "Attach this outcome at the cursor");
  editMenu->Append(GBT_MENU_OUTCOMES_DETACH, "Detach",
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
				(char *) m_doc->GetNfg().GetLabel()));
    }
    else {
      SetTitle(wxString::Format("Gambit - Outcomes: %s",
				(char *) m_doc->GetNfg().GetLabel()));
    }
  }
  Show(m_doc->ShowOutcomes());
}

void gbtOutcomeFrame::OnOutcomeNew(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdNewOutcome());
}

void gbtOutcomeFrame::OnOutcomeDelete(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdDeleteOutcome(m_grid->GetCursorRow() + 1));
}

void gbtOutcomeFrame::OnOutcomeAttach(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdAttachOutcome(m_grid->GetCursorRow() + 1));
}

void gbtOutcomeFrame::OnOutcomeDetach(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdAttachOutcome(0));
}
