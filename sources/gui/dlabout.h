//
// FILE: dlabout.h -- Declaration of "about" dialog class
//
// $Id$
//

#ifndef DLABOUT_H
#define DLABOUT_H

#include "wx/wx.h"

class dialogAbout : public wxDialog {
public:
  dialogAbout(wxWindow *p_parent, const wxString &p_title,
	      const wxString &p_programName, const wxString &p_versionString);
  virtual ~dialogAbout() { }
};

#endif  // DLABOUT_H


