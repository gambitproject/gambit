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
//                  class NfgProfileList: Member functions
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(NfgProfileList, wxListCtrl)
  EVT_RIGHT_DOWN(NfgProfileList::OnRightClick)
END_EVENT_TABLE()

NfgProfileList::NfgProfileList(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxListCtrl(p_parent, idNFG_SOLUTION_LIST, wxDefaultPosition,
	       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL),
    gbtGameView(p_doc)
{
  m_menu = new wxMenu("Profiles");
  m_menu->Append(GBT_NFG_MENU_PROFILES_NEW, "New profile", "Create a new profile");
  m_menu->Append(GBT_NFG_MENU_PROFILES_DUPLICATE, "Duplicate profile",
		 "Duplicate this profile");
  m_menu->Append(GBT_NFG_MENU_PROFILES_DELETE, "Delete profile", "Delete this profile");
  m_menu->Append(GBT_NFG_MENU_PROFILES_PROPERTIES, "Properties",
		 "View and edit properties of this profile");
  m_menu->Append(GBT_NFG_MENU_PROFILES_REPORT, "Report",
		 "Generate a report with information on profiles");
}

NfgProfileList::~NfgProfileList()
{ }

void NfgProfileList::OnUpdate(gbtGameView *)
{
  ClearAll();
  InsertColumn(0, "Name");
  InsertColumn(1, "Creator");
  InsertColumn(2, "Nash");
  InsertColumn(3, "Perfect");
  InsertColumn(4, "Liap Value");
  InsertColumn(5, "Qre Lambda");
  
  gbtNfgGame nfg = m_doc->GetNfg();
  int maxColumn = 5;

  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    gbtNfgPlayer player = nfg.GetPlayer(pl);
    for (int st = 1; st <= player.NumStrategies(); st++) {
      InsertColumn(++maxColumn,
		   wxString::Format("%d:%d", pl, st));
    }
  }

  for (int i = 1; i <= m_doc->AllMixedProfiles().Length(); i++) {
    const MixedSolution &profile = m_doc->AllMixedProfiles()[i];
    InsertItem(i - 1, (char *) profile.GetName());
    SetItem(i - 1, 1, (char *) profile.Creator());
    SetItem(i - 1, 2, (char *) ToText(profile.IsNash()));
    SetItem(i - 1, 3, (char *) ToText(profile.IsPerfect()));
    SetItem(i - 1, 4, (char *) ToText(profile.LiapValue()));
    if (profile.Creator() == "Qre[NFG]") {
      SetItem(i - 1, 5, (char *) ToText(profile.QreLambda()));
    }
    else {
      SetItem(i - 1, 5, "--");
    }

    int column = 5;
    for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
      gbtNfgPlayer player = nfg.GetPlayer(pl);
      for (int st = 1; st <= player.NumStrategies(); st++) {
	SetItem(i - 1, ++column,
		(char *) ToText(profile(player.GetStrategy(st))));
      }
    }    
  }

  if (m_doc->IsProfileSelected()) {
    wxListItem item;
    item.m_mask = wxLIST_MASK_STATE;
    item.m_itemId = m_doc->AllMixedProfiles().Find(m_doc->GetMixedProfile()) - 1;
    item.m_state = wxLIST_STATE_SELECTED;
    item.m_stateMask = wxLIST_STATE_SELECTED;
    SetItem(item);
  }
}

void NfgProfileList::OnRightClick(wxMouseEvent &p_event)
{
  m_menu->Enable(GBT_NFG_MENU_PROFILES_DUPLICATE,
		 m_doc->IsProfileSelected());
  m_menu->Enable(GBT_NFG_MENU_PROFILES_DELETE, 
		 m_doc->IsProfileSelected());
  m_menu->Enable(GBT_NFG_MENU_PROFILES_PROPERTIES,
		 m_doc->IsProfileSelected());
  m_menu->Enable(GBT_NFG_MENU_PROFILES_REPORT,
		 m_doc->IsProfileSelected());
  PopupMenu(m_menu, p_event.m_x, p_event.m_y);
}

wxString NfgProfileList::GetReport(void) const
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

