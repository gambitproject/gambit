//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to show QRE correspondence and optionally write PXI file
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

#ifndef DLQREFILE_H
#define DLQREFILE_H

#include "wx/listctrl.h"

//
// Keeping both types of solution here is more than a bit kludgy;
// need a much better way of doing this.
// Really, this should evolve into a flexible dialog for listing
// and selecting profiles, and the .pxifile exporting stuff should
// be moved to a set of qretools
//
class dialogQreFile : public wxDialog {
private:
  wxListCtrl *m_qreList;
  gList<MixedSolution> m_mixedProfiles;
  gList<BehavSolution> m_behavProfiles;

  // Event handlers
  void OnPxiFile(wxCommandEvent &);

public:
  dialogQreFile(wxWindow *, const gList<MixedSolution> &);
  dialogQreFile(wxWindow *, const gList<BehavSolution> &);

  DECLARE_EVENT_TABLE()
};

#endif   // DLQREFILE_H



