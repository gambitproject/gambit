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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include "wxstatus.h"

gbtProgressDialog::gbtProgressDialog(wxWindow *p_parent, const gbtText &p_caption)
  : wxProgressDialog(wxString::Format(wxT("%s"), (char *) p_caption),
		     wxT(""), 100, p_parent,
		     wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL |
		     wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME |
		     wxPD_REMAINING_TIME),
    m_width(0), m_prec(6), m_represent('f'), m_sig(false), m_value(0)
{ }

gbtProgressDialog::~gbtProgressDialog()
{ }

void gbtProgressDialog::Get(void) const  
{
  wxProgressDialog *nonconstthis = (wxProgressDialog *) this;
  if (!nonconstthis->Update(m_value)) {
    throw gbtSignalBreak();
  }
  wxGetApp().Yield();
}

gbtOutput &gbtProgressDialog::operator<<(int x)
{
  m_buffer += wxString::Format(wxT("%d"), x);
  wxGetApp().Yield();
  return *this;
}

gbtOutput &gbtProgressDialog::operator<<(unsigned int x)
{ 
  m_buffer += wxString::Format(wxT("%d"), x);
  wxGetApp().Yield();
  return *this;
}

gbtOutput &gbtProgressDialog::operator<<(bool x)
{
  if (x) {
    m_buffer += wxT("True");
  }
  else {
    m_buffer += wxT("False");
  }
  wxGetApp().Yield();
  return *this;
}

gbtOutput &gbtProgressDialog::operator<<(long x)
{
  m_buffer += wxString::Format(wxT("%ld"), x);
  wxGetApp().Yield();
  return *this;
}

gbtOutput &gbtProgressDialog::operator<<(char x)
{
  m_buffer += wxString::Format(wxT("%c"), x);
  wxGetApp().Yield();
  return *this;
}

gbtOutput &gbtProgressDialog::operator<<(double x)
{
  m_buffer += wxString::Format(wxT("%lf"), x);
  wxGetApp().Yield();
  return *this;
}

gbtOutput &gbtProgressDialog::operator<<(long double x)
{
  m_buffer += wxString::Format(wxT("%Lf"), x);
  wxGetApp().Yield();
  return *this;
}

gbtOutput &gbtProgressDialog::operator<<(float x)
{
  m_buffer += wxString::Format(wxT("%f"), x);
  wxGetApp().Yield();
  return *this;
}

gbtOutput &gbtProgressDialog::operator<<(const char *x)
{
  m_buffer += wxString::Format(wxT("%s"), x);
  while (m_buffer.Contains("\n")) {
    wxLogVerbose(m_buffer.Left(m_buffer.Find("\n")));
    m_buffer = m_buffer.Mid(m_buffer.Find("\n") + 1);
  }
  wxGetApp().Yield();
  return *this; 
}

gbtOutput &gbtProgressDialog::operator<<(const void *x)
{
  m_buffer += wxString::Format(wxT("%p"), x);
  wxGetApp().Yield();
  return *this;
}

void gbtProgressDialog::SetProgress(double p_value)
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
  wxGetApp().Yield();
}

void gbtProgressDialog::SetProgress(double p_value, const gbtText &p_message)
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
  Update(m_value, wxString::Format(wxT("%s"), (const char *) p_message));
  wxGetApp().Yield();
}
