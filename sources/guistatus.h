//
// FILE: guistatus.h -- Progress/Cancel dialog
//
// $Id$
//

#ifndef GUISTATUS_H
#define GUISTATUS_H

#include "gstatus.h"

class guiStatus : public wxDialog, public gStatus {
protected:
  wxGauge *m_gauge;
  bool m_sig;

  void OnCancel(wxCommandEvent &);

public:
  guiStatus(wxFrame *p_frame, const char *title);
  virtual ~guiStatus();

  int GetWidth(void) const { return 0; }
  gOutput &SetWidth(int) { return *this; }
  int GetPrec(void) const { return 0; }
  gOutput &SetPrec(int) { return *this; }
  gOutput &SetExpMode(void) { return *this; }
  gOutput &SetFloatMode(void) { return *this; }
  char GetRepMode(void) const { return 'f'; }

  gOutput &operator<<(int) { return *this; }
  gOutput &operator<<(unsigned int) { return *this; }
  gOutput &operator<<(bool) { return *this; }
  gOutput &operator<<(long) { return *this; }
  gOutput &operator<<(char) { return *this; }
  gOutput &operator<<(double) { return *this; }
  gOutput &operator<<(float) { return *this; }
  gOutput &operator<<(const char *) { return *this; }
  gOutput &operator<<(const void *) { return *this; }
  
  bool IsValid(void) const { return true; }

  virtual void SetProgress(double p);

  virtual void SetSignal(void) { m_sig = true; }
  virtual void Get(void) const;
  virtual void Reset(void) { m_sig = false; }

  DECLARE_EVENT_TABLE()
};

#endif  // GUISTATUS_H
