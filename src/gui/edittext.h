//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/edittext.h
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

#ifndef EDITTEXT_H
#define EDITTEXT_H

//!
//! A gbtStaticTextButton is a wxStaticText object that generates a
//! button click event when it is left-clicked
//!
class gbtStaticTextButton : public wxStaticText {
private:
  // Event handlers
  void OnLeftClick(wxMouseEvent &);
  
public:
  gbtStaticTextButton(wxWindow *p_parent, int p_id,
		      const wxString &,
		      const wxPoint & = wxDefaultPosition, 
		      const wxSize & = wxDefaultSize,
		      long = 0);

  DECLARE_EVENT_TABLE()
};

//!
//! This control looks like a wxStaticText, but when clicked it shows
//! a wxTextCtrl to edit the value.
//!
class gbtEditableText : public wxPanel {
private:
  gbtStaticTextButton *m_staticText;
  wxTextCtrl *m_textCtrl;

  /// @name Event handlers
  //@{
  /// Called when the static text is clicked
  void OnClick(wxCommandEvent &);
  /// Called when the text control is dismissed via enter
  void OnAccept(wxCommandEvent &);
  //@}

public:
  gbtEditableText(wxWindow *p_parent, int p_id,
		  const wxString &p_value,
		  const wxPoint &p_position,
		  const wxSize &p_size);

  bool IsEditing(void) const { return GetSizer()->IsShown(m_textCtrl); }
  void BeginEdit(void);
  void EndEdit(bool p_accept = true);

  wxString GetValue(void) const;
  void SetValue(const wxString &p_value);

  // @name Overriding wxWindow methods
  //@{
  bool SetForegroundColour(const wxColour &);
  bool SetBackgroundColour(const wxColour &);
  bool SetFont(const wxFont &);
  //@}
}; 

#endif // EDITTEXT_H
