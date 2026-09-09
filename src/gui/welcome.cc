#include <vector>

#include <wx/bmpbndl.h>
#include <wx/button.h>
#include <wx/commandlinkbutton.h>
#include <wx/dcgraph.h>
#include <wx/dcmemory.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/popupwin.h>
#include <wx/settings.h>
#include <wx/time.h>
#include <wx/vlbox.h>

#include "games.h"

#include "welcome.h"
#include "app.h"
#include "dlnewtable.h"
#include "gamedoc.h"
#include "gameframe.h"

using namespace Gambit;
using namespace Gambit::GUI;

wxDEFINE_EVENT(wxEVT_WELCOME_OPEN, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_WELCOME_NEW, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_WELCOME_OPEN_RECENT, wxCommandEvent);

namespace {
constexpr int ID_WELCOME_OPEN = wxID_HIGHEST + 100;
constexpr int ID_WELCOME_NEW_NORMAL_FORM = wxID_HIGHEST + 101;
constexpr int ID_WELCOME_NEW_EXTENSIVE_FORM = wxID_HIGHEST + 102;
constexpr int ID_WELCOME_RECENT = wxID_HIGHEST + 103;

constexpr int BUTTON_WIDTH = 340;
constexpr int BUTTON_HEIGHT = 72;
/// The width of the dropdown part of the "Open existing game" split button.
constexpr int RECENT_BUTTON_WIDTH = 36;

/// Write the user's home directory as "~".
wxString AbbreviateHomeDirectory(const wxString &p_path)
{
  const wxString homeDir = wxFileName::GetHomeDir();
  if (!homeDir.empty() &&
      (p_path == homeDir || p_path.StartsWith(homeDir + wxFileName::GetPathSeparator()))) {
    return wxT("~") + p_path.Mid(homeDir.length());
  }
  return p_path;
}

/// A downward-pointing triangle of the given size, on a transparent background.
wxBitmap DrawDropArrow(int p_size, const wxColour &p_colour)
{
  wxBitmap bitmap(p_size, p_size, 32);
  bitmap.UseAlpha();
  {
    wxMemoryDC dc(bitmap);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    wxGCDC gc(dc);
    gc.SetPen(*wxTRANSPARENT_PEN);
    gc.SetBrush(wxBrush(p_colour));
    // Ten wide by five high, centred, in units of a 24-pixel canvas.
    const wxPoint points[] = {wxPoint(p_size * 7 / 24, p_size * 10 / 24),
                              wxPoint(p_size * 17 / 24, p_size * 10 / 24),
                              wxPoint(p_size * 12 / 24, p_size * 15 / 24)};
    gc.DrawPolygon(3, points);
  }
  return bitmap;
}

/// The marker for the dropdown half of the "Open" split button.
wxBitmapBundle MakeDropArrowBitmap()
{
  const wxColour colour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
  return wxBitmapBundle::FromBitmaps(DrawDropArrow(24, colour), DrawDropArrow(48, colour));
}
} // namespace

// --------------------
// WelcomePanel
// --------------------

WelcomePanel::WelcomePanel(wxWindow *parent) : wxPanel(parent)
{
  CreateControls();
  LayoutControls();
}

#include "bitmaps/newtable.h"
#include "bitmaps/newtree.h"
#include "bitmaps/open.h"
#include "bitmaps/gambitbig.xpm"

namespace {

wxBitmap MakeScaledBitmap(const char *const *xpm, int targetHeight)
{
  wxImage image(xpm);

  if (!image.IsOk()) {
    return wxBitmap();
  }

  const int width = image.GetWidth();
  const int height = image.GetHeight();
  if (width <= 0 || height <= 0) {
    return wxBitmap();
  }

  const int targetWidth = (width * targetHeight) / height;
  image.Rescale(targetWidth, targetHeight, wxIMAGE_QUALITY_HIGH);
  return wxBitmap(image);
}

} // namespace

// --------------------
// RecentFilesPopup
// --------------------

/// The rows of the recent-files dropdown: the file name on the left, its directory on the right.
class RecentFilesList : public wxVListBox {
public:
  RecentFilesList(RecentFilesPopup *p_popup, int p_width);

  void SetFiles(const std::vector<wxString> &p_files);
  const wxString &GetFile(size_t n) const { return m_files[n]; }

protected:
  void OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const override;
  void OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const override;
  wxCoord OnMeasureItem(size_t) const override { return m_rowHeight; }

private:
  void OnMotion(wxMouseEvent &p_event);
  void OnLeftUp(wxMouseEvent &p_event);
  void OnKeyDown(wxKeyEvent &p_event);

  RecentFilesPopup *m_popup;
  std::vector<wxString> m_files;
  int m_rowHeight;
};

/// The dropdown attached to the "Open existing game" button, listing the recently-used files.
class RecentFilesPopup : public wxPopupTransientWindow {
public:
  RecentFilesPopup(WelcomePanel *p_panel, int p_width);

  /// Re-read the application's file history; returns false if there is nothing to show.
  bool Populate();
  /// Show the dropdown just below the given rectangle, given in screen coordinates.
  void ShowBelow(const wxRect &p_anchor);
  /// Open the n-th listed file.
  void Activate(size_t n);
  /// Whether the dropdown was dismissed a moment ago, e.g. by the click now being handled.
  bool WasJustDismissed() const;

protected:
  void OnDismiss() override;

private:
  WelcomePanel *m_panel;
  RecentFilesList *m_list{nullptr};
  wxLongLong m_dismissedAt{0};
};

RecentFilesList::RecentFilesList(RecentFilesPopup *p_popup, int p_width)
  : wxVListBox(p_popup, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_popup(p_popup), m_rowHeight(GetCharHeight() + 12)
{
  const wxColour window = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
  SetBackgroundColour(window);
  // Highlight the row under the mouse with a light touch of the window colour.
  SetSelectionBackground(
      window.ChangeLightness(wxSystemSettings::GetAppearance().IsDark() ? 115 : 92));
  SetMinSize(wxSize(p_width, m_rowHeight));
  Bind(wxEVT_MOTION, &RecentFilesList::OnMotion, this);
  Bind(wxEVT_LEFT_UP, &RecentFilesList::OnLeftUp, this);
  Bind(wxEVT_KEY_DOWN, &RecentFilesList::OnKeyDown, this);
}

void RecentFilesList::SetFiles(const std::vector<wxString> &p_files)
{
  m_files = p_files;
  SetItemCount(m_files.size());
  SetMinSize(wxSize(GetMinWidth(), static_cast<int>(m_files.size()) * m_rowHeight));
  if (!m_files.empty()) {
    SetSelection(0);
  }
  RefreshAll();
}

void RecentFilesList::OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const
{
  if (IsSelected(n)) {
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(GetSelectionBackground()));
    dc.DrawRectangle(rect);
  }
}

void RecentFilesList::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const
{
  constexpr int margin = 12;
  constexpr int gap = 16;
  const wxFileName path(m_files[n]);

  dc.SetFont(GetFont());

  // The name gets at most three-fifths of the row and is shortened in the middle.
  const wxString name =
      wxControl::Ellipsize(path.GetFullName(), dc, wxELLIPSIZE_MIDDLE,
                           (rect.width - 2 * margin) * 3 / 5, wxELLIPSIZE_FLAGS_NONE);
  const wxSize nameSize = dc.GetTextExtent(name);
  dc.SetTextForeground(GetForegroundColour());
  dc.DrawText(name, rect.x + margin, rect.y + (rect.height - nameSize.y) / 2);

  const int directoryWidth = rect.width - 2 * margin - nameSize.x - gap;
  if (directoryWidth <= 0) {
    return;
  }
  const wxString directory =
      wxControl::Ellipsize(AbbreviateHomeDirectory(path.GetPath()), dc, wxELLIPSIZE_START,
                           directoryWidth, wxELLIPSIZE_FLAGS_NONE);
  const wxSize directorySize = dc.GetTextExtent(directory);
  dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
  dc.DrawText(directory, rect.x + rect.width - margin - directorySize.x,
              rect.y + (rect.height - directorySize.y) / 2);
}

void RecentFilesList::OnMotion(wxMouseEvent &p_event)
{
  // Follow the mouse with the highlight, as a menu does.
  const int item = VirtualHitTest(p_event.GetPosition().y);
  if (item == wxNOT_FOUND) {
    UnsetToolTip();
  }
  else {
    SetSelection(item);
    if (GetToolTipText() != m_files[item]) {
      SetToolTip(m_files[item]);
    }
  }
  p_event.Skip();
}

void RecentFilesList::OnLeftUp(wxMouseEvent &p_event)
{
  const int item = VirtualHitTest(p_event.GetPosition().y);
  if (item != wxNOT_FOUND) {
    m_popup->Activate(static_cast<size_t>(item));
  }
  else {
    p_event.Skip();
  }
}

void RecentFilesList::OnKeyDown(wxKeyEvent &p_event)
{
  switch (p_event.GetKeyCode()) {
  case WXK_RETURN:
  case WXK_NUMPAD_ENTER:
    if (GetSelection() != wxNOT_FOUND) {
      m_popup->Activate(static_cast<size_t>(GetSelection()));
    }
    break;
  case WXK_ESCAPE:
    m_popup->Dismiss();
    break;
  default:
    // Up, Down, Home and End are handled by wxVListBox itself.
    p_event.Skip();
  }
}

RecentFilesPopup::RecentFilesPopup(WelcomePanel *p_panel, int p_width)
  : wxPopupTransientWindow(p_panel, wxBORDER_SIMPLE), m_panel(p_panel)
{
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

  auto *heading = new wxStaticText(this, wxID_ANY, _("Recent games"));
  auto headingFont = heading->GetFont();
  headingFont.MakeSmaller();
  heading->SetFont(headingFont);
  heading->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

  m_list = new RecentFilesList(this, p_width);

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(heading, 0, wxLEFT | wxRIGHT | wxTOP, 12);
  sizer->Add(m_list, 0, wxEXPAND | wxTOP | wxBOTTOM, 6);
  SetSizer(sizer);
}

bool RecentFilesPopup::Populate()
{
  const auto files = wxGetApp().GetRecentFiles();
  m_list->SetFiles(files);
  GetSizer()->Fit(this);
  return !files.empty();
}

void RecentFilesPopup::ShowBelow(const wxRect &p_anchor)
{
  // Position() puts the popup below and to the *right* of the rectangle it is given.
  Position(p_anchor.GetTopLeft(), wxSize(0, p_anchor.GetHeight()));
  Popup(m_list);
}

void RecentFilesPopup::Activate(size_t n)
{
  Dismiss();
  // Deferred so that the dropdown has fully closed before any dialog can come up.
  m_panel->CallAfter(
      [panel = m_panel, filename = m_list->GetFile(n)]() { panel->OpenRecentFile(filename); });
}

void RecentFilesPopup::OnDismiss() { m_dismissedAt = wxGetUTCTimeMillis(); }

bool RecentFilesPopup::WasJustDismissed() const
{
  return wxGetUTCTimeMillis() - m_dismissedAt < 250;
}

// --------------------
// WelcomePanel
// --------------------

void WelcomePanel::CreateControls()
{
  m_logoBitmap = new wxStaticBitmap(this, wxID_ANY, MakeScaledBitmap(gambitbig_xpm, 72));

  m_titleText = new wxStaticText(this, wxID_ANY, "Welcome to Gambit");
  auto titleFont = m_titleText->GetFont();
  titleFont.SetPointSize(titleFont.GetPointSize() + 6);
  titleFont.SetWeight(wxFONTWEIGHT_BOLD);
  m_titleText->SetFont(titleFont);

  m_openButton = new wxCommandLinkButton(this, ID_WELCOME_OPEN, "Open existing game",
                                         "Load a saved .efg or .nfg file");

  m_newNormalFormButton =
      new wxCommandLinkButton(this, ID_WELCOME_NEW_NORMAL_FORM, "New strategic form game",
                              "Create a game with a strategic representation");

  m_newExtensiveFormButton =
      new wxCommandLinkButton(this, ID_WELCOME_NEW_EXTENSIVE_FORM, "New extensive form game",
                              "Create a game with a tree representation");

  m_openButton->SetBitmap(wxBitmapBundle::FromSVG(open_svg, wxSize(24, 24)));
  m_openButton->SetBitmapMargins(24, 12);

  m_newNormalFormButton->SetBitmap(wxBitmapBundle::FromSVG(newtable_svg, wxSize(24, 24)));
  m_newNormalFormButton->SetBitmapMargins(24, 12);

  m_newExtensiveFormButton->SetBitmap(wxBitmapBundle::FromSVG(newtree_svg, wxSize(24, 24)));
  m_newExtensiveFormButton->SetBitmapMargins(24, 12);

  // The "Open" button is a split button: the dropdown to its right lists recent files.
  m_openButton->SetMinSize(wxSize(BUTTON_WIDTH - RECENT_BUTTON_WIDTH, BUTTON_HEIGHT));
  m_newNormalFormButton->SetMinSize(wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));
  m_newExtensiveFormButton->SetMinSize(wxSize(BUTTON_WIDTH, BUTTON_HEIGHT));

  m_recentButton = new wxButton(this, ID_WELCOME_RECENT, wxEmptyString, wxDefaultPosition,
                                wxDefaultSize, wxBU_EXACTFIT);
  m_recentButton->SetBitmap(MakeDropArrowBitmap());
  m_recentButton->SetMinSize(wxSize(RECENT_BUTTON_WIDTH, BUTTON_HEIGHT));
  RefreshRecentFiles();

  Bind(wxEVT_BUTTON, &WelcomePanel::OnOpen, this, ID_WELCOME_OPEN);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnNewNormalForm, this, ID_WELCOME_NEW_NORMAL_FORM);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnNewExtensiveForm, this, ID_WELCOME_NEW_EXTENSIVE_FORM);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnShowRecent, this, ID_WELCOME_RECENT);
}

void WelcomePanel::RefreshRecentFiles()
{
  const bool hasRecentFiles = !wxGetApp().GetRecentFiles().empty();
  m_recentButton->Enable(hasRecentFiles);
  m_recentButton->SetToolTip(hasRecentFiles ? _("Recent games") : _("No recent games"));
}

void WelcomePanel::LayoutControls()
{
  auto *outer = new wxBoxSizer(wxVERTICAL);
  auto *row = new wxBoxSizer(wxHORIZONTAL);
  auto *content = new wxBoxSizer(wxVERTICAL);

  content->Add(m_logoBitmap, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 16);
  content->Add(m_titleText, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 20);

  auto *openRow = new wxBoxSizer(wxHORIZONTAL);
  openRow->Add(m_openButton, 1, wxEXPAND);
  openRow->Add(m_recentButton, 0, wxEXPAND);

  content->Add(openRow, 0, wxEXPAND | wxBOTTOM, 16);
  content->Add(m_newNormalFormButton, 0, wxEXPAND | wxBOTTOM, 16);
  content->Add(m_newExtensiveFormButton, 0, wxEXPAND, 0);

  row->AddStretchSpacer(1);
  row->Add(content, 0, wxEXPAND | wxLEFT | wxRIGHT, 24);
  row->AddStretchSpacer(1);

  outer->AddStretchSpacer(1);
  outer->Add(row, 0, wxEXPAND);
  outer->AddStretchSpacer(1);

  SetSizer(outer);
}

void WelcomePanel::OnOpen(wxCommandEvent &) { SendOpenEvent(); }

void WelcomePanel::OnNewNormalForm(wxCommandEvent &)
{
  SendNewEvent(WelcomeNewProblemKind::NormalForm);
}

void WelcomePanel::OnNewExtensiveForm(wxCommandEvent &)
{
  SendNewEvent(WelcomeNewProblemKind::ExtensiveForm);
}

void WelcomePanel::OnShowRecent(wxCommandEvent &)
{
  if (!m_recentPopup) {
    m_recentPopup = new RecentFilesPopup(this, BUTTON_WIDTH);
  }
  if (m_recentPopup->WasJustDismissed()) {
    return;
  }
  if (!m_recentPopup->Populate()) {
    RefreshRecentFiles();
    return;
  }
  m_recentPopup->ShowBelow(m_openButton->GetScreenRect());
}

void WelcomePanel::OpenRecentFile(const wxString &p_filename)
{
  wxCommandEvent event(wxEVT_WELCOME_OPEN_RECENT);
  event.SetEventObject(this);
  event.SetString(p_filename);
  GetParent()->ProcessWindowEvent(event);
}

void WelcomePanel::SendOpenEvent()
{
  wxCommandEvent event(wxEVT_WELCOME_OPEN);
  event.SetEventObject(this);
  GetParent()->ProcessWindowEvent(event);
}

void WelcomePanel::SendNewEvent(WelcomeNewProblemKind p_kind)
{
  wxCommandEvent event(wxEVT_WELCOME_NEW);
  event.SetEventObject(this);
  event.SetInt(static_cast<int>(p_kind));
  GetParent()->ProcessWindowEvent(event);
}

// --------------------
// WelcomeFrame
// --------------------

WelcomeFrame::WelcomeFrame(wxWindow *parent)
  : wxFrame(parent, wxID_ANY, wxT("Gambit"), wxDefaultPosition, wxSize(600, 500),
            wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
  CreateControls();
  LayoutControls();

  Bind(wxEVT_WELCOME_OPEN, &WelcomeFrame::OnWelcomeOpen, this);
  Bind(wxEVT_WELCOME_NEW, &WelcomeFrame::OnWelcomeNew, this);
  Bind(wxEVT_WELCOME_OPEN_RECENT, &WelcomeFrame::OnWelcomeOpenRecent, this);
  Bind(wxEVT_CLOSE_WINDOW, &WelcomeFrame::OnClose, this);

  CentreOnScreen();
}

void WelcomeFrame::CreateControls() { m_panel = new WelcomePanel(this); }

void WelcomeFrame::LayoutControls()
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_panel, 1, wxEXPAND);
  SetSizer(topSizer);
}

void WelcomeFrame::OnWelcomeOpen(wxCommandEvent &)
{
  if (DoOpen()) {
    Destroy();
  }
}

void WelcomeFrame::OnWelcomeNew(wxCommandEvent &p_event)
{
  const auto kind = static_cast<WelcomeNewProblemKind>(p_event.GetInt());

  if (DoCreateNew(kind)) {
    Destroy();
  }
}

void WelcomeFrame::OnWelcomeOpenRecent(wxCommandEvent &p_event)
{
  if (DoOpenRecent(p_event.GetString())) {
    Destroy();
  }
}

void WelcomeFrame::OnClose(wxCloseEvent &p_event) { p_event.Skip(); }

bool WelcomeFrame::DoOpen()
{
  wxFileDialog dialog(
      this, _("Choose file to open"), wxGetApp().GetCurrentDir(), _T(""),
      wxT("Gambit workbooks (*.gbt)|*.gbt|") wxT("Gambit extensive games (*.efg)|*.efg|")
          wxT("Gambit strategic games (*.nfg)|*.nfg|") wxT("All files (*.*)|*.*"));

  if (dialog.ShowModal() != wxID_OK) {
    return false;
  }
  const wxString filename = dialog.GetPath();
  wxGetApp().SetCurrentDir(wxPathOnly(filename));
  return wxGetApp().LoadFile(filename, this) == AppLoadResult::Success;
}

bool WelcomeFrame::DoOpenRecent(const wxString &p_filename)
{
  if (wxGetApp().LoadFile(p_filename, this) != AppLoadResult::Success) {
    m_panel->RefreshRecentFiles();
    return false;
  }
  wxGetApp().SetCurrentDir(wxPathOnly(p_filename));
  return true;
}

bool WelcomeFrame::DoCreateNew(WelcomeNewProblemKind p_kind)
{
  switch (p_kind) {
  case WelcomeNewProblemKind::NormalForm: {
    NewTableDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK) {
      return false;
    }
    new GameFrame(nullptr, NewTableDocument(dialog.GetDimensions()));
  } break;
  case WelcomeNewProblemKind::ExtensiveForm:
    new GameFrame(nullptr, NewTreeDocument());
    break;
  }

  return true;
}
