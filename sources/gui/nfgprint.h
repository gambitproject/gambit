//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to printouts for normal forms
//

#ifndef NFGPRINT_H
#define NFGPRINT_H

#include "wx/html/htmprint.h"
#include "nfgshow.h"
#include "nfgtable.h"

//
// For normal forms, we will print grids by creating corresponding HTML.
// 
class NfgPrintout : public wxHtmlPrintout {
private:
  NfgTable *m_table;
    
  // Create HTML for game
  wxString BuildHtml(const Nfg &, int, int);

public:
  NfgPrintout(const Nfg &, int, int, const wxString &);
  virtual ~NfgPrintout() { }
};

#endif  // NFGPRINT_H
