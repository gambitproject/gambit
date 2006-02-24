//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Toolbar for interacting with player list
//
// This file is part of Gambit
// Copyright (c) 2006, The Gambit Project
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/dnd.h>           // for drag-and-drop features
#include <wx/image.h>         // for creating drag-and-drop cursor
#include <wx/colordlg.h>      // for picking player colors

#include "gamedoc.h"
#include "players.h"

#include "bitmaps/person.xpm"
#include "bitmaps/dice.xpm"

//=====================================================================
//                    class gbtPlayerToolbar
//=====================================================================

gbtPlayerToolbar::gbtPlayerToolbar(wxWindow *p_parent,
				   gbtGameDocument *p_doc)
  : wxSheet(p_parent, wxID_ANY), gbtGameView(p_doc),
    m_isHorizontal(true)
{
  SetSize(-1, 35);
  CreateGrid(1, 0);
  SetRowLabelWidth(0);
  SetColLabelHeight(0);
  SetRowHeight(0, 35);
  DisableDragGridSize();
  EnableGridLines(false);

  Connect(wxEVT_SIZE, wxSizeEventHandler(gbtPlayerToolbar::OnSize));
  Connect(wxEVT_MOTION,
	  wxMouseEventHandler(gbtPlayerToolbar::OnMouseEvents));
  Connect(GetId(), wxEVT_SHEET_CELL_RIGHT_DOWN,
	  (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, wxSheetEventFunction(&gbtPlayerToolbar::OnCellRightClick)));
}

wxString gbtPlayerToolbar::GetCellValue(const wxSheetCoords &p_coords)
{
  try {
    int player = p_coords.GetCol() + ((m_doc->IsTree()) ? 0 : 1);
    if (player == 0) {
      return wxT("Chance");
    }
    return wxString(m_doc->GetGame()->GetPlayer(player)->GetLabel().c_str(),
		    *wxConvCurrent);
  }
  catch (...) {
    return wxT("Whoops!");
  }
}

void gbtPlayerToolbar::SetCellValue(const wxSheetCoords &p_coords,
				    const wxString &p_value)
{
  int player = p_coords.GetCol() + ((m_doc->IsTree()) ? 0 : 1);
  m_doc->GetGame()->GetPlayer(player)->SetLabel((const char *) p_value.mb_str());
  m_doc->UpdateViews(GBT_DOC_MODIFIED_LABELS);
  AutoSizeCols();
}

void gbtPlayerToolbar::OnUpdate(void)
{
  int needCols = m_doc->NumPlayers() + ((m_doc->IsTree()) ? 1 : 0);
  if (GetNumberCols() < needCols) {
    InsertCols(0, needCols - GetNumberCols());
  }
  else if (GetNumberCols() > needCols) {
    DeleteCols(0, GetNumberCols() - needCols);
  }

  for (int col = 0; col < GetNumberCols(); col++) {
    if (m_doc->IsTree() && col == 0) {
      SetAttrRenderer(wxSheetCoords(0, col),
		      wxSheetCellRenderer(new wxSheetCellBitmapRendererRefData(wxBitmap(dice_xpm), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL)));
      SetAttrBackgroundColour(wxSheetCoords(0, col),
			      m_doc->GetStyle().ChanceColor());
      SetAttrReadOnly(wxSheetCoords(0, col), true);
    }
    else {
      SetAttrRenderer(wxSheetCoords(0, col),
		      wxSheetCellRenderer(new wxSheetCellBitmapRendererRefData(wxBitmap(person_xpm), wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL)));
      int player = col + ((m_doc->IsTree()) ? 0 : 1); 
      SetAttrBackgroundColour(wxSheetCoords(0, col),
			      m_doc->GetStyle().GetPlayerColor(player));
    }

    SetAttrForegroundColour(wxSheetCoords(0, col), *wxWHITE);
    SetAttrFont(wxSheetCoords(0, col),
		wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    SetAttrAlignment(wxSheetCoords(0, col),
		     wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
  }

  AutoSizeCols();
}

void gbtPlayerToolbar::PostPendingChanges(void)
{
  if (IsCellEditControlShown()) {
    DisableCellEditControl(true);
  }
}

void gbtPlayerToolbar::OnMouseEvents(wxMouseEvent &p_event)
{
  if (p_event.LeftIsDown() && p_event.Dragging()) {
    wxSheetCoords coords = XYToGridCell(p_event.GetX(), p_event.GetY());

    int player = coords.GetCol() + ((m_doc->IsTree()) ? 0 : 1);
    wxBitmap bitmap;
    if (player == 0) {
      bitmap = wxBitmap(dice_xpm);
    }
    else {
      bitmap = wxBitmap(person_xpm);
    }

#if defined( __WXMSW__) or defined(__WXMAC__)
    wxImage image = bitmap.ConvertToImage();
#else
    wxIcon image;
    image.CopyFromBitmap(bitmap);
#endif // _WXMSW__
      
    if (wxSheet::IsGridCell(coords)) {
      wxTextDataObject textData(wxString::Format(wxT("P%d"), player));
      wxDropSource source(textData, this, image, image, image);
      source.DoDragDrop(wxDrag_DefaultMove);
    }
  }
  else {
    p_event.Skip();
  }
}

void gbtPlayerToolbar::OnCellRightClick(wxSheetEvent &p_event)
{
  int player = p_event.GetCol() + ((m_doc->IsTree()) ? 0 : 1);
  wxColourData data;
  if (player == 0) {
    data.SetColour(m_doc->GetStyle().ChanceColor());
  }
  else {
    data.SetColour(m_doc->GetStyle().GetPlayerColor(player));
  }
  wxColourDialog dialog(this, &data);
  if (player == 0) {
    dialog.SetTitle(_("Choose color for chance player"));
  }
  else {
    dialog.SetTitle(wxString::Format(_("Choose color for player %d"),
				     player));
  }
  
  if (dialog.ShowModal() == wxID_OK) {
    wxColour color = dialog.GetColourData().GetColour();
    if (player == 0) {
      m_doc->GetStyle().SetChanceColor(color);
    }
    else {
      m_doc->GetStyle().SetPlayerColor(player, color);
    }
    m_doc->UpdateViews(GBT_DOC_MODIFIED_VIEWS);
  }
}

void gbtPlayerToolbar::OnSize(wxSizeEvent &p_event)
{
  wxSheet::OnSize(p_event);
  SetRowHeight(0, GetClientSize().GetHeight());
}
