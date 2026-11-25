//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/style.h
// Class to store settings related to graphical interface styling
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

#ifndef STYLE_H
#define STYLE_H

#include <functional>

#include "games/game.h"
#include "workspace/treestyle.h"

namespace Gambit::GUI {

class AnalysisWorkspace;

/// A function that computes the label text of a node, or of the branch leading to it
using LabelGenerator = std::function<wxString(const GameNode &)>;

/// The generator of node labels of the given kind
LabelGenerator GetNodeLabelGenerator(NodeLabelStyle p_which, const AnalysisWorkspace &p_workspace);
/// The generator of branch labels of the given kind
LabelGenerator GetBranchLabelGenerator(BranchLabelStyle p_which,
                                       const AnalysisWorkspace &p_workspace);

/// Converts a colour to wxWidgets' representation
inline wxColour ToWxColour(const Rgb &p_color)
{
  return {p_color.red, p_color.green, p_color.blue};
}
/// Converts a colour from wxWidgets' representation
inline Rgb ToRgb(const wxColour &p_color)
{
  return {p_color.Red(), p_color.Green(), p_color.Blue()};
}

/// The font for labels drawn in the given style
wxFont GetLabelFont(const TreeStyle &p_style);
/// Describes a font for storing in a style
TreeFont ToTreeFont(const wxFont &p_font);

/// Builds a small solid-color square bitmap, e.g. for use as a player-color swatch
/// on a menu item or button.
wxBitmap MakeColorSwatch(const wxColour &p_color, int p_size = 16);

} // namespace Gambit::GUI

#endif // STYLE_H
