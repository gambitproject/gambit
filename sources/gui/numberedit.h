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

#ifndef NUMBEREDIT_H
#define NUMBEREDIT_H

class NumberEditor : public wxGridCellTextEditor {
private:
  wxString m_valueOld;

protected:
  // string representation of m_valueOld
  wxString GetString() const;

public:
  NumberEditor(void);

  virtual void Create(wxWindow* parent, wxWindowID id,
		      wxEvtHandler* evtHandler);

  virtual bool IsAcceptedKey(wxKeyEvent& event);
  virtual void BeginEdit(int row, int col, wxGrid* grid);
  virtual bool EndEdit(int row, int col, wxGrid* grid);

  virtual void Reset();
  virtual void StartingKey(wxKeyEvent& event);

  virtual wxGridCellEditor *Clone() const
  { return new NumberEditor; }

  // parameters string format is "width,precision"
  virtual void SetParameters(const wxString& params);
};

#endif  // NUMBEREDIT_H

