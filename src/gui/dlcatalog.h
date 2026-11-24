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

#include <optional>

#include <wx/string.h>

class wxWindow;

namespace Gambit::GUI {

/// A game chosen from the catalog.
struct CatalogChoice {
  wxString path, slug, version;
};

/// Loads the catalog of games and lets the user choose a game from it. Returns nothing if
/// the user cancels or the catalog cannot be loaded, in which case the user has been told
/// why.
std::optional<CatalogChoice> ChooseCatalogGame(wxWindow *p_parent);

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLCATALOG_H
