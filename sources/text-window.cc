//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A custom static-text type window
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#include "text-window.h"

BEGIN_EVENT_TABLE(gbtTextWindow, wxScrolledWindow)
  EVT_LEFT_DOWN(gbtTextWindow::OnLeftDown)
  EVT_PAINT(gbtTextWindow::OnPaint)
END_EVENT_TABLE()

gbtTextWindow::gbtTextWindow(wxWindow *p_parent, wxWindowID p_id,
			     const wxString &p_label, 
			     const wxString &p_longestLabel)
  : wxScrolledWindow(p_parent, p_id), 
    m_underline(false), m_label(p_label), m_longestLabel(p_longestLabel)
{
  if (m_longestLabel == wxT(""))  m_longestLabel = p_label;
  SetSize(GetBestSize());
}

wxSize gbtTextWindow::GetBestSize(void) const
{
  wxClientDC dc(const_cast<gbtTextWindow *>(this));
  dc.SetFont(GetFont());
  wxCoord width, height;
  dc.GetTextExtent(m_longestLabel, &width, &height);
  return wxSize(width + 5, height + 5);
}

void gbtTextWindow::OnPaint(wxPaintEvent &)
{
  wxPaintDC dc(this);
  wxSize size(GetClientSize());
  wxCoord width, height;
  dc.SetFont(GetFont());
  dc.GetTextExtent(m_label, &width, &height);

  dc.DrawText(m_label,
	      (size.GetWidth() - width) / 2, (size.GetHeight() - height) / 2);

  if (m_underline) {
    dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
    dc.DrawLine((size.GetWidth() - width) / 2,
		(size.GetHeight() + height) / 2 + 1,
		(size.GetWidth() + width) / 2,
		(size.GetHeight() + height) / 2 + 1);
  }
}

void gbtTextWindow::OnLeftDown(wxMouseEvent &)
{
  wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
  wxPostEvent(GetParent(), event);
}


