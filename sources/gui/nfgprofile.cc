//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Normal form mixed profile window
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
#include "nfgprofile.h"
#include "nfgconst.h"

//-------------------------------------------------------------------------
//                      class gbtNfgProfileTable
//-------------------------------------------------------------------------

//
// Design choice: One profile table class, the behavior of which depends
// on the "profile style" chosen in the document preferences.
// Alternative possibility would be an appropriate base class/derived class
// setup; not sure that's not just overkill here.
//
class gbtNfgProfileTable : public wxGridTableBase {
private:
  gbtGameDocument *m_doc;

public:
  gbtNfgProfileTable(gbtGameDocument *);
  virtual ~gbtNfgProfileTable() { }

  int GetNumberRows(void);
  int GetNumberCols(void);
  wxString GetValue(int, int);
  wxString GetColLabelValue(int);
  void SetValue(int, int, const wxString &) { }
  bool IsEmptyCell(int, int) { return false; }

  bool InsertRows(size_t pos = 0, size_t numRows = 1);
  bool AppendRows(size_t numRows = 1);
  bool DeleteRows(size_t pos = 0, size_t numRows = 1);

  wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind);
};

gbtNfgProfileTable::gbtNfgProfileTable(gbtGameDocument *p_doc)
  : m_doc(p_doc)
{ }

int gbtNfgProfileTable::GetNumberRows(void)
{ return m_doc->AllMixedProfiles().Length(); }

int gbtNfgProfileTable::GetNumberCols(void)
{
  switch (m_doc->GetPreferences().ProfileStyle()) {
  case GBT_PROFILES_GRID:
    return 5 + m_doc->GetNfg().ProfileLength();
  case GBT_PROFILES_VECTOR:
  case GBT_PROFILES_MYERSON:
  default:
    return 5 + m_doc->GetNfg().NumPlayers();
  }
}

wxString gbtNfgProfileTable::GetColLabelValue(int p_col)
{
  wxString labels[] = { "Name", "Creator", "Nash", "Perfect", "Liap" };
  if (p_col < 5) {
    return labels[p_col];
  }

  if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
    for (int pl = 1, col = 5; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
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
    return (char *) m_doc->GetNfg().GetPlayer(p_col - 4).GetLabel();
  }

  return "";
}

wxString gbtNfgProfileTable::GetValue(int p_row, int p_col) 
{
  const MixedSolution &profile = m_doc->AllMixedProfiles()[p_row + 1];
  switch (p_col) {
  case 0:
    return (char *) profile.GetName();
  case 1:
    return (char *) profile.Creator();
  case 2:
    return (char *) ToText(profile.IsNash());
  case 3:
    return (char *) ToText(profile.IsPerfect());
  case 4:
    return (char *) ToText(profile.LiapValue(),
			   m_doc->GetPreferences().NumDecimals());
  default:
    if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
      return (char *) ToText((*profile.Profile())[p_col - 4],
			     m_doc->GetPreferences().NumDecimals());
    }
    else if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_VECTOR) {
      wxString ret = _T("("); 
      gbtNfgPlayer player = m_doc->GetNfg().GetPlayer(p_col - 4);
      for (int st = 1; st <= player.NumStrategies(); st++) {
	gbtNfgStrategy strategy = player.GetStrategy(st);
	if (st > 1) {
	  ret += _T(",");
	}
	ret += (char *) ToText(profile(strategy),
			       m_doc->GetPreferences().NumDecimals());
      }
      return ret + _T(")");
    }
    else if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_MYERSON) {
      wxString ret;
      gbtNfgPlayer player = m_doc->GetNfg().GetPlayer(p_col - 4);
      for (int st = 1; st <= player.NumStrategies(); st++) {
	gbtNfgStrategy strategy = player.GetStrategy(st);
	if (profile(strategy) > gNumber(0)) {
	  if (ret != "") {
	    ret += _T("+");
	  }
	  ret += (char *) ToText(profile(strategy),
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

bool gbtNfgProfileTable::InsertRows(size_t pos, size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
			 pos, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool gbtNfgProfileTable::AppendRows(size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool gbtNfgProfileTable::DeleteRows(size_t pos, size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
			 pos, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

wxGridCellAttr *
gbtNfgProfileTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind)
{
  wxGridCellAttr *attr = new wxGridCellAttr;

  if (m_doc->GetPreferences().ProfileStyle() == GBT_PROFILES_GRID) {
    int firstCol = 5;
    int lastCol = 4 + m_doc->GetNfg().GetPlayer(1).NumStrategies();
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
    if (col >= 5) {
      attr->SetTextColour(m_doc->GetPreferences().PlayerColor(col - 4));
    }
  }

  if (col >= 2) {
    attr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  }

  return attr;
}

//-------------------------------------------------------------------------
//                  class gbtNfgProfileGrid: Member functions
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtNfgProfileGrid, wxGrid)
  EVT_GRID_CELL_LEFT_CLICK(gbtNfgProfileGrid::OnLeftClick)
  EVT_GRID_CELL_RIGHT_CLICK(gbtNfgProfileGrid::OnRightClick)
  EVT_GRID_LABEL_RIGHT_CLICK(gbtNfgProfileGrid::OnRightClick)
END_EVENT_TABLE()

gbtNfgProfileGrid::gbtNfgProfileGrid(gbtGameDocument *p_doc, 
				     wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    gbtGameView(p_doc)
{
  SetTable(new gbtNfgProfileTable(m_doc), true);
  SetEditable(false);
  DisableDragRowSize();
  DisableDragColSize();
  SetLabelSize(wxVERTICAL, 0);
  SetSelectionMode(wxGrid::wxGridSelectRows);

  m_menu = new wxMenu("Profiles");
  m_menu->Append(GBT_NFG_MENU_PROFILES_NEW, "New profile", 
		 "Create a new profile");
  m_menu->Append(GBT_NFG_MENU_PROFILES_DUPLICATE, "Duplicate profile",
		 "Duplicate this profile");
  m_menu->Append(GBT_NFG_MENU_PROFILES_DELETE, "Delete profile", 
		 "Delete this profile");
  m_menu->Append(GBT_NFG_MENU_PROFILES_PROPERTIES, "Properties",
		 "View and edit properties of this profile");
  m_menu->Append(GBT_NFG_MENU_PROFILES_REPORT, "Report",
		 "Generate a report with information on profiles");
}

gbtNfgProfileGrid::~gbtNfgProfileGrid()
{ }

void gbtNfgProfileGrid::OnUpdate(gbtGameView *)
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
  for (int col = 5; col < GetCols(); col++) {
    if (GetColSize(col) > max) {
      max = GetColSize(col);
    }
  }
  for (int col = 5; col < GetCols(); col++) {
    SetColSize(col, max);
  }

  AdjustScrollbars();
  ForceRefresh();
  if (m_doc->IsProfileSelected()) {
    int row = m_doc->AllMixedProfiles().Find(m_doc->GetMixedProfile()) - 1;
    SelectRow(row);
    MakeCellVisible(row, 0);
  }
}

void gbtNfgProfileGrid::OnRightClick(wxGridEvent &p_event)
{
  m_menu->Enable(GBT_NFG_MENU_PROFILES_DUPLICATE,
		 m_doc->IsProfileSelected());
  m_menu->Enable(GBT_NFG_MENU_PROFILES_DELETE, 
		 m_doc->IsProfileSelected());
  m_menu->Enable(GBT_NFG_MENU_PROFILES_PROPERTIES,
		 m_doc->IsProfileSelected());
  m_menu->Enable(GBT_NFG_MENU_PROFILES_REPORT,
		 m_doc->IsProfileSelected());
  PopupMenu(m_menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void gbtNfgProfileGrid::OnLeftClick(wxGridEvent &p_event)
{
  m_doc->SetCurrentProfile(p_event.GetRow() + 1);
  // Veto this event; when grid refreshes, correct row will be selected
  p_event.Veto();
}

wxString gbtNfgProfileGrid::GetReport(void) const
{
  wxString report;
  const gList<MixedSolution> &profiles = m_doc->AllMixedProfiles();
  gbtNfgGame nfg = m_doc->GetNfg();

  report += wxString::Format("Mixed strategy profiles on game '%s' [%s]\n\n",
			     (const char *) nfg.GetTitle(),
			     m_doc->GetFilename().c_str());

  report += wxString::Format("Number of profiles: %d\n", profiles.Length());

  for (int i = 1; i <= profiles.Length(); i += 4) {
    report += "\n----------\n\n";

    report += "          ";
    for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
      report += wxString::Format("%-15s ", 
				 (const char *) profiles[i+j].GetName());
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
				 (const char *) profiles[i+j].Creator());
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
				 (const char *) ToText(profiles[i+j].LiapValue()));
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

