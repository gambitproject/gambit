//
// FILE: wxstatus.h -- Progress indication / cancel dialog
//
// $Id$
//

#ifndef WXSTATUS_H
#define WXSTATUS_H

#include "wx/wx.h"
#include "wx/progdlg.h"
#include "base/gstatus.h"

class wxStatus : public wxProgressDialog, public gStatus {
protected:
  int m_width, m_prec;
  char m_represent;
  bool m_sig;
  
  int m_value;
  
public:
  wxStatus(wxWindow *, const gText &);
  virtual ~wxStatus();
    
  // functions for gOutput
  int GetWidth(void) const { return m_width; }
  gOutput &SetWidth(int p_width) { m_width = p_width; return *this; }
  int GetPrec(void) const { return m_prec; }
  gOutput &SetPrec(int p_prec)  { m_prec = p_prec; return *this; }
  gOutput &SetExpMode(void) { m_represent = 'e'; return *this; }
  gOutput &SetFloatMode(void) { m_represent = 'f'; return *this; }
  char GetRepMode(void) const { return m_represent; }

  gOutput &operator<<(int x);
  gOutput &operator<<(unsigned int x);
  gOutput &operator<<(bool x);
  gOutput &operator<<(long x);
  gOutput &operator<<(char x);
  gOutput &operator<<(double x);
  gOutput &operator<<(long double x);
  gOutput &operator<<(float x);
  gOutput &operator<<(const char *x);
  gOutput &operator<<(const void *x);

  bool IsValid(void) const { return true; }

  // functions for gProgress
  virtual void SetProgress(double p);
  virtual void SetProgress(double, const gText &);

  // functions for gSignal
  virtual void SetSignal(void) { m_sig = true; }
  virtual void Get(void) const;
  virtual void Reset(void)     { m_sig = false; }
};

#endif  // WXSTATUS_H
