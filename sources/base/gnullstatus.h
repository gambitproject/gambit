//
// FILE: gnullstatus.h -- Declaration of a null status object
//
// $Id$
//

#ifndef GNULLSTATUS_H
#define GNULLSTATUS_H

#include "gstatus.h"

class gNullStatus : public gStatus  {
public:
  virtual ~gNullStatus(void) { }

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
  gOutput &operator<<(long double) { return *this; }
  gOutput &operator<<(float) { return *this; }
  gOutput &operator<<(const char *) { return *this; }
  gOutput &operator<<(const void *) { return *this; }

  bool IsValid(void) const { return true; }
  void SetProgress(double) { }
  void SetProgress(double, const gText &) { }

  void Get(void) const { }
  void Reset(void) { }
};

#endif  // GNULLSTATUS_H
