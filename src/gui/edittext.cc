//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/edittext.cc
// A widget that functions as an editable wxStaticText
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

#include "edittext.h"

//=========================================================================
//                    class gbtStaticTextButton
//=========================================================================

BEGIN_EVENT_TABLE(gbtStaticTextButton, wxStaticText)
  EVT_LEFT_DOWN(gbtStaticTextButton::OnLeftClick)
END_EVENT_TABLE()

gbtStaticTextButton::gbtStaticTextButton(wxWindow *p_parent, int p_id,
					 const wxString &p_label,
					 const wxPoint &p_position,
					 const wxSize &p_size,
					 long p_style)
  : wxStaticText(p_parent, -1, p_label, p_position, p_size, p_style)
{ }

void gbtStaticTextButton::OnLeftClick(wxMouseEvent &p_event)
{
  wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED);
  event.SetId(GetId());
  wxPostEvent(GetParent(), event);

  p_event.Skip();
}

//=========================================================================
//                       class gbtEditableText
//=========================================================================

gbtEditableText::gbtEditableText(wxWindow *p_parent, int p_id,
				 const wxString &p_value,
				 const wxPoint &p_position,
				 const wxSize &p_size)
  : wxPanel(p_parent, p_id, p_position, p_size)
{
  m_staticText = new gbtStaticTextButton(this, -1, p_value, 
					 wxPoint(0, 0), p_size,
					 wxALIGN_LEFT);
  Connect(m_staticText->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtEditableText::OnClick));

  m_textCtrl = new wxTextCtrl(this, -1, p_value, wxPoint(0, 0), 
			      p_size, wxTE_PROCESS_ENTER);
  Connect(m_textCtrl->GetId(), wxEVT_COMMAND_TEXT_ENTER,
	  wxCommandEventHandler(gbtEditableText::OnAccept));
  
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(m_staticText, 1, wxALIGN_CENTER, 0);
  topSizer->Add(m_textCtrl, 1, wxEXPAND | wxALIGN_CENTER, 0);
  topSizer->Show(m_textCtrl, false);
  SetSizer(topSizer);
  Layout();
}

void gbtEditableText::BeginEdit(void)
{
  m_textCtrl->SetValue(m_staticText->GetLabel());
  m_textCtrl->SetSelection(-1, -1);
  GetSizer()->Show(m_staticText, false);
  GetSizer()->Show(m_textCtrl, true);
  GetSizer()->Layout();
  m_textCtrl->SetFocus();
}

void gbtEditableText::EndEdit(bool p_accept)
{
  if (p_accept) {
    m_staticText->SetLabel(m_textCtrl->GetValue());
  }

  GetSizer()->Show(m_textCtrl, false);
  GetSizer()->Show(m_staticText, true);
  GetSizer()->Layout();
}

wxString gbtEditableText::GetValue(void) const
{
  
  if (GetSizer()->IsShown(m_textCtrl)) {
    return m_textCtrl->GetValue();
  }
  else {
    return m_staticText->GetLabel();
  }
}

void gbtEditableText::SetValue(const wxString &p_value)
{
  m_textCtrl->SetValue(p_value);
  m_staticText->SetLabel(p_value);
}

bool gbtEditableText::SetForegroundColour(const wxColour &p_color)
{
  m_staticText->SetForegroundColour(p_color);
  m_textCtrl->SetForegroundColour(p_color);
  return true;
}

bool gbtEditableText::SetBackgroundColour(const wxColour &p_color)
{
  m_staticText->SetBackgroundColour(p_color);
  m_textCtrl->SetBackgroundColour(p_color);
  return true;
}

bool gbtEditableText::SetFont(const wxFont &p_font)
{
  m_staticText->SetFont(p_font);
  m_textCtrl->SetFont(p_font);
  return true;
}

void gbtEditableText::OnClick(wxCommandEvent &)
{
  wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED);
  event.SetId(GetId());
  wxPostEvent(GetParent(), event);
}

void gbtEditableText::OnAccept(wxCommandEvent &)
{
  EndEdit(true);
  wxCommandEvent event(wxEVT_COMMAND_TEXT_ENTER);
  event.SetId(GetId());
  wxPostEvent(GetParent(), event);
}

