//
// FILE: wxgcl.h -- Declaration of wxWindows-based GCL application class
//
// $Id$
//

#ifndef WXGCL_H
#define WXGCL_H

#include "wx/wx.h"

class GclApp : public wxApp {
private:
  bool OnInit(void);

public:
  virtual ~GclApp() { }
};

DECLARE_APP(GclApp)

#endif  // WXGCL_H
