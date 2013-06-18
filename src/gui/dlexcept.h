//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlexcept.h
// Declaration of internal exception error dialog.
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

#ifndef DLEXCEPT_H
#define DLEXCEPT_H

#include <wx/wx.h>

//
// A general-purpose dialog box to display the description of an internal
// exception in Gambit.
//
class gbtExceptionDialog : public wxMessageDialog  {
public:
  gbtExceptionDialog(wxWindow *p_parent, const std::string &p_message)
    : wxMessageDialog(p_parent, 
		      wxString(p_message.c_str(), *wxConvCurrent), 
		      wxT("Internal exception in Gambit"),
		      wxICON_ERROR | wxCANCEL) 
    { }
};

#endif  // DLEXCEPT_H

