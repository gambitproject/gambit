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

#include <libgambit/nash/nfglogit.h>

#include "panel-qre.h"

const int GBT_BUTTON_START = 2000;
const int GBT_THREAD_DONE = 2001;

class gbtMixedLogitThread : public wxThread {
private:
  gbtQrePanel *m_parent;
  gbtMixedLogitBranch m_cor;

public:
  gbtMixedLogitThread(gbtQrePanel *p_parent);

  // Overriding members to implement thread
  void *Entry(void);
  void OnExit(void);
};

gbtMixedLogitThread::gbtMixedLogitThread(gbtQrePanel *p_parent)
  : m_parent(p_parent)
{ }

void gbtMixedLogitThread::OnExit(void)
{ }

void *gbtMixedLogitThread::Entry(void)
{
  try {
    gbtLogitPrincipalNfg(m_parent->GetDocument()->GetGame(), 
			 10000000, m_cor);
  }
  catch (gbtException &) {
    // Just eat it for now... need to issue a message
  }

  m_parent->SetCorrespondence(m_cor);

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_THREAD_DONE);
  wxPostEvent(m_parent, event);

  return NULL;
}

class gbtMixedLogitCtrl : public wxSheet, public gbtGameView {
private:
  gbtMixedLogitBranch &m_cor;

  // Overriding wxSheet members for data access
  wxString GetCellValue(const wxSheetCoords &);
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const;

public:
  gbtMixedLogitCtrl(wxWindow *p_parent, gbtGameDocument *p_doc,
		    gbtMixedLogitBranch &p_cor);

  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtMixedLogitCtrl, wxSheet)
END_EVENT_TABLE()

gbtMixedLogitCtrl::gbtMixedLogitCtrl(wxWindow *p_parent,
				     gbtGameDocument *p_doc,
				     gbtMixedLogitBranch &p_cor)
  : wxSheet(p_parent, -1), gbtGameView(p_doc),
    m_cor(p_cor)
{
  CreateGrid(m_cor.NumPoints(), p_doc->GetGame()->NumPlayers());

  EnableEditing(false);
  DisableDragRowSize();
  DisableDragColSize();
  SetCursorCellHighlightColour(*wxWHITE);

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

wxString gbtMixedLogitCtrl::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    return wxString::Format("%f", 
			    m_cor.GetLambda(p_coords.GetRow() + 1));
  }
  else if (IsColLabelCell(p_coords)) {
    return m_doc->GetGame()->GetPlayer(p_coords.GetCol() + 1)->GetLabel().c_str();
  }
  else if (IsCornerLabelCell(p_coords)) {
    return "Lambda";
  }

  return ToMyerson(m_cor.GetProfile(p_coords.GetRow() + 1),
		   m_doc->GetGame()->GetPlayer(p_coords.GetCol() + 1));
}

wxSheetCellAttr gbtMixedLogitCtrl::GetAttr(const wxSheetCoords &p_coords,
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


void gbtMixedLogitCtrl::OnUpdate(void)
{
  if (m_cor.NumPoints() > GetNumberRows()) {
    AppendRows(m_cor.NumPoints() - GetNumberRows());
  }
  else if (m_cor.NumPoints() < GetNumberRows()) {
    DeleteRows(GetNumberRows() - m_cor.NumPoints());
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


class gbtMixedLogitGraph : public wxScrolledWindow, public gbtGameView {
private:
  gbtMixedLogitBranch &m_cor;
  int m_leftMargin, m_rightMargin, m_topMargin, m_bottomMargin;

  // Auxiliary functions for plotting
  void DrawXAxis(wxDC &);
  void DrawYAxis(wxDC &);
  void DrawComponent(wxDC &, const gbtGameStrategy &);

  double GetMaxLambda(void) const;
  wxString GetXAxisLabel(double p_percent) const;
  double LambdaToPct(double p_lambda) const;

  // Event handlers
  void OnRightDown(wxMouseEvent &);
  void OnSave(wxCommandEvent &);

public:
  gbtMixedLogitGraph(wxWindow *, gbtGameDocument *, gbtMixedLogitBranch &);

  void OnUpdate(void) { Refresh(); }

  void OnDraw(wxDC &);

  DECLARE_EVENT_TABLE()
};

const int GBT_LOGIT_SAVE_PNG = 4000;
const int GBT_LOGIT_SAVE_BMP = 4001;
const int GBT_LOGIT_SAVE_JPG = 4002;
const int GBT_LOGIT_SAVE_PS = 4003;

BEGIN_EVENT_TABLE(gbtMixedLogitGraph, wxScrolledWindow)
  EVT_MENU(GBT_LOGIT_SAVE_BMP, gbtMixedLogitGraph::OnSave)
  EVT_MENU(GBT_LOGIT_SAVE_JPG, gbtMixedLogitGraph::OnSave)
  EVT_MENU(GBT_LOGIT_SAVE_PNG, gbtMixedLogitGraph::OnSave)
  EVT_RIGHT_DOWN(gbtMixedLogitGraph::OnRightDown)
END_EVENT_TABLE()

gbtMixedLogitGraph::gbtMixedLogitGraph(wxWindow *p_parent,
				       gbtGameDocument *p_doc,
				       gbtMixedLogitBranch &p_cor)
  : wxScrolledWindow(p_parent, -1), gbtGameView(p_doc),
    m_cor(p_cor), 
    m_leftMargin(50), m_rightMargin(10), m_topMargin(10), m_bottomMargin(30)
{
  SetBackgroundColour(*wxWHITE);
}

void gbtMixedLogitGraph::DrawXAxis(wxDC &p_dc)
{
  wxSize size = GetClientSize();

  p_dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
  p_dc.DrawLine(m_leftMargin, size.GetHeight() - m_bottomMargin,
		size.GetWidth() - m_rightMargin, 
		size.GetHeight() - m_bottomMargin);

  const int c_numTicks = 10;
  int tickWidth = (int) ((double) (size.GetWidth() - m_leftMargin - m_rightMargin) / 
			 (double) c_numTicks - 1);

  p_dc.SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD));
  for (int i = 0; i <= c_numTicks; i++) {
    int y = size.GetHeight() - m_bottomMargin;
    int x = m_leftMargin + i * tickWidth; 

    int c_tickLength = 5;
    p_dc.DrawLine(x, y - c_tickLength / 2, x, y + c_tickLength / 2);

    wxString label = GetXAxisLabel((double) i / (double) c_numTicks);
    wxCoord tw, th;
    p_dc.GetTextExtent(label, &tw, &th);
    p_dc.DrawText(label, x - tw / 2, y + 10);
  }
}

double gbtMixedLogitGraph::GetMaxLambda(void) const
{
  if (m_cor.NumPoints() == 0) {
    return 1.0;
  }
  else {
    return m_cor.GetLambda(m_cor.NumPoints());
  }
}

const double alpha = 0.25;

wxString gbtMixedLogitGraph::GetXAxisLabel(double p_percent) const
{
  double maxlambda = GetMaxLambda();
  double maxnu = alpha * maxlambda / (1.0 + alpha * maxlambda);

  double thisnu = p_percent * maxnu;
  double lambda = thisnu / (1.0 - thisnu) / alpha;
  
  if (lambda < 10000.0) {
    return wxString::Format("%3.2f", lambda);
  }
  else {
    return wxString::Format("%.3e", lambda);
  }
}

double gbtMixedLogitGraph::LambdaToPct(double p_lambda) const
{
  double maxlambda = GetMaxLambda();
  double maxnu = alpha * maxlambda / (1.0 + alpha * maxlambda);

  double thisnu = alpha * p_lambda / (1.0 + alpha * p_lambda);
  return thisnu / maxnu;
}

void gbtMixedLogitGraph::DrawYAxis(wxDC &p_dc)
{
  wxSize size = GetClientSize();

  p_dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
  p_dc.DrawLine(m_leftMargin, size.GetHeight() - m_bottomMargin,
		m_leftMargin, m_topMargin);
  
  const int c_numTicks = 10;
  int tickHeight = (int) ((double) (size.GetHeight() - m_topMargin - m_bottomMargin) / 
			  (double) c_numTicks - 1);
  for (int i = 0; i <= c_numTicks; i++) {
    int x = m_leftMargin;
    int y = (m_topMargin +
	     (int) ((double) (c_numTicks - i) / (double) c_numTicks * (double) (size.GetHeight() - m_topMargin - m_bottomMargin)));

    int c_tickLength = 5;
    p_dc.DrawLine(x - c_tickLength / 2, y, x + c_tickLength / 2, y);

    wxString label = wxString::Format("%3.2f", 
				      (double) i / (double) c_numTicks);
    wxCoord tw, th;
    p_dc.GetTextExtent(label, &tw, &th);
    p_dc.DrawText(label, 
		  x - c_tickLength / 2 - tw - 10,
		  y - th / 2);
  }
}

void gbtMixedLogitGraph::DrawComponent(wxDC &p_dc, 
				       const gbtGameStrategy &p_strategy)
{
  wxSize size = GetClientSize();
  int graphWidth = size.GetWidth() - m_leftMargin - m_rightMargin;
  int graphHeight = size.GetHeight() - m_topMargin - m_bottomMargin;

  for (int i = 1; i < m_cor.NumPoints(); i++) {
    int x1 = (m_leftMargin + 
	      (int) (LambdaToPct(m_cor.GetLambda(i)) * graphWidth));
    int y1 = (m_topMargin +
	      (int) ((1.0 - m_cor.GetProfile(i)->GetStrategyProb(p_strategy)) * (double) graphHeight));
    
    int x2 = (m_leftMargin + 
	      (int) (LambdaToPct(m_cor.GetLambda(i+1)) * graphWidth));
    int y2 = (m_topMargin +
	      (int) ((1.0 - m_cor.GetProfile(i+1)->GetStrategyProb(p_strategy)) * (double) graphHeight));
    p_dc.DrawLine(x1, y1, x2, y2);
  }
}

void gbtMixedLogitGraph::OnDraw(wxDC &p_dc)
{
  DrawXAxis(p_dc);
  DrawYAxis(p_dc);
  for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
    gbtGamePlayer player = m_doc->GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (st % 2 == 0) {
	p_dc.SetPen(wxPen(m_doc->GetPlayerColor(pl), 1, wxSOLID));
      }
      else {
	p_dc.SetPen(wxPen(m_doc->GetPlayerColor(pl), 1, wxDOT));
      }
      DrawComponent(p_dc, player->GetStrategy(st));
    }
  }
}

void gbtMixedLogitGraph::OnRightDown(wxMouseEvent &p_event)
{
  wxMenu *menu = new wxMenu;

  menu->Append(GBT_LOGIT_SAVE_BMP, _("Save as BMP"),
	       _("Save graph in BMP format"));
  menu->Append(GBT_LOGIT_SAVE_JPG, _("Save as JPG"),
	       _("Save graph in JPG format"));
  menu->Append(GBT_LOGIT_SAVE_PNG, _("Save as PNG"), 
	       _("Save graph in PNG format"));

  PopupMenu(menu, p_event.GetX(), p_event.GetY());
}

void gbtMixedLogitGraph::OnSave(wxCommandEvent &p_event)
{
  wxString filter;
  switch (p_event.GetId()) {
  case GBT_LOGIT_SAVE_BMP:
    filter = "BMP files (*.bmp)|*.bmp";
    break;
  case GBT_LOGIT_SAVE_JPG:
    filter = "JPG files (*.jpg)|*.jpg";
    break;
  case GBT_LOGIT_SAVE_PNG:
    filter = "PNG files (*.png)|*.png";
    break;
  default:
    break;
  }

  filter += "|All files (*.*)|*.*";

  wxFileDialog dialog(this, "Choose output file", "", "",
		      filter, wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(GetClientSize().GetWidth(),
		    GetClientSize().GetHeight());
    dc.SelectObject(bitmap);
    PrepareDC(dc);
    dc.Clear();
    OnDraw(dc);

    int filetype;
    switch (p_event.GetId()) {
    case GBT_LOGIT_SAVE_BMP:
      filetype = wxBITMAP_TYPE_BMP;
      break;
    case GBT_LOGIT_SAVE_JPG:
      filetype = wxBITMAP_TYPE_JPEG;
      break;
    case GBT_LOGIT_SAVE_PNG:
      filetype = wxBITMAP_TYPE_PNG;
      break;
    default:
      break;
    }

    if (!bitmap.SaveFile(dialog.GetPath(), filetype)) {
      wxMessageBox(wxString::Format("An error occurred in writing '%s'.\n",
				    dialog.GetPath().c_str()),
		   "Error", wxOK, this);
    }
  }
}

BEGIN_EVENT_TABLE(gbtQrePanel, wxPanel)
  EVT_BUTTON(GBT_BUTTON_START, gbtQrePanel::OnStartButton)
  EVT_MENU(GBT_THREAD_DONE, gbtQrePanel::OnThreadDone)
END_EVENT_TABLE()

gbtQrePanel::gbtQrePanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc), m_thread(0),
    m_cor(new gbtMixedLogitBranch)
{ 
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *paramSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "Computing logit equilibria"),
			 wxVERTICAL);

  m_startButton = new wxButton(this, GBT_BUTTON_START, "Start");
  paramSizer->Add(m_startButton, 0, wxALL | wxALIGN_CENTER, 5);

  topSizer->Add(paramSizer, 0, wxALL | wxEXPAND, 5);


  wxStaticBoxSizer *listSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "Points on the correspondence branch"),
			 wxVERTICAL);
  m_profileCtrl = new gbtMixedLogitCtrl(this, p_doc, *m_cor);
  listSizer->Add(m_profileCtrl, 1, wxALL | wxEXPAND, 5);

  topSizer->Add(listSizer, 1, wxALL | wxEXPAND, 5);


  wxStaticBoxSizer *graphSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "Correspondence graph"),
			 wxVERTICAL);
  m_branchGraph = new gbtMixedLogitGraph(this, p_doc, *m_cor);
  graphSizer->Add(m_branchGraph, 1, wxALL | wxEXPAND, 5);
    
  topSizer->Add(graphSizer, 1, wxALL | wxEXPAND, 5);

  SetSizer(topSizer);
  Layout();
}

gbtQrePanel::~gbtQrePanel()
{
  delete m_cor;
}

void gbtQrePanel::OnStartButton(wxCommandEvent &)
{
  m_startButton->Enable(false);
  m_thread = new gbtMixedLogitThread(this);
  m_thread->Create();
  if (m_thread->Run() != wxTHREAD_NO_ERROR) {
    printf("Whoops\n");
  }
}

void gbtQrePanel::OnThreadDone(wxCommandEvent &)
{
  printf("Number of points in list = %d\n", m_cor->NumPoints());
  m_thread = 0;
  m_startButton->Enable(true);
  m_profileCtrl->OnUpdate();
  m_branchGraph->OnUpdate();
}

void gbtQrePanel::SetCorrespondence(const gbtMixedLogitBranch &p_cor)
{
  *m_cor = p_cor;
}
