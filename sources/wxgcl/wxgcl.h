//
// FILE: wxgcl.h -- Declaration of wxWindows-based GCL application class
//
// $Id$
//

#ifndef WXGCL_H
#define WXGCL_H

#include "wx/wx.h"
#include "wx/wxhtml.h"   // for wxHtmlHelpController

class GclApp : public wxApp {
private:
  wxHtmlHelpController m_help;

  bool OnInit(void);

public:
  virtual ~GclApp() { }

  wxHtmlHelpController &HelpController(void) { return m_help; }
};

DECLARE_APP(GclApp)

#endif  // WXGCL_H
