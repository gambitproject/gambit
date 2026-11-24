//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlcatalog.cc
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

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/bmpbndl.h>
#include <wx/graphics.h>
#include <wx/renderer.h>
#include <wx/settings.h>
#include <wx/srchctrl.h>
#include <wx/tokenzr.h>
#include <wx/vlbox.h>

#include "catalogdata.h"
#include "dialogstyle.h"
#include "dlcatalog.h"

#include "bitmaps/newtree.h"
#include "bitmaps/table.h"

namespace Gambit::GUI {

namespace {

/// Separates the parts of a line of facts
const wxString c_factSeparator = wxString(" ") + wxUniChar(0x00B7) + " ";

enum class SortOrder { Source, Title };

/// The browser's search, filters and selection, kept for the session so that the dialog
/// reopens as it was left
struct BrowserState {
  wxString search;
  wxString source; ///< A category, or empty for every source
  wxString format; ///< "efg" or "nfg", or empty for either
  int players{0};  ///< 0 for any number
  SortOrder sort{SortOrder::Source};
  wxString selectedSlug;
  /// The sources whose games are hidden beneath their headings
  std::set<wxString> collapsed;
  wxSize size{wxDefaultSize}; ///< wxDefaultSize until the dialog has been shown
};

BrowserState &SessionState()
{
  static BrowserState state;
  return state;
}

wxString FormatName(const CatalogEntry &p_entry)
{
  return p_entry.format == "efg" ? _("Extensive form") : _("Strategic form");
}

wxString PlayersName(int p_players)
{
  return wxString::Format(wxPLURAL("%d player", "%d players", p_players), p_players);
}

wxString SourceName(const CatalogEntry &p_entry)
{
  return p_entry.category + c_factSeparator + p_entry.group;
}

/// The facts about a game: optionally its form, then its number of players and whether it is
/// constant-sum
wxString FactsLine(const CatalogEntry &p_entry, bool p_withFormat)
{
  wxString facts = p_withFormat ? FormatName(p_entry) + c_factSeparator : wxString();
  facts += PlayersName(p_entry.numPlayers);
  if (p_entry.isConstSum) {
    facts += c_factSeparator + _("constant-sum");
  }
  return facts;
}

/// Joins hard-wrapped lines into paragraphs, which are separated by blank lines
wxString Reflow(const wxString &p_text)
{
  wxString result, paragraph;
  const auto endParagraph = [&]() {
    if (!paragraph.empty()) {
      result += (result.empty() ? "" : "\n\n") + paragraph;
      paragraph.clear();
    }
  };
  wxStringTokenizer lines(p_text, "\n", wxTOKEN_RET_EMPTY_ALL);
  while (lines.HasMoreTokens()) {
    const wxString line = lines.GetNextToken().Strip(wxString::both);
    if (line.empty()) {
      endParagraph();
    }
    else {
      paragraph += (paragraph.empty() ? "" : " ") + line;
    }
  }
  endParagraph();
  return result;
}

/// The icon of a game's form, drawn in the given colour
wxBitmapBundle FormIcon(const wxString &p_format, const wxColour &p_colour, int p_size)
{
  wxString svg = (p_format == "efg") ? newtree_svg : table_svg;
  svg.Replace("currentColor", p_colour.GetAsString(wxC2S_HTML_SYNTAX));
  return wxBitmapBundle::FromSVG(svg.utf8_str().data(), wxSize(p_size, p_size));
}

/// The colour of text that is secondary to text of colour p_text on background p_background:
/// readable, but quieter than the text it accompanies
wxColour SecondaryTextColour(const wxColour &p_text, const wxColour &p_background)
{
  constexpr double weight = 0.6;
  const auto mix = [](unsigned char p_fore, unsigned char p_back) {
    return static_cast<unsigned char>(weight * p_fore + (1 - weight) * p_back);
  };
  return {mix(p_text.Red(), p_background.Red()), mix(p_text.Green(), p_background.Green()),
          mix(p_text.Blue(), p_background.Blue())};
}

/// Secondary text on a window's own background
wxColour SecondaryTextColour(const wxWindow *p_window)
{
  return SecondaryTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),
                             p_window->GetBackgroundColour());
}

/// Secondary text in a list
wxColour SecondaryListTextColour()
{
  return SecondaryTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT),
                             wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
}

//=========================================================================
//                          class ResultList
//=========================================================================

/// A row of the result list: a game, or the heading over the games of one source
struct ResultRow {
  const CatalogEntry *entry{nullptr};
  /// For a heading: the source, the number of its games listed, and whether they are hidden
  wxString source;
  size_t count{0};
  bool collapsed{false};
};

ResultRow GameRow(const CatalogEntry *p_entry)
{
  return {.entry = p_entry, .source = wxString(), .count = 0, .collapsed = false};
}

ResultRow HeadingRow(const wxString &p_source, size_t p_count, bool p_collapsed)
{
  return {.entry = nullptr, .source = p_source, .count = p_count, .collapsed = p_collapsed};
}

/// The games matching the search and filters, two lines to a game. Only the rows in view are
/// drawn, so the list stays quick with any number of games.
class ResultList final : public wxVListBox {
public:
  explicit ResultList(wxWindow *p_parent);

  /// Shows p_rows; each game's source is shown beneath its title if p_showSource. The list
  /// returns to the top unless p_keepScroll.
  void SetRows(std::vector<ResultRow> p_rows, bool p_showSource, bool p_keepScroll);
  /// Sets the function called with a heading's row when it is clicked, and whether the
  /// click was to collapse or expand all headings
  void OnHeadingClick(std::function<void(size_t, bool)> p_handler)
  {
    m_onHeadingClick = std::move(p_handler);
  }
  const ResultRow &GetRow(size_t p_row) const { return m_rows[p_row]; }
  /// The row of the game with slug p_slug, if it is listed
  std::optional<size_t> FindGame(const wxString &p_slug) const;
  /// The row of the heading of source p_source, if it is listed
  std::optional<size_t> FindHeading(const wxString &p_source) const;
  /// The first row holding a game, starting from p_from and moving by p_step
  std::optional<size_t> FindGameFrom(int p_from, int p_step) const;
  /// The selected game, or null if none is
  const CatalogEntry *GetSelectedGame() const;

protected:
  wxCoord OnMeasureItem(size_t p_row) const override;
  void OnDrawBackground(wxDC &p_dc, const wxRect &p_rect, size_t p_row) const override;
  void OnDrawItem(wxDC &p_dc, const wxRect &p_rect, size_t p_row) const override;

private:
  std::vector<ResultRow> m_rows;
  bool m_showSource{false};
  wxFont m_titleFont, m_headingFont;
  int m_lineHeight, m_headingHeight{0};
  /// Icons of each form, as drawn on an unselected and on a selected row
  wxBitmapBundle m_treeIcon[2], m_tableIcon[2];
  std::function<void(size_t, bool)> m_onHeadingClick;

  void OnClick(wxMouseEvent &p_event);
};

ResultList::ResultList(wxWindow *p_parent)
  : wxVListBox(p_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME),
    m_titleFont(GetFont().Bold()), m_headingFont(GetFont().Smaller()),
    m_lineHeight(GetCharHeight())
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
  int headingTextHeight;
  GetTextExtent("Ag", nullptr, &headingTextHeight, nullptr, nullptr, &m_headingFont);
  m_headingHeight = headingTextHeight + FromDIP(2 * c_spaceTight + 2);

  const int iconSize = 16;
  for (int selected = 0; selected < 2; selected++) {
    const wxColour colour = (selected != 0)
                                ? wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT)
                                : SecondaryListTextColour();
    m_treeIcon[selected] = FormIcon("efg", colour, iconSize);
    m_tableIcon[selected] = FormIcon("nfg", colour, iconSize);
  }
  // A heading toggles on each click, so the second click of a double click toggles it back
  // rather than activating a row.
  Bind(wxEVT_LEFT_DOWN, &ResultList::OnClick, this);
  Bind(wxEVT_LEFT_DCLICK, &ResultList::OnClick, this);
}

void ResultList::OnClick(wxMouseEvent &p_event)
{
  const int row = VirtualHitTest(p_event.GetPosition().y);
  if (row != wxNOT_FOUND && !m_rows[row].entry && m_onHeadingClick) {
    m_onHeadingClick(static_cast<size_t>(row), p_event.AltDown());
    return;
  }
  p_event.Skip();
}

void ResultList::SetRows(std::vector<ResultRow> p_rows, bool p_showSource, bool p_keepScroll)
{
  const size_t top = p_keepScroll ? GetVisibleRowsBegin() : 0;
  // Clearing the selection redraws the selected row, so it must happen while that row exists.
  SetSelection(wxNOT_FOUND);
  m_rows = std::move(p_rows);
  m_showSource = p_showSource;
  SetItemCount(m_rows.size());
  if (!m_rows.empty()) {
    ScrollToRow(std::min(top, m_rows.size() - 1));
  }
  RefreshAll();
}

std::optional<size_t> ResultList::FindGame(const wxString &p_slug) const
{
  const auto row = std::ranges::find_if(
      m_rows, [&](const ResultRow &r) { return r.entry && r.entry->slug == p_slug; });
  if (row == m_rows.end()) {
    return std::nullopt;
  }
  return static_cast<size_t>(row - m_rows.begin());
}

std::optional<size_t> ResultList::FindHeading(const wxString &p_source) const
{
  const auto row = std::ranges::find_if(
      m_rows, [&](const ResultRow &r) { return !r.entry && r.source == p_source; });
  if (row == m_rows.end()) {
    return std::nullopt;
  }
  return static_cast<size_t>(row - m_rows.begin());
}

std::optional<size_t> ResultList::FindGameFrom(int p_from, int p_step) const
{
  for (int row = p_from; row >= 0 && std::cmp_less(row, m_rows.size()); row += p_step) {
    if (m_rows[row].entry) {
      return static_cast<size_t>(row);
    }
  }
  return std::nullopt;
}

const CatalogEntry *ResultList::GetSelectedGame() const
{
  const int row = GetSelection();
  return (row == wxNOT_FOUND) ? nullptr : m_rows[row].entry;
}

wxCoord ResultList::OnMeasureItem(size_t p_row) const
{
  if (!m_rows[p_row].entry) {
    return m_headingHeight;
  }
  return 2 * m_lineHeight + FromDIP(2 * c_spaceRelated - 2);
}

void ResultList::OnDrawBackground(wxDC &p_dc, const wxRect &p_rect, size_t p_row) const
{
  if (!m_rows[p_row].entry) {
    const bool dark = wxSystemSettings::GetAppearance().IsDark();
    p_dc.SetPen(*wxTRANSPARENT_PEN);
    p_dc.SetBrush(GetBackgroundColour().ChangeLightness(dark ? 120 : 95));
    p_dc.DrawRectangle(p_rect);
    return;
  }
  if (IsSelected(p_row)) {
    // The selection is the game Open will open, so it is drawn as current even while the
    // search field has the focus.
    wxRendererNative::Get().DrawItemSelectionRect(const_cast<ResultList *>(this), p_dc, p_rect,
                                                  wxCONTROL_SELECTED | wxCONTROL_FOCUSED);
  }
}

void ResultList::OnDrawItem(wxDC &p_dc, const wxRect &p_rect, size_t p_row) const
{
  const int margin = FromDIP(c_spaceRelated + 2);
  const ResultRow &row = m_rows[p_row];

  if (!row.entry) {
    const int buttonSize = FromDIP(12);
    wxRendererNative::Get().DrawTreeItemButton(
        const_cast<ResultList *>(this), p_dc,
        wxRect(p_rect.x + margin, p_rect.y + (p_rect.height - buttonSize) / 2, buttonSize,
               buttonSize),
        row.collapsed ? 0 : static_cast<int>(wxCONTROL_EXPANDED));

    p_dc.SetFont(m_headingFont);
    p_dc.SetTextForeground(SecondaryListTextColour());
    const wxString count = wxString::Format("%zu", row.count);
    const wxSize countSize = p_dc.GetTextExtent(count);
    const int textY = p_rect.y + (p_rect.height - countSize.y) / 2;
    p_dc.DrawText(count, p_rect.GetRight() - margin - countSize.x, textY);

    const int textX = p_rect.x + margin + buttonSize + FromDIP(c_spaceTight + 2);
    const int textWidth =
        p_rect.GetRight() - margin - countSize.x - FromDIP(c_spaceRelated) - textX;
    if (textWidth > 0) {
      p_dc.DrawText(wxControl::Ellipsize(row.source, p_dc, wxELLIPSIZE_END, textWidth,
                                         wxELLIPSIZE_FLAGS_NONE),
                    textX, textY);
    }
    return;
  }

  const CatalogEntry &entry = *row.entry;
  const int selected = IsSelected(p_row) ? 1 : 0;
  const wxColour selectedText = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT);
  const wxColour primary =
      selected ? selectedText : wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
  const wxColour secondary = selected ? selectedText : SecondaryListTextColour();

  const int top = p_rect.y + FromDIP(c_spaceRelated - 2);
  const wxBitmap icon =
      ((entry.format == "efg") ? m_treeIcon : m_tableIcon)[selected].GetBitmapFor(this);
  const wxSize iconSize = icon.GetLogicalSize();
  p_dc.DrawBitmap(icon, p_rect.x + margin, top + (m_lineHeight - iconSize.y) / 2, true);

  const int textX = p_rect.x + margin + iconSize.x + FromDIP(c_spaceRelated);
  const int textWidth = p_rect.GetRight() - margin - textX;
  if (textWidth <= 0) {
    return;
  }

  p_dc.SetFont(m_titleFont);
  p_dc.SetTextForeground(primary);
  p_dc.DrawText(
      wxControl::Ellipsize(entry.title, p_dc, wxELLIPSIZE_END, textWidth, wxELLIPSIZE_FLAGS_NONE),
      textX, top);

  const wxString detail = m_showSource ? entry.group + c_factSeparator + FactsLine(entry, false)
                                       : FactsLine(entry, false);
  p_dc.SetFont(GetFont());
  p_dc.SetTextForeground(secondary);
  p_dc.DrawText(
      wxControl::Ellipsize(detail, p_dc, wxELLIPSIZE_END, textWidth, wxELLIPSIZE_FLAGS_NONE),
      textX, top + m_lineHeight + FromDIP(2));
}

//=========================================================================
//                          class ThumbnailView
//=========================================================================

/// Shows a game's thumbnail scaled to fit, or its form's icon if it has none
class ThumbnailView final : public wxWindow {
public:
  explicit ThumbnailView(wxWindow *p_parent);

  /// Shows p_image, or the icon of form p_format if p_image is not valid; an empty p_format
  /// shows nothing
  void SetImage(const wxImage &p_image, const wxString &p_format);

private:
  wxBitmap m_bitmap;
  wxBitmapBundle m_placeholder;

  void OnPaint(wxPaintEvent &);
};

ThumbnailView::ThumbnailView(wxWindow *p_parent) : wxWindow(p_parent, wxID_ANY)
{
  SetMinSize(wxSize(FromDIP(200), FromDIP(240)));
  Bind(wxEVT_PAINT, &ThumbnailView::OnPaint, this);
  Bind(wxEVT_SIZE, [this](wxSizeEvent &p_event) {
    Refresh();
    p_event.Skip();
  });
}

void ThumbnailView::SetImage(const wxImage &p_image, const wxString &p_format)
{
  m_bitmap = p_image.IsOk() ? wxBitmap(p_image) : wxNullBitmap;
  m_placeholder = (p_image.IsOk() || p_format.empty())
                      ? wxBitmapBundle()
                      : FormIcon(p_format, SecondaryTextColour(this), 48);
  Refresh();
}

void ThumbnailView::OnPaint(wxPaintEvent &)
{
  wxPaintDC dc(this);
  const wxSize area = GetClientSize();

  if (m_bitmap.IsOk()) {
    // Drawing through a graphics context scales from the full-resolution image, so that it
    // stays sharp on high-resolution displays.
    const double scale = std::min({1.0, static_cast<double>(area.x) / m_bitmap.GetWidth(),
                                   static_cast<double>(area.y) / m_bitmap.GetHeight()});
    const double width = m_bitmap.GetWidth() * scale, height = m_bitmap.GetHeight() * scale;
    const double x = (area.x - width) / 2, y = (area.y - height) / 2;
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    gc->SetInterpolationQuality(wxINTERPOLATION_BEST);
    gc->DrawBitmap(m_bitmap, x, y, width, height);
    gc->SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW)));
    gc->SetBrush(*wxTRANSPARENT_BRUSH);
    gc->DrawRectangle(x, y, width, height);
  }
  else if (m_placeholder.IsOk()) {
    const wxBitmap icon = m_placeholder.GetBitmapFor(this);
    const wxSize size = icon.GetLogicalSize();
    dc.DrawBitmap(icon, (area.x - size.x) / 2, (area.y - size.y) / 2, true);
  }
}

//=========================================================================
//                       class CatalogBrowserDialog
//=========================================================================

/// Lets the user search, filter and preview the games of a catalog, and pick one to open
class CatalogBrowserDialog final : public wxDialog {
public:
  CatalogBrowserDialog(wxWindow *p_parent, const wxString &p_catalogRoot,
                       std::vector<CatalogEntry> p_entries);

  /// The chosen game, or null if none is selected
  const CatalogEntry *GetSelectedGame() const { return m_list->GetSelectedGame(); }
  /// Records the search, filters, selection and size for the next time the dialog is shown
  void SaveState() const;

private:
  wxString m_catalogRoot;
  std::vector<CatalogEntry> m_entries;
  /// The lower-cased text searched for each game
  std::vector<wxString> m_searchText;
  /// The values offered by the source and players filters, after "any"
  std::vector<wxString> m_sources;
  std::vector<int> m_playerCounts;

  wxString m_selectedSlug;
  std::set<wxString> m_collapsed;
  /// The row selected before the current one, to step over headings in the same direction
  int m_lastRow{wxNOT_FOUND};
  wxString m_titleLabel, m_sourceLabel, m_descriptionLabel;

  wxSearchCtrl *m_search{nullptr};
  wxChoice *m_sourceFilter{nullptr}, *m_formFilter{nullptr}, *m_playersFilter{nullptr},
      *m_sortOrder{nullptr};
  wxStaticText *m_count{nullptr};
  ResultList *m_list{nullptr};
  wxStaticText *m_noMatches{nullptr};
  wxSizer *m_resultsSizer{nullptr};
  wxPanel *m_preview{nullptr};
  ThumbnailView *m_thumbnail{nullptr};
  wxStaticText *m_title{nullptr}, *m_source{nullptr}, *m_facts{nullptr};
  wxScrolledWindow *m_descriptionWindow{nullptr};
  wxStaticText *m_description{nullptr};
  wxButton *m_openButton{nullptr};

  void CreateControls();
  void RestoreState(const BrowserState &p_state);
  SortOrder GetSortOrder() const;

  /// Lists the games that match the search and filters, keeping the selection if it is listed.
  /// If p_toggledSource names the source whose heading was just clicked, the list keeps its
  /// place, and a selected game that is now hidden gives way to the next game below it.
  void ApplyFilter(const wxString &p_toggledSource = wxEmptyString);
  /// The games that match the search and filters
  std::vector<const CatalogEntry *> FindMatches() const;
  /// The rows listing p_games in the chosen order, under their sources' headings if sorted by
  /// source
  std::vector<ResultRow> BuildRows(std::vector<const CatalogEntry *> p_games) const;
  /// Collapses or expands the source of the heading in row p_row, or every source if p_all
  void ToggleHeading(size_t p_row, bool p_all);
  /// Selects row p_row, or the nearest game to it in direction p_step if it is a heading
  void SelectRow(int p_row, int p_step);
  void UpdatePreview();
  /// Wraps the preview's text to its width
  void WrapPreview();

  void OnListSelect(wxCommandEvent &);
  void OnCharHook(wxKeyEvent &p_event);
};

CatalogBrowserDialog::CatalogBrowserDialog(wxWindow *p_parent, const wxString &p_catalogRoot,
                                           std::vector<CatalogEntry> p_entries)
  : wxDialog(p_parent, wxID_ANY, _("Open from catalog"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_catalogRoot(p_catalogRoot), m_entries(std::move(p_entries))
{
  std::set<wxString> sources;
  std::set<int> playerCounts;
  for (const auto &entry : m_entries) {
    m_searchText.push_back((entry.title + "\n" + entry.category + "\n" + entry.group + "\n" +
                            entry.description + "\n" + entry.slug)
                               .Lower());
    sources.insert(entry.category);
    playerCounts.insert(entry.numPlayers);
  }
  m_sources.assign(sources.begin(), sources.end());
  m_playerCounts.assign(playerCounts.begin(), playerCounts.end());

  CreateControls();
  RestoreState(SessionState());
  ApplyFilter();
  m_search->SetFocus();
}

void CatalogBrowserDialog::CreateControls()
{
  const int related = FromDIP(c_spaceRelated);
  const int margin = FromDIP(c_spaceMargin);
  auto *content = new wxBoxSizer(wxVERTICAL);

  m_search = new wxSearchCtrl(this, wxID_ANY);
  m_search->ShowCancelButton(true);
  m_search->SetDescriptiveText(_("Search titles, sources and descriptions"));
  content->Add(m_search, 0, wxEXPAND | wxBOTTOM, related);

  auto *filters = new wxBoxSizer(wxHORIZONTAL);
  wxArrayString sourceChoices{_("All sources")};
  for (const auto &source : m_sources) {
    sourceChoices.Add(source);
  }
  m_sourceFilter = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sourceChoices);
  filters->Add(m_sourceFilter, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, related);

  const wxArrayString formChoices{_("Any form"), _("Extensive form"), _("Strategic form")};
  m_formFilter = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, formChoices);
  filters->Add(m_formFilter, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, related);

  wxArrayString playersChoices{_("Any number of players")};
  for (const int count : m_playerCounts) {
    playersChoices.Add(PlayersName(count));
  }
  m_playersFilter = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, playersChoices);
  filters->Add(m_playersFilter, 0, wxALIGN_CENTER_VERTICAL);

  filters->AddStretchSpacer();
  m_count = new wxStaticText(this, wxID_STATIC, wxEmptyString);
  m_count->SetForegroundColour(SecondaryTextColour(this));
  filters->Add(m_count, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, margin);
  filters->Add(new wxStaticText(this, wxID_STATIC, _("Sort by")), 0,
               wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(c_spaceTight));
  const wxArrayString sortChoices{_("Source"), _("Title")};
  m_sortOrder = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sortChoices);
  filters->Add(m_sortOrder, 0, wxALIGN_CENTER_VERTICAL);
  content->Add(filters, 0, wxEXPAND | wxBOTTOM, related);

  auto *body = new wxBoxSizer(wxHORIZONTAL);
  m_resultsSizer = new wxBoxSizer(wxVERTICAL);
  m_list = new ResultList(this);
  m_list->SetMinSize(FromDIP(wxSize(360, 320)));
  m_resultsSizer->Add(m_list, 1, wxEXPAND);
  m_noMatches = new wxStaticText(this, wxID_STATIC, _("No games match the search and filters."),
                                 wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
  m_noMatches->SetForegroundColour(SecondaryTextColour(this));
  m_resultsSizer->Add(m_noMatches, 1, wxEXPAND | wxTOP, FromDIP(48));
  m_resultsSizer->Hide(m_noMatches);
  body->Add(m_resultsSizer, 3, wxEXPAND | wxRIGHT, margin);

  m_preview = new wxPanel(this);
  auto *previewSizer = new wxBoxSizer(wxVERTICAL);
  m_thumbnail = new ThumbnailView(m_preview);
  previewSizer->Add(m_thumbnail, 0, wxEXPAND | wxBOTTOM, margin);
  m_title = new wxStaticText(m_preview, wxID_STATIC, wxEmptyString);
  m_title->SetFont(m_title->GetFont().Bold().Larger());
  previewSizer->Add(m_title, 0, wxEXPAND | wxBOTTOM, FromDIP(c_spaceTight));
  m_source = new wxStaticText(m_preview, wxID_STATIC, wxEmptyString);
  m_source->SetForegroundColour(SecondaryTextColour(this));
  previewSizer->Add(m_source, 0, wxEXPAND | wxBOTTOM, FromDIP(c_spaceTight));
  m_facts = new wxStaticText(m_preview, wxID_STATIC, wxEmptyString);
  previewSizer->Add(m_facts, 0, wxEXPAND | wxBOTTOM, related);
  m_descriptionWindow = new wxScrolledWindow(m_preview, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                             wxVSCROLL | wxBORDER_NONE);
  m_descriptionWindow->SetScrollRate(0, FromDIP(10));
  m_description = new wxStaticText(m_descriptionWindow, wxID_STATIC, wxEmptyString);
  auto *descriptionSizer = new wxBoxSizer(wxVERTICAL);
  descriptionSizer->Add(m_description, 0, wxEXPAND);
  m_descriptionWindow->SetSizer(descriptionSizer);
  previewSizer->Add(m_descriptionWindow, 1, wxEXPAND);
  m_preview->SetSizer(previewSizer);
  m_preview->SetMinSize(wxSize(FromDIP(280), -1));
  body->Add(m_preview, 2, wxEXPAND);
  content->Add(body, 1, wxEXPAND);

  SetDialogLayout(this, content, wxOK | wxCANCEL);
  m_openButton = dynamic_cast<wxButton *>(FindWindow(wxID_OK));
  if (m_openButton) {
    m_openButton->SetLabel(_("Open"));
  }
  SetMinSize(GetSize());

  m_search->Bind(wxEVT_TEXT, [this](wxCommandEvent &) { ApplyFilter(); });
  for (auto *choice : {m_sourceFilter, m_formFilter, m_playersFilter, m_sortOrder}) {
    choice->Bind(wxEVT_CHOICE, [this](wxCommandEvent &) { ApplyFilter(); });
  }
  m_list->Bind(wxEVT_LISTBOX, &CatalogBrowserDialog::OnListSelect, this);
  m_list->OnHeadingClick([this](size_t p_row, bool p_all) { ToggleHeading(p_row, p_all); });
  m_list->Bind(wxEVT_LISTBOX_DCLICK, [this](wxCommandEvent &) {
    if (GetSelectedGame()) {
      EndModal(wxID_OK);
    }
  });
  m_preview->Bind(wxEVT_SIZE, [this](wxSizeEvent &) { WrapPreview(); });
  Bind(wxEVT_CHAR_HOOK, &CatalogBrowserDialog::OnCharHook, this);
}

void CatalogBrowserDialog::RestoreState(const BrowserState &p_state)
{
  m_search->ChangeValue(p_state.search);
  const auto source = std::ranges::find(m_sources, p_state.source);
  m_sourceFilter->SetSelection(
      (source == m_sources.end()) ? 0 : static_cast<int>(source - m_sources.begin()) + 1);
  m_formFilter->SetSelection((p_state.format == "efg") ? 1 : (p_state.format == "nfg") ? 2 : 0);
  const auto players = std::ranges::find(m_playerCounts, p_state.players);
  m_playersFilter->SetSelection((players == m_playerCounts.end())
                                    ? 0
                                    : static_cast<int>(players - m_playerCounts.begin()) + 1);
  m_sortOrder->SetSelection((p_state.sort == SortOrder::Title) ? 1 : 0);
  m_selectedSlug = p_state.selectedSlug;
  m_collapsed = p_state.collapsed;

  wxSize size = (p_state.size == wxDefaultSize) ? FromDIP(wxSize(920, 620)) : p_state.size;
  size.IncTo(GetMinSize());
  SetSize(size);
  CentreOnParent();
}

void CatalogBrowserDialog::SaveState() const
{
  BrowserState &state = SessionState();
  state.search = m_search->GetValue();
  const int source = m_sourceFilter->GetSelection();
  state.source = (source > 0) ? m_sources[source - 1] : wxString();
  const int form = m_formFilter->GetSelection();
  state.format = (form == 1) ? "efg" : (form == 2) ? "nfg" : "";
  const int players = m_playersFilter->GetSelection();
  state.players = (players > 0) ? m_playerCounts[players - 1] : 0;
  state.sort = GetSortOrder();
  state.selectedSlug = m_selectedSlug;
  state.collapsed = m_collapsed;
  state.size = GetSize();
}

SortOrder CatalogBrowserDialog::GetSortOrder() const
{
  return (m_sortOrder->GetSelection() == 1) ? SortOrder::Title : SortOrder::Source;
}

std::vector<const CatalogEntry *> CatalogBrowserDialog::FindMatches() const
{
  const wxArrayString words = wxSplit(m_search->GetValue().Lower(), ' ', '\0');
  const int sourceIndex = m_sourceFilter->GetSelection();
  const wxString source = (sourceIndex > 0) ? m_sources[sourceIndex - 1] : wxString();
  const int formIndex = m_formFilter->GetSelection();
  const wxString format = (formIndex == 1) ? "efg" : (formIndex == 2) ? "nfg" : "";
  const int playersIndex = m_playersFilter->GetSelection();
  const int players = (playersIndex > 0) ? m_playerCounts[playersIndex - 1] : 0;

  std::vector<const CatalogEntry *> matches;
  for (size_t i = 0; i < m_entries.size(); i++) {
    const CatalogEntry &entry = m_entries[i];
    const bool passesFilters = (source.empty() || entry.category == source) &&
                               (format.empty() || entry.format == format) &&
                               (players == 0 || entry.numPlayers == players);
    if (passesFilters && std::ranges::all_of(words, [&](const wxString &word) {
          return word.empty() || m_searchText[i].Contains(word);
        })) {
      matches.push_back(&entry);
    }
  }
  return matches;
}

std::vector<ResultRow>
CatalogBrowserDialog::BuildRows(std::vector<const CatalogEntry *> p_games) const
{
  const SortOrder order = GetSortOrder();
  std::ranges::sort(p_games, [order](const CatalogEntry *a, const CatalogEntry *b) {
    if (order == SortOrder::Source) {
      if (const int c = a->category.CmpNoCase(b->category); c != 0) {
        return c < 0;
      }
      if (const int c = wxCmpNatural(a->group, b->group); c != 0) {
        return c < 0;
      }
    }
    return wxCmpNatural(a->title, b->title) < 0;
  });

  std::vector<ResultRow> rows;
  if (order == SortOrder::Title) {
    std::ranges::transform(p_games, std::back_inserter(rows), GameRow);
    return rows;
  }
  for (auto game = p_games.begin(); game != p_games.end();) {
    const auto end = std::find_if(game, p_games.end(), [&](const CatalogEntry *e) {
      return e->category != (*game)->category || e->group != (*game)->group;
    });
    const wxString source = SourceName(**game);
    const bool collapsed = m_collapsed.contains(source);
    rows.push_back(HeadingRow(source, static_cast<size_t>(end - game), collapsed));
    if (!collapsed) {
      std::transform(game, end, std::back_inserter(rows), GameRow);
    }
    game = end;
  }
  return rows;
}

void CatalogBrowserDialog::ApplyFilter(const wxString &p_toggledSource)
{
  const std::vector<const CatalogEntry *> matches = FindMatches();
  m_list->SetRows(BuildRows(matches), GetSortOrder() == SortOrder::Title,
                  !p_toggledSource.empty());

  m_count->SetLabel(
      (matches.size() == m_entries.size())
          ? wxString::Format(wxPLURAL("%zu game", "%zu games", m_entries.size()), m_entries.size())
          : wxString::Format(_("%zu of %zu games"), matches.size(), m_entries.size()));
  m_resultsSizer->Show(m_list, !matches.empty());
  m_resultsSizer->Show(m_noMatches, matches.empty());
  GetSizer()->Layout();

  // Selecting scrolls the selection into view; after a click on a heading, the list stays put.
  const size_t top = m_list->GetVisibleRowsBegin();
  m_lastRow = wxNOT_FOUND;
  if (const auto row = m_list->FindGame(m_selectedSlug)) {
    SelectRow(static_cast<int>(*row), 1);
  }
  else if (const auto heading = m_list->FindHeading(p_toggledSource)) {
    SelectRow(static_cast<int>(*heading), 1);
  }
  else {
    SelectRow(0, 1);
  }
  if (!p_toggledSource.empty() && m_list->GetItemCount() > 0) {
    m_list->ScrollToRow(top);
  }
}

void CatalogBrowserDialog::ToggleHeading(size_t p_row, bool p_all)
{
  const ResultRow &heading = m_list->GetRow(p_row);
  const wxString source = heading.source;
  if (p_all) {
    const bool collapse = !heading.collapsed;
    m_collapsed.clear();
    if (collapse) {
      for (const auto &entry : m_entries) {
        m_collapsed.insert(SourceName(entry));
      }
    }
  }
  else if (heading.collapsed) {
    m_collapsed.erase(source);
  }
  else {
    m_collapsed.insert(source);
  }
  ApplyFilter(source);
}

void CatalogBrowserDialog::SelectRow(int p_row, int p_step)
{
  auto row = m_list->FindGameFrom(p_row, p_step);
  if (!row) {
    row = m_list->FindGameFrom(p_row, -p_step);
  }
  m_list->SetSelection(row ? static_cast<int>(*row) : wxNOT_FOUND);
  m_lastRow = m_list->GetSelection();
  if (const auto *entry = m_list->GetSelectedGame()) {
    m_selectedSlug = entry->slug;
  }
  UpdatePreview();
}

void CatalogBrowserDialog::OnListSelect(wxCommandEvent &)
{
  const int row = m_list->GetSelection();
  if (row == wxNOT_FOUND) {
    UpdatePreview();
    return;
  }
  SelectRow(row, (m_lastRow == wxNOT_FOUND || row >= m_lastRow) ? 1 : -1);
}

void CatalogBrowserDialog::OnCharHook(wxKeyEvent &p_event)
{
  // Up and down in the search field move through the results, so a game can be found and
  // opened without leaving the keyboard.
  const int key = p_event.GetKeyCode();
  const wxWindow *focus = FindFocus();
  while (focus && focus != m_search) {
    focus = focus->GetParent();
  }
  if (focus && (key == WXK_UP || key == WXK_DOWN) && m_list->GetItemCount() > 0) {
    const int step = (key == WXK_DOWN) ? 1 : -1;
    const int current = m_list->GetSelection();
    SelectRow((current == wxNOT_FOUND) ? 0 : current + step, step);
    return;
  }
  p_event.Skip();
}

void CatalogBrowserDialog::UpdatePreview()
{
  const CatalogEntry *entry = m_list->GetSelectedGame();
  if (m_openButton) {
    m_openButton->Enable(entry != nullptr);
  }
  if (!entry) {
    m_titleLabel.clear();
    m_sourceLabel.clear();
    m_descriptionLabel.clear();
    m_facts->SetLabel(wxEmptyString);
    m_thumbnail->SetImage(wxNullImage, wxEmptyString);
    WrapPreview();
    return;
  }

  m_titleLabel = entry->title;
  m_sourceLabel = SourceName(*entry);
  m_descriptionLabel = Reflow(entry->description);
  m_facts->SetLabel(FactsLine(*entry, true));

  const wxString thumbnailPath = CatalogThumbnailPath(m_catalogRoot, *entry);
  wxImage image;
  if (!wxFileExists(thumbnailPath) || !image.LoadFile(thumbnailPath, wxBITMAP_TYPE_PNG)) {
    image = wxNullImage;
  }
  m_thumbnail->SetImage(image, entry->format);
  WrapPreview();
}

void CatalogBrowserDialog::WrapPreview()
{
  const int width = m_preview->GetClientSize().x;
  if (width <= 0) {
    return;
  }
  m_title->SetLabelText(m_titleLabel);
  m_title->Wrap(width);
  m_source->SetLabelText(m_sourceLabel);
  m_source->Wrap(width);
  // Leave room for a scroll bar beside the description
  m_description->SetLabelText(m_descriptionLabel);
  m_description->Wrap(std::max(1, width - FromDIP(16)));
  m_preview->Layout();
  m_descriptionWindow->FitInside();
  m_descriptionWindow->Scroll(0, 0);
}

} // namespace

std::optional<CatalogChoice> ChooseCatalogGame(wxWindow *p_parent)
{
  const wxString root = FindCatalogRoot();
  if (root.IsEmpty()) {
    ShowOperationError(p_parent, _("Catalog not found"), _("Gambit could not find the catalog."),
                       _("The catalog is installed with Gambit; reinstalling Gambit should "
                         "restore it."));
    return {};
  }
  wxString error;
  auto catalog = LoadCatalogManifest(root, &error);
  if (!error.IsEmpty()) {
    ShowOperationError(p_parent, _("Unable to read catalog"),
                       _("Gambit could not read the catalog."), error);
    return {};
  }
  if (catalog.entries.empty()) {
    ShowOperationError(p_parent, _("Empty catalog"), _("The catalog of games has no games."),
                       root);
    return {};
  }
  CatalogBrowserDialog dialog(p_parent, root, std::move(catalog.entries));
  const bool chosen = dialog.ShowModal() == wxID_OK && dialog.GetSelectedGame();
  dialog.SaveState();
  if (!chosen) {
    return {};
  }
  const CatalogEntry &entry = *dialog.GetSelectedGame();
  return CatalogChoice{
      .path = CatalogGameFilePath(root, entry), .slug = entry.slug, .version = catalog.version};
}

} // namespace Gambit::GUI
