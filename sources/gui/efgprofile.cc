//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Extensive form behavior profile window
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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
#include "efgprofile.h"

//-------------------------------------------------------------------------
//                  class EfgProfileList: Member functions
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(EfgProfileList, wxListCtrl)
  EVT_RIGHT_DOWN(EfgProfileList::OnRightClick)
END_EVENT_TABLE()

EfgProfileList::EfgProfileList(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxListCtrl(p_parent, idEFG_SOLUTION_LIST, wxDefaultPosition,
	       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL),
    m_parent(p_efgShow)
{
  m_menu = new wxMenu("Profiles");
  m_menu->Append(GBT_EFG_MENU_PROFILES_NEW, "New profile", "Create a new profile");
  m_menu->Append(GBT_EFG_MENU_PROFILES_DUPLICATE, "Duplicate profile",
		 "Duplicate this profile");
  m_menu->Append(GBT_EFG_MENU_PROFILES_DELETE, "Delete profile", 
		 "Delete this profile");
  m_menu->Append(GBT_EFG_MENU_PROFILES_PROPERTIES, "Properties",
		 "View and edit properties of this profile");
  m_menu->Append(GBT_EFG_MENU_PROFILES_REPORT, "Report",
		 "Generate a report with information on profiles");
  UpdateValues();
}

EfgProfileList::~EfgProfileList()
{ }

void EfgProfileList::UpdateValues(void)
{
  ClearAll();
  InsertColumn(0, "Name");
  InsertColumn(1, "Creator");
  InsertColumn(2, "Nash");
  InsertColumn(3, "Perfect");
  InsertColumn(4, "Sequential");
  InsertColumn(5, "Liap Value");
  InsertColumn(6, "Qre Lambda");

  const efgGame &efg = *m_parent->Game();
  int maxColumn = 6;

  for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
    gbtEfgPlayer player = efg.GetPlayer(pl);
    for (int iset = 1; iset <= player.NumInfosets(); iset++) {
      Infoset *infoset = player.GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	InsertColumn(++maxColumn,
		     wxString::Format("(%d,%d):%d", pl, iset, act));
      }
    }
  }

  for (int i = 1; i <= m_parent->Profiles().Length(); i++) {
    const BehavSolution &profile = m_parent->Profiles()[i];
    InsertItem(i - 1, (char *) profile.GetName());
    SetItem(i - 1, 1, (char *) profile.Creator());
    SetItem(i - 1, 2, (char *) ToText(profile.IsNash()));
    SetItem(i - 1, 3, (char *) ToText(profile.IsSubgamePerfect()));
    SetItem(i - 1, 4, (char *) ToText(profile.IsSequential()));
    SetItem(i - 1, 5, (char *) ToText(profile.LiapValue()));
    if (profile.Creator() == "Qre[EFG]" || profile.Creator() == "Qre[NFG]") {
      SetItem(i - 1, 6, (char *) ToText(profile.QreLambda()));
    }
    else {
      SetItem(i - 1, 6, "--");
    }

    int column = 6;
    for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
      gbtEfgPlayer player = efg.GetPlayer(pl);
      for (int iset = 1; iset <= player.NumInfosets(); iset++) {
	Infoset *infoset = player.GetInfoset(iset);
	for (int act = 1; act <= infoset->NumActions(); act++) {
	  SetItem(i - 1, ++column,
		  (char *) ToText(profile(infoset->GetAction(act))));
	}
      }
    }
  }    

  if (m_parent->Profiles().Length() > 0) {
    wxListItem item;
    item.m_mask = wxLIST_MASK_STATE;
    item.m_itemId = m_parent->CurrentProfile() - 1;
    item.m_state = wxLIST_STATE_SELECTED;
    item.m_stateMask = wxLIST_STATE_SELECTED;
    SetItem(item);
  }
}

void EfgProfileList::OnRightClick(wxMouseEvent &p_event)
{
  m_menu->Enable(GBT_EFG_MENU_PROFILES_DUPLICATE, m_parent->CurrentProfile() > 0);
  m_menu->Enable(GBT_EFG_MENU_PROFILES_DELETE, m_parent->CurrentProfile() > 0);
  m_menu->Enable(GBT_EFG_MENU_PROFILES_PROPERTIES, m_parent->CurrentProfile() > 0);
  m_menu->Enable(GBT_EFG_MENU_PROFILES_REPORT, m_parent->CurrentProfile() > 0);
  PopupMenu(m_menu, p_event.m_x, p_event.m_y);
}

wxString EfgProfileList::GetReport(void) const
{
  wxString report;
  const gList<BehavSolution> &profiles = m_parent->Profiles();
  const efgGame &efg = *m_parent->Game();

  report += wxString::Format("Behavior strategy profiles on game '%s' [%s]\n\n",
			     (const char *) efg.GetTitle(),
			     m_parent->Filename().c_str());

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
				 (const char *) ToText(profiles[i+j].IsSubgamePerfect()));
    }
    report += "\n";

    report += "Liap:     ";
    for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
      report += wxString::Format("%-15s ",
				 (const char *) ToText(profiles[i+j].LiapValue()));
    }
    report += "\n\n";

    for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
      gbtEfgPlayer player = efg.GetPlayer(pl);
      report += wxString::Format("%s\n", (const char *) player.GetLabel());

      for (int iset = 1; iset <= player.NumInfosets(); iset++) {
	Infoset *infoset = player.GetInfoset(iset);

	report += wxString::Format("%s\n", (const char *) infoset->GetName());

	for (int act = 1; act <= infoset->NumActions(); act++) {
	  report += wxString::Format("%2d: %-6s", act,
				     (const char *) infoset->GetAction(act)->GetName());

	  for (int j = 0; j < 4 && i + j <= profiles.Length(); j++) {
	    report += wxString::Format("%-15s ", 
				       (const char *) ToText((*profiles[i+j].Profile())(pl, iset, act)));
	  }
	  report += "\n";

	}
	report += "\n";
      }

      report += "\n";
    }
  }

  return report;
}

