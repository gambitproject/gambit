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

#include "nfgshow.h"

class NfgNavigateWindow : public wxPanel, public gbtGameView {
private:
  wxStaticText **m_playerNames;
  wxChoice *m_rowChoice, *m_colChoice, **m_stratProfile;

  int m_rowPlayer, m_colPlayer;
  
  void OnStrategyChange(wxCommandEvent &);
  void OnRowPlayerChange(wxCommandEvent &);
  void OnColPlayerChange(wxCommandEvent &);

  bool IsEfgView(void) const { return false; }
  bool IsNfgView(void) const { return true; }
  void OnUpdate(gbtGameView *);
  
public:
  NfgNavigateWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~NfgNavigateWindow();
  
  DECLARE_EVENT_TABLE()
};


#endif  // NFGNAVIGATE_H
