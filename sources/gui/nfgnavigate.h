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

class NfgNavigateWindow : public wxPanel {
private:
  NfgShow *m_parent;
  wxStaticText **m_playerNames;
  wxChoice *m_rowChoice, *m_colChoice, **m_stratProfile;

  int m_rowPlayer, m_colPlayer;
  gbtNfgSupport m_support;
  
  void OnStrategyChange(wxCommandEvent &);
  void OnRowPlayerChange(wxCommandEvent &);
  void OnColPlayerChange(wxCommandEvent &);

public:
  NfgNavigateWindow(NfgShow *p_nfgShow, wxWindow *p_parent);
  virtual ~NfgNavigateWindow();
  
  void SetProfile(const gArray<int> &profile);
  gArray<int> GetProfile(void) const;

  void SetStrategy(int p_player, int p_strategy);
  void SetPlayers(int p_rowPlayer, int p_colPlayer);

  int GetRowStrategy(void) const;
  int GetColStrategy(void) const;

  void SetSupport(const gbtNfgSupport &);
  const gbtNfgSupport &GetSupport(void) const { return m_support; }

  int GetRowPlayer(void) const { return m_rowPlayer; }
  int GetColPlayer(void) const { return m_colPlayer; }

  void UpdateLabels(void);

  DECLARE_EVENT_TABLE()
};


#endif  // NFGNAVIGATE_H
