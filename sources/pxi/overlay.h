//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Namespace for storing, setting, and reading/writing overlay options
//

#ifndef OVERLAY_H
#define OVERLAY_H

#include "wx/config.h"
#include "guishare/fontpanel.h"

namespace Overlay {

  typedef enum {
    tokenNUMBER = 0, tokenX = 1, tokenPLUS = 2, tokenCIRCLE = 3,
    tokenDEL = 4, tokenTRIANGLE = 5, tokenSQUARE = 6, tokenSTAR = 7
  } TokenType;
    

  class Properties {
  public:
    TokenType m_token;  
    bool m_lines;   // connect overlay points?
    int m_tokenSize;   // size of token
    wxFont m_font;

    Properties(void);
    
    void ReadConfig(wxConfig &);
    void WriteConfig(wxConfig &) const;
  };

  class PatternsPanel;

  class Dialog : public wxDialog {
  private:
    wxNotebook *m_notebook;
    PatternsPanel *m_patternsPanel;
    FontPanel *m_fontPanel;
    
  public:
    Dialog(wxWindow *p_parent, const Properties &);

    Properties GetProperties(void) const;
  };

}  // namespace Overlay

#endif  // OVERLAY_H
