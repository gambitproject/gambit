
#include <algorithm>

#include <wx/button.h>
#include <wx/commandlinkbutton.h>
#include <wx/dcclient.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/hyperlink.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/statline.h>

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

/// The application tracks more recently-used files, showing only the first few.
constexpr int MAX_RECENT_FILES = 5;
constexpr int ID_WELCOME_RECENT_FIRST = wxID_HIGHEST + 110;
constexpr int ID_WELCOME_RECENT_LAST = ID_WELCOME_RECENT_FIRST + MAX_RECENT_FILES - 1;

/// Drop `p_filename` from the application's most-recently-used list, if it is there.
void RemoveFromFileHistory(const wxString &p_filename)
{
  const auto count = static_cast<int>(wxGetApp().GetHistoryCount());
  for (int i = 0; i < count; i++) {
    if (wxGetApp().GetHistoryFile(i) == p_filename) {
      wxGetApp().RemoveHistoryFile(i);
      return;
    }
  }
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

  const wxSize buttonSize(340, 72);
  m_openButton->SetMinSize(buttonSize);
  m_newNormalFormButton->SetMinSize(buttonSize);
  m_newExtensiveFormButton->SetMinSize(buttonSize);

  m_recentLine = new wxStaticLine(this);
  m_recentHeading = new wxStaticText(this, wxID_ANY, _("Recent games"));
  auto headingFont = m_recentHeading->GetFont();
  headingFont.SetWeight(wxFONTWEIGHT_BOLD);
  m_recentHeading->SetFont(headingFont);

  m_recentPanel = new wxPanel(this);
  RefreshRecentFiles();

  Bind(wxEVT_BUTTON, &WelcomePanel::OnOpen, this, ID_WELCOME_OPEN);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnNewNormalForm, this, ID_WELCOME_NEW_NORMAL_FORM);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnNewExtensiveForm, this, ID_WELCOME_NEW_EXTENSIVE_FORM);
  Bind(wxEVT_HYPERLINK, &WelcomePanel::OnOpenRecent, this, ID_WELCOME_RECENT_FIRST,
       ID_WELCOME_RECENT_LAST);
}

bool WelcomePanel::RefreshRecentFiles()
{
  m_recentFiles.clear();
  m_recentPanel->DestroyChildren();

  auto *sizer = new wxBoxSizer(wxVERTICAL);

  const auto count = std::min(static_cast<int>(wxGetApp().GetHistoryCount()), MAX_RECENT_FILES);
  for (int i = 0; i < count; i++) {
    const wxString filename = wxGetApp().GetHistoryFile(i);
    if (filename.empty()) {
      continue;
    }

    const wxFileName path(filename);
    const auto id = ID_WELCOME_RECENT_FIRST + static_cast<int>(m_recentFiles.size());

    auto *row = new wxBoxSizer(wxHORIZONTAL);

    const int rowWidth = m_openButton->GetMinSize().GetWidth();
    const int gap = 8;
    const wxClientDC dc(m_recentPanel);
    const wxString name = wxControl::Ellipsize(path.GetFullName(), dc, wxELLIPSIZE_MIDDLE,
                                               rowWidth * 2 / 3, wxELLIPSIZE_FLAGS_NONE);

    auto *link =
        new wxHyperlinkCtrl(m_recentPanel, id, wxControl::EscapeMnemonics(name), wxEmptyString,
                            wxDefaultPosition, wxDefaultSize, wxHL_ALIGN_LEFT | wxNO_BORDER);
    link->SetVisitedColour(link->GetNormalColour());
    link->SetToolTip(filename);
    row->Add(link, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);

    auto *directory = new wxStaticText(m_recentPanel, wxID_ANY, wxEmptyString);
    directory->SetLabelText(wxControl::Ellipsize(path.GetPath(), dc, wxELLIPSIZE_MIDDLE,
                                                 rowWidth - link->GetBestSize().GetWidth() - gap,
                                                 wxELLIPSIZE_FLAGS_NONE));
    directory->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    directory->SetToolTip(filename);
    row->Add(directory, 0, wxALIGN_CENTER_VERTICAL);

    sizer->Add(row, 0, wxBOTTOM, 4);
    m_recentFiles.push_back(filename);
  }

  m_recentPanel->SetSizer(sizer);
  m_recentPanel->Layout();

  const bool hasRecentFiles = !m_recentFiles.empty();
  m_recentLine->Show(hasRecentFiles);
  m_recentHeading->Show(hasRecentFiles);
  m_recentPanel->Show(hasRecentFiles);
  return hasRecentFiles;
}

void WelcomePanel::LayoutControls()
{
  auto *outer = new wxBoxSizer(wxVERTICAL);
  auto *row = new wxBoxSizer(wxHORIZONTAL);
  auto *content = new wxBoxSizer(wxVERTICAL);

  content->Add(m_logoBitmap, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 16);
  content->Add(m_titleText, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 20);

  content->Add(m_openButton, 0, wxEXPAND | wxBOTTOM, 16);
  content->Add(m_newNormalFormButton, 0, wxEXPAND | wxBOTTOM, 16);
  content->Add(m_newExtensiveFormButton, 0, wxEXPAND, 0);

  // Hidden, and contributing no height, when there is no file history to show.
  content->Add(m_recentLine, 0, wxEXPAND | wxTOP | wxBOTTOM, 20);
  content->Add(m_recentHeading, 0, wxBOTTOM, 8);
  content->Add(m_recentPanel, 0, wxEXPAND);

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

void WelcomePanel::OnOpenRecent(wxCommandEvent &p_event)
{
  const auto index = static_cast<size_t>(p_event.GetId() - ID_WELCOME_RECENT_FIRST);
  if (index < m_recentFiles.size()) {
    SendOpenRecentEvent(m_recentFiles[index]);
  }
}

void WelcomePanel::SendOpenEvent()
{
  wxCommandEvent event(wxEVT_WELCOME_OPEN);
  event.SetEventObject(this);
  GetParent()->ProcessWindowEvent(event);
}

void WelcomePanel::SendOpenRecentEvent(const wxString &p_filename)
{
  wxCommandEvent event(wxEVT_WELCOME_OPEN_RECENT);
  event.SetEventObject(this);
  event.SetString(p_filename);
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
  : wxFrame(parent, wxID_ANY, wxT("Gambit"), wxDefaultPosition, wxDefaultSize,
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
  FitToContents();
}

void WelcomeFrame::FitToContents()
{
  const int contentHeight = m_panel->GetSizer()->GetMinSize().GetHeight() + 64;
  m_panel->SetMinSize(wxSize(600, std::max(500, contentHeight)));
  GetSizer()->SetSizeHints(this);
  Layout();
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
  if (!wxFileName::FileExists(p_filename)) {
    wxMessageBox(wxString::Format(_("Gambit could not find the file %s.\n\n"
                                    "It has been removed from the list of recent games."),
                                  p_filename),
                 _("File not found"), wxOK | wxICON_EXCLAMATION, this);
    RemoveFromFileHistory(p_filename);
    CallAfter([this]() {
      m_panel->RefreshRecentFiles();
      FitToContents();
    });
    return false;
  }

  wxGetApp().SetCurrentDir(wxPathOnly(p_filename));
  return wxGetApp().LoadFile(p_filename, this) == AppLoadResult::Success;
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
