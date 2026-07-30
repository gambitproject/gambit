//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/workspace.h
// Reader and writer for Gambit's legacy .gbt workspace format
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

#ifndef GAMBIT_GAMES_WORKSPACE_H
#define GAMBIT_GAMES_WORKSPACE_H

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace Gambit {

struct LegacyWorkspaceFile {
  struct Color {
    int player{-2};
    int red{0};
    int green{0};
    int blue{0};
  };

  struct Font {
    int size{10};
    int family{0};
    std::string face;
    int style{0};
    int weight{0};
  };

  struct Layout {
    int node_size{10};
    int terminal_spacing{50};
    std::string chance_token{"dot"};
    std::string player_token{"dot"};
    std::string terminal_token{"dot"};
    int branch_length{60};
    int tine_length{20};
    std::string branch_style{"forktine"};
    std::string branch_labels{"horizontal"};
    std::string infoset_style{"circles"};
  };

  struct Labels {
    std::string node_above{"label"};
    std::string node_below{"isetid"};
    std::string branch_above{"label"};
    std::string branch_below{"probs"};
  };

  struct Profile {
    std::string type;
    std::string probabilities;
  };

  struct Analysis {
    std::string description;
    std::vector<Profile> profiles;
  };

  std::vector<Color> colors;
  std::optional<Font> font;
  std::optional<Layout> layout;
  std::optional<Labels> labels;
  std::optional<int> decimals;
  std::string game_format;
  std::string game;
  std::vector<Analysis> analyses;
};

/// Read a legacy .gbt workspace. Throws std::runtime_error if malformed.
LegacyWorkspaceFile ReadLegacyWorkspace(std::istream &p_stream);

/// Write a legacy .gbt workspace in the canonical format.
void WriteLegacyWorkspace(std::ostream &p_stream, const LegacyWorkspaceFile &p_workspace);

} // namespace Gambit

#endif // GAMBIT_GAMES_WORKSPACE_H
