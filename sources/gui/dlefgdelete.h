//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for deleting moves/subtrees from extensive form
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef DLEFGDELETE_H
#define DLEFGDELETE_H

class dialogEfgDelete : public wxDialog {
private:
  Node *m_node;
  wxRadioBox *m_deleteTree;
  wxListBox *m_branchList;

  void OnDeleteTree(wxCommandEvent &);

public:
  dialogEfgDelete(wxWindow *, Node *);
  virtual ~dialogEfgDelete() { }

  Node *KeepNode(void) const
    { return m_node->GetChild(m_branchList->GetSelection() + 1); }
  bool DeleteTree(void) const 
    { return (m_deleteTree->GetSelection() == 0); }

  DECLARE_EVENT_TABLE()
};

#endif  // DLEFGDELETE_H
