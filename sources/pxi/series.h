//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Namespace for storing, setting, and reading/writing series options
//

#ifndef SERIES_H
#define SERIES_H

#include "wx/config.h"

namespace Series {

  class Properties {
  public:
    bool m_connect;   

    Properties(void);

    void ReadConfig(wxConfig &);
    void WriteConfig(wxConfig &) const;
  };

  class Dialog : public wxDialog {
  private:
    wxCheckBox *m_connect;

  public:
    Dialog(wxWindow *p_parent, const Properties &);

    Properties GetProperties(void) const;
  };

}  // namespace Series


#endif   // SERIES_H
