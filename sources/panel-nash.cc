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
#include "sheet.h"    // for wxSheet

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


BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_EVENT_TYPE(GBT_MIXED_PROFILE_SELECTED, 7779)
END_DECLARE_EVENT_TYPES()

#define EVT_MIXED_PROFILE_SELECTED(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        GBT_MIXED_PROFILE_SELECTED, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

DEFINE_EVENT_TYPE(GBT_MIXED_PROFILE_SELECTED)

const int GBT_ID_MIXED_PROFILE_CTRL = 2000;

class gbtMixedProfileCtrl : public wxSheet, public gbtGameView {
private:
  gbtList<gbtMixedProfile<double> > &m_eqa;

  // Overriding wxSheet members for data access
  wxString GetCellValue(const wxSheetCoords &);
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const;

  // Event handlers
  void OnRowSelected(wxSheetRangeSelectEvent &);

public:
  gbtMixedProfileCtrl(wxWindow *p_parent, gbtGameDocument *p_doc,
		      gbtList<gbtMixedProfile<double> > &p_eqa);

  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtMixedProfileCtrl, wxSheet)
  EVT_SHEET_RANGE_SELECTED(GBT_ID_MIXED_PROFILE_CTRL,
			   gbtMixedProfileCtrl::OnRowSelected)
END_EVENT_TABLE()

gbtMixedProfileCtrl::gbtMixedProfileCtrl(wxWindow *p_parent,
					 gbtGameDocument *p_doc,
					 gbtList<gbtMixedProfile<double> > &p_eqa)
  : wxSheet(p_parent, GBT_ID_MIXED_PROFILE_CTRL), gbtGameView(p_doc),
    m_eqa(p_eqa)
{
  CreateGrid(m_eqa.Length(), p_doc->GetGame()->NumPlayers());

  EnableEditing(false);
  DisableDragRowSize();
  DisableDragColSize();
  SetCursorCellHighlightColour(*wxWHITE);
  SetSelectionMode(wxSHEET_SelectRows);

  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  AdjustScrollbars();
}

static wxString ToMyerson(const gbtMixedProfile<double> &p_profile,
			  const gbtGamePlayer &p_player)
{
  wxString ret = "";
  for (int st = 1; st <= p_player->NumStrategies(); st++) {
    gbtGameStrategy strategy = p_player->GetStrategy(st);
    if (p_profile->GetStrategyProb(strategy) > 0.0) {
      if (ret != "") {
	ret += " + ";
      }

      if (strategy->GetLabel() != "") {
	ret += wxString::Format("%f*[%s]",
				p_profile->GetStrategyProb(strategy),
				strategy->GetLabel().c_str());
      }
      else {
	ret += wxString::Format("%f*[<%d>]",
				p_profile->GetStrategyProb(strategy), st);
      }
    }
  }

  return ret;
}

wxString gbtMixedProfileCtrl::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    return wxString::Format("%d", p_coords.GetRow() + 1);
  }
  else if (IsColLabelCell(p_coords)) {
    return m_doc->GetGame()->GetPlayer(p_coords.GetCol() + 1)->GetLabel().c_str();
  }
  else if (IsCornerLabelCell(p_coords)) {
    return "#";
  }

  return ToMyerson(m_eqa[p_coords.GetRow() + 1],
		   m_doc->GetGame()->GetPlayer(p_coords.GetCol() + 1));
}

wxSheetCellAttr gbtMixedProfileCtrl::GetAttr(const wxSheetCoords &p_coords,
					     wxSheetAttr_Type) const
{
  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(*wxBLACK);
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(m_doc->GetPlayerColor(p_coords.GetCol() + 1));
    return attr;
  }
  else if (IsCornerLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(*wxBLACK);
    return attr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetReadOnly(true);
  attr.SetForegroundColour(m_doc->GetPlayerColor(p_coords.GetCol() + 1));
  return attr;
}


void gbtMixedProfileCtrl::OnUpdate(void)
{
  if (m_eqa.Length() > GetNumberRows()) {
    AppendRows(m_eqa.Length() - GetNumberRows());
  }
  else if (m_eqa.Length() < GetNumberRows()) {
    DeleteRows(GetNumberRows() - m_eqa.Length());
  }

  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  AdjustScrollbars();
}

void gbtMixedProfileCtrl::OnRowSelected(wxSheetRangeSelectEvent &p_event)
{
  if (p_event.Selecting()) {
    wxCommandEvent event(GBT_MIXED_PROFILE_SELECTED);
    event.SetInt(p_event.GetTopRow() + 1);
    wxPostEvent(this, event);
  }
}


class gbtMixedProfileDetail : public wxSheet, public gbtGameView {
private:
  gbtList<gbtMixedProfile<double> > &m_eqa;
  int m_index;

  // Overriding wxSheet members for data access
  wxString GetCellValue(const wxSheetCoords &);
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const;

public:
  gbtMixedProfileDetail(wxWindow *p_parent, gbtGameDocument *p_doc,
			gbtList<gbtMixedProfile<double> > &p_eqa);

  void OnUpdate(void);

  void SetIndex(int p_index) { m_index = p_index; ForceRefresh(); }
};

gbtMixedProfileDetail::gbtMixedProfileDetail(wxWindow *p_parent,
					     gbtGameDocument *p_doc,
					     gbtList<gbtMixedProfile<double> > &p_eqa)
  : wxSheet(p_parent, -1), gbtGameView(p_doc),
    m_eqa(p_eqa), m_index(0)
{
  CreateGrid(1 + p_doc->GetGame()->NumPlayers(), 1);

  EnableEditing(false);
  DisableDragRowSize();
  DisableDragColSize();
  SetCursorCellHighlightColour(*wxWHITE);
  SetColLabelHeight(1);

  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  AdjustScrollbars();
}

void gbtMixedProfileDetail::OnUpdate(void)
{
  ForceRefresh();
}

wxString gbtMixedProfileDetail::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    if (p_coords.GetRow() == 0) {
      return "Lyapunov";
    }
    else {
      gbtGamePlayer player = m_doc->GetGame()->GetPlayer(p_coords.GetRow());
      if (player->GetLabel() != "") {
	return wxString::Format("Payoff to %s", player->GetLabel().c_str());
      }
      else {
	return wxString::Format("Payoff to Player %d", p_coords.GetRow());
      }
    }
  }
  else if (IsColLabelCell(p_coords) || IsCornerLabelCell(p_coords)) {
    return "";
  }

  if (m_index == 0)  return "";

  if (p_coords.GetRow() == 0) {
    return wxString::Format("%f", m_eqa[m_index]->GetLiapValue(false));
  }
  else {
    return wxString::Format("%f", 
			    m_eqa[m_index]->GetPayoff(m_doc->GetGame()->GetPlayer(p_coords.GetRow())));
  }
}

wxSheetCellAttr gbtMixedProfileDetail::GetAttr(const wxSheetCoords &p_coords,
					       wxSheetAttr_Type) const
{
  if (p_coords.GetRow() == 0) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    if (IsRowLabelCell(p_coords)) {
      attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    }
    else {
      attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    }
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(*wxBLACK);
    return attr;
  }
  else {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    if (IsRowLabelCell(p_coords)) {
      attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    }
    else {
      attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    }
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(m_doc->GetPlayerColor(p_coords.GetRow()));
    return attr;
  }
  return GetSheetRefData()->m_defaultGridCellAttr;
}

BEGIN_EVENT_TABLE(gbtNashPanel, wxPanel)
  EVT_BUTTON(GBT_BUTTON_START, gbtNashPanel::OnStartButton)
  EVT_BUTTON(GBT_BUTTON_CANCEL, gbtNashPanel::OnCancelButton)
  EVT_MENU(GBT_THREAD_DONE, gbtNashPanel::OnThreadDone)
  EVT_MENU(GBT_MENU_COUNT_ONE, gbtNashPanel::OnMenu)
  EVT_MENU(GBT_MENU_COUNT_ALL, gbtNashPanel::OnMenu)
  EVT_BUTTON(GBT_BUTTON_COUNT, gbtNashPanel::OnCountButton)
  EVT_MIXED_PROFILE_SELECTED(gbtNashPanel::OnProfileSelected)
END_EVENT_TABLE()

gbtNashPanel::gbtNashPanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc),
    m_countValue(GBT_MENU_COUNT_ONE), m_thread(0)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *paramSizer = 
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
  paramSizer->Add(countSizer, 0, wxALL | wxALIGN_CENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  
  m_startButton = new wxButton(this, GBT_BUTTON_START, "Start");
  buttonSizer->Add(m_startButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_cancelButton = new wxButton(this, GBT_BUTTON_CANCEL, "Cancel");
  buttonSizer->Add(m_cancelButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_cancelButton->Enable(false);

  paramSizer->Add(buttonSizer, 0, wxALIGN_CENTER, 0);

  topSizer->Add(paramSizer, 1, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *listSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "List of computed equilibria"),
			 wxVERTICAL);
  m_profileCtrl = new gbtMixedProfileCtrl(this, p_doc, m_eqa);
  listSizer->Add(m_profileCtrl, 1, wxALL | wxEXPAND, 5);

  topSizer->Add(listSizer, 1, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *detailSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "Equilibrium details"),
			 wxVERTICAL);
  m_profileDetail = new gbtMixedProfileDetail(this, p_doc, m_eqa);
  detailSizer->Add(m_profileDetail, 1, wxALL | wxEXPAND, 5);
  
  topSizer->Add(detailSizer, 1, wxALL | wxEXPAND, 5);

  SetSizer(topSizer);
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
  m_profileCtrl->OnUpdate();
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

void gbtNashPanel::OnProfileSelected(wxCommandEvent &p_event)
{
  m_profileDetail->SetIndex(p_event.GetInt());
}
