//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// wxWindows grid cell editor specialized to handle entry of 
// floating-point/rational numbers
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

#include <ctype.h>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/grid.h"
#include "numberedit.h"

gbtNumberEditor::gbtNumberEditor(void)
{ }

void gbtNumberEditor::Create(wxWindow* parent,
			  wxWindowID id,
			  wxEvtHandler* evtHandler)
{
  wxGridCellTextEditor::Create(parent, id, evtHandler);
}

void gbtNumberEditor::BeginEdit(int row, int col, wxGrid* grid)
{
  // first get the value
  wxGridTableBase *table = grid->GetTable();
  m_valueOld = table->GetValue(row, col);

  DoBeginEdit(GetString());
  ((wxTextCtrl *) GetControl())->SetSelection(0, 1000);
}

bool gbtNumberEditor::EndEdit(int row, int col, wxGrid* grid)
{
  grid->GetTable()->SetValue(row, col, Text()->GetValue());
  return true;
}

void gbtNumberEditor::Reset()
{
  DoReset(GetString());
}

void gbtNumberEditor::StartingKey(wxKeyEvent &event)
{
  int keycode = (int)event.KeyCode();
  if (isdigit(keycode) ||
      keycode == '+' || keycode == '-' || keycode == '.') {
    Text()->SetValue(wxString::Format(wxT("%c"), keycode));
    Text()->SetInsertionPointEnd();
    return;
  }
  event.Skip();
}

void gbtNumberEditor::SetParameters(const wxString &/*params*/)
{ }

wxString gbtNumberEditor::GetString(void) const
{
  return m_valueOld;
}

bool gbtNumberEditor::IsAcceptedKey(wxKeyEvent& event)
{
  if (wxGridCellEditor::IsAcceptedKey(event)) {
    int keycode = event.GetKeyCode();
    switch (keycode) {
    case WXK_NUMPAD0:
    case WXK_NUMPAD1:
    case WXK_NUMPAD2:
    case WXK_NUMPAD3:
    case WXK_NUMPAD4:
    case WXK_NUMPAD5:
    case WXK_NUMPAD6:
    case WXK_NUMPAD7:
    case WXK_NUMPAD8:
    case WXK_NUMPAD9:
    case WXK_ADD:
    case WXK_NUMPAD_ADD:
    case WXK_SUBTRACT:
    case WXK_NUMPAD_SUBTRACT:
    case WXK_DECIMAL:
    case WXK_NUMPAD_DECIMAL:
      return true;

    default:
      if ((keycode < 128) &&
	  (isdigit(keycode) || 
	   keycode == '-' || keycode == '/' || keycode == '.')) {
	return true;
      }
    }
  }

  return false;
}
