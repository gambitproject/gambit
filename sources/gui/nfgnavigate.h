//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to navigation window for normal forms
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

#ifndef NFGNAVIGATE_H
#define NFGNAVIGATE_H

#include <wx/grid.h>
#include "nfgshow.h"

class gbtNfgNavigate : public wxGrid, public gbtGameView {
private:
  // wxGrid members
  // Overriding this suppresses drawing of the grid cell highlight
  virtual void DrawCellHighlight(wxDC &, const wxGridCellAttr *) { }

  // Event handlers
  void OnLeftClick(wxGridEvent &);

  bool IsEfgView(void) const { return false; }
  bool IsNfgView(void) const { return true; }
  void OnUpdate(gbtGameView *);
  
public:
  gbtNfgNavigate(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~gbtNfgNavigate();
  
  DECLARE_EVENT_TABLE()
};


#endif  // NFGNAVIGATE_H
