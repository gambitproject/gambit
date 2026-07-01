//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#endif // WX_PRECOMP

#include "edittext.h"

namespace Gambit::GUI {
//=========================================================================
//                    class StaticTextButton
//=========================================================================

BEGIN_EVENT_TABLE(StaticTextButton, wxStaticText)
EVT_LEFT_DOWN(StaticTextButton::OnLeftClick)
END_EVENT_TABLE()

StaticTextButton::StaticTextButton(wxWindow *p_parent, wxWindowID p_id, const wxString &p_label,
                                   const wxPoint &p_position, const wxSize &p_size, long p_style)
  : wxStaticText(p_parent, p_id, p_label, p_position, p_size, p_style)
{
}

void StaticTextButton::OnLeftClick(wxMouseEvent &p_event)
{
  wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED);
  event.SetId(GetId());
  wxPostEvent(GetParent(), event);

  p_event.Skip();
}

//=========================================================================
//                       class EditableText
//=========================================================================

EditableLabelText::EditableLabelText(wxWindow *p_parent, int p_id, const wxString &p_value,
                                     const wxPoint &p_position, const wxSize &p_size)
  : wxPanel(p_parent, p_id, p_position, p_size)
{
  m_staticText =
      new StaticTextButton(this, wxID_ANY, p_value, wxPoint(0, 0), p_size, wxALIGN_LEFT);
  Connect(m_staticText->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(EditableLabelText::OnClick));

  m_textCtrl = new LabelTextCtrl(this, wxID_ANY, p_value, LabelCharacterPolicy::AsciiOnly,
                                 wxPoint(0, 0), p_size, wxTE_PROCESS_ENTER);
  Connect(m_textCtrl->GetId(), wxEVT_COMMAND_TEXT_ENTER,
          wxCommandEventHandler(EditableLabelText::OnAccept));

  auto *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(m_staticText, 1, wxALIGN_CENTER, 0);
  topSizer->Add(m_textCtrl, 1, wxEXPAND, 0);
  topSizer->Show(m_textCtrl, false);
  SetSizer(topSizer);
  wxWindowBase::Layout();
}

void EditableLabelText::BeginEdit()
{
  m_textCtrl->SetValue(m_staticText->GetLabel());
  m_textCtrl->SetSelection(-1, -1);
  GetSizer()->Show(m_staticText, false);
  GetSizer()->Show(m_textCtrl, true);
  GetSizer()->Layout();
  m_textCtrl->SetFocus();
}

void EditableLabelText::EndEdit(bool p_accept)
{
  if (p_accept) {
    m_staticText->SetLabel(m_textCtrl->GetNormalizedValue());
  }

  GetSizer()->Show(m_textCtrl, false);
  GetSizer()->Show(m_staticText, true);
  GetSizer()->Layout();
}

wxString EditableLabelText::GetValue() const
{

  if (GetSizer()->IsShown(m_textCtrl)) {
    return m_textCtrl->GetNormalizedValue();
  }
  else {
    return m_staticText->GetLabel();
  }
}

void EditableLabelText::SetValue(const wxString &p_value)
{
  m_textCtrl->SetValue(p_value);
  m_staticText->SetLabel(p_value);
}

bool EditableLabelText::SetForegroundColour(const wxColour &p_color)
{
  m_staticText->SetForegroundColour(p_color);
  m_textCtrl->SetForegroundColour(p_color);
  return true;
}

bool EditableLabelText::SetBackgroundColour(const wxColour &p_color)
{
  m_staticText->SetBackgroundColour(p_color);
  m_textCtrl->SetBackgroundColour(p_color);
  return true;
}

bool EditableLabelText::SetFont(const wxFont &p_font)
{
  m_staticText->SetFont(p_font);
  m_textCtrl->SetFont(p_font);
  return true;
}

void EditableLabelText::OnClick(wxCommandEvent &)
{
  wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED);
  event.SetId(GetId());
  wxPostEvent(GetParent(), event);
}

void EditableLabelText::OnAccept(wxCommandEvent &)
{
  EndEdit(true);
  wxCommandEvent event(wxEVT_COMMAND_TEXT_ENTER);
  event.SetId(GetId());
  wxPostEvent(GetParent(), event);
}
} // namespace Gambit::GUI
