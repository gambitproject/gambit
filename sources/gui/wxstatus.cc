//
// FILE: wxstatus.cc -- Progress indication / cancel dialog
//
// $Id$
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
  m_value = (int) (p_value * 100);
  Update(p_value * 100);
}

void wxStatus::SetProgress(double p_value, const gText &p_message)
{
  m_value = (int) (p_value * 100);
  Update(p_value * 100, (const char *) p_message);
}

