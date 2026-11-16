//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/catalogdata.cc
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

#include "catalogdata.h"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>

#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

// Set via -DGAMBIT_SRCDIR="$(abs_top_srcdir)" in Makefile.am's AM_CPPFLAGS. Guarded here
// so the file still compiles (with the dev-checkout check simply never matching) if built
// outside the normal autotools flow.
#ifndef GAMBIT_SRCDIR
#define GAMBIT_SRCDIR ""
#endif

namespace Gambit::GUI {

namespace {

// ---------------------------------------------------------------------------
// A minimal JSON parser, scoped to what catalog/games/manifest.json actually contains:
// an array of flat objects with string/number/bool values. Not a general-purpose JSON
// library -- Gambit's C++ core has no JSON dependency, and both the producer
// (catalog/build.py) and consumer of this format are Gambit's own code, so a parser
// exactly matched to this shape is simpler to reason about than vendoring one.
// ---------------------------------------------------------------------------

class JsonParseError : public std::runtime_error {
public:
  explicit JsonParseError(const std::string &p_message) : std::runtime_error(p_message) {}
};

class JsonValue {
public:
  enum class Type { Null, Bool, Number, String, Array, Object };

  Type m_type = Type::Null;
  bool m_boolValue = false;
  double m_numberValue = 0.0;
  std::string m_stringValue;
  std::vector<JsonValue> m_arrayValue;
  std::map<std::string, JsonValue> m_objectValue;

  wxString GetString(const std::string &p_key) const
  {
    auto it = m_objectValue.find(p_key);
    return (it != m_objectValue.end() && it->second.m_type == Type::String)
               ? wxString::FromUTF8(it->second.m_stringValue)
               : wxString();
  }

  int GetInt(const std::string &p_key) const
  {
    auto it = m_objectValue.find(p_key);
    return (it != m_objectValue.end() && it->second.m_type == Type::Number)
               ? static_cast<int>(it->second.m_numberValue)
               : 0;
  }

  bool GetBool(const std::string &p_key) const
  {
    auto it = m_objectValue.find(p_key);
    return it != m_objectValue.end() && it->second.m_type == Type::Bool && it->second.m_boolValue;
  }
};

class JsonParser {
public:
  explicit JsonParser(const std::string &p_text) : m_text(p_text), m_pos(0) {}

  JsonValue Parse()
  {
    JsonValue value = ParseValue();
    SkipWhitespace();
    if (m_pos != m_text.size()) {
      throw JsonParseError("Unexpected trailing content after top-level JSON value");
    }
    return value;
  }

private:
  const std::string &m_text;
  size_t m_pos;

  char Peek() const
  {
    if (m_pos >= m_text.size()) {
      throw JsonParseError("Unexpected end of input");
    }
    return m_text[m_pos];
  }

  char Next()
  {
    if (m_pos >= m_text.size()) {
      throw JsonParseError("Unexpected end of input");
    }
    return m_text[m_pos++];
  }

  void SkipWhitespace()
  {
    while (m_pos < m_text.size() && (m_text[m_pos] == ' ' || m_text[m_pos] == '\t' ||
                                     m_text[m_pos] == '\n' || m_text[m_pos] == '\r')) {
      m_pos++;
    }
  }

  void Expect(char p_char)
  {
    if (Peek() != p_char) {
      throw JsonParseError(std::string("Expected '") + p_char + "'");
    }
    m_pos++;
  }

  bool Consume(const std::string &p_literal)
  {
    if (m_text.compare(m_pos, p_literal.size(), p_literal) == 0) {
      m_pos += p_literal.size();
      return true;
    }
    return false;
  }

  JsonValue ParseValue()
  {
    SkipWhitespace();
    char c = Peek();
    if (c == '{') {
      return ParseObject();
    }
    if (c == '[') {
      return ParseArray();
    }
    if (c == '"') {
      return ParseString();
    }
    if (c == 't' || c == 'f') {
      return ParseBool();
    }
    if (c == 'n') {
      return ParseNull();
    }
    return ParseNumber();
  }

  JsonValue ParseObject()
  {
    JsonValue result;
    result.m_type = JsonValue::Type::Object;
    Expect('{');
    SkipWhitespace();
    if (Peek() == '}') {
      m_pos++;
      return result;
    }
    while (true) {
      SkipWhitespace();
      JsonValue key = ParseString();
      SkipWhitespace();
      Expect(':');
      JsonValue value = ParseValue();
      result.m_objectValue.emplace(key.m_stringValue, std::move(value));
      SkipWhitespace();
      char c = Next();
      if (c == '}') {
        break;
      }
      if (c != ',') {
        throw JsonParseError("Expected ',' or '}' in object");
      }
    }
    return result;
  }

  JsonValue ParseArray()
  {
    JsonValue result;
    result.m_type = JsonValue::Type::Array;
    Expect('[');
    SkipWhitespace();
    if (Peek() == ']') {
      m_pos++;
      return result;
    }
    while (true) {
      result.m_arrayValue.push_back(ParseValue());
      SkipWhitespace();
      char c = Next();
      if (c == ']') {
        break;
      }
      if (c != ',') {
        throw JsonParseError("Expected ',' or ']' in array");
      }
    }
    return result;
  }

  static void AppendUtf8(std::string &p_out, unsigned int p_codepoint)
  {
    if (p_codepoint <= 0x7F) {
      p_out += static_cast<char>(p_codepoint);
    }
    else if (p_codepoint <= 0x7FF) {
      p_out += static_cast<char>(0xC0 | (p_codepoint >> 6));
      p_out += static_cast<char>(0x80 | (p_codepoint & 0x3F));
    }
    else {
      p_out += static_cast<char>(0xE0 | (p_codepoint >> 12));
      p_out += static_cast<char>(0x80 | ((p_codepoint >> 6) & 0x3F));
      p_out += static_cast<char>(0x80 | (p_codepoint & 0x3F));
    }
  }

  JsonValue ParseString()
  {
    Expect('"');
    std::string s;
    while (true) {
      char c = Next();
      if (c == '"') {
        break;
      }
      if (c == '\\') {
        char esc = Next();
        switch (esc) {
        case '"':
          s += '"';
          break;
        case '\\':
          s += '\\';
          break;
        case '/':
          s += '/';
          break;
        case 'n':
          s += '\n';
          break;
        case 't':
          s += '\t';
          break;
        case 'r':
          s += '\r';
          break;
        case 'b':
          s += '\b';
          break;
        case 'f':
          s += '\f';
          break;
        case 'u': {
          // The manifest is generated by Python's json.dumps(ensure_ascii=False), so in
          // practice \u escapes here are limited to control characters; surrogate pairs
          // are not expected, but each \u is still decoded as its own code point if seen.
          if (m_pos + 4 > m_text.size()) {
            throw JsonParseError("Truncated \\u escape");
          }
          unsigned int codepoint = 0;
          for (int i = 0; i < 4; i++) {
            char h = Next();
            codepoint <<= 4;
            if (h >= '0' && h <= '9') {
              codepoint |= (h - '0');
            }
            else if (h >= 'a' && h <= 'f') {
              codepoint |= (h - 'a' + 10);
            }
            else if (h >= 'A' && h <= 'F') {
              codepoint |= (h - 'A' + 10);
            }
            else {
              throw JsonParseError("Invalid \\u escape");
            }
          }
          AppendUtf8(s, codepoint);
          break;
        }
        default:
          throw JsonParseError("Invalid escape sequence");
        }
      }
      else {
        s += c;
      }
    }
    JsonValue result;
    result.m_type = JsonValue::Type::String;
    result.m_stringValue = std::move(s);
    return result;
  }

  JsonValue ParseBool()
  {
    JsonValue result;
    result.m_type = JsonValue::Type::Bool;
    if (Consume("true")) {
      result.m_boolValue = true;
    }
    else if (Consume("false")) {
      result.m_boolValue = false;
    }
    else {
      throw JsonParseError("Invalid literal");
    }
    return result;
  }

  JsonValue ParseNull()
  {
    if (!Consume("null")) {
      throw JsonParseError("Invalid literal");
    }
    return JsonValue();
  }

  JsonValue ParseNumber()
  {
    size_t start = m_pos;
    if (m_pos < m_text.size() && m_text[m_pos] == '-') {
      m_pos++;
    }
    while (m_pos < m_text.size() &&
           (std::isdigit(static_cast<unsigned char>(m_text[m_pos])) || m_text[m_pos] == '.' ||
            m_text[m_pos] == 'e' || m_text[m_pos] == 'E' || m_text[m_pos] == '+' ||
            m_text[m_pos] == '-')) {
      m_pos++;
    }
    if (m_pos == start) {
      throw JsonParseError("Invalid number");
    }
    JsonValue result;
    result.m_type = JsonValue::Type::Number;
    result.m_numberValue = std::stod(m_text.substr(start, m_pos - start));
    return result;
  }
};

wxString JoinPath(const wxString &p_base, const wxString &p_child)
{
  if (p_base.IsEmpty()) {
    return p_child;
  }
  return p_base + wxFILE_SEP_PATH + p_child;
}

/// A directory is the catalog root if it directly contains manifest.json.
bool IsCatalogRoot(const wxString &p_dir)
{
  return !p_dir.IsEmpty() && wxFileExists(JoinPath(p_dir, "manifest.json"));
}

} // namespace

wxString FindCatalogRoot()
{
  // A co-located source checkout, first: lets a developer running the GUI straight out
  // of the build tree see live catalog edits with no packaging step needed.
  wxString srcdirCatalog =
      JoinPath(JoinPath(wxString::FromUTF8(GAMBIT_SRCDIR), "catalog"), "games");
  if (IsCatalogRoot(srcdirCatalog)) {
    return srcdirCatalog;
  }

#if defined(__WXMAC__)
  wxString bundled = JoinPath(wxStandardPaths::Get().GetResourcesDir(), "catalog");
  if (IsCatalogRoot(bundled)) {
    return bundled;
  }
#elif defined(__WXMSW__)
  wxString exeDir = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
  wxString bundled = JoinPath(exeDir, "catalog");
  if (IsCatalogRoot(bundled)) {
    return bundled;
  }
#else
  wxString bundled = JoinPath(wxStandardPaths::Get().GetDataDir(), "catalog");
  if (IsCatalogRoot(bundled)) {
    return bundled;
  }
#endif

  return wxString();
}

std::vector<CatalogEntry> LoadCatalogManifest(const wxString &p_catalogRoot, wxString *p_error)
{
  std::vector<CatalogEntry> entries;
  if (p_error) {
    p_error->Clear();
  }

  wxString manifestPath = JoinPath(p_catalogRoot, "manifest.json");
  std::ifstream file(manifestPath.ToStdString(), std::ios::binary);
  if (!file) {
    if (p_error) {
      *p_error = wxString::Format("Could not open catalog manifest at %s", manifestPath);
    }
    return entries;
  }
  std::ostringstream buffer;
  buffer << file.rdbuf();
  std::string text = buffer.str();

  JsonValue root;
  try {
    root = JsonParser(text).Parse();
  }
  catch (const JsonParseError &e) {
    if (p_error) {
      *p_error = wxString::Format("Could not parse catalog manifest: %s", e.what());
    }
    return entries;
  }

  if (root.m_type != JsonValue::Type::Array) {
    if (p_error) {
      *p_error = "Catalog manifest is malformed (expected a top-level array)";
    }
    return entries;
  }

  entries.reserve(root.m_arrayValue.size());
  for (const JsonValue &item : root.m_arrayValue) {
    CatalogEntry entry;
    entry.slug = item.GetString("slug");
    entry.title = item.GetString("title");
    entry.description = item.GetString("description");
    entry.format = item.GetString("format");
    entry.category = item.GetString("category");
    entry.group = item.GetString("group");
    entry.thumbnail = item.GetString("thumbnail");
    entry.numPlayers = item.GetInt("n_players");
    entry.isTree = item.GetBool("is_tree");
    entry.isConstSum = item.GetBool("is_const_sum");
    entry.numStrategies = item.GetInt("n_strategies");
    entries.push_back(std::move(entry));
  }
  return entries;
}

wxString CatalogGameFilePath(const wxString &p_catalogRoot, const CatalogEntry &p_entry)
{
  return JoinPath(p_catalogRoot, p_entry.slug + "." + p_entry.format);
}

} // namespace Gambit::GUI
