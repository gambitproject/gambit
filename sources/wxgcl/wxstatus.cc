//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Status display window
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
#endif  // WX_PRECOMP
#include "wxstatus.h"

wxStatus::wxStatus(wxWindow *p_parent, const gText &p_caption)
  : wxProgressDialog((char *) p_caption, "", 100, p_parent,
		     wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL |
		     wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME |
		     wxPD_REMAINING_TIME),
    m_width(0), m_prec(6), m_represent('f'), m_sig(false), m_value(0)
{ }

wxStatus::~wxStatus()
{ }

void wxStatus::Get(void) const  
{
  wxProgressDialog *nonconstthis = (wxProgressDialog *) this;
  if (!nonconstthis->Update(m_value)) {
    throw gSignalBreak();
  }
}

gOutput &wxStatus::operator<<(int x)
{ return *this; }

gOutput &wxStatus::operator<<(unsigned int x)
{ return *this; }

gOutput &wxStatus::operator<<(bool x)
{ return *this; }

gOutput &wxStatus::operator<<(long x)
{ return *this; }

gOutput &wxStatus::operator<<(char x)
{ return *this; }

gOutput &wxStatus::operator<<(double x)
{ return *this; }

gOutput &wxStatus::operator<<(long double x)
{ return *this; }

gOutput &wxStatus::operator<<(float x)
{ return *this; }

gOutput &wxStatus::operator<<(const char *x)
{
  Update(m_value, x);
  return *this; 
}

gOutput &wxStatus::operator<<(const void *x)
{ return *this; }

void wxStatus::SetProgress(double p_value)
{
  if (p_value >= 1.0) {
    m_value = 100;
  }
  else if (p_value <= 0.0) {
    m_value = 0;
  }
  else {
    m_value = (int) (p_value * 100.0);
  }
  Update(m_value);
}

void wxStatus::SetProgress(double p_value, const gText &p_message)
{
  if (p_value >= 1.0) {
    m_value = 100;
  }
  else if (p_value <= 0.0) {
    m_value = 0;
  }
  else {
    m_value = (int) (p_value * 100.0);
  }
  Update(m_value, (const char *) p_message);
}

