//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to printouts for normal forms
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef NFGPRINT_H
#define NFGPRINT_H

#include "wx/html/htmprint.h"
#include "nfgshow.h"
#include "nfgtable.h"

//
// This creates a string containing a representation of the game
// in HTML
//
wxString gbtBuildHtml(const Nfg &, int, int);

//
// For normal forms, we will print grids by creating corresponding HTML.
// 
class NfgPrintout : public wxHtmlPrintout {
private:
  NfgTable *m_table;

public:
  NfgPrintout(const Nfg &, int, int, const wxString &);
  virtual ~NfgPrintout() { }
};

#endif  // NFGPRINT_H
