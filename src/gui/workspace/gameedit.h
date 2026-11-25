//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/workspace/gameedit.h
// Edits to the game made by the graphical interface
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

#ifndef GAMBIT_GUI_WORKSPACE_GAMEEDIT_H
#define GAMBIT_GUI_WORKSPACE_GAMEEDIT_H

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "games.h"
#include "treestyle.h"

namespace Gambit::GUI {

/// The kinds of change made to a workspace, combined as flags
enum class GameModificationType : unsigned int {
  None = 0x00,
  /// The structure of the game (players, strategies, tree) changed
  GameForm = 0x01,
  /// Payoffs changed, but not the structure
  GamePayoffs = 0x02,
  /// Labels of the game or its objects changed
  GameLabels = 0x04,
  /// Analyses saved with the workspace changed
  Workspace = 0x08,
  /// How the game is drawn changed
  Presentation = 0x10,
  All = 0x1f
};

inline GameModificationType operator|(GameModificationType p_left, GameModificationType p_right)
{
  return static_cast<GameModificationType>(static_cast<unsigned int>(p_left) |
                                           static_cast<unsigned int>(p_right));
}

inline GameModificationType operator&(GameModificationType p_left, GameModificationType p_right)
{
  return static_cast<GameModificationType>(static_cast<unsigned int>(p_left) &
                                           static_cast<unsigned int>(p_right));
}

inline GameModificationType &operator|=(GameModificationType &p_left, GameModificationType p_right)
{
  p_left = p_left | p_right;
  return p_left;
}

inline bool HasModification(GameModificationType p_modifications, GameModificationType p_test)
{
  return static_cast<unsigned int>(p_modifications & p_test) != 0;
}

//
// Each edit is a plain description of one change to the game or how it is drawn;
// GameWorkspace::Apply makes it as one undoable step. Labels are UTF-8.
//
// Where a list of items is restructured, `stableLabels` identifies each item as it was before
// the edit (an existing item's label, or a placeholder for a new one) and `labels` is that item's
// label after it. Structure is resolved first, from `stableLabels` alone, and labels are then
// reassigned, so a rename that reuses the label of a deleted item does not collide with it.
//

/// Sets the title and description, restructures and relabels the players (chance excluded), and
/// sets the players' colours. An edit that only relabels players does not restructure, so it also
/// applies to games whose players cannot be changed (e.g. action graph games) and can swap two
/// players' labels.
struct EditGameProperties {
  std::string title;
  std::string description;
  std::vector<std::string> stableLabels;
  std::vector<std::string> labels;
  /// The colour of each player, by position in `labels`
  std::vector<Rgb> playerColors;
  /// The colour of chance; used only for an extensive game
  Rgb chanceColor;
};

/// Replaces how the game is drawn
struct SetTreeStyle {
  TreeStyle style;
};

/// Restructures and relabels the strategies of a player
struct SetStrategies {
  GamePlayer player;
  std::vector<std::string> stableLabels;
  std::vector<std::string> labels;
};

/// Sets the label, player and actions of an information set. The player is changed only for a
/// personal move; the probabilities are used only for a chance move.
struct EditMove {
  GameInfoset infoset;
  std::string infosetLabel;
  GamePlayer player;
  std::vector<std::string> stableLabels;
  std::vector<std::string> labels;
  std::vector<Number> probs;
};

/// Sets the label and outcome of a node and, unless it is terminal, its information set
struct EditNode {
  GameNode node;
  std::string label;
  /// The null outcome removes the node's outcome
  GameOutcome outcome;
  /// The information set to join; null puts the node in an information set of its own
  GameInfoset infoset;
};

/// Refines the information sets that follow an information set, for each of a list of players,
/// by the action taken there
struct RefineInfosets {
  GameInfoset infoset;
  std::vector<GamePlayer> players;
};

/// Places a non-terminal node in an existing information set
struct JoinInfoset {
  GameNode node;
  GameInfoset infoset;
};

/// Appends a move at a terminal node, as a member of an existing information set
struct AppendMoveInInfoset {
  GameNode node;
  GameInfoset infoset;
};

/// Appends a new move for a player at a terminal node. The probabilities are used only for the
/// chance player.
struct AppendMove {
  GameNode node;
  GamePlayer player;
  std::vector<std::string> labels;
  std::vector<Number> probs;
};

/// Inserts a new move for a player before a node, which becomes the first action's subtree. The
/// probabilities are used only for the chance player.
struct InsertMove {
  GameNode node;
  GamePlayer player;
  std::vector<std::string> labels;
  std::vector<Number> probs;
};

/// Copies the subtree at `source` to the terminal node `target`
struct CopyTree {
  GameNode target;
  GameNode source;
};

/// Moves the subtree at `source` to the terminal node `target`
struct MoveTree {
  GameNode target;
  GameNode source;
};

/// Deletes the move before a node, keeping only the node's subtree
struct DeleteParent {
  GameNode node;
};

/// Deletes the subtree below a node
struct DeleteTree {
  GameNode node;
};

/// Assigns the move at a node, with its whole information set, to another personal player
struct SetMovePlayer {
  GameNode node;
  GamePlayer player;
};

/// Attaches an existing outcome to a node
struct SetNodeOutcome {
  GameNode node;
  GameOutcome outcome;
};

/// Removes the outcome from a node
struct RemoveNodeOutcome {
  GameNode node;
};

/// Moves the outcome of `source` to `target`
struct MoveOutcome {
  GameNode target;
  GameNode source;
};

/// Sets the label and payoffs of a node's outcome, creating one if the node has none
struct SetNodeOutcomeData {
  GameNode node;
  std::string label;
  std::vector<Number> payoffs;
};

/// Sets the label and payoffs of an outcome
struct SetOutcomeData {
  GameOutcome outcome;
  std::string label;
  std::vector<Number> payoffs;
};

/// Gives a set of contingencies one shared, newly created outcome
struct SetContingencyOutcome {
  std::vector<PureStrategyProfile> profiles;
  std::string label;
  std::vector<Number> payoffs;
};

/// Removes the outcomes from a set of contingencies; those without one are left as they are
struct RemoveContingencyOutcome {
  std::vector<PureStrategyProfile> profiles;
};

/// Sets one player's payoff at a contingency, creating an outcome if it has none
struct SetContingencyPayoff {
  PureStrategyProfile profile;
  GamePlayer player;
  Number value;
};

using GameEdit =
    std::variant<EditGameProperties, SetStrategies, EditMove, EditNode, RefineInfosets,
                 JoinInfoset, AppendMoveInInfoset, AppendMove, InsertMove, CopyTree, MoveTree,
                 DeleteParent, DeleteTree, SetMovePlayer, SetNodeOutcome, RemoveNodeOutcome,
                 MoveOutcome, SetNodeOutcomeData, SetOutcomeData, SetContingencyOutcome,
                 RemoveContingencyOutcome, SetContingencyPayoff, SetTreeStyle>;

/// Returns why `p_edit` cannot be made on `p_game`, or nothing if it can
std::optional<std::string> CheckEdit(const Game &p_game, const GameEdit &p_edit);

/// Makes `p_edit` on `p_game` and `p_style`; the edit must have passed `CheckEdit`. Returns the
/// kinds of change made (`None` if the edit changes nothing).
GameModificationType PerformEdit(const Game &p_game, TreeStyle &p_style, const GameEdit &p_edit);

/// Returns a label of the form "Outcome N" that no outcome of `p_game` has
std::string GenerateOutcomeLabel(const Game &p_game);

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_WORKSPACE_GAMEEDIT_H
