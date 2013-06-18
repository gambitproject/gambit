//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlnfglogit.cc
// Dialog for monitoring progress of logit equilibrium computation
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

#include <fstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/stdpaths.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/process.h>
#include <wx/print.h>

#include "wx/sheet/sheet.h"
#include "wx/plotctrl/plotctrl.h"
#include "wx/wxthings/spinctld.h"   // for wxSpinCtrlDbl

#include "gamedoc.h"
#include "menuconst.h"            // for tool IDs

using namespace Gambit;


// Use an anonymous namespace to encapsulate the helper classes

namespace {

//========================================================================
//                    class LogitMixedBranch
//========================================================================

/// Represents one branch of a logit equilibrium correspondence
class LogitMixedBranch {
private:
  gbtGameDocument *m_doc;
  List<double> m_lambdas;
  List<MixedStrategyProfile<double> > m_profiles;

public:
  LogitMixedBranch(gbtGameDocument *p_doc) : m_doc(p_doc) { }

  void AddProfile(const wxString &p_text);

  int NumPoints(void) const { return m_lambdas.Length(); }
  double GetLambda(int p_index) const { return m_lambdas[p_index]; }
  const List<double> &GetLambdas(void) const { return m_lambdas; }
  const MixedStrategyProfile<double> &GetProfile(int p_index)
  { return m_profiles[p_index]; }
  const List<MixedStrategyProfile<double> > &GetProfiles(void) const 
  { return m_profiles; }
};
  
void LogitMixedBranch::AddProfile(const wxString &p_text)
{
  MixedStrategyProfile<double> profile(m_doc->GetGame()->NewMixedStrategyProfile(0.0));

  wxStringTokenizer tok(p_text, wxT(","));

  m_lambdas.Append((double) lexical_cast<Rational>(std::string((const char *) tok.GetNextToken().mb_str())));

  for (int i = 1; i <= profile.MixedProfileLength(); i++) {
    profile[i] = lexical_cast<Rational>(std::string((const char *) tok.GetNextToken().mb_str()));
  }

  m_profiles.Append(profile);
}

//========================================================================
//                      class LogitMixedSheet
//========================================================================

class LogitMixedSheet : public wxSheet {
private:
  gbtGameDocument *m_doc;
  LogitMixedBranch &m_branch;

  // Overriding wxSheet members for data access
  wxString GetCellValue(const wxSheetCoords &);
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const;
  
  // Overriding wxSheet members to disable selection behavior
  bool SelectRow(int, bool = false, bool = false)
    { return false; }
  bool SelectRows(int, int, bool = false, bool = false)
    { return false; }
  bool SelectCol(int, bool = false, bool = false)
    { return false; }
  bool SelectCols(int, int, bool = false, bool = false)
    { return false; }
  bool SelectCell(const wxSheetCoords&, bool = false, bool = false)
    { return false; }
  bool SelectBlock(const wxSheetBlock&, bool = false, bool = false)
    { return false; }
  bool SelectAll(bool = false) { return false; }

  // Overriding wxSheet member to suppress drawing of cursor
  void DrawCursorCellHighlight(wxDC&, const wxSheetCellAttr &) { }

public:
  LogitMixedSheet(wxWindow *p_parent, gbtGameDocument *p_doc,
		     LogitMixedBranch &p_branch);
  virtual ~LogitMixedSheet();
};

LogitMixedSheet::LogitMixedSheet(wxWindow *p_parent, 
				       gbtGameDocument *p_doc,
				       LogitMixedBranch &p_branch) 
  : wxSheet(p_parent, -1), m_doc(p_doc), m_branch(p_branch)
{
  CreateGrid(p_branch.NumPoints(), p_doc->GetGame()->MixedProfileLength()+1);
  SetRowLabelWidth(40);
  SetColLabelHeight(25);
}

LogitMixedSheet::~LogitMixedSheet()
{ }

wxString LogitMixedSheet::GetCellValue(const wxSheetCoords &p_coords)
{
  if (!m_doc->GetGame())  return wxT("");

  if (IsRowLabelCell(p_coords)) {
    return wxString::Format(wxT("%d"), p_coords.GetRow() + 1);
  }
  else if (IsColLabelCell(p_coords)) {
    if (p_coords.GetCol() == 0) {
      return wxT("Lambda");
    }
    else {
      int index = 1;
      for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
	GamePlayer player = m_doc->GetGame()->GetPlayer(pl);
	for (int st = 1; st <= player->NumStrategies(); st++) {
	  if (index++ == p_coords.GetCol()) {
	    return (wxString::Format(wxT("%d: "), pl) +
		    wxString(player->GetStrategy(st)->GetLabel().c_str(),
			    *wxConvCurrent));
	  }
	}
      }
      return wxT("");
    }
  }
  else if (IsCornerLabelCell(p_coords)) {
    return wxT("#");
  }

  if (p_coords.GetCol() == 0) {
    return wxString(lexical_cast<std::string>(m_branch.GetLambda(p_coords.GetRow()+1),
			   m_doc->GetStyle().NumDecimals()).c_str(),
		    *wxConvCurrent);
  }
  else {
    const MixedStrategyProfile<double> &profile = 
      m_branch.GetProfile(p_coords.GetRow()+1);
    return wxString(lexical_cast<std::string>(profile[p_coords.GetCol()],
				   m_doc->GetStyle().NumDecimals()).c_str(), 
		    *wxConvCurrent);
  }
}


static wxColour GetPlayerColor(gbtGameDocument *p_doc, int p_index)
{
  if (!p_doc->GetGame())  return *wxBLACK;

  int index = 1;
  for (int pl = 1; pl <= p_doc->GetGame()->NumPlayers(); pl++) {
    GamePlayer player = p_doc->GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (index++ == p_index) {
	return p_doc->GetStyle().GetPlayerColor(pl);
      }
    }
  }
  return *wxBLACK;
}

wxSheetCellAttr LogitMixedSheet::GetAttr(const wxSheetCoords &p_coords, 
					 wxSheetAttr_Type) const
{
  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultRowLabelAttr);
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultColLabelAttr);
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(GetPlayerColor(m_doc, p_coords.GetCol()));
    return attr;
  }
  else if (IsCornerLabelCell(p_coords)) {
    return GetSheetRefData()->m_defaultCornerLabelAttr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  attr.SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetForegroundColour(GetPlayerColor(m_doc, p_coords.GetCol()));
  attr.SetReadOnly(true);
  return attr;
}

class LogitBranchDialog : public wxDialog {
private:
  LogitMixedSheet *m_sheet;

public:
  LogitBranchDialog(wxWindow *p_parent,
		       gbtGameDocument *p_doc,
		       LogitMixedBranch &p_branch);
};

LogitBranchDialog::LogitBranchDialog(wxWindow *p_parent,
					   gbtGameDocument *p_doc,
					   LogitMixedBranch &p_branch)
  : wxDialog(p_parent, wxID_ANY, wxT("Logit equilibrium correspondence"),
	     wxDefaultPosition)
{
  m_sheet = new LogitMixedSheet(this, p_doc, p_branch);
  m_sheet->AutoSizeCol(0);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(m_sheet, 0, wxALL, 5);
  sizer->Add(CreateButtonSizer(wxOK), 0, wxALL | wxEXPAND, 5);

  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
}


//========================================================================
//                      class gbtLogitPlotCtrl
//========================================================================

class gbtLogitPlotCtrl : public wxPlotCtrl {
private:
  gbtGameDocument *m_doc;
  double m_scaleFactor;

  /// Overriding x (lambda) axis labeling
  void CalcXAxisTickPositions(void);
  
public:
  gbtLogitPlotCtrl(wxWindow *p_parent, gbtGameDocument *p_doc);

  double LambdaToX(double p_lambda)
  { return m_scaleFactor*p_lambda / (1.0 + m_scaleFactor*p_lambda); }
  double XToLambda(double p_x)
  { return p_x / (m_scaleFactor * (1.0 - p_x)); }

  void SetScaleFactor(double p_scale) { m_scaleFactor = p_scale; }
};

gbtLogitPlotCtrl::gbtLogitPlotCtrl(wxWindow *p_parent, 
				   gbtGameDocument *p_doc)
  : wxPlotCtrl(p_parent), m_doc(p_doc), m_scaleFactor(1.0)
{
  SetAxisLabelColour(*wxBLUE);
  wxFont labelFont(8, wxSWISS, wxNORMAL, wxBOLD);
  SetAxisLabelFont(labelFont);
  SetAxisColour(*wxBLUE);
  SetAxisFont(labelFont);
  SetDrawSymbols(false);

  // SetAxisFont resets the width of the y axis labels, assuming
  // a fairly long label.
  int x=6, y=12, descent=0, leading=0;
  GetTextExtent(wxT("0.88"), &x, &y, &descent, &leading, &labelFont);
  m_y_axis_text_width = x + leading;

  SetXAxisLabel(wxT("Lambda"));
  SetShowXAxisLabel(true);
  SetYAxisLabel(wxT("Probability"));
  SetShowYAxisLabel(true);

  SetShowKey(true);

  m_xAxisTick_step = 0.2;
  SetViewRect(wxRect2DDouble(0, 0, 1, 1));
} 

//
// This differs from the wxPlotWindow original only by the use of
// XToLambda() to construct the tick labels.
//
void gbtLogitPlotCtrl::CalcXAxisTickPositions(void)
{
  double current = ceil(m_viewRect.GetLeft() / m_xAxisTick_step) * m_xAxisTick_step;
  m_xAxisTicks.Clear();
  m_xAxisTickLabels.Clear();
  int i, x, windowWidth = GetPlotAreaRect().width;
  for (i=0; i<m_xAxisTick_count; i++) {
    if (!IsFinite(current, wxT("axis label is not finite"))) return;
                
    x = GetClientCoordFromPlotX( current );
            
    if ((x >= -1) && (x < windowWidth+2)) {
      m_xAxisTicks.Add(x);
      m_xAxisTickLabels.Add(wxString::Format(m_xAxisTickFormat.c_str(), 
					     XToLambda(current)));
    }

    current += m_xAxisTick_step;
  }
}


//========================================================================
//                     class gbtLogitPlotStrategyList
//========================================================================

class gbtLogitPlotStrategyList : public wxSheet {
private:
  gbtGameDocument *m_doc;

  //!
  //! @name Overriding wxSheet members to disable selection behavior
  //!
  //@{
  bool SelectRow(int, bool = false, bool = false)
    { return false; }
  bool SelectRows(int, int, bool = false, bool = false)
    { return false; }
  bool SelectCol(int, bool = false, bool = false)
    { return false; }
  bool SelectCols(int, int, bool = false, bool = false)
    { return false; }
  bool SelectCell(const wxSheetCoords&, bool = false, bool = false)
    { return false; }
  bool SelectBlock(const wxSheetBlock&, bool = false, bool = false)
    { return false; }
  bool SelectAll(bool = false) { return false; }

  bool HasSelection(bool = true) const { return false; }
  bool IsCellSelected(const wxSheetCoords &) const { return false; }
  bool IsRowSelected(int) const { return false; }
  bool IsColSelected(int) const { return false; }
  bool DeselectBlock(const wxSheetBlock &, bool = false) { return false; }
  bool ClearSelection(bool = false) { return false; }
  //@}

  /// Overriding wxSheet member to suppress drawing of cursor
  void DrawCursorCellHighlight(wxDC&, const wxSheetCellAttr &) { }

  // Event handlers
  // This disables moving the (unseen) cursor
  void OnLeftDown(wxSheetEvent &) { }
  void OnLeftUp(wxSheetEvent &);

public:
  gbtLogitPlotStrategyList(wxWindow *p_parent, gbtGameDocument *p_doc);

  bool IsShown(int p_index)
  { return GetCellValue(wxSheetCoords(p_index-1, 2)) == wxT("1"); }
};

gbtLogitPlotStrategyList::gbtLogitPlotStrategyList(wxWindow *p_parent,
						   gbtGameDocument *p_doc)
  : wxSheet(p_parent, wxID_ANY), m_doc(p_doc)
{
  CreateGrid(m_doc->GetGame()->MixedProfileLength(), 3);

  SetRowLabelWidth(0);
  SetColLabelHeight(0);
  SetGridLineColour(*wxWHITE);

  for (int st = 1; st <= m_doc->GetGame()->MixedProfileLength(); st++) {
    GameStrategy strategy = m_doc->GetGame()->GetStrategy(st);
    GamePlayer player = strategy->GetPlayer();
    wxColour color = m_doc->GetStyle().GetPlayerColor(player->GetNumber());

    if (strategy->GetNumber() == 1) {
      SetCellSpan(wxSheetCoords(st-1, 0),
		  wxSheetCoords(player->NumStrategies(), 1));
      SetCellValue(wxSheetCoords(st-1, 0),
		   wxString(player->GetLabel().c_str(), *wxConvCurrent));
      SetAttrForegroundColour(wxSheetCoords(st-1, 0), color);
      SetAttrAlignment(wxSheetCoords(st-1, 0),
		       wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL);
    }
    

    SetCellValue(wxSheetCoords(st-1, 1),
		 wxString(strategy->GetLabel().c_str(), *wxConvCurrent));
    SetAttrForegroundColour(wxSheetCoords(st-1, 1), color);

    SetCellValue(wxSheetCoords(st-1, 2), wxT("1"));
    SetAttrForegroundColour(wxSheetCoords(st-1, 2), color);
    SetAttrRenderer(wxSheetCoords(st-1, 2),
		    wxSheetCellRenderer(new wxSheetCellBoolRendererRefData()));
    SetAttrEditor(wxSheetCoords(st-1, 2),
		  wxSheetCellEditor(new wxSheetCellBoolEditorRefData()));
  }

  AutoSizeCols();

  Connect(GetId(), wxEVT_SHEET_CELL_LEFT_DOWN,
	  (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, wxSheetEventFunction(&gbtLogitPlotStrategyList::OnLeftDown)));

  Connect(GetId(), wxEVT_SHEET_CELL_LEFT_UP,
	  (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, wxSheetEventFunction(&gbtLogitPlotStrategyList::OnLeftUp)));
}

void gbtLogitPlotStrategyList::OnLeftUp(wxSheetEvent &p_event)
{
  if (p_event.GetCoords().GetCol() != 2) {
    return;
  }

  if (GetCellValue(p_event.GetCoords()) == wxT("1")) {
    SetCellValue(p_event.GetCoords(), wxT("0"));
  }
  else {
    SetCellValue(p_event.GetCoords(), wxT("1"));
  }

  // Allow normal processing -- parent window will want to update
  p_event.Skip();
}


//========================================================================
//                       class LogitPlotPanel
//========================================================================

class LogitPlotPanel : public wxPanel {
private:
  gbtGameDocument *m_doc;
  LogitMixedBranch m_branch;
  gbtLogitPlotStrategyList *m_plotStrategies;
  gbtLogitPlotCtrl *m_plotCtrl;

  // Event handlers
  void OnChangeStrategies(wxSheetEvent &) { Plot(); }

public:
  LogitPlotPanel(wxWindow *p_parent, gbtGameDocument *p_doc);

  void AddProfile(const wxString &p_text)
  { m_branch.AddProfile(p_text); }

  void SetScaleFactor(double p_scale);
  void FitZoom(void);
  void Plot(void);

  LogitMixedBranch &GetBranch(void) { return m_branch; }
  wxPlotCtrl *GetPlotCtrl(void) const { return m_plotCtrl; }
};

LogitPlotPanel::LogitPlotPanel(wxWindow *p_parent,
				     gbtGameDocument *p_doc)
  : wxPanel(p_parent, wxID_ANY), m_doc(p_doc), m_branch(p_doc)
{
  m_plotCtrl = new gbtLogitPlotCtrl(this, p_doc);
  m_plotCtrl->SetSizeHints(wxSize(600, 400));

  m_plotStrategies = new gbtLogitPlotStrategyList(this, p_doc);
  wxStaticBoxSizer *playerSizer =
    new wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Show strategies"));
  playerSizer->Add(m_plotStrategies, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(playerSizer, 0, wxALL | wxEXPAND, 5);
  sizer->Add(m_plotCtrl, 0, wxALL, 5);

  Connect(m_plotStrategies->GetId(), wxEVT_SHEET_CELL_LEFT_UP,
	  (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, wxSheetEventFunction(&LogitPlotPanel::OnChangeStrategies)));

  SetSizer(sizer);
  Layout();
}
				    
void LogitPlotPanel::Plot(void)
{
  if (m_branch.NumPoints() == 0)  return;

  m_plotCtrl->DeleteCurve(-1);

  for (int st = 1; st <= m_doc->GetGame()->MixedProfileLength(); st++) {
    if (!m_plotStrategies->IsShown(st)) continue;

    wxPlotData *curve = new wxPlotData(m_branch.NumPoints());

    GameStrategy strategy = m_doc->GetGame()->GetStrategy(st);
    GamePlayer player = strategy->GetPlayer();

    curve->SetFilename(wxString(player->GetLabel().c_str(), *wxConvCurrent) +
		       wxT(":") +
		       wxString(strategy->GetLabel().c_str(), *wxConvCurrent));
    
    for (int i = 0; i < m_branch.NumPoints(); i++) {
      curve->SetValue(i, m_plotCtrl->LambdaToX(m_branch.GetLambda(i+1)), 
		      m_branch.GetProfile(i+1)[st]);
    }

    curve->SetPen(wxPLOTPEN_NORMAL, 
		  wxPen(m_doc->GetStyle().GetPlayerColor(player->GetNumber()),
			1, wxSOLID));

    m_plotCtrl->AddCurve(curve, false);
  }
}

void LogitPlotPanel::SetScaleFactor(double p_scale)
{
  m_plotCtrl->SetScaleFactor(p_scale);
  Plot();
}

void LogitPlotPanel::FitZoom(void)
{
  m_plotCtrl->MakeCurveVisible(-1);
}

//========================================================================
//                       class LogitPrintout
//========================================================================

class LogitPrintout : public wxPrintout {
private:
  wxPlotCtrl *m_plot;
    
public:
  LogitPrintout(wxPlotCtrl *p_plot, const wxString &p_label)
    : wxPrintout(p_label), m_plot(p_plot) { }
  virtual ~LogitPrintout() { }

  bool OnPrintPage(int)
  { wxSize size = GetDC()->GetSize();
    m_plot->DrawWholePlot(GetDC(), wxRect(50, 50, 
					  size.GetWidth() - 100,
					  size.GetHeight() - 100));
    return true; 
  }
  bool HasPage(int page) { return (page <= 1); }
  void GetPageInfo(int *minPage, int *maxPage,
		   int *selPageFrom, int *selPageTo)
  { *minPage = 1; *maxPage = 1; *selPageFrom = 1; *selPageTo = 1; }
};


//========================================================================
//                      class LogitMixedDialog
//========================================================================

class LogitMixedDialog : public wxDialog {
private:
  gbtGameDocument *m_doc;
  int m_pid;
  wxProcess *m_process;
  LogitPlotPanel *m_plot;
  wxSpinCtrlDbl *m_scaler;
  wxToolBar *m_toolBar;
  wxStaticText *m_statusText;
  wxButton *m_stopButton, *m_okButton;
  wxTimer m_timer;
  wxString m_output;

  void OnStop(wxCommandEvent &);
  void OnTimer(wxTimerEvent &);
  void OnIdle(wxIdleEvent &);
  void OnEndProcess(wxProcessEvent &);
  void OnSave(wxCommandEvent &);
  void OnPrint(wxCommandEvent &);

  void OnChangeScale(wxSpinEvent &);
  void OnZoomFit(wxCommandEvent &);
  void OnViewData(wxCommandEvent &);

  void Start(void);

public:
  LogitMixedDialog(wxWindow *p_parent, gbtGameDocument *p_doc);

  DECLARE_EVENT_TABLE()
};

const int GBT_ID_TIMER = 2000;
const int GBT_ID_PROCESS = 2001;
const int GBT_MENU_VIEW_DATA = 2002;

BEGIN_EVENT_TABLE(LogitMixedDialog, wxDialog)
  EVT_END_PROCESS(GBT_ID_PROCESS, LogitMixedDialog::OnEndProcess)
  EVT_IDLE(LogitMixedDialog::OnIdle)
  EVT_TIMER(GBT_ID_TIMER, LogitMixedDialog::OnTimer)
  EVT_MENU(wxID_SAVE, LogitMixedDialog::OnSave)
  EVT_MENU(wxID_PRINT, LogitMixedDialog::OnPrint)
  EVT_MENU(GBT_MENU_VIEW_ZOOMFIT, LogitMixedDialog::OnZoomFit)
  EVT_MENU(GBT_MENU_VIEW_DATA, LogitMixedDialog::OnViewData)
END_EVENT_TABLE()

#include "bitmaps/stop.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/savedata.xpm"
#include "bitmaps/datasrc.xpm"
#include "bitmaps/zoomfit.xpm"

LogitMixedDialog::LogitMixedDialog(wxWindow *p_parent, 
				   gbtGameDocument *p_doc)
  : wxDialog(p_parent, -1, wxT("Compute quantal response equilibria"),
	     wxDefaultPosition),
    m_doc(p_doc), m_process(0), m_timer(this, GBT_ID_TIMER)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *startSizer = new wxBoxSizer(wxHORIZONTAL);

  m_statusText = new wxStaticText(this, wxID_STATIC,
				  wxT("The computation is currently in progress."));
  m_statusText->SetForegroundColour(*wxBLUE);
  startSizer->Add(m_statusText, 0, wxALL | wxALIGN_CENTER, 5);

  m_stopButton = new wxBitmapButton(this, wxID_CANCEL, wxBitmap(stop_xpm));
  m_stopButton->SetToolTip(_("Stop the computation"));
  startSizer->Add(m_stopButton, 0, wxALL | wxALIGN_CENTER, 5);
  Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(LogitMixedDialog::OnStop));

  sizer->Add(startSizer, 0, wxALL | wxALIGN_CENTER, 5);

  m_toolBar = new wxToolBar(this, wxID_ANY);
  m_toolBar->SetWindowStyle(wxTB_HORIZONTAL | wxTB_FLAT);
  m_toolBar->SetMargins(4, 4);
  m_toolBar->SetToolBitmapSize(wxSize(24, 24));

  m_toolBar->AddTool(wxID_SAVE,
                     wxEmptyString,
                     wxBitmap(savedata_xpm),
                     wxNullBitmap,
                     wxITEM_NORMAL,
		                 _("Save the correspondence to a CSV file"),
		                 _("Save the correspondence to a CSV file"));

  m_toolBar->EnableTool(wxID_SAVE, false);

  m_toolBar->AddTool(GBT_MENU_VIEW_DATA,
                     wxEmptyString,
                     wxBitmap(datasrc_xpm),
                     wxNullBitmap,
                     wxITEM_NORMAL,
		                 _("View the points in the correspondence"),
		                 _("View the points in the correspondence"));

  m_toolBar->EnableTool(GBT_MENU_VIEW_DATA, false);

  m_toolBar->AddTool(wxID_PRINT,
                     wxEmptyString,
                     wxBitmap(print_xpm),
                     wxNullBitmap,
                     wxITEM_NORMAL,
                     _("Print the graph"),
                     _("Print the graph"));

  m_toolBar->AddSeparator();

  m_toolBar->AddTool(GBT_MENU_VIEW_ZOOMFIT,
                     wxEmptyString,
                     wxBitmap(zoomfit_xpm),
                     wxNullBitmap,
                     wxITEM_NORMAL,
                     _("Show the whole graph"),
                     _("Show the whole graph"));

  m_toolBar->AddControl(new wxStaticText(m_toolBar, wxID_STATIC,
					 wxT("Graph scaling:")));
  m_scaler = new wxSpinCtrlDbl(*m_toolBar, wxID_ANY,
			       wxT(""),
			       wxDefaultPosition, wxDefaultSize,
			       0, 0.1, 10.0, 1.0, 0.1);
  m_toolBar->AddControl(m_scaler);
  Connect(m_scaler->GetId(), wxEVT_COMMAND_SPINCTRL_UPDATED,
	  wxSpinEventHandler(LogitMixedDialog::OnChangeScale));

  m_toolBar->Realize();
  sizer->Add(m_toolBar, 0, wxALL | wxEXPAND, 5);


  wxBoxSizer *midSizer = new wxBoxSizer(wxHORIZONTAL);
  m_plot = new LogitPlotPanel(this, m_doc);
  midSizer->Add(m_plot, 0, wxALL | wxALIGN_CENTER, 5);

  sizer->Add(midSizer, 0, wxALL | wxALIGN_CENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_okButton = new wxButton(this, wxID_OK, wxT("OK"));
  buttonSizer->Add(m_okButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_okButton->Enable(false);

  sizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
  Start();
}

void LogitMixedDialog::Start(void)
{
  m_doc->BuildNfg();

  m_process = new wxProcess(this, GBT_ID_PROCESS);
  m_process->Redirect();

#ifdef __WXMAC__
  m_pid = wxExecute(wxStandardPaths::Get().GetExecutablePath() + 
		    wxT("-logit -S"),
                    wxEXEC_ASYNC, m_process);
#else
  m_pid = wxExecute(wxT("gambit-logit -S"), wxEXEC_ASYNC, m_process);
#endif // __WXMAC__
  
  std::ostringstream s;
  m_doc->GetGame()->Write(s, "nfg");
  wxString str(wxString(s.str().c_str(), *wxConvCurrent));
  
  // It is possible that the whole string won't write on one go, so
  // we should take this possibility into account.  If the write doesn't
  // complete the whole way, we take a 100-millisecond siesta and try
  // again.  (This seems to primarily be an issue with -- you guessed it --
  // Windows!)
  while (str.length() > 0) {
    wxTextOutputStream os(*m_process->GetOutputStream());

    // It appears that (at least with mingw) the string itself contains
    // only '\n' for newlines.  If we don't SetMode here, these get
    // converted to '\r\n' sequences, and so the number of characters
    // LastWrite() returns does not match the number of characters in
    // our string.  Setting this explicitly solves this problem.
    os.SetMode(wxEOL_UNIX);
    os.WriteString(str);
    str.Remove(0, m_process->GetOutputStream()->LastWrite());
    wxMilliSleep(100);
  }
  m_process->CloseOutput();

  m_timer.Start(1000, false);
}

void LogitMixedDialog::OnIdle(wxIdleEvent &p_event)
{
  if (!m_process)  return;

  if (m_process->IsInputAvailable()) {
    wxTextInputStream tis(*m_process->GetInputStream());

    wxString msg;
    msg << tis.ReadLine();
    m_plot->AddProfile(msg);
    //m_mixedList->AddProfile(msg, false);
    m_output += msg;
    m_output += wxT("\n");

    p_event.RequestMore();
  }
  else {
    m_timer.Start(1000, false);
  }
}

void LogitMixedDialog::OnTimer(wxTimerEvent &p_event)
{
  wxWakeUpIdle();
}

void LogitMixedDialog::OnEndProcess(wxProcessEvent &p_event)
{
  m_stopButton->Enable(false);
  m_timer.Stop();

  while (m_process->IsInputAvailable()) {
    wxTextInputStream tis(*m_process->GetInputStream());

    wxString msg;
    msg << tis.ReadLine();

    if (msg != wxT("")) {
      m_plot->AddProfile(msg);
      //m_mixedList->AddProfile(msg, true);
      m_output += msg;
      m_output += wxT("\n");
    }
  }

  if (p_event.GetExitCode() == 0) {
    m_statusText->SetLabel(wxT("The computation has completed."));
    m_statusText->SetForegroundColour(wxColour(0, 192, 0));
  }
  else {
    m_statusText->SetLabel(wxT("The computation ended abnormally."));
    m_statusText->SetForegroundColour(*wxRED);
  }

  m_okButton->Enable(true);
  m_toolBar->EnableTool(wxID_SAVE, true);
  m_toolBar->EnableTool(GBT_MENU_VIEW_DATA, true);
  m_plot->Plot();
}

void LogitMixedDialog::OnStop(wxCommandEvent &)
{
  // Per the wxWidgets wiki, under Windows, programs that run
  // without a console window don't respond to the more polite
  // SIGTERM, so instead we must be rude and SIGKILL it.
  m_stopButton->Enable(false);

#ifdef __WXMSW__
  wxProcess::Kill(m_pid, wxSIGKILL);
#else
  wxProcess::Kill(m_pid, wxSIGTERM);
#endif  // __WXMSW__
}

void LogitMixedDialog::OnSave(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"), wxT(""), wxT(""),
		      wxT("CSV files (*.csv)|*.csv|")
			  wxT("All files (*.*)|*.*"),
		      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    std::ofstream file((const char *) dialog.GetPath().mb_str());
    file << ((const char *) m_output.mb_str());
  }
}

void LogitMixedDialog::OnPrint(wxCommandEvent &)
{
  wxPrintDialogData data;
  wxPrinter printer(&data);

  wxPrintout *printout = new LogitPrintout(m_plot->GetPlotCtrl(),
					   wxT("Logit correspondence"));

  if (!printer.Print(this, printout, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      wxMessageBox(_("There was an error in printing"), _("Error"), wxOK);
    }
    // Otherwise, user hit "cancel"; just be quiet and return.
    return;
  }
}

void LogitMixedDialog::OnChangeScale(wxSpinEvent &)
{
  m_plot->SetScaleFactor(m_scaler->GetValue());
}

void LogitMixedDialog::OnZoomFit(wxCommandEvent &)
{
  m_plot->FitZoom();
}

void LogitMixedDialog::OnViewData(wxCommandEvent &)
{
  LogitBranchDialog dialog(this, m_doc, m_plot->GetBranch());
  dialog.ShowModal();
}


}  // end encapsulating anonymous namespace

//========================================================================
//                        External interface
//========================================================================

void LogitStrategic(wxWindow *p_parent, gbtGameDocument *p_doc)
{
  LogitMixedDialog(p_parent, p_doc).ShowModal();
}
