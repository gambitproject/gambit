//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a node
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#ifndef DIALOG_NODE_H
#define DIALOG_NODE_H

class gbtNodeDialog : public wxDialog {
private:
  gbtGameNode m_node;
  wxTextCtrl *m_nodeLabel;
  wxChoice *m_outcome, *m_infoset;
  gbtBlock<gbtGameInfoset> m_infosetList;

public:
  // Lifecycle
  gbtNodeDialog(wxWindow *p_parent, gbtGame p_game, gbtGameNode p_node);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  std::string GetNodeLabel(void) const 
    { return (const char *) m_nodeLabel->GetValue().mb_str(); }
  int GetOutcome(void) const { return m_outcome->GetSelection(); }
  gbtGameInfoset GetInfoset(void) const;
};

#endif   // DIALOG_NODE_H
