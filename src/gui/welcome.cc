
#include <wx/button.h>
#include <wx/commandlinkbutton.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "gambit.h"

#include "welcome.h"
#include "app.h"
#include "gamedoc.h"
#include "gameframe.h"

using namespace Gambit;
using namespace Gambit::GUI;

wxDEFINE_EVENT(wxEVT_WELCOME_OPEN, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_WELCOME_NEW, wxCommandEvent);

namespace {
constexpr int ID_WELCOME_OPEN = wxID_HIGHEST + 100;
constexpr int ID_WELCOME_NEW_NORMAL_FORM = wxID_HIGHEST + 101;
constexpr int ID_WELCOME_NEW_EXTENSIVE_FORM = wxID_HIGHEST + 102;
} // namespace

// --------------------
// WelcomePanel
// --------------------

WelcomePanel::WelcomePanel(wxWindow *parent) : wxPanel(parent)
{
  CreateControls();
  LayoutControls();
}

#include "bitmaps/newtable.xpm"
#include "bitmaps/newtree.xpm"
#include "bitmaps/open.xpm"
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

  m_openButton->SetBitmap(open_xpm);
  m_openButton->SetBitmapMargins(24, 12);

  m_newNormalFormButton->SetBitmap(newtable_xpm);
  m_newNormalFormButton->SetBitmapMargins(24, 12);

  m_newExtensiveFormButton->SetBitmap(newtree_xpm);
  m_newExtensiveFormButton->SetBitmapMargins(24, 12);

  const wxSize buttonSize(340, 72);
  m_openButton->SetMinSize(buttonSize);
  m_newNormalFormButton->SetMinSize(buttonSize);
  m_newExtensiveFormButton->SetMinSize(buttonSize);

  Bind(wxEVT_BUTTON, &WelcomePanel::OnOpen, this, ID_WELCOME_OPEN);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnNewNormalForm, this, ID_WELCOME_NEW_NORMAL_FORM);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnNewExtensiveForm, this, ID_WELCOME_NEW_EXTENSIVE_FORM);
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
  wxGetApp().LoadFile(filename, this);
  return true;
}

bool WelcomeFrame::DoCreateNew(WelcomeNewProblemKind p_kind)
{
  switch (p_kind) {
  case WelcomeNewProblemKind::NormalForm:
    new GameFrame(nullptr, NewTableDocument());
    ;
    break;
  case WelcomeNewProblemKind::ExtensiveForm:
    new GameFrame(nullptr, NewTreeDocument());
    break;
  }

  return true;
}
