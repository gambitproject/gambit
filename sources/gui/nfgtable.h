//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Panel to display normal form games in tabular format
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

#ifndef NFGTABLE_H
#define NFGTABLE_H

#include "wx/grid.h"

class NfgTable : public wxPanel, public gbtGameView {
private:
  wxGrid *m_grid;
  bool m_editable;
  int m_showProb, m_showDom, m_showValue;

  // Event handlers
  void OnLeftClick(wxGridEvent &);
  void OnLeftDoubleClick(wxGridEvent &);
  void OnLabelLeftClick(wxGridEvent &);

  // Overriding view members
  bool IsEfgView(void) const { return false; }
  bool IsNfgView(void) const { return true; }
  void OnUpdate(gbtGameView *);

public:
  NfgTable(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~NfgTable() { }

  void ToggleProbs(void);
  int ShowProbs(void) const { return m_showProb; }
  void ToggleDominance(void);
  int ShowDominance(void) const { return m_showDom; }
  void ToggleValues(void);
  int ShowValues(void) const { return m_showValue; }

  bool IsEditable(void) const { return m_editable; }
  void SetEditable(bool p_editable) { m_editable = p_editable; }

  DECLARE_EVENT_TABLE()
};

#endif  // NFGTABLE_H






