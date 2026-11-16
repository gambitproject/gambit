//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlcatalog.h
// Dialog for browsing and opening a game from Gambit's games catalog
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

#ifndef GAMBIT_GUI_DLCATALOG_H
#define GAMBIT_GUI_DLCATALOG_H

#include <wx/dialog.h>

#include "catalogdata.h"

class wxSearchCtrl;
class wxChoice;
class wxDataViewListCtrl;
class wxDataViewEvent;
class wxStaticText;
class wxStaticBitmap;
class wxTextCtrl;
class wxButton;

namespace Gambit::GUI {

/// Lets the user search, filter, and preview games from Gambit's catalog, and pick one to
/// open. On wxID_OK, GetSelectedFilePath() returns the file to pass to
/// Application::LoadFile(); if the catalog could not be located or its manifest could not
/// be parsed, the dialog shows that error in place of the browser and disables opening.
class CatalogBrowserDialog final : public wxDialog {
public:
  explicit CatalogBrowserDialog(wxWindow *p_parent);

  /// Valid only after ShowModal() has returned wxID_OK.
  wxString GetSelectedFilePath() const;

private:
  wxString m_catalogRoot;
  std::vector<CatalogEntry> m_allEntries;
  std::vector<size_t> m_visibleIndices; // indices into m_allEntries, in current row order
  wxString m_selectedFilePath;

  wxSearchCtrl *m_searchCtrl = nullptr;
  wxChoice *m_formatFilter = nullptr;
  wxDataViewListCtrl *m_list = nullptr;
  wxStaticText *m_detailTitle = nullptr;
  wxStaticBitmap *m_detailThumbnail = nullptr;
  wxTextCtrl *m_detailDescription = nullptr;
  wxButton *m_openButton = nullptr;

  void CreateControls();
  void ApplyFilter();
  void UpdateDetailsPane();
  int GetSelectedEntryIndex() const;

  void OnSearchText(wxCommandEvent &p_event);
  void OnFormatFilter(wxCommandEvent &p_event);
  void OnSelectionChanged(wxDataViewEvent &p_event);
  void OnItemActivated(wxDataViewEvent &p_event);
  void OnOpen(wxCommandEvent &p_event);
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLCATALOG_H
