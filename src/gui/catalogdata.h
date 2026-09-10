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

/// One entry from catalog/games/manifest.json, describing a single catalog game.
struct CatalogEntry {
  wxString slug; // e.g. "books/watson2013/exercise29_6"; also its path under the
                 // catalog root, without a format extension
  wxString title;
  wxString description; // citation markup already resolved to plain text by catalog/build.py
  wxString format;      // "efg" or "nfg"
  wxString category;    // top-level grouping, e.g. "Books"
  wxString group;       // immediate grouping, e.g. "Watson (2013) -- Strategy: ..."
  wxString thumbnail;   // path to a preview image, relative to the catalog root
  int numPlayers = 0;
  bool isTree = false;
  bool isConstSum = false;
  int numStrategies = 0;
};

/// Locates the root directory of the games catalog (the directory containing
/// manifest.json, the game files, and img/), or returns an empty string if none can be
/// found. Checks a co-located source checkout first (so a developer running the GUI
/// straight out of the build tree sees live catalog edits with no packaging step needed),
/// then falls back to the platform-specific bundled/installed location.
wxString FindCatalogRoot();

/// Parses manifest.json from catalogRoot, returning one CatalogEntry per game. On failure
/// (missing or malformed file), returns an empty vector and sets *error to a message
/// suitable for display to the user; on success, *error (if non-null) is cleared.
std::vector<CatalogEntry> LoadCatalogManifest(const wxString &catalogRoot,
                                              wxString *error = nullptr);

/// Returns the full path to entry's game file under catalogRoot (e.g.
/// "<catalogRoot>/books/watson2013/exercise29_6.efg"), ready to pass to
/// Application::LoadFile.
wxString CatalogGameFilePath(const wxString &catalogRoot, const CatalogEntry &entry);

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_CATALOGDATA_H
