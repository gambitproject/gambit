//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/gtdraw.cc
// Reading and writing games in gtdraw's drawing format
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

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>

#include "games.h"
#include "gametree.h"
#include "layout.h"
#include "fileutils.h"

namespace Gambit {

namespace {

// The format is documented in gtdraw's docs/ef_format.md.  A drawing restates a game:
// players, the moves along each history, the information sets, chance probabilities and
// payoffs.  Only the reader checks positions, for syntax, and drops them.

constexpr int kGtdrawMaxPlayers = 4;

//=========================================================================
//                  Writer
//=========================================================================

// Labels in gtdraw's format contain no spaces; "~" is a non-breaking space in TeX
std::string GtdrawText(std::string p_text)
{
  std::ranges::replace(p_text, ' ', '~');
  return p_text;
}

std::string FormatGtdrawReal(const double p_value)
{
  std::ostringstream s;
  s << std::fixed << std::setprecision(12) << p_value;
  auto text = s.str();
  text.erase(text.find_last_not_of('0') + 1);
  if (text.back() == '.') {
    text.pop_back();
  }
  return (text == "-0") ? "0" : text;
}

// A chance move's probability, which gtdraw reads as the last "~"-separated part of its label
std::string GtdrawProbability(const std::string &p_probability)
{
  const auto slash = p_probability.find('/');
  if (slash == std::string::npos) {
    return "~" + p_probability;
  }
  return "~(\\frac{" + p_probability.substr(0, slash) + "}{" + p_probability.substr(slash + 1) +
         "})";
}

// The depth of a node in the drawing, from its level and sublevel in the layout
double GtdrawLevel(const int p_level, const int p_sublevel, const GtdrawOptions &p_options)
{
  const double levelStep = 4 * p_options.levelScaling;
  const double sublevelStep = 2 * p_options.sublevelScaling;
  const double depth = p_level * levelStep;
  return depth + ((p_sublevel != 0) ? (p_sublevel - 1) * sublevelStep : 0.0);
}

int GtdrawPlayerId(const GamePlayer &p_player)
{
  return p_player->IsChance() ? 0 : p_player->GetNumber();
}

// What the drawing needs to know about the tree as a whole.  Nodes are numbered in
// preorder, from 1.
struct GtdrawTree {
  std::shared_ptr<Layout> layout;
  std::vector<GameNode> nodes;
  std::map<GameNode, int> ids;
  std::map<GameInfoset, std::vector<int>> isetMembers;
  std::vector<GameInfoset> isets; // in order of first appearance
  int highestLevel{0};
  int highestSublevel{0};
  double midpoint{0};
};

GtdrawTree SurveyGtdrawTree(const Game &p_game)
{
  GtdrawTree tree;
  tree.layout = CreateLayout(p_game);
  for (const auto &node : p_game->GetNodes()) {
    tree.nodes.push_back(node);
    tree.ids[node] = static_cast<int>(tree.nodes.size());
    if (const auto infoset = node->GetInfoset()) {
      if (!tree.isetMembers.contains(infoset)) {
        tree.isets.push_back(infoset);
      }
      tree.isetMembers[infoset].push_back(tree.ids[node]);
    }
    tree.highestLevel = std::max(tree.highestLevel, tree.layout->GetNodeLevel(node));
    tree.highestSublevel = std::max(tree.highestSublevel, tree.layout->GetNodeSublevel(node));
  }
  tree.midpoint = (tree.layout->GetMinOffset() + tree.layout->GetMaxOffset()) / 2;
  return tree;
}

// A node is drawn below its parent
double GtdrawNodeLevel(const GtdrawTree &p_tree, const GtdrawOptions &p_options,
                       const GameNode &p_node)
{
  const auto &layout = *p_tree.layout;
  double level =
      (p_node->IsTerminal() && p_options.sharedTerminalDepth)
          ? GtdrawLevel(p_tree.highestLevel, p_tree.highestSublevel, p_options)
          : GtdrawLevel(layout.GetNodeLevel(p_node), layout.GetNodeSublevel(p_node), p_options);
  const double levelStep = 4 * p_options.levelScaling;
  if (const auto parent = p_node->GetParent(); parent && levelStep > 0) {
    const double parentLevel =
        GtdrawLevel(layout.GetNodeLevel(parent), layout.GetNodeSublevel(parent), p_options);
    while (level <= parentLevel) {
      level += levelStep;
    }
  }
  return level;
}

double GtdrawShift(const GtdrawTree &p_tree, const GtdrawOptions &p_options,
                   const GameNode &p_node)
{
  const auto offset = [&](const GameNode &n) {
    return (p_tree.layout->GetNodeOffset(n) - p_tree.midpoint) * 2 * p_options.widthScaling;
  };
  const auto parent = p_node->GetParent();
  return offset(p_node) - (parent ? offset(parent) : 0.0);
}

// The move leading to a node, with the probability if it is a chance move
std::string GtdrawMove(const GameNode &p_node)
{
  const auto action = p_node->GetPriorAction();
  auto move = GtdrawText(action->GetLabel());
  if (p_node->GetParent()->GetPlayer()->IsChance()) {
    move += GtdrawProbability(std::string(action->GetInfoset()->GetActionProb(action)));
  }
  return move;
}

std::string GtdrawPayoffs(const Game &p_game, const GameNode &p_node)
{
  std::string payoffs = "payoffs";
  if (const auto outcome = p_node->GetOutcome(); !outcome->IsNull()) {
    for (const auto &player : p_game->GetPlayers()) {
      payoffs += " " + outcome->GetPayoff<std::string>(player);
    }
  }
  return payoffs;
}

std::string GtdrawNodeLine(const Game &p_game, const GtdrawTree &p_tree,
                           const GtdrawOptions &p_options, const GameNode &p_node)
{
  const double level = GtdrawNodeLevel(p_tree, p_options, p_node);
  std::ostringstream f;
  f << "level " << FormatGtdrawReal(level) << " node " << p_tree.ids.at(p_node);
  // A player in a larger information set is given by its iset line
  if (p_node->GetInfoset() && p_tree.isetMembers.at(p_node->GetInfoset()).size() == 1) {
    f << " player " << GtdrawPlayerId(p_node->GetPlayer());
  }
  if (level > 0) {
    f << " xshift " << FormatGtdrawReal(GtdrawShift(p_tree, p_options, p_node));
  }
  if (const auto parent = p_node->GetParent()) {
    f << " from " << p_tree.ids.at(parent) << " move " << GtdrawMove(p_node);
  }
  if (p_node->IsTerminal()) {
    f << " " << GtdrawPayoffs(p_game, p_node);
  }
  return f.str();
}

//=========================================================================
//                  Reader
//=========================================================================

[[noreturn]] void GtdrawError(const int p_line, const std::string &p_message)
{
  throw InvalidFileException("line " + std::to_string(p_line) + ": " + p_message);
}

std::vector<std::string> SplitWords(const std::string &p_text)
{
  std::vector<std::string> words;
  std::string word;
  for (const char c : p_text) {
    if (!IsAsciiSpace(c)) {
      word.push_back(c);
    }
    else if (!word.empty()) {
      words.push_back(word);
      word.clear();
    }
  }
  if (!word.empty()) {
    words.push_back(word);
  }
  return words;
}

std::vector<std::string> SplitOn(const std::string &p_text, const char p_separator)
{
  std::vector<std::string> parts{""};
  for (const char c : p_text) {
    if (c == p_separator) {
      parts.emplace_back();
    }
    else {
      parts.back().push_back(c);
    }
  }
  return parts;
}

bool AllDigits(const std::string &p_text)
{
  return !p_text.empty() && std::ranges::all_of(p_text, IsAsciiDigit);
}

bool AllZeros(const std::string &p_text)
{
  return std::ranges::all_of(p_text, [](const char c) { return c == '0'; });
}

/// A number as the format allows it, or nullopt: an integer, a decimal, a/b or \frac{a}{b},
/// with an optional leading minus and optional surrounding dollar signs.  The value
/// returned is the text in the form Gambit reads numbers.
std::optional<std::string> GtdrawNumber(std::string p_text)
{
  if (p_text.size() >= 2 && p_text.front() == '$' && p_text.back() == '$') {
    p_text = p_text.substr(1, p_text.size() - 2);
  }
  const bool negative = !p_text.empty() && p_text.front() == '-';
  if (negative) {
    p_text.erase(0, 1);
  }
  if (p_text.starts_with("\\frac{") && p_text.ends_with('}')) {
    const auto middle = p_text.find("}{");
    if (middle == std::string::npos) {
      return std::nullopt;
    }
    p_text = p_text.substr(6, middle - 6) + "/" + p_text.substr(middle + 2);
    p_text.pop_back();
  }
  if (const auto slash = p_text.find('/'); slash != std::string::npos) {
    const auto numerator = p_text.substr(0, slash);
    const auto denominator = p_text.substr(slash + 1);
    if (!AllDigits(numerator) || !AllDigits(denominator) || AllZeros(denominator)) {
      return std::nullopt;
    }
  }
  else {
    const auto point = p_text.find('.');
    const auto whole = p_text.substr(0, point);
    const auto fraction = (point == std::string::npos) ? "" : p_text.substr(point + 1);
    const bool digits = std::ranges::all_of(whole, IsAsciiDigit) &&
                        std::ranges::all_of(fraction, IsAsciiDigit) &&
                        !(whole.empty() && fraction.empty());
    if (!digits || (point == std::string::npos && whole.empty())) {
      return std::nullopt;
    }
  }
  return (negative ? "-" : "") + p_text;
}

std::string RequireGtdrawNumber(const int p_line, const std::string &p_text,
                                const std::string &p_what)
{
  if (const auto number = GtdrawNumber(p_text)) {
    return *number;
  }
  GtdrawError(p_line, p_what + " '" + p_text +
                          "' is not a number the format allows (an integer, a decimal, a/b or "
                          "\\frac{a}{b})");
}

/// A real number in full, as used for levels and shifts
std::optional<double> GtdrawReal(const std::string &p_text)
{
  std::istringstream stream(p_text);
  stream.imbue(std::locale::classic());
  double value = 0;
  stream >> value;
  if (stream.fail() || stream.peek() != std::istringstream::traits_type::eof() ||
      !std::isfinite(value)) {
    return std::nullopt;
  }
  return value;
}

std::string RequireGtdrawReal(const int p_line, const std::string &p_text,
                              const std::string &p_what)
{
  if (!GtdrawReal(p_text)) {
    GtdrawError(p_line, p_what + " must be a number, not '" + p_text + "'");
  }
  return p_text;
}

/// A level in the form that identifies a node in EF 0, where "2" and "2.0" are one level
std::string CanonicalLevel(const double p_level)
{
  if (p_level == std::floor(p_level) && std::abs(p_level) < 1e15) {
    return std::to_string(static_cast<long long>(p_level));
  }
  std::ostringstream stream;
  stream.imbue(std::locale::classic());
  stream << std::setprecision(15) << p_level;
  return stream.str();
}

int GtdrawPlayerNumber(const int p_line, const std::string &p_text)
{
  if (!AllDigits(p_text) || p_text.size() > 3 || std::stoi(p_text) > kGtdrawMaxPlayers) {
    GtdrawError(p_line, "player must be a number from 0 to " + std::to_string(kGtdrawMaxPlayers) +
                            ", not '" + p_text + "'");
  }
  return std::stoi(p_text);
}

struct GtdrawNode {
  int line{0};
  std::string id;
  std::optional<std::string> from;
  int player{-1}; // -1: none given
  std::optional<std::string> move;
  std::vector<std::string> payoffs;
  int parent{-1};
  std::vector<int> children;
  int group{-1};
};

struct GtdrawIsetLine {
  int line{0};
  int player{-1};
  std::vector<std::string> refs;
};

/// The nodes that share an information set; a decision node alone is a group of one
struct GtdrawGroup {
  int line{0};
  int player{-1};
  std::vector<int> members; // in the order of the file
};

struct GtdrawFile {
  int version{1}; // the EF version
  std::map<int, std::string> playerNames;
  std::vector<GtdrawNode> nodes;
  std::map<std::string, int> index;
  std::vector<GtdrawIsetLine> isetLines;
  std::vector<GtdrawGroup> groups;
  std::set<std::string> shifts; // the xshift variables defined so far
  int root{-1};
  int numPlayers{0};
};

/// EF 0 if any 'from' reference on a level line has a comma, as gtdraw detects it
int GtdrawVersion(const std::vector<std::pair<int, std::vector<std::string>>> &p_lines)
{
  for (const auto &[line, words] : p_lines) {
    if (words.size() >= 4 && words[0] == "level") {
      const auto from = std::ranges::find(words, std::string("from"));
      if (from != words.end() && std::next(from) != words.end() &&
          std::next(from)->find(',') != std::string::npos) {
        return 0;
      }
    }
  }
  return 1;
}

/// A reference to a node: the name in EF 1, level,name in EF 0
std::string GtdrawRef(const GtdrawFile &p_file, const int p_line, const std::string &p_text)
{
  if (p_file.version == 1) {
    return p_text;
  }
  const auto comma = p_text.find(',');
  const auto level = GtdrawReal(p_text.substr(0, comma));
  if (comma == std::string::npos || !level) {
    GtdrawError(p_line, "node reference '" + p_text + "' must have the form level,node in EF 0");
  }
  return CanonicalLevel(*level) + "," + p_text.substr(comma + 1);
}

bool IsGtdrawName(const std::string &p_text)
{
  return !p_text.empty() && !IsAsciiDigit(p_text.front()) &&
         std::ranges::all_of(p_text, [](const char c) {
           return IsAsciiDigit(c) || std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_';
         });
}

/// Checks an xshift value ([-][n][name][=number]) and records the variable it defines
void CheckGtdrawShift(GtdrawFile &p_file, const int p_line, std::string p_text)
{
  const auto fail = [&] { GtdrawError(p_line, "xshift '" + p_text + "' is not valid"); };
  std::string value = p_text;
  if (value.starts_with('-')) {
    value.erase(0, 1);
  }
  const auto equals = value.find('=');
  const auto target = value.substr(0, equals);
  const auto prefix = target.find_first_not_of("0123456789.");
  const auto coefficient = target.substr(0, prefix);
  const auto name = (prefix == std::string::npos) ? "" : target.substr(prefix);
  if (!coefficient.empty() && !GtdrawReal(coefficient)) {
    fail();
  }
  if (!name.empty() && !IsGtdrawName(name)) {
    fail();
  }
  if (equals != std::string::npos) {
    if (name.empty() || !GtdrawReal(value.substr(equals + 1))) {
      fail();
    }
    p_file.shifts.insert(name);
  }
  else if (name.empty() ? coefficient.empty() : !p_file.shifts.contains(name)) {
    GtdrawError(p_line, name.empty() ? "xshift is empty" : "xshift '" + name + "' is undefined");
  }
}

/// Checks a move keyword: move, move:l, move:r, move::fraction, move:r:fraction
bool IsGtdrawMoveKeyword(const std::string &p_key)
{
  const auto parts = SplitOn(p_key, ':');
  if (parts[0] != "move" || parts.size() > 3) {
    return false;
  }
  if (parts.size() > 1 && !parts[1].empty() && parts[1] != "l" && parts[1] != "r" &&
      parts[1] != "L" && parts[1] != "R") {
    return false;
  }
  if (parts.size() > 2 && !parts[2].empty()) {
    const auto fraction = GtdrawReal(parts[2]);
    return fraction && *fraction >= 0 && *fraction <= 1;
  }
  return true;
}

/// Reads the field of a level line that begins at word p_first; returns where the next begins
size_t ParseGtdrawField(GtdrawFile &p_file, GtdrawNode &p_node,
                        const std::vector<std::string> &p_words, const size_t p_first)
{
  const auto &key = p_words[p_first];
  const auto argument = [&]() -> const std::string & {
    if (p_first + 1 >= p_words.size()) {
      GtdrawError(p_node.line, "'" + key + "' needs a value");
    }
    return p_words[p_first + 1];
  };
  if (key == "player") {
    p_node.player = GtdrawPlayerNumber(p_node.line, argument());
  }
  else if (key == "xshift") {
    CheckGtdrawShift(p_file, p_node.line, argument());
  }
  else if (key == "yshift") {
    RequireGtdrawReal(p_node.line, argument(), "yshift");
  }
  else if (key == "from") {
    p_node.from = GtdrawRef(p_file, p_node.line, argument());
  }
  else if (IsGtdrawMoveKeyword(key)) {
    p_node.move = argument();
  }
  else if (key == "payoffs") {
    const auto numbers = p_words.size() - p_first - 1;
    if (numbers > kGtdrawMaxPlayers) {
      GtdrawError(p_node.line, std::to_string(numbers) +
                                   " payoffs, but the format draws at most " +
                                   std::to_string(kGtdrawMaxPlayers) + " players");
    }
    for (size_t i = p_first + 1; i < p_words.size(); i++) {
      p_node.payoffs.push_back(RequireGtdrawNumber(p_node.line, p_words[i], "payoff"));
    }
    return p_words.size();
  }
  else {
    GtdrawError(p_node.line, "unknown keyword '" + key + "'");
  }
  return p_first + 2;
}

void ParseGtdrawLevel(GtdrawFile &p_file, const int p_line,
                      const std::vector<std::string> &p_words)
{
  if (p_words.size() < 4 || p_words[2] != "node") {
    GtdrawError(p_line, "expected 'level <number> node <name>'");
  }
  const auto level = GtdrawReal(p_words[1]);
  if (!level) {
    GtdrawError(p_line, "level must be a number, not '" + p_words[1] + "'");
  }
  GtdrawNode node;
  node.line = p_line;
  node.id = (p_file.version == 1) ? p_words[3] : CanonicalLevel(*level) + "," + p_words[3];
  for (size_t next = 4; next < p_words.size();) {
    next = ParseGtdrawField(p_file, node, p_words, next);
  }
  if (!p_file.index.emplace(node.id, static_cast<int>(p_file.nodes.size())).second) {
    GtdrawError(p_line, "node '" + node.id + "' is defined twice");
  }
  p_file.nodes.push_back(node);
}

void ParseGtdrawPlayer(GtdrawFile &p_file, const int p_line,
                       const std::vector<std::string> &p_words)
{
  if (p_words.size() != 4 || p_words[2] != "name") {
    GtdrawError(p_line, "expected 'player <number> name <name>'");
  }
  auto name = p_words[3];
  std::ranges::replace(name, '~', ' ');
  const auto player = GtdrawPlayerNumber(p_line, p_words[1]);
  if (player > 0 && !IsValidLabel(name)) {
    GtdrawError(p_line, "'" + p_words[3] + "' is not a valid player name");
  }
  p_file.playerNames[player] = name;
}

void ParseGtdrawIset(GtdrawFile &p_file, const int p_line, const std::vector<std::string> &p_words)
{
  GtdrawIsetLine iset;
  iset.line = p_line;
  for (size_t i = 1; i < p_words.size(); i++) {
    if (p_words[i] != "player") {
      iset.refs.push_back(GtdrawRef(p_file, p_line, p_words[i]));
    }
    else if (i + 1 < p_words.size()) {
      iset.player = GtdrawPlayerNumber(p_line, p_words[++i]);
    }
    else {
      GtdrawError(p_line, "'player' needs a value");
    }
  }
  if (iset.refs.empty()) {
    GtdrawError(p_line, "an iset needs nodes");
  }
  p_file.isetLines.push_back(iset);
}

GtdrawFile ParseGtdrawFile(std::istream &p_stream)
{
  std::vector<std::pair<int, std::vector<std::string>>> lines;
  std::string text;
  int number = 0;
  while (std::getline(p_stream, text)) {
    number++;
    if (auto words = SplitWords(text); !words.empty() && !words[0].starts_with('%')) {
      lines.emplace_back(number, std::move(words));
    }
  }
  GtdrawFile file;
  file.version = GtdrawVersion(lines);
  for (const auto &[line, words] : lines) {
    if (words[0] == "player") {
      ParseGtdrawPlayer(file, line, words);
    }
    else if (words[0] == "level") {
      ParseGtdrawLevel(file, line, words);
    }
    else if (words[0] == "iset") {
      ParseGtdrawIset(file, line, words);
    }
    else {
      GtdrawError(line, "unknown keyword '" + words[0] + "'");
    }
  }
  if (file.nodes.empty()) {
    throw InvalidFileException("The drawing has no nodes");
  }
  return file;
}

/// Sets each node's parent and children, in the order of the level lines, and finds the root
void LinkGtdrawTree(GtdrawFile &p_file)
{
  for (size_t i = 0; i < p_file.nodes.size(); i++) {
    auto &node = p_file.nodes[i];
    if (!node.from) {
      if (p_file.root >= 0) {
        GtdrawError(node.line, "node '" + node.id + "' has no 'from', and so has the root (line " +
                                   std::to_string(p_file.nodes[p_file.root].line) + ")");
      }
      p_file.root = static_cast<int>(i);
      continue;
    }
    const auto parent = p_file.index.find(*node.from);
    if (parent == p_file.index.end()) {
      GtdrawError(node.line,
                  "the parent '" + *node.from + "' of node '" + node.id + "' is not defined");
    }
    node.parent = parent->second;
    p_file.nodes[parent->second].children.push_back(static_cast<int>(i));
  }
  if (p_file.root < 0) {
    GtdrawError(p_file.nodes.front().line, "every node has a 'from', so there is no root");
  }
  std::vector<bool> reached(p_file.nodes.size(), false);
  std::vector<int> pending{p_file.root};
  while (!pending.empty()) {
    const auto next = pending.back();
    pending.pop_back();
    reached[next] = true;
    pending.insert(pending.end(), p_file.nodes[next].children.begin(),
                   p_file.nodes[next].children.end());
  }
  if (const auto lost = std::ranges::find(reached, false); lost != reached.end()) {
    const auto &node = p_file.nodes[std::distance(reached.begin(), lost)];
    GtdrawError(node.line, "node '" + node.id + "' does not descend from the root");
  }
}

/// Checks one group of nodes sharing an information set, and settles its player
void CheckGtdrawGroup(const GtdrawFile &p_file, GtdrawGroup &p_group)
{
  const auto &first = p_file.nodes[p_group.members.front()];
  for (const auto member : p_group.members) {
    const auto &node = p_file.nodes[member];
    if (node.children.empty()) {
      GtdrawError(p_group.line, "node '" + node.id + "' in an iset has no moves");
    }
    if (node.children.size() != first.children.size()) {
      GtdrawError(node.line, "node '" + node.id + "' has " + std::to_string(node.children.size()) +
                                 " moves, but node '" + first.id + "' in its iset has " +
                                 std::to_string(first.children.size()));
    }
    if (node.player >= 0 && p_group.player >= 0 && node.player != p_group.player) {
      GtdrawError(node.line, "node '" + node.id + "' is for player " +
                                 std::to_string(node.player) + ", but its iset is for player " +
                                 std::to_string(p_group.player));
    }
    if (node.player >= 0) {
      p_group.player = node.player;
    }
  }
  if (p_group.player < 0) {
    GtdrawError(first.line,
                "node '" + first.id + "' has moves but no player, nor an iset with one");
  }
}

/// Collects the nodes with moves into information sets: those named in an iset line, and each
/// of the others alone.  Checks that the nodes are consistent with the format's rules.
void GroupGtdrawNodes(GtdrawFile &p_file)
{
  for (const auto &iset : p_file.isetLines) {
    GtdrawGroup group{.line = iset.line, .player = iset.player, .members = {}};
    for (const auto &ref : iset.refs) {
      const auto found = p_file.index.find(ref);
      if (found == p_file.index.end()) {
        GtdrawError(iset.line, "node '" + ref + "' in an iset is not defined");
      }
      auto &node = p_file.nodes[found->second];
      if (node.group >= 0) {
        GtdrawError(iset.line, "node '" + node.id + "' is in more than one iset");
      }
      node.group = static_cast<int>(p_file.groups.size());
      group.members.push_back(found->second);
    }
    std::ranges::sort(group.members);
    p_file.groups.push_back(group);
  }
  for (size_t i = 0; i < p_file.nodes.size(); i++) {
    auto &node = p_file.nodes[i];
    if (!node.children.empty() && node.group < 0) {
      node.group = static_cast<int>(p_file.groups.size());
      p_file.groups.push_back({.line = node.line, .player = -1, .members = {static_cast<int>(i)}});
    }
    if (!node.children.empty() && !node.payoffs.empty()) {
      GtdrawError(node.line, "node '" + node.id + "' has both moves and payoffs");
    }
  }
  for (auto &group : p_file.groups) {
    CheckGtdrawGroup(p_file, group);
  }
}

/// The number of players: the highest named in a player, node or iset, or paid in a payoff.
/// A terminal node that lists fewer payoffs is paid zero for the others when the game is built.
void CountGtdrawPlayers(GtdrawFile &p_file)
{
  if (!p_file.playerNames.empty()) {
    p_file.numPlayers = p_file.playerNames.rbegin()->first;
  }
  for (const auto &group : p_file.groups) {
    p_file.numPlayers = std::max(p_file.numPlayers, group.player);
  }
  for (const auto &node : p_file.nodes) {
    p_file.numPlayers = std::max(p_file.numPlayers, static_cast<int>(node.payoffs.size()));
  }
}

struct GtdrawChanceMove {
  std::string label;
  Number probability;
};

/// A chance move's probability is the last '~'-separated part of its label, bare or in
/// parentheses; the rest is the action's label.
GtdrawChanceMove ParseGtdrawChanceMove(const GtdrawNode &p_child)
{
  if (!p_child.move) {
    GtdrawError(p_child.line, "the chance move to node '" + p_child.id + "' has no probability");
  }
  auto parts = SplitOn(*p_child.move, '~');
  auto last = parts.back();
  parts.pop_back();
  if (last.size() >= 2 && last.front() == '(' && last.back() == ')') {
    last = last.substr(1, last.size() - 2);
  }
  const auto probability = GtdrawNumber(last);
  if (!probability) {
    GtdrawError(p_child.line, "the chance move '" + *p_child.move +
                                  "' has no probability (the last part after '~' must be a "
                                  "number)");
  }
  std::string label;
  for (const auto &part : parts) {
    label += (label.empty() ? "" : " ") + part;
  }
  return {.label = label, .probability = Number(*probability)};
}

/// The label of the action a node's move, whose '~' stand for spaces
std::string GtdrawActionLabel(const GtdrawNode &p_child)
{
  auto label = p_child.move.value_or("");
  std::ranges::replace(label, '~', ' ');
  return label;
}

/// The label a node's move gives its action, which must be one a game can hold
std::string GtdrawMoveLabel(const GtdrawNode &p_child, const bool p_chance)
{
  const auto label = p_chance ? ParseGtdrawChanceMove(p_child).label : GtdrawActionLabel(p_child);
  if (!label.empty() && !IsValidLabel(label)) {
    GtdrawError(p_child.line, "'" + p_child.move.value_or("") + "' is not a valid move label");
  }
  return label;
}

/// An information set's action labels: for each action, the first label among its members
std::vector<std::string> GtdrawGroupLabels(const GtdrawFile &p_file, const GtdrawGroup &p_group,
                                           const bool p_chance)
{
  const auto actions = p_file.nodes[p_group.members.front()].children.size();
  std::vector<std::string> labels(actions);
  for (size_t action = 0; action < actions; action++) {
    for (const auto member : p_group.members) {
      const auto &child = p_file.nodes[p_file.nodes[member].children[action]];
      const auto label = GtdrawMoveLabel(child, p_chance);
      if (labels[action].empty()) {
        labels[action] = label;
      }
    }
  }
  NormalizeLabelStrings(labels);
  return labels;
}

/// A chance information set's probabilities, which every member must give alike
std::vector<Number> GtdrawGroupProbabilities(const GtdrawFile &p_file, const GtdrawGroup &p_group)
{
  std::vector<Number> probabilities;
  for (const auto member : p_group.members) {
    std::vector<Number> these;
    for (const auto child : p_file.nodes[member].children) {
      these.push_back(ParseGtdrawChanceMove(p_file.nodes[child]).probability);
    }
    if (probabilities.empty()) {
      probabilities = these;
    }
    for (size_t i = 0; i < these.size(); i++) {
      if (static_cast<const Rational &>(these[i]) !=
          static_cast<const Rational &>(probabilities[i])) {
        GtdrawError(p_file.nodes[member].line,
                    "node '" + p_file.nodes[member].id +
                        "' has different chance probabilities from the rest of its iset");
      }
    }
  }
  return probabilities;
}

/// Gives a node its moves: a new information set if it is the first of its group to be met
void AttachGtdrawMove(const Game &p_game, const GtdrawFile &p_file, const GtdrawNode &p_node,
                      const GameNode &p_gameNode, std::vector<GameInfoset> &p_infosets)
{
  auto &infoset = p_infosets[p_node.group];
  if (infoset) {
    p_game->AppendMove(p_gameNode, infoset);
    return;
  }
  const auto &group = p_file.groups[p_node.group];
  const auto labels = GtdrawGroupLabels(p_file, group, group.player == 0);
  infoset = (group.player == 0)
                ? p_game->AppendEvent(p_gameNode, labels, GtdrawGroupProbabilities(p_file, group))
                : p_game->AppendMove(p_gameNode, p_game->GetPlayer(group.player), labels);
}

Game BuildGtdrawGame(const GtdrawFile &p_file)
{
  const Game game = NewTree();
  std::vector<std::string> names;
  for (int player = 1; player <= p_file.numPlayers; player++) {
    const auto name = p_file.playerNames.find(player);
    names.push_back((name != p_file.playerNames.end()) ? name->second : std::to_string(player));
  }
  NormalizeLabelStrings(names);
  int line = p_file.nodes[p_file.root].line;
  try {
    if (!names.empty()) {
      game->SetPlayers(names);
    }
    std::vector<GameNode> gameNodes(p_file.nodes.size());
    std::vector<GameInfoset> infosets(p_file.groups.size());
    std::vector<int> terminals;
    std::vector<int> pending{p_file.root};
    gameNodes[p_file.root] = game->GetRoot();
    while (!pending.empty()) {
      const auto next = pending.back();
      pending.pop_back();
      const auto &node = p_file.nodes[next];
      line = node.line;
      if (node.children.empty()) {
        terminals.push_back(next);
        continue;
      }
      AttachGtdrawMove(game, p_file, node, gameNodes[next], infosets);
      auto child = node.children.begin();
      for (const auto &gameChild : gameNodes[next]->GetChildren()) {
        gameNodes[*child++] = gameChild;
      }
      pending.insert(pending.end(), node.children.rbegin(), node.children.rend());
    }
    int outcome = 0;
    for (const auto terminal : terminals) {
      const auto &node = p_file.nodes[terminal];
      line = node.line;
      if (!node.payoffs.empty()) {
        std::vector<Number> payoffs;
        std::ranges::transform(node.payoffs, std::back_inserter(payoffs),
                               [](const std::string &text) { return Number(text); });
        payoffs.resize(p_file.numPlayers, Number("0"));
        game->MakeOutcome({gameNodes[terminal]}, payoffs, std::to_string(++outcome));
      }
    }
  }
  catch (const InvalidFileException &) {
    throw;
  }
  catch (const std::exception &ex) {
    GtdrawError(line, ex.what());
  }
  return game;
}

} // end anonymous namespace

std::string WriteGtdrawFile(const Game &p_game, const GtdrawOptions &p_options)
{
  if (p_game->NumPlayers() > kGtdrawMaxPlayers) {
    throw std::invalid_argument("gtdraw's format draws at most " +
                                std::to_string(kGtdrawMaxPlayers) + " players");
  }
  if (!(p_options.levelScaling >= 0 && p_options.sublevelScaling >= 0 &&
        p_options.widthScaling >= 0)) {
    throw std::invalid_argument("scalings must not be negative");
  }

  const auto tree = SurveyGtdrawTree(p_game);
  std::ostringstream f;
  for (const auto &player : p_game->GetPlayers()) {
    f << "player " << player->GetNumber() << " name " << GtdrawText(player->GetLabel()) << "\n";
  }
  for (const auto &node : tree.nodes) {
    f << GtdrawNodeLine(p_game, tree, p_options, node) << "\n";
  }
  for (const auto &infoset : tree.isets) {
    if (const auto &members = tree.isetMembers.at(infoset); members.size() > 1) {
      f << "iset";
      for (const auto id : members) {
        f << " " << id;
      }
      f << " player " << GtdrawPlayerId(infoset->GetPlayer()) << "\n";
    }
  }
  return f.str();
}

Game ReadGtdrawFile(std::istream &p_stream)
{
  auto file = ParseGtdrawFile(p_stream);
  LinkGtdrawTree(file);
  GroupGtdrawNodes(file);
  CountGtdrawPlayers(file);
  return BuildGtdrawGame(file);
}

} // end namespace Gambit
