//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Window for displaying profiles
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "profile.h"
#include "id.h"

//-------------------------------------------------------------------------
//                      class gbtProfileTable
//-------------------------------------------------------------------------

//
// Design choice: One profile table class, the behavior of which depends
// on the "profile style" chosen in the document preferences.
// Alternative possibility would be an appropriate base class/derived class
// setup; not sure that's not just overkill here.
//
class gbtProfileTable : public wxGridTableBase {
private:
  gbtGameDocument *m_doc;

public:
  gbtProfileTable(gbtGameDocument *);
  virtual ~gbtProfileTable() { }

  // Overriding members of wxGridTableBase
  int GetNumberRows(void);
  int GetNumberCols(void);
  wxString GetValue(int, int);
  wxString GetColLabelValue(int);
  void SetValue(int, int, const wxString &) { }
  bool IsEmptyCell(int, int) { return false; }

  bool InsertRows(size_t pos = 0, size_t numRows = 1);
  bool AppendRows(size_t numRows = 1);
  bool DeleteRows(size_t pos = 0, size_t numRows = 1);

  bool InsertCols(size_t pos = 0, size_t numCols = 1);
  bool AppendCols(size_t numCols = 1);
  bool DeleteCols(size_t pos = 0, size_t numCols = 1);

  wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind);

  // State and data access
  int GetInfoColumns(void) const;
  int GetBehavColumns(void) const;
  int GetMixedColumns(void) const;

  int GetPlayerNumber(int p_col) const;
};

gbtProfileTable::gbtProfileTable(gbtGameDocument *p_doc)
  : m_doc(p_doc)
{ }


//--------------------------------------------------------------------------
//                gbtProfileTable: State and data access
//--------------------------------------------------------------------------

int gbtProfileTable::GetInfoColumns(void) const
{
  return 9;
}

int gbtProfileTable::GetBehavColumns(void) const
{
  if (!m_doc->HasEfg()) {
    return 0;
  }

  switch (m_doc->GetPreferences().ProfileStyle()) {
  case GBT_PROFILES_GRID:
    return m_doc->GetEfg().ProfileLength();
  case GBT_PROFILES_VECTOR:
  case GBT_PROFILES_MYERSON:
  default:
    return m_doc->GetEfg().NumPlayerInfosets();
  }
}

int gbtProfileTable::GetMixedColumns(void) const
{
  switch (m_doc->GetPreferences().ProfileStyle()) {
  case GBT_PROFILES_GRID:
    return m_doc->GetNfg().ProfileLength();
  case GBT_PROFILES_VECTOR:
  case GBT_PROFILES_MYERSON:
  default:
    return m_doc->GetNfg().NumPlayers();
  }
}

int gbtProfileTable::GetPlayerNumber(int p_col) const
{
  if (p_col < GetInfoColumns()) {
    return 0;
  }
  else if (p_col < GetInfoColumns() + GetBehavColumns()) {
    for (int pl = 1, col = GetInfoColumns(); 
	 pl <= m_doc->GetEfg().NumPlayers();
	 pl++) {
      for (int iset = 1; iset <= m_doc->GetEfg().GetPlayer(pl).NumInfosets();
	   iset++) {
	if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
	  if ((col += m_doc->GetEfg().GetPlayer(pl).GetInfoset(iset).NumActions()) > p_col) {
	    return pl;
	  }
	}
	else {
	  if (col++ == p_col) {
	    return pl;
	  }
	}
      }
    }
  }

  return 0;
}

//--------------------------------------------------------------------------
//          gbtProfileTable: Overriding wxGridTableBase members
//--------------------------------------------------------------------------

int gbtProfileTable::GetNumberRows(void)
{ return m_doc->AllMixedProfiles().Length(); }

int gbtProfileTable::GetNumberCols(void)
{
  return GetInfoColumns() + GetBehavColumns() + GetMixedColumns();
}

wxString gbtProfileTable::GetColLabelValue(int p_col)
{
  wxString labels[] = { "Name", "Creator", "BNash", "SPE", "Seq", "BLiap",
			"MNash", "THPE", "NLiap" };
  if (p_col < GetInfoColumns()) {
    return labels[p_col];
  }
  else if (p_col < GetInfoColumns() + GetBehavColumns()) {
    for (int pl = 1, col = GetInfoColumns(); 
	 pl <= m_doc->GetEfg().NumPlayers(); pl++) {
      gbtEfgPlayer player = m_doc->GetEfg().GetPlayer(pl);
      if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
	for (int iset = 1; iset <= player.NumInfosets(); iset++) {
	  gbtEfgInfoset infoset = player.GetInfoset(iset);
	  for (int act = 1; act <= infoset.NumActions(); col++, act++) {
	    if (col == p_col) {
	      return wxString::Format("%s:%s:%s",
				      (char *) player.GetLabel(),
				      (char *) infoset.GetLabel(),
				      (char *) infoset.GetAction(act).GetLabel());
	    }
	  }
	}
      }
      else {
	for (int iset = 1; iset <= player.NumInfosets(); col++, iset++) {
	  if (col == p_col) {
	    return wxString::Format("%s:%s",
				    (char *) player.GetLabel(),
				    (char *) player.GetInfoset(iset).GetLabel());
	  }
	}
      }
    }
  }
  else {
    if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
      for (int pl = 1, col = GetInfoColumns() + GetBehavColumns();
	   pl <= m_doc->GetNfg().NumPlayers(); pl++) {
	gbtNfgPlayer player = m_doc->GetNfg().GetPlayer(pl);
	for (int st = 1; st <= player.NumStrategies(); col++, st++) {
	  if (col == p_col) {
	    return wxString::Format("%s:%s",
				    (char *) player.GetLabel(),
				    (char *) player.GetStrategy(st).GetLabel());
	  }
	}
      }
    }
    else {
      return (char *) m_doc->GetNfg().GetPlayer(p_col - GetInfoColumns() - GetBehavColumns() + 1).GetLabel();
    }
  }

  return "";
}

wxString gbtProfileTable::GetValue(int p_row, int p_col) 
{
  const MixedSolution *mixed = &m_doc->AllMixedProfiles()[p_row + 1];
  const BehavSolution *behav = 0;
  if (m_doc->HasEfg()) {
    behav = &m_doc->AllBehavProfiles()[p_row + 1];
  }

  switch (p_col) {
  case 0:
    return (char *) mixed->GetLabel();
  case 1:
    return (char *) mixed->GetCreator();
  case 2:
    if (behav) {
      return (char *) ToText(behav->IsNash());
    }
    else {
      return "";
    }
  case 3:
    if (behav) {
      return (char *) ToText(behav->IsSubgamePerfect());
    }
    else {
      return "";
    }
  case 4:
    if (behav) {
      return (char *) ToText(behav->IsSequential());
    }
    else {
      return "";
    }
  case 5:
    if (behav) {
      return (char *) ToText(behav->GetLiapValue(),
			     m_doc->GetPreferences().NumDecimals());
    }
    else {
      return "";
    }
  case 6:
    return (char *) ToText(mixed->IsNash());
  case 7:
    return (char *) ToText(mixed->IsPerfect());
  case 8:
    return (char *) ToText(mixed->GetLiapValue(),
			   m_doc->GetPreferences().NumDecimals());
  default:
    if (p_col < GetInfoColumns() + GetBehavColumns()) {
      int offset = GetInfoColumns() - 1;
      if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
	return (char *) ToText((*behav->Profile())[p_col - offset],
			       m_doc->GetPreferences().NumDecimals());
      }
      else if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_VECTOR) {
	const gPVector<gNumber> &profile = behav->Profile()->GetPVector();
	wxString ret = _T("("); 
	for (int i = 1; i <= profile.Lengths()[p_col - offset]; i++) {
	  if (i > 1) {
	    ret += _T(",");
	  }
	  ret += (char *) ToText(profile(p_col - offset, i),
				 m_doc->GetPreferences().NumDecimals());
	}
	return ret + _T(")");
      }
      else if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_MYERSON) {
	for (int pl = 1, col = GetInfoColumns();
	     pl <= m_doc->GetEfg().NumPlayers(); pl++) {
	  gbtEfgPlayer player = m_doc->GetEfg().GetPlayer(pl);
	  for (int iset = 1; iset <= player.NumInfosets(); iset++, col++) {
	    if (col == p_col) {
	      gbtEfgInfoset infoset = player.GetInfoset(iset);
	      wxString ret;
	      for (int act = 1; act <= infoset.NumActions(); act++) {
		if ((*behav)(infoset.GetAction(act)) > gNumber(0)) {
		  if (ret != "") {
		    ret += _T("+");
		  }
		  ret += (char *) ToText((*behav)(infoset.GetAction(act)),
					 m_doc->GetPreferences().NumDecimals());
		  if (infoset.GetAction(act).GetLabel() != "") {
		    ret += _T("*[") + infoset.GetAction(act).GetLabel() + _T("]");
		  }
		  else {
		    ret += wxString::Format("*[#%d]", act);
		  }
		}
	      }
	      return ret;
	    }
	  }
	}
      }
      else {
	return "Unimplemented";
      }
    }
    else {
      int offset = GetInfoColumns() + GetBehavColumns() - 1;
      if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
	return (char *) ToText((*mixed->Profile())[p_col - offset],
			       m_doc->GetPreferences().NumDecimals());
      }
      else if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_VECTOR) {
	wxString ret = _T("("); 
	gbtNfgPlayer player = m_doc->GetNfg().GetPlayer(p_col - offset);
	for (int st = 1; st <= player.NumStrategies(); st++) {
	  gbtNfgStrategy strategy = player.GetStrategy(st);
	  if (st > 1) {
	    ret += _T(",");
	  }
	  ret += (char *) ToText((*mixed)(strategy),
				 m_doc->GetPreferences().NumDecimals());
	}
	return ret + _T(")");
      }
      else if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_MYERSON) {
	wxString ret;
	gbtNfgPlayer player = m_doc->GetNfg().GetPlayer(p_col - offset);
	for (int st = 1; st <= player.NumStrategies(); st++) {
	  gbtNfgStrategy strategy = player.GetStrategy(st);
	  if ((*mixed)(strategy) > gNumber(0)) {
	    if (ret != "") {
	      ret += _T("+");
	    }
	    ret += (char *) ToText((*mixed)(strategy),
				   m_doc->GetPreferences().NumDecimals());
	    if (strategy.GetLabel() != "") {
	      ret += _T("*[") + player.GetStrategy(st).GetLabel() + _T("]");
	    }
	    else {
	      ret += wxString::Format("*[#%d]", st);
	    }
	  }
	}
	return ret;
      }
      else {
	return "Unimplemented";
      }
    }
  }
  return "";
}

bool gbtProfileTable::InsertRows(size_t pos, size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
			 pos, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool gbtProfileTable::AppendRows(size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool gbtProfileTable::DeleteRows(size_t pos, size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
			 pos, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool gbtProfileTable::InsertCols(size_t pos, size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_INSERTED,
			 pos, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool gbtProfileTable::AppendCols(size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_APPENDED, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool gbtProfileTable::DeleteCols(size_t pos, size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_DELETED,
			 pos, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

wxGridCellAttr *
gbtProfileTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind)
{
  wxGridCellAttr *attr = new wxGridCellAttr;

  if (col < GetInfoColumns() + GetBehavColumns()) {
    attr->SetTextColour(m_doc->GetPreferences().PlayerColor(GetPlayerNumber(col)));
  }
  else {
    if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
      int firstCol = GetInfoColumns() + GetBehavColumns();
      int lastCol = firstCol - 1 + m_doc->GetNfg().GetPlayer(1).NumStrategies();
      for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
	if (col >= firstCol && col <= lastCol) {
	  attr->SetTextColour(m_doc->GetPreferences().PlayerColor(pl));
	  break;
	}
	if (pl == m_doc->GetNfg().NumPlayers()) {
	  break;
	}
	firstCol = lastCol + 1;
	lastCol += m_doc->GetNfg().GetPlayer(pl+1).NumStrategies();
      }
    }
    else {
      int player = col - GetInfoColumns() + GetBehavColumns() + 1;
      attr->SetTextColour(m_doc->GetPreferences().PlayerColor(player));
    }
  }

  if (col >= 2) {
    attr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  }
  else {
    attr->SetAlignment(wxALIGN_LEFT, wxALIGN_CENTER);
  }

  if (row % 2 == 0) {
    attr->SetBackgroundColour(wxColour(200, 200, 200));
  }
  else {
    attr->SetBackgroundColour(wxColour(225, 225, 225));
  }

  return attr;
}

//-------------------------------------------------------------------------
//                     class gbtProfileGrid
//-------------------------------------------------------------------------

class gbtProfileGrid : public wxGrid {
private:
  gbtGameDocument *m_doc;
  gbtProfileTable *m_table;
  bool m_showInfo, m_showBehav, m_showMixed;

  // wxGrid members
  // Overriding this suppresses drawing of the grid cell highlight
  virtual void DrawCellHighlight(wxDC &, const wxGridCellAttr *) { }

  // Event handlers
  void OnLeftClick(wxGridEvent &);

  // Overriding view members
  bool IsEfgView(void) const { return false; }
  bool IsNfgView(void) const { return true; }

public:
  gbtProfileGrid(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~gbtProfileGrid();

  wxString GetReport(void) const;

  // Override grid's GetBestSize() for some custom logic
  virtual wxSize GetBestSize(void) const;

  // Access to state
  bool GetShowInfo(void) const { return m_showInfo; }
  void SetShowInfo(bool p_show) { m_showInfo = p_show; }

  bool GetShowBehav(void) const { return m_showBehav; }
  void SetShowBehav(bool p_show) { m_showBehav = p_show; }

  bool GetShowMixed(void) const { return m_showMixed; }
  void SetShowMixed(bool p_show) { m_showMixed = p_show; }

  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(gbtProfileGrid, wxGrid)
  EVT_GRID_CELL_LEFT_CLICK(gbtProfileGrid::OnLeftClick)
END_EVENT_TABLE()

gbtProfileGrid::gbtProfileGrid(gbtGameDocument *p_doc, 
			       wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_doc(p_doc), 
    m_showInfo(true),
    m_showBehav(m_doc->HasEfg()),
    m_showMixed(!m_doc->HasEfg())
{
  m_table = new gbtProfileTable(m_doc);
  SetTable(m_table, true);
  SetEditable(false);
  DisableDragRowSize();
  DisableDragColSize();
  EnableGridLines(false);
  SetLabelSize(wxVERTICAL, 0);
  SetSelectionMode(wxGrid::wxGridSelectRows);
  SetMargins(0, 0);
}

gbtProfileGrid::~gbtProfileGrid()
{ }

void gbtProfileGrid::OnUpdate(void)
{
  SetDefaultCellFont(m_doc->GetPreferences().GetDataFont());
  SetLabelFont(m_doc->GetPreferences().GetLabelFont());
  SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

  if (GetRows() > m_doc->AllMixedProfiles().Length())  {
    DeleteRows(0, GetRows() - m_doc->AllMixedProfiles().Length());
  }
  else if (GetRows() < m_doc->AllMixedProfiles().Length()) {
    AppendRows(m_doc->AllMixedProfiles().Length() - GetRows());
  }
  AutoSizeRows();
  AutoSizeColumns();
  // Set all probability columns to be the same width, which is
  // the narrowest width which fits all the entries
  int max = 0;
  for (int col = m_table->GetInfoColumns(); col < GetCols(); col++) {
    if (GetColSize(col) > max) {
      max = GetColSize(col);
    }
  }
  for (int col = m_table->GetInfoColumns(); col < GetCols(); col++) {
    SetColSize(col, max);
  }

  if (!m_showInfo) {
    for (int col = 2; col < m_table->GetInfoColumns(); 
	 SetColSize(col++, 0));
  }
  if (!m_showBehav) {
    for (int col = m_table->GetInfoColumns(); 
	 col < m_table->GetInfoColumns() + m_table->GetBehavColumns();
	 SetColSize(col++, 0));
  }
  if (!m_showMixed) {
    for (int col = m_table->GetInfoColumns() + m_table->GetBehavColumns();
	 col < GetCols(); SetColSize(col++, 0));
  }

  AdjustScrollbars();
  ForceRefresh();
  if (m_doc->IsProfileSelected()) {
    int row = m_doc->AllMixedProfiles().Find(m_doc->GetMixedProfile()) - 1;
    SelectRow(row);
    MakeCellVisible(row, 0);
  }
}

void gbtProfileGrid::OnLeftClick(wxGridEvent &p_event)
{
  m_doc->SetCurrentProfile(p_event.GetRow() + 1);
  // Veto this event; when grid refreshes, correct row will be selected
  p_event.Veto();
}

wxSize gbtProfileGrid::GetBestSize(void) const
{
  wxSize size = wxGrid::GetBestSize();
  if (const_cast<gbtProfileGrid *>(this)->GetNumberRows() == 0) {
    size.SetHeight(size.GetHeight() * 2);
  }
  return size;
}

wxString gbtProfileGrid::GetReport(void) const
{
  wxString report;
  const gList<MixedSolution> &profiles = m_doc->AllMixedProfiles();
  gbtNfgGame nfg = m_doc->GetNfg();

  report += wxString::Format("Mixed strategy profiles on game '%s' [%s]\n\n",
			     (const char *) nfg.GetLabel(),
			     m_doc->GetFilename().c_str());

  report += wxString::Format("Number of profiles: %d\n", profiles.Length());

  for (int i = 1; i <= profiles.Length(); i += 4) {
    report += "\n----------\n\n";

    report += "          ";
    for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
      report += wxString::Format("%-15s ", 
				 (const char *) profiles[i+j].GetLabel());
    }
    report += "\n";

    report += "          ";
    for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
      report += ("--------------- "); 
    }
    report += "\n";

    report += "Creator:  ";
    for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
      report += wxString::Format("%-15s ",
				 (const char *) profiles[i+j].GetCreator());
    }
    report += "\n";

    report += "Nash?     ";
    for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
      report += wxString::Format("%-15s ",
				 (const char *) ToText(profiles[i+j].IsNash()));
    }
    report += "\n";

    report += "Perfect?  ";
    for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
      report += wxString::Format("%-15s ",
				 (const char *) ToText(profiles[i+j].IsPerfect()));
    }
    report += "\n";

    report += "Liap:     ";
    for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
      report += wxString::Format("%-15s ",
				 (const char *) ToText(profiles[i+j].GetLiapValue()));
    }
    report += "\n\n";

    for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
      gbtNfgPlayer player = nfg.GetPlayer(pl);
      report += wxString::Format("%s\n", (const char *) player.GetLabel());

      for (int st = 1; st <= player.NumStrategies(); st++) {
	report += wxString::Format("%2d: %-6s", st,
				   (const char *) player.GetStrategy(st).GetLabel());

	for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
	  report += wxString::Format("%-15s ", 
				     (const char *) ToText((*profiles[i+j].Profile())(pl, st)));
	}
	report += "\n";
      }

      report += "\n";
    }
  }

  return report;
}

//-------------------------------------------------------------------------
//                      class gbtProfileFrame
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtProfileFrame, wxFrame)
  EVT_MENU(wxID_CLOSE, gbtProfileFrame::Close)
  EVT_MENU(GBT_MENU_PROFILES_INFO, gbtProfileFrame::OnViewInfo)
  EVT_MENU(GBT_MENU_PROFILES_BEHAV, gbtProfileFrame::OnViewBehavior)
  EVT_MENU(GBT_MENU_PROFILES_MIXED, gbtProfileFrame::OnViewMixed)
  EVT_MENU(GBT_MENU_PROFILES_GRID, gbtProfileFrame::OnViewGrid)
  EVT_MENU(GBT_MENU_PROFILES_MYERSON, gbtProfileFrame::OnViewMyerson)
  EVT_MENU(GBT_MENU_PROFILES_VECTOR, gbtProfileFrame::OnViewVector)
  EVT_CLOSE(gbtProfileFrame::OnClose)
END_EVENT_TABLE()

gbtProfileFrame::gbtProfileFrame(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(300, 200)),
    gbtGameView(p_doc)
{
  m_grid = new gbtProfileGrid(p_doc, this);

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(GBT_MENU_PROFILES_REPORT, "Report",
		   "Generate a report with information on profiles");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_CLOSE, "&Close", "Close this window");

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(GBT_MENU_PROFILES_NEW, "New",
		   "Create a new profile");
  editMenu->Append(GBT_MENU_PROFILES_DUPLICATE, "Duplicate",
		   "Duplicate this profile");
  editMenu->Append(GBT_MENU_PROFILES_DELETE, "Delete",
		   "Delete this profile");
  editMenu->Append(GBT_MENU_PROFILES_PROPERTIES, "Properties",
		   "View and edit properties of this profile");

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(GBT_MENU_PROFILES_INFO, "Info",
		   "Display general information about profiles", true);
  viewMenu->Append(GBT_MENU_PROFILES_BEHAV, "Behavior",
		   "Display behavior strategy representations", true);
  viewMenu->Enable(GBT_MENU_PROFILES_BEHAV, m_doc->HasEfg());
  viewMenu->Append(GBT_MENU_PROFILES_MIXED, "Mixed",
		   "Display mixed strategy representations", true);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_PROFILES_GRID, "Grid",
		   "Display probabilities in grid format", true);
  viewMenu->Append(GBT_MENU_PROFILES_MYERSON, "Myerson",
		   "Display probabilities in Myerson (bracket) format", true);
  viewMenu->Append(GBT_MENU_PROFILES_VECTOR, "Vector",
		   "Display probabilities in vector format", true);

  wxMenu *formatMenu = new wxMenu;

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(formatMenu, "&Format");
  SetMenuBar(menuBar);

  Show(false);
}

gbtProfileFrame::~gbtProfileFrame()
{ }

//--------------------------------------------------------------------------
//                  gbtProfileFrame: Event handlers
//--------------------------------------------------------------------------

void gbtProfileFrame::OnViewInfo(wxCommandEvent &)
{
  m_grid->SetShowInfo(!m_grid->GetShowInfo());
  OnUpdate(0);
}

void gbtProfileFrame::OnViewBehavior(wxCommandEvent &)
{
  m_grid->SetShowBehav(!m_grid->GetShowBehav());
  OnUpdate(0);
}

void gbtProfileFrame::OnViewMixed(wxCommandEvent &)
{
  m_grid->SetShowMixed(!m_grid->GetShowMixed());
  OnUpdate(0);
}

void gbtProfileFrame::OnViewGrid(wxCommandEvent &)
{
  if (m_doc->GetPreferences().ProfileStyle() != GBT_PROFILES_GRID) {
    if (m_doc->HasEfg()) {
      m_grid->AppendCols(m_doc->GetEfg().ProfileLength() -
			 m_doc->GetEfg().NumPlayerInfosets());
    }
    m_grid->AppendCols(m_doc->GetNfg().ProfileLength() - 
		       m_doc->GetNfg().NumPlayers());
  }
  m_doc->GetPreferences().SetProfileStyle(GBT_PROFILES_GRID);
  m_doc->UpdateViews(this);
}

void gbtProfileFrame::OnViewMyerson(wxCommandEvent &)
{
  if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
    if (m_doc->HasEfg()) {
      m_grid->DeleteCols(0,
			 m_doc->GetEfg().ProfileLength() -
			 m_doc->GetEfg().NumPlayerInfosets());
    }
    m_grid->DeleteCols(0,
		       m_doc->GetNfg().ProfileLength() - 
		       m_doc->GetNfg().NumPlayers());
  }
  m_doc->GetPreferences().SetProfileStyle(GBT_PROFILES_MYERSON);
  m_doc->UpdateViews(this);
}

void gbtProfileFrame::OnViewVector(wxCommandEvent &)
{
  if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
    if (m_doc->HasEfg()) {
      m_grid->DeleteCols(0,
			 m_doc->GetEfg().ProfileLength() -
			 m_doc->GetEfg().NumPlayerInfosets());
    }
    m_grid->DeleteCols(0,
		       m_doc->GetNfg().ProfileLength() - 
		       m_doc->GetNfg().NumPlayers());
  }
  m_doc->GetPreferences().SetProfileStyle(GBT_PROFILES_VECTOR);
  m_doc->UpdateViews(this);
}

void gbtProfileFrame::OnClose(wxCloseEvent &p_event)
{
  m_doc->SetShowProfiles(false);
  // Frame is now hidden; leave it that way, don't actually close
  p_event.Veto();
}

void gbtProfileFrame::OnUpdate(gbtGameView *p_sender)
{
  if (m_doc->ShowProfiles()) {
    GetMenuBar()->Check(GBT_MENU_PROFILES_INFO, m_grid->GetShowInfo());
    GetMenuBar()->Check(GBT_MENU_PROFILES_BEHAV, m_grid->GetShowBehav());
    GetMenuBar()->Check(GBT_MENU_PROFILES_MIXED, m_grid->GetShowMixed());

    int style = m_doc->GetPreferences().ProfileStyle();
    GetMenuBar()->Check(GBT_MENU_PROFILES_GRID,
			style == GBT_PROFILES_GRID);
    GetMenuBar()->Check(GBT_MENU_PROFILES_MYERSON,
			style == GBT_PROFILES_MYERSON);
    GetMenuBar()->Check(GBT_MENU_PROFILES_VECTOR,
			style == GBT_PROFILES_VECTOR);

    m_grid->OnUpdate();
    wxSize size = m_grid->GetBestSize();
    SetClientSize(size);
    m_grid->SetSize(size.GetWidth() + 1, size.GetHeight() + 1);
    m_grid->SetScrollRate(0, 0);

    if (m_doc->GetFilename() != "") {
      SetTitle(wxString::Format("Gambit - Profiles: [%s] %s", 
				m_doc->GetFilename().c_str(), 
				(char *) m_doc->GetNfg().GetLabel()));
    }
    else {
      SetTitle(wxString::Format("Gambit - Profiles: %s",
				(char *) m_doc->GetNfg().GetLabel()));
    }
  }
  Show(m_doc->ShowProfiles());
}


