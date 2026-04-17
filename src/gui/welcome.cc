
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "gambit.h"

#include "welcome.h"
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

void WelcomePanel::CreateControls()
{
  m_titleText = new wxStaticText(this, wxID_ANY, wxT("Welcome"));
  m_messageText = new wxStaticText(
      this, wxID_ANY, wxT("Open an existing file or create a new problem representation."));

  m_openButton = new wxButton(this, ID_WELCOME_OPEN, wxT("Open..."));
  m_newNormalFormButton =
      new wxButton(this, ID_WELCOME_NEW_NORMAL_FORM, wxT("New normal form game"));
  m_newExtensiveFormButton =
      new wxButton(this, ID_WELCOME_NEW_EXTENSIVE_FORM, wxT("New extensive form game"));

  Bind(wxEVT_BUTTON, &WelcomePanel::OnOpen, this, ID_WELCOME_OPEN);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnNewNormalForm, this, ID_WELCOME_NEW_NORMAL_FORM);
  Bind(wxEVT_BUTTON, &WelcomePanel::OnNewExtensiveForm, this, ID_WELCOME_NEW_EXTENSIVE_FORM);
}

void WelcomePanel::LayoutControls()
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);
  auto *buttonSizer = new wxBoxSizer(wxVERTICAL);

  buttonSizer->Add(m_openButton, 0, wxEXPAND | wxBOTTOM, 8);
  buttonSizer->Add(m_newNormalFormButton, 0, wxEXPAND | wxBOTTOM, 8);
  buttonSizer->Add(m_newExtensiveFormButton, 0, wxEXPAND, 0);

  topSizer->AddStretchSpacer(1);
  topSizer->Add(m_titleText, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 12);
  topSizer->Add(m_messageText, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxBOTTOM, 20);
  topSizer->Add(buttonSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 20);
  topSizer->AddStretchSpacer(1);

  SetSizer(topSizer);
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
  : wxFrame(parent, wxID_ANY, wxT("Gambit"), wxDefaultPosition, wxSize(480, 320),
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
  wxFileDialog dialog(this, wxT("Open game"), wxEmptyString, wxEmptyString,
                      wxT("Game files (*.efg;*.nfg)|*.efg;*.nfg|All files (*.*)|*.*"),
                      wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (dialog.ShowModal() != wxID_OK) {
    return false;
  }

  const wxString filename = dialog.GetPath();

  // TODO: Replace with your existing application/document loading path.
  //
  // Example shape:
  //   auto *frame = MyApp::GetApp().OpenDocumentFrame(filename);
  //   if (!frame) return false;
  //   frame->Show(true);

  wxMessageBox(wxT("TODO: open file:\n") + filename, wxT("Stub"), wxOK | wxICON_INFORMATION, this);

  return true;
}

bool WelcomeFrame::DoCreateNew(WelcomeNewProblemKind p_kind)
{
  // TODO: Replace with your existing creation path.
  //
  // Example shape:
  //   wxFrame *frame = nullptr;
  //   switch (p_kind) {
  //   case WelcomeNewProblemKind::NormalForm:
  //     frame = MyApp::GetApp().CreateNewNormalFormFrame();
  //     break;
  //   case WelcomeNewProblemKind::ExtensiveForm:
  //     frame = MyApp::GetApp().CreateNewExtensiveFormFrame();
  //     break;
  //   }
  //   if (!frame) return false;
  //   frame->Show(true);

  wxString message;
  switch (p_kind) {
  case WelcomeNewProblemKind::NormalForm: {
    std::vector<int> dim = {2, 2};
    const Game nfg = NewTable(dim);
    nfg->SetTitle("Untitled Strategic Game");
    nfg->GetPlayer(1)->SetLabel("Player 1");
    nfg->GetPlayer(2)->SetLabel("Player 2");
    auto *doc = new GameDocument(nfg);
    (void)new GameFrame(nullptr, doc);
  } break;
  case WelcomeNewProblemKind::ExtensiveForm:
    const Game efg = NewTree();
    efg->NewPlayer()->SetLabel("Player 1");
    efg->NewPlayer()->SetLabel("Player 2");
    efg->SetTitle("Untitled Extensive Game");

    auto *game = new GameDocument(efg);
    (void)new GameFrame(nullptr, game);
    break;
  }

  return true;
}
