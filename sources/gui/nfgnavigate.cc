//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of normal form navigation window
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
#endif // WX_PRECOMP
#include "nfgnavigate.h"


BEGIN_EVENT_TABLE(gbtNfgNavigate, wxGrid)
  EVT_GRID_CELL_LEFT_CLICK(gbtNfgNavigate::OnLeftClick)
END_EVENT_TABLE()

gbtNfgNavigate::gbtNfgNavigate(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    gbtGameView(p_doc)
{
  gbtNfgGame nfg = m_doc->GetNfg();
  CreateGrid(nfg->NumPlayers(), 6);
  SetEditable(false);
  SetLabelSize(wxVERTICAL, 0);
  EnableGridLines(false);

  DisableDragRowSize();
  DisableDragColSize();
  OnUpdate(0);
  Show(true);
}

gbtNfgNavigate::~gbtNfgNavigate()
{
}

void gbtNfgNavigate::OnUpdate(gbtGameView *)
{
  SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  SetDefaultCellFont(m_doc->GetPreferences().GetDataFont());
  SetLabelFont(m_doc->GetPreferences().GetLabelFont());

  SetColLabelValue(0, wxT("R/C"));
  SetColLabelValue(1, _("Player"));
  SetColLabelValue(2, wxT("#"));
  SetColLabelValue(3, _("Strategy"));
  SetColLabelValue(4, wxT(""));
  SetColLabelValue(5, wxT(""));

  if (GetNumberRows() < m_doc->GetNfg()->NumPlayers()) {
    AppendRows(m_doc->GetNfg()->NumPlayers() - GetNumberRows());
  }
  else if (GetNumberRows() > m_doc->GetNfg()->NumPlayers()) {
    DeleteRows(0, GetNumberRows() - m_doc->GetNfg()->NumPlayers());
  }

  const gbtNfgSupport &support = m_doc->GetNfgSupportList().GetCurrent();

  for (int pl = 1; pl <= m_doc->GetNfg()->NumPlayers(); pl++) {
    for (int col = 0; col < GetNumberCols(); col++) {
      SetCellTextColour(pl - 1, col, 
			m_doc->GetPreferences().PlayerColor(pl));
    }

    if (m_doc->GetRowPlayer() == pl) {
      SetCellValue(pl - 1, 0, wxT("(row)"));
    }
    else if (m_doc->GetColPlayer() == pl) {
      SetCellValue(pl - 1, 0, wxT("(col)"));
    } 
    else {
      SetCellValue(pl - 1, 0, wxT(""));
    }

    SetCellValue(pl - 1, 1, 
		 wxString::Format(wxT("%s"),
				  (char *) m_doc->GetNfg()->GetPlayer(pl)->GetLabel()));
    SetCellValue(pl - 1, 2, 
		 wxString::Format(wxT("%s"), 
				  (char *) ToText(m_doc->GetContingency()[pl])));
    SetCellValue(pl - 1, 3, 
		 wxString::Format(wxT("%s"),
				  (char *) support.GetStrategy(pl,
							       m_doc->GetContingency()[pl])->GetLabel()));
    SetCellValue(pl - 1, 4, wxT("+"));
    SetCellValue(pl - 1, 5, wxT("-"));
  }

  AutoSizeRows();
  AutoSizeColumns();
  AdjustScrollbars();
}

void gbtNfgNavigate::OnLeftClick(wxGridEvent &p_event)
{
  int player = p_event.GetRow() + 1;

  if (p_event.GetCol() == 0) {
    if (m_doc->GetNfg()->NumPlayers() == 2) {
      // Just implement a toggle
      m_doc->SetRowPlayer(3 - m_doc->GetRowPlayer());
    }
    else if (player == m_doc->GetRowPlayer()) {
      // Toggle row and column
      m_doc->SetColPlayer(player);
    }
    else if (player == m_doc->GetColPlayer()) {
      // Toggle row and column
      m_doc->SetRowPlayer(player);
    }
    else {
      m_doc->SetRowPlayer(player);
    }
  }
  else if (p_event.GetCol() == 4) {
    gbtArray<int> cont = m_doc->GetContingency();
    if (cont[player] < m_doc->GetNfgSupportList().GetCurrent().NumStrats(player)) {
      cont[player]++;
    }
    m_doc->SetContingency(cont);
  }
  else if (p_event.GetCol() == 5) {
    gbtArray<int> cont = m_doc->GetContingency();
    if (cont[player] > 1) {
      cont[player]--;
    }
    m_doc->SetContingency(cont);
  }
}
