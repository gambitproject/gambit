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
#include <limits>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>

#include <wx/ffile.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

// The release fetched into the source tree, set by Makefile.am.
#ifndef GAMBIT_CATALOG_RELEASE_DIR
#define GAMBIT_CATALOG_RELEASE_DIR ""
#endif

namespace Gambit::GUI {

namespace {

// ---------------------------------------------------------------------------
// A minimal JSON parser, sufficient for the catalog's manifest.json; Gambit's C++ core
// has no JSON dependency.
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
    const auto it = m_objectValue.find(p_key);
    return (it != m_objectValue.end() && it->second.m_type == Type::String)
               ? wxString::FromUTF8(it->second.m_stringValue)
               : wxString();
  }

  int GetInt(const std::string &p_key) const
  {
    const auto it = m_objectValue.find(p_key);
    if (it == m_objectValue.end() || it->second.m_type != Type::Number) {
      return 0;
    }
    const double value = it->second.m_numberValue;
    return (value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max())
               ? static_cast<int>(value)
               : 0;
  }

  bool GetBool(const std::string &p_key) const
  {
    const auto it = m_objectValue.find(p_key);
    return it != m_objectValue.end() && it->second.m_type == Type::Bool && it->second.m_boolValue;
  }
};

class JsonParser {
public:
  explicit JsonParser(const std::string &p_text) : m_text(p_text) {}

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
  size_t m_pos{0};

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
    const char c = Peek();
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
      result.m_objectValue.emplace(std::move(key.m_stringValue), std::move(value));
      SkipWhitespace();
      const char c = Next();
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
      const char c = Next();
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

  static unsigned int HexValue(char p_digit)
  {
    if (p_digit >= '0' && p_digit <= '9') {
      return p_digit - '0';
    }
    if (p_digit >= 'a' && p_digit <= 'f') {
      return p_digit - 'a' + 10;
    }
    if (p_digit >= 'A' && p_digit <= 'F') {
      return p_digit - 'A' + 10;
    }
    throw JsonParseError("Invalid \\u escape");
  }

  /// Appends the character denoted by the escape sequence following a backslash.
  void ParseEscape(std::string &p_out)
  {
    const char esc = Next();
    switch (esc) {
    case '"':
    case '\\':
    case '/':
      p_out += esc;
      break;
    case 'n':
      p_out += '\n';
      break;
    case 't':
      p_out += '\t';
      break;
    case 'r':
      p_out += '\r';
      break;
    case 'b':
      p_out += '\b';
      break;
    case 'f':
      p_out += '\f';
      break;
    case 'u': {
      // Each \u is decoded as its own code point; the manifest has no surrogate pairs.
      if (m_pos + 4 > m_text.size()) {
        throw JsonParseError("Truncated \\u escape");
      }
      unsigned int codepoint = 0;
      for (int i = 0; i < 4; i++) {
        codepoint = (codepoint << 4) | HexValue(Next());
      }
      AppendUtf8(p_out, codepoint);
      break;
    }
    default:
      throw JsonParseError("Invalid escape sequence");
    }
  }

  JsonValue ParseString()
  {
    Expect('"');
    std::string s;
    while (true) {
      const char c = Next();
      if (c == '"') {
        break;
      }
      if (c == '\\') {
        ParseEscape(s);
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
    return {};
  }

  JsonValue ParseNumber()
  {
    const size_t start = m_pos;
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
    // Parsed in the classic locale, whatever the GUI's locale uses as decimal point.
    std::istringstream stream(m_text.substr(start, m_pos - start));
    stream.imbue(std::locale::classic());
    JsonValue result;
    result.m_type = JsonValue::Type::Number;
    if (!(stream >> result.m_numberValue) || stream.peek() != std::char_traits<char>::eof()) {
      throw JsonParseError("Invalid number '" + m_text.substr(start, m_pos - start) + "'");
    }
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

wxString NativePath(const wxString &p_path)
{
  wxString path = p_path;
  path.Replace("/", wxString(wxFILE_SEP_PATH));
  return path;
}

bool IsCatalogRoot(const wxString &p_dir)
{
  return !p_dir.IsEmpty() && wxFileExists(JoinPath(p_dir, "manifest.json"));
}

} // namespace

wxString FindCatalogRoot()
{
  wxString fromEnvironment;
  if (wxGetEnv("GAMBIT_CATALOG_DIR", &fromEnvironment) && IsCatalogRoot(fromEnvironment)) {
    return fromEnvironment;
  }

#if defined(__WXMAC__)
  const wxString installed = wxStandardPaths::Get().GetResourcesDir();
#elif defined(__WXMSW__)
  const wxString installed = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
#else
  const wxString installed = wxStandardPaths::Get().GetDataDir();
#endif
  wxString installedRoot = JoinPath(JoinPath(installed, "catalog"), "games");
  if (IsCatalogRoot(installedRoot)) {
    return installedRoot;
  }

  wxString fetchedRoot =
      JoinPath(NativePath(wxString::FromUTF8(GAMBIT_CATALOG_RELEASE_DIR)), "games");
  if (IsCatalogRoot(fetchedRoot)) {
    return fetchedRoot;
  }
  return {};
}

Catalog LoadCatalogManifest(const wxString &p_catalogRoot, wxString *p_error)
{
  Catalog catalog;
  auto &entries = catalog.entries;
  if (p_error) {
    p_error->Clear();
  }

  const wxString manifestPath = JoinPath(p_catalogRoot, "manifest.json");
  const auto unreadable = [&]() {
    if (p_error) {
      *p_error = wxString::Format(_("Could not read the catalog manifest at %s."), manifestPath);
    }
    return catalog;
  };
  wxFFile file;
  if (!wxFileExists(manifestPath) || !file.Open(manifestPath, "rb")) {
    return unreadable();
  }
  std::string text(static_cast<size_t>(file.Length()), '\0');
  if (file.Read(text.data(), text.size()) != text.size()) {
    return unreadable();
  }

  JsonValue root;
  try {
    root = JsonParser(text).Parse();
  }
  catch (const JsonParseError &e) {
    if (p_error) {
      *p_error = wxString::Format(_("Could not parse the catalog manifest at %s: %s"),
                                  manifestPath, wxString::FromUTF8(e.what()));
    }
    return catalog;
  }

  if (root.m_type != JsonValue::Type::Object) {
    if (p_error) {
      *p_error =
          wxString::Format(_("The catalog manifest at %s is not a JSON object."), manifestPath);
    }
    return catalog;
  }
  const auto schema = root.m_objectValue.find("schema_version");
  if (schema == root.m_objectValue.end() || schema->second.m_type != JsonValue::Type::Number ||
      schema->second.m_numberValue != CATALOG_SCHEMA_VERSION) {
    if (p_error) {
      *p_error = wxString::Format(_("The catalog manifest at %s is not in schema version %d, "
                                    "the version this Gambit reads."),
                                  manifestPath, CATALOG_SCHEMA_VERSION);
    }
    return catalog;
  }
  const auto games = root.m_objectValue.find("games");
  if (games == root.m_objectValue.end() || games->second.m_type != JsonValue::Type::Array) {
    if (p_error) {
      *p_error =
          wxString::Format(_("The catalog manifest at %s has no list of games."), manifestPath);
    }
    return catalog;
  }

  catalog.version = root.GetString("catalog_version");
  entries.reserve(games->second.m_arrayValue.size());
  for (const JsonValue &item : games->second.m_arrayValue) {
    CatalogEntry entry;
    entry.slug = item.GetString("slug");
    entry.title = item.GetString("title");
    entry.description = item.GetString("description");
    entry.format = item.GetString("format");
    entry.game = item.GetString("game");
    entry.category = item.GetString("category");
    entry.group = item.GetString("group");
    entry.numPlayers = item.GetInt("n_players");
    entry.isConstSum = item.GetBool("is_const_sum");
    entry.numStrategies = item.GetInt("n_strategies");
    entries.push_back(std::move(entry));
  }
  return catalog;
}

wxString CatalogGameFilePath(const wxString &p_catalogRoot, const CatalogEntry &p_entry)
{
  return JoinPath(p_catalogRoot, NativePath(p_entry.game));
}

wxString CatalogThumbnailPath(const wxString &p_catalogRoot, const CatalogEntry &p_entry)
{
  wxFileName thumbs = wxFileName::DirName(p_catalogRoot);
  thumbs.RemoveLastDir();
  thumbs.AppendDir("thumbs");
  return JoinPath(thumbs.GetPath(), NativePath(p_entry.slug + ".png"));
}

} // namespace Gambit::GUI
