//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/catalogdata.h
// Loading and locating Gambit's games catalog for the catalog browser dialog
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

#ifndef GAMBIT_GUI_CATALOGDATA_H
#define GAMBIT_GUI_CATALOGDATA_H

#include <wx/string.h>
#include <vector>

namespace Gambit::GUI {

/// The manifest schema version this build of the GUI reads.
constexpr int CATALOG_SCHEMA_VERSION = 0;

/// One game from the catalog's manifest.json.
struct CatalogEntry {
  wxString slug; ///< e.g. "books/myerson1991/fig2_1"
  wxString title;
  wxString description;
  wxString format;   ///< "efg" or "nfg"
  wxString game;     ///< Path of the game file, relative to the games directory
  wxString category; ///< Top-level grouping, e.g. "Books"
  wxString group;    ///< Immediate grouping, e.g. a book's title
  int numPlayers = 0;
  bool isConstSum = false;
  int numStrategies = 0;
};

/// The games of one catalog release.
struct Catalog {
  wxString version;
  std::vector<CatalogEntry> entries;
};

/// Locates the games directory of the catalog (the one holding manifest.json), or
/// returns an empty string if there is none.
wxString FindCatalogRoot();

/// Reads manifest.json from the games directory catalogRoot. On failure returns no
/// entries and sets *error to a message for the user; on success clears *error.
Catalog LoadCatalogManifest(const wxString &catalogRoot, wxString *error = nullptr);

/// Returns the full path of entry's game file.
wxString CatalogGameFilePath(const wxString &catalogRoot, const CatalogEntry &entry);

/// Returns the full path of entry's thumbnail image, which may not exist.
wxString CatalogThumbnailPath(const wxString &catalogRoot, const CatalogEntry &entry);

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_CATALOGDATA_H
