//
// FILE: gsmconsole.h -- implementation of GSM for console-based interface
//
// $Id$
//

#ifndef GSMCONSOLE_H
#define GSMCONSOLE_H

#include "gclstatus.h"
#include "gsm.h"

class gsmConsole : public GSM {
friend class gclStatus;
private:
  static gclStatus s_status;

public:
  gsmConsole(gInput &p_input, gOutput &p_output, gOutput &p_error)
    : GSM(p_input, p_output, p_error) { }
  virtual ~gsmConsole() { }

  static void gclStatusHandler(int);
  gStatus &GetStatusMonitor(void);
};


#endif  // GSMCONSOLE_H
 
