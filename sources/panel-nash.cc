//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of panel for controlling equilibrium computation
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "panel-nash.h"

const int GBT_BUTTON_COUNT = 899;
const int GBT_MENU_COUNT_ONE = 900;
const int GBT_MENU_COUNT_ALL = 902;

const int GBT_BUTTON_START = 997;
const int GBT_BUTTON_CANCEL = 998;
const int GBT_THREAD_DONE = 999;

#include <gambit/nash/nfgbfslcp.h>
#include <gambit/nash/nfgpns.h>

class gbtOneNashThread : public wxThread {
private:
  gbtNashPanel *m_parent;
  gbtList<gbtMixedProfile<double> > &m_eqa;

public:
  gbtOneNashThread(gbtNashPanel *p_parent,
		   gbtList<gbtMixedProfile<double> > &p_eqa);

  // Overriding members to implement thread
  void *Entry(void);
  void OnExit(void);
};

gbtOneNashThread::gbtOneNashThread(gbtNashPanel *p_parent,
				   gbtList<gbtMixedProfile<double> > &p_eqa)
  : m_parent(p_parent), m_eqa(p_eqa)
{ }

void gbtOneNashThread::OnExit(void)
{ }

void *gbtOneNashThread::Entry(void)
{
  try {
    m_eqa = gbtNashBfsLcpNfg(m_parent->GetDocument()->GetGame(),
			     (double) 0.0);
  }
  catch (gbtException &) {
    // Just eat it for now... need to issue a message
  }

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_THREAD_DONE);
  wxPostEvent(m_parent, event);

  return NULL;
}

class gbtAllNashThread : public wxThread {
private:
  gbtNashPanel *m_parent;
  gbtList<gbtMixedProfile<double> > &m_eqa;

public:
  gbtAllNashThread(gbtNashPanel *p_parent,
		   gbtList<gbtMixedProfile<double> > &p_eqa);

  // Overriding members to implement thread
  void *Entry(void);
  void OnExit(void);
};

gbtAllNashThread::gbtAllNashThread(gbtNashPanel *p_parent,
				   gbtList<gbtMixedProfile<double> > &p_eqa)
  : m_parent(p_parent), m_eqa(p_eqa)
{ }

void gbtAllNashThread::OnExit(void)
{ }

void *gbtAllNashThread::Entry(void)
{
  try {
    m_eqa = gbtPNSNfg(m_parent->GetDocument()->GetGame(), 0);
  }
  catch (gbtException &) {
    // Just eat it for now... need to issue a message
  }

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_THREAD_DONE);
  wxPostEvent(m_parent, event);

  return NULL;
}

class gbtTextWindow : public wxScrolledWindow {
private:
  bool m_underline;
  wxString m_label, m_longestLabel;

  void OnPaint(wxPaintEvent &);
  void OnLeftDown(wxMouseEvent &);

public:
  gbtTextWindow(wxWindow *p_parent, wxWindowID, 
		const wxString &, const wxString & = "");
  virtual ~gbtTextWindow() { }

  wxSize GetBestSize(void) const;

  void SetLabel(const wxString &p_label)  { m_label = p_label; Refresh(); }

  bool GetUnderline(void) const { return m_underline; }
  void SetUnderline(bool p_underline) { m_underline = p_underline; Refresh(); }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtTextWindow, wxScrolledWindow)
  EVT_LEFT_DOWN(gbtTextWindow::OnLeftDown)
  EVT_PAINT(gbtTextWindow::OnPaint)
END_EVENT_TABLE()

gbtTextWindow::gbtTextWindow(wxWindow *p_parent, wxWindowID p_id,
			     const wxString &p_label, 
			     const wxString &p_longestLabel)
  : wxScrolledWindow(p_parent, p_id), 
    m_underline(false), m_label(p_label), m_longestLabel(p_longestLabel)
{
  if (m_longestLabel == "")  m_longestLabel = p_label;
  SetSize(GetBestSize());
}

wxSize gbtTextWindow::GetBestSize(void) const
{
  wxClientDC dc(const_cast<gbtTextWindow *>(this));
  dc.SetFont(GetFont());
  wxCoord width, height;
  dc.GetTextExtent(m_longestLabel, &width, &height);
  return wxSize(width + 5, height + 5);
}

void gbtTextWindow::OnPaint(wxPaintEvent &)
{
  wxPaintDC dc(this);
  wxSize size(GetClientSize());
  wxCoord width, height;
  dc.SetFont(GetFont());
  dc.GetTextExtent(m_label, &width, &height);

  dc.DrawText(m_label,
	      (size.GetWidth() - width) / 2, (size.GetHeight() - height) / 2);

  if (m_underline) {
    dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
    dc.DrawLine((size.GetWidth() - width) / 2,
		(size.GetHeight() + height) / 2 + 1,
		(size.GetWidth() + width) / 2,
		(size.GetHeight() + height) / 2 + 1);
  }
}

void gbtTextWindow::OnLeftDown(wxMouseEvent &)
{
  wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
  wxPostEvent(GetParent(), event);
}


BEGIN_EVENT_TABLE(gbtNashPanel, wxPanel)
  EVT_BUTTON(GBT_BUTTON_START, gbtNashPanel::OnStartButton)
  EVT_BUTTON(GBT_BUTTON_CANCEL, gbtNashPanel::OnCancelButton)
  EVT_MENU(GBT_THREAD_DONE, gbtNashPanel::OnThreadDone)
  EVT_MENU(GBT_MENU_COUNT_ONE, gbtNashPanel::OnMenu)
  EVT_MENU(GBT_MENU_COUNT_ALL, gbtNashPanel::OnMenu)
  EVT_BUTTON(GBT_BUTTON_COUNT, gbtNashPanel::OnCountButton)
END_EVENT_TABLE()

gbtNashPanel::gbtNashPanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc),
    m_countValue(GBT_MENU_COUNT_ONE), m_thread(0)
{
  wxStaticBoxSizer *sizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "Computing Nash equilibria"),
			 wxVERTICAL);

  wxBoxSizer *countSizer = new wxBoxSizer(wxHORIZONTAL);
  countSizer->Add(new gbtTextWindow(this, wxID_STATIC,
				    "Compute"),
		  0, wxALL | wxALIGN_CENTER, 5);
  m_count = new gbtTextWindow(this, GBT_BUTTON_COUNT, "one Nash equilibrium");
  m_count->SetUnderline(true);
  countSizer->Add(m_count, 0, wxALL | wxALIGN_CENTER, 5);
  sizer->Add(countSizer, 0, wxALL | wxALIGN_CENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  
  m_startButton = new wxButton(this, GBT_BUTTON_START, "Start");
  buttonSizer->Add(m_startButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_cancelButton = new wxButton(this, GBT_BUTTON_CANCEL, "Cancel");
  buttonSizer->Add(m_cancelButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_cancelButton->Enable(false);

  sizer->Add(buttonSizer, 0, wxALIGN_CENTER, 0);
  
  SetSizer(sizer);
  Layout();
}

void gbtNashPanel::OnStartButton(wxCommandEvent &)
{
  m_startButton->Enable(false);
  m_cancelButton->Enable(true);
  m_eqa.Flush();
  if (m_countValue == GBT_MENU_COUNT_ONE) {
    m_thread = new gbtOneNashThread(this, m_eqa);
  }
  else {
    m_thread = new gbtAllNashThread(this, m_eqa);
  }
    
  m_thread->Create();
  if (m_thread->Run() != wxTHREAD_NO_ERROR) {
    printf("Whoops!\n");
  }
}

void gbtNashPanel::OnCancelButton(wxCommandEvent &)
{
  // Doesn't actually do anything yet!
}

void gbtNashPanel::OnThreadDone(wxCommandEvent &)
{
  printf("Number of equilibria found = %d\n", m_eqa.Length());
  m_thread = 0;
  m_cancelButton->Enable(false);
  m_startButton->Enable(true);
}

void gbtNashPanel::OnUpdate(void)
{ }

void gbtNashPanel::OnCountButton(wxCommandEvent &)
{
  wxMenu *menu = new wxMenu;
  menu->Append(GBT_MENU_COUNT_ONE, "one Nash equilibrium");
  menu->Append(GBT_MENU_COUNT_ALL, "all Nash equilibriua");
  PopupMenu(menu,
	    m_count->GetPosition().x,
	    m_count->GetPosition().y = m_count->GetSize().GetHeight());
}

void gbtNashPanel::OnMenu(wxCommandEvent &p_event)
{
  switch (p_event.GetId()) {
  case GBT_MENU_COUNT_ONE:
    m_count->SetLabel("one Nash equilibrium");
    m_countValue = GBT_MENU_COUNT_ONE;
    break;
  case GBT_MENU_COUNT_ALL:
    m_count->SetLabel("all Nash equilibria");
    m_countValue = GBT_MENU_COUNT_ALL;
    break;
  default:
    break;
  }
}
