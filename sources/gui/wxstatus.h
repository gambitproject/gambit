//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Progress indication window
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

#ifndef WXSTATUS_H
#define WXSTATUS_H

#include <wx/progdlg.h>
#include "base/gstatus.h"
#include "gambit.h"

class gbtProgressDialog : public wxProgressDialog, public gbtStatus {
protected:
  int m_width, m_prec;
  char m_represent;
  bool m_sig;
  wxString m_buffer;
  
  int m_value;
  
public:
  gbtProgressDialog(wxWindow *, const gbtText &);
  virtual ~gbtProgressDialog();
    
  // functions for gbtOutput
  int GetWidth(void) const { return m_width; }
  gbtOutput &SetWidth(int p_width) { m_width = p_width; return *this; }
  int GetPrec(void) const { return m_prec; }
  gbtOutput &SetPrec(int p_prec)  { m_prec = p_prec; return *this; }
  gbtOutput &SetExpMode(void) { m_represent = 'e'; return *this; }
  gbtOutput &SetFloatMode(void) { m_represent = 'f'; return *this; }
  char GetRepMode(void) const { return m_represent; }

  gbtOutput &operator<<(int x);
  gbtOutput &operator<<(unsigned int x);
  gbtOutput &operator<<(bool x);
  gbtOutput &operator<<(long x);
  gbtOutput &operator<<(char x);
  gbtOutput &operator<<(double x);
  gbtOutput &operator<<(float x);
  gbtOutput &operator<<(const char *x);
  gbtOutput &operator<<(const void *x);

  bool IsValid(void) const { return true; }

  // functions for gbtProgress
  virtual void SetProgress(double p);
  virtual void SetProgress(double, const gbtText &);

  // functions for gbtSignal
  virtual void SetSignal(void) { m_sig = true; }
  virtual void Get(void) const;
  virtual void Reset(void)     { m_sig = false; }
};

#endif  // WXSTATUS_H
