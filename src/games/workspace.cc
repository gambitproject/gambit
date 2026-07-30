//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/workspace.cc
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

#include "workspace.h"

#include <charconv>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace Gambit {
namespace {

class Element {
  friend class Parser;

  std::string m_name;
  std::string m_text;
  std::map<std::string, std::string> m_attributes;
  std::vector<std::unique_ptr<Element>> m_children;

public:
  const std::string &Name() const { return m_name; }
  const std::string &Text() const { return m_text; }
  const std::string *Attribute(const std::string &p_name) const;
  bool IntAttribute(const std::string &p_name, int &p_value) const;
  const Element *Child(const std::string &p_name) const;
  std::vector<const Element *> Children(const std::string &p_name = {}) const;
};

class Parser {
  const std::string &m_input;
  size_t m_pos{0};

  [[noreturn]] void Fail(const std::string &p_message) const
  {
    throw std::runtime_error("Invalid .gbt XML at byte " + std::to_string(m_pos) + ": " +
                             p_message);
  }

  bool StartsWith(const std::string &p_text) const
  {
    return m_input.compare(m_pos, p_text.size(), p_text) == 0;
  }

  void SkipWhitespace()
  {
    while (m_pos < m_input.size() && (m_input[m_pos] == ' ' || m_input[m_pos] == '\t' ||
                                      m_input[m_pos] == '\r' || m_input[m_pos] == '\n')) {
      ++m_pos;
    }
  }

  void SkipMarkup()
  {
    if (StartsWith("<?")) {
      const auto end = m_input.find("?>", m_pos + 2);
      if (end == std::string::npos) {
        Fail("unterminated processing instruction");
      }
      m_pos = end + 2;
    }
    else if (StartsWith("<!--")) {
      const auto end = m_input.find("-->", m_pos + 4);
      if (end == std::string::npos) {
        Fail("unterminated comment");
      }
      m_pos = end + 3;
    }
    else {
      Fail("unsupported markup");
    }
  }

  std::string Name()
  {
    const size_t begin = m_pos;
    while (m_pos < m_input.size()) {
      const char c = m_input[m_pos];
      if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == ':' ||
            c == '.')) {
        break;
      }
      ++m_pos;
    }
    if (begin == m_pos) {
      Fail("expected a name");
    }
    return m_input.substr(begin, m_pos - begin);
  }

  static void AppendUtf8(std::string &p_out, unsigned long p_codepoint)
  {
    if (p_codepoint <= 0x7f) {
      p_out.push_back(static_cast<char>(p_codepoint));
    }
    else if (p_codepoint <= 0x7ff) {
      p_out.push_back(static_cast<char>(0xc0 | (p_codepoint >> 6)));
      p_out.push_back(static_cast<char>(0x80 | (p_codepoint & 0x3f)));
    }
    else if (p_codepoint <= 0xffff) {
      p_out.push_back(static_cast<char>(0xe0 | (p_codepoint >> 12)));
      p_out.push_back(static_cast<char>(0x80 | ((p_codepoint >> 6) & 0x3f)));
      p_out.push_back(static_cast<char>(0x80 | (p_codepoint & 0x3f)));
    }
    else if (p_codepoint <= 0x10ffff) {
      p_out.push_back(static_cast<char>(0xf0 | (p_codepoint >> 18)));
      p_out.push_back(static_cast<char>(0x80 | ((p_codepoint >> 12) & 0x3f)));
      p_out.push_back(static_cast<char>(0x80 | ((p_codepoint >> 6) & 0x3f)));
      p_out.push_back(static_cast<char>(0x80 | (p_codepoint & 0x3f)));
    }
    else {
      throw std::runtime_error("Invalid Unicode character in .gbt XML");
    }
  }

  std::string Decode(const std::string &p_text) const
  {
    std::string result;
    for (size_t pos = 0; pos < p_text.size();) {
      if (p_text[pos] != '&') {
        result.push_back(p_text[pos++]);
        continue;
      }
      const auto end = p_text.find(';', pos + 1);
      if (end == std::string::npos) {
        throw std::runtime_error("Invalid entity in .gbt XML");
      }
      const std::string entity = p_text.substr(pos + 1, end - pos - 1);
      if (entity == "amp") {
        result.push_back('&');
      }
      else if (entity == "lt") {
        result.push_back('<');
      }
      else if (entity == "gt") {
        result.push_back('>');
      }
      else if (entity == "quot") {
        result.push_back('"');
      }
      else if (entity == "apos") {
        result.push_back('\'');
      }
      else if (!entity.empty() && entity[0] == '#') {
        const bool hex = entity.size() > 1 && (entity[1] == 'x' || entity[1] == 'X');
        const std::string digits = entity.substr(hex ? 2 : 1);
        unsigned long value = 0;
        try {
          value = std::stoul(digits, nullptr, hex ? 16 : 10);
        }
        catch (...) {
          throw std::runtime_error("Invalid numeric entity in .gbt XML");
        }
        AppendUtf8(result, value);
      }
      else {
        throw std::runtime_error("Unknown entity in .gbt XML");
      }
      pos = end + 1;
    }
    return result;
  }

public:
  explicit Parser(const std::string &p_input) : m_input(p_input) {}

  std::unique_ptr<Element> ParseElement()
  {
    if (m_pos >= m_input.size() || m_input[m_pos++] != '<') {
      Fail("expected '<'");
    }
    auto element = std::make_unique<Element>();
    element->m_name = Name();
    while (true) {
      SkipWhitespace();
      if (StartsWith("/>")) {
        m_pos += 2;
        return element;
      }
      if (StartsWith(">")) {
        ++m_pos;
        break;
      }
      const std::string name = Name();
      SkipWhitespace();
      if (m_pos >= m_input.size() || m_input[m_pos++] != '=') {
        Fail("expected '='");
      }
      SkipWhitespace();
      if (m_pos >= m_input.size() || (m_input[m_pos] != '"' && m_input[m_pos] != '\'')) {
        Fail("expected quoted attribute value");
      }
      const char quote = m_input[m_pos++];
      const size_t begin = m_pos;
      const auto end = m_input.find(quote, begin);
      if (end == std::string::npos) {
        Fail("unterminated attribute value");
      }
      element->m_attributes[name] = Decode(m_input.substr(begin, end - begin));
      m_pos = end + 1;
    }

    // SaveWorkspace embeds these payloads verbatim rather than XML-escaping
    // them.  Treat them as opaque text so game labels and descriptions that
    // contain '<' or '&' remain readable.
    if (element->m_name == "efgfile" || element->m_name == "nfgfile" ||
        element->m_name == "description" || element->m_name == "profile") {
      const std::string close = "</" + element->m_name + ">";
      const auto end = m_input.find(close, m_pos);
      if (end == std::string::npos) {
        Fail("unterminated element " + element->m_name);
      }
      element->m_text = m_input.substr(m_pos, end - m_pos);
      m_pos = end + close.size();
      return element;
    }

    while (true) {
      if (m_pos >= m_input.size()) {
        Fail("unterminated element " + element->m_name);
      }
      if (StartsWith("</")) {
        m_pos += 2;
        const std::string closing = Name();
        SkipWhitespace();
        if (m_pos >= m_input.size() || m_input[m_pos++] != '>') {
          Fail("expected '>'");
        }
        if (closing != element->m_name) {
          Fail("mismatched closing element " + closing);
        }
        return element;
      }
      if (StartsWith("<!--") || StartsWith("<?")) {
        SkipMarkup();
      }
      else if (StartsWith("<![CDATA[")) {
        const size_t begin = m_pos + 9;
        const auto end = m_input.find("]]>", begin);
        if (end == std::string::npos) {
          Fail("unterminated CDATA");
        }
        element->m_text.append(m_input, begin, end - begin);
        m_pos = end + 3;
      }
      else if (m_input[m_pos] == '<') {
        element->m_children.push_back(ParseElement());
      }
      else {
        const size_t begin = m_pos;
        const auto end = m_input.find('<', begin);
        if (end == std::string::npos) {
          Fail("unterminated element " + element->m_name);
        }
        element->m_text += Decode(m_input.substr(begin, end - begin));
        m_pos = end;
      }
    }
  }

  std::vector<std::unique_ptr<Element>> ParseDocument()
  {
    std::vector<std::unique_ptr<Element>> roots;
    while (true) {
      SkipWhitespace();
      if (m_pos == m_input.size()) {
        return roots;
      }
      if (StartsWith("<?") || StartsWith("<!--")) {
        SkipMarkup();
      }
      else if (m_input[m_pos] == '<') {
        roots.push_back(ParseElement());
      }
      else {
        Fail("text outside the document element");
      }
    }
  }
};

const std::string *Element::Attribute(const std::string &p_name) const
{
  const auto it = m_attributes.find(p_name);
  return it == m_attributes.end() ? nullptr : &it->second;
}

bool Element::IntAttribute(const std::string &p_name, int &p_value) const
{
  const auto *value = Attribute(p_name);
  if (!value) {
    return false;
  }
  int parsed;
  const auto result = std::from_chars(value->data(), value->data() + value->size(), parsed);
  if (result.ec != std::errc() || result.ptr != value->data() + value->size()) {
    return false;
  }
  p_value = parsed;
  return true;
}

const Element *Element::Child(const std::string &p_name) const
{
  for (const auto &child : m_children) {
    if (child->Name() == p_name) {
      return child.get();
    }
  }
  return nullptr;
}

std::vector<const Element *> Element::Children(const std::string &p_name) const
{
  std::vector<const Element *> result;
  for (const auto &child : m_children) {
    if (p_name.empty() || child->Name() == p_name) {
      result.push_back(child.get());
    }
  }
  return result;
}

const Element *Root(const std::vector<std::unique_ptr<Element>> &p_roots,
                    const std::string &p_name)
{
  for (const auto &root : p_roots) {
    if (root->Name() == p_name) {
      return root.get();
    }
  }
  return nullptr;
}

std::string EscapeAttribute(const std::string &p_value)
{
  std::string result;
  for (const char c : p_value) {
    if (c == '&') {
      result += "&amp;";
    }
    else if (c == '<') {
      result += "&lt;";
    }
    else if (c == '>') {
      result += "&gt;";
    }
    else if (c == '"') {
      result += "&quot;";
    }
    else {
      result.push_back(c);
    }
  }
  return result;
}

std::string Attribute(const Element *p_element, const std::string &p_name,
                      const std::string &p_default = {})
{
  const auto *value = p_element->Attribute(p_name);
  return value ? *value : p_default;
}

std::string UnwrapPayload(std::string p_value)
{
  if (p_value.rfind("\r\n", 0) == 0) {
    p_value.erase(0, 2);
  }
  else if (!p_value.empty() && p_value.front() == '\n') {
    p_value.erase(0, 1);
  }
  if (p_value.size() >= 2 && p_value.compare(p_value.size() - 2, 2, "\r\n") == 0) {
    p_value.erase(p_value.size() - 2);
  }
  else if (!p_value.empty() && p_value.back() == '\n') {
    p_value.pop_back();
  }
  return p_value;
}

} // namespace

LegacyWorkspaceFile ReadLegacyWorkspace(std::istream &p_stream)
{
  std::ostringstream text;
  text << p_stream.rdbuf();
  const std::string input = text.str();
  const auto roots = Parser(input).ParseDocument();
  const Element *document = Root(roots, "gambit:document");
  const auto topLevel = [&](const std::string &name) {
    return document ? document->Child(name) : Root(roots, name);
  };

  LegacyWorkspaceFile workspace;
  if (const Element *colors = topLevel("colors")) {
    for (const auto *player : colors->Children("player")) {
      LegacyWorkspaceFile::Color color;
      player->IntAttribute("id", color.player);
      player->IntAttribute("red", color.red);
      player->IntAttribute("green", color.green);
      player->IntAttribute("blue", color.blue);
      workspace.colors.push_back(color);
    }
  }
  if (const Element *font = topLevel("font")) {
    workspace.font = LegacyWorkspaceFile::Font{};
    font->IntAttribute("size", workspace.font->size);
    font->IntAttribute("family", workspace.font->family);
    workspace.font->face = Attribute(font, "face");
    font->IntAttribute("style", workspace.font->style);
    font->IntAttribute("weight", workspace.font->weight);
  }
  if (const Element *layout = topLevel("autolayout")) {
    workspace.layout = LegacyWorkspaceFile::Layout{};
    if (const Element *nodes = layout->Child("nodes")) {
      nodes->IntAttribute("size", workspace.layout->node_size);
      nodes->IntAttribute("spacing", workspace.layout->terminal_spacing);
      workspace.layout->chance_token = Attribute(nodes, "chance", "dot");
      workspace.layout->player_token = Attribute(nodes, "player", "dot");
      workspace.layout->terminal_token = Attribute(nodes, "terminal", "dot");
    }
    if (const Element *branches = layout->Child("branches")) {
      branches->IntAttribute("size", workspace.layout->branch_length);
      branches->IntAttribute("tine", workspace.layout->tine_length);
      workspace.layout->branch_style = Attribute(branches, "branch", "forktine");
      workspace.layout->branch_labels = Attribute(branches, "labels", "horizontal");
    }
    if (const Element *infosets = layout->Child("infosets")) {
      workspace.layout->infoset_style = Attribute(infosets, "style", "circles");
    }
  }
  if (const Element *labels = topLevel("labels")) {
    workspace.labels = LegacyWorkspaceFile::Labels{};
    workspace.labels->node_above = Attribute(labels, "abovenode", "label");
    workspace.labels->node_below = Attribute(labels, "belownode", "isetid");
    workspace.labels->branch_above = Attribute(labels, "abovebranch", "label");
    workspace.labels->branch_below = Attribute(labels, "belowbranch", "probs");
  }
  if (const Element *numbers = topLevel("numbers")) {
    int decimals = 4;
    numbers->IntAttribute("decimals", decimals);
    workspace.decimals = decimals;
  }

  const Element *game = topLevel("game");
  if (!game) {
    throw std::runtime_error("No game representation found in .gbt document");
  }
  if (const Element *efg = game->Child("efgfile")) {
    workspace.game_format = "efg";
    workspace.game = UnwrapPayload(efg->Text());
  }
  else if (const Element *nfg = game->Child("nfgfile")) {
    workspace.game_format = "nfg";
    workspace.game = UnwrapPayload(nfg->Text());
  }
  else {
    throw std::runtime_error("No game representation found in .gbt document");
  }

  for (const auto *analysis : game->Children("analysis")) {
    if (Attribute(analysis, "type") != "list") {
      continue;
    }
    LegacyWorkspaceFile::Analysis result;
    if (const Element *description = analysis->Child("description")) {
      result.description = UnwrapPayload(description->Text());
    }
    for (const auto *profile : analysis->Children("profile")) {
      result.profiles.push_back({Attribute(profile, "type"), UnwrapPayload(profile->Text())});
    }
    workspace.analyses.push_back(std::move(result));
  }
  return workspace;
}

void WriteLegacyWorkspace(std::ostream &p_stream, const LegacyWorkspaceFile &p_workspace)
{
  if (p_workspace.game_format != "efg" && p_workspace.game_format != "nfg") {
    throw std::runtime_error("Invalid game format for .gbt workspace");
  }
  p_stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           << "<gambit:document xmlns:gambit=\"http://gambit.sourceforge.net/\" "
              "version=\"0.1\">\n";
  p_stream << "<colors>\n";
  for (const auto &color : p_workspace.colors) {
    p_stream << "<player id=\"" << color.player << "\" red=\"" << color.red << "\" green=\""
             << color.green << "\" blue=\"" << color.blue << "\" />\n";
  }
  p_stream << "</colors>\n";
  if (p_workspace.font) {
    const auto &font = *p_workspace.font;
    p_stream << "<font size=\"" << font.size << "\" family=\"" << font.family << "\" face=\""
             << EscapeAttribute(font.face) << "\" style=\"" << font.style << "\" weight=\""
             << font.weight << "\" />\n";
  }
  if (p_workspace.layout) {
    const auto &layout = *p_workspace.layout;
    p_stream << "<autolayout>\n<nodes size=\"" << layout.node_size << "\" spacing=\""
             << layout.terminal_spacing << "\" chance=\"" << layout.chance_token << "\" player=\""
             << layout.player_token << "\" terminal=\"" << layout.terminal_token
             << "\"/>\n<branches size=\"" << layout.branch_length << "\" tine=\""
             << layout.tine_length << "\" branch=\"" << layout.branch_style << "\" labels=\""
             << layout.branch_labels << "\"/>\n<infosets style=\"" << layout.infoset_style
             << "\"/>\n</autolayout>\n";
  }
  if (p_workspace.labels) {
    const auto &labels = *p_workspace.labels;
    p_stream << "<labels abovenode=\"" << labels.node_above << "\" belownode=\""
             << labels.node_below << "\" abovebranch=\"" << labels.branch_above
             << "\" belowbranch=\"" << labels.branch_below << "\" />\n";
  }
  if (p_workspace.decimals) {
    p_stream << "<numbers decimals=\"" << *p_workspace.decimals << "\"/>\n";
  }
  p_stream << "<game>\n<" << p_workspace.game_format << "file>\n" << p_workspace.game;
  if (p_workspace.game.empty() || p_workspace.game.back() != '\n') {
    p_stream << '\n';
  }
  p_stream << "</" << p_workspace.game_format << "file>\n";
  for (const auto &analysis : p_workspace.analyses) {
    p_stream << "<analysis type=\"list\">\n<description>\n"
             << analysis.description << "\n</description>\n";
    for (const auto &profile : analysis.profiles) {
      p_stream << "<profile type=\"" << profile.type << "\">\n"
               << profile.probabilities << "\n</profile>\n";
    }
    p_stream << "</analysis>\n";
  }
  p_stream << "</game>\n</gambit:document>\n";
}

} // namespace Gambit
