//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of class to print out normal forms
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
#include "game/nfg.h"
#include "game/nfgciter.h"
#include "nfgprint.h"

wxString gbtBuildHtml(const Nfg &p_nfg, int p_rowPlayer, int p_colPlayer)
{
  wxString theHtml;
  gbtNfgSupport support(p_nfg);
  NfgContIter iter(support);
  iter.Freeze(p_rowPlayer);
  iter.Freeze(p_colPlayer);
  iter.First();

  theHtml += wxString::Format("<center><h1>%s<h1></center>\n",
			      (char *) p_nfg.GetTitle());

  do {
    gArray<int> cont(iter.Get());

    if (p_nfg.NumPlayers() > 2) {
      theHtml += "<center><b>Subtable with strategies:</b></center>";
      for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
	if (pl == p_rowPlayer || pl == p_colPlayer) {
	  continue;
	}

	theHtml += 
	  wxString::Format("<center><b>Player %d: Strategy %d</b></center>",
			   pl, cont[pl]);
      }
    }
    
    theHtml += "<table>";
    theHtml += "<tr>";
    theHtml += "<td></td>";
    for (int st = 1; st <= p_nfg.NumStrats(p_colPlayer); st++) {
      theHtml += wxString::Format("<td align=center><b>%s</b></td>",
				  (char *) p_nfg.GetPlayer(p_colPlayer).GetStrategy(st).GetLabel());
    } 
    theHtml += "</tr>";
    for (int st1 = 1; st1 <= p_nfg.NumStrats(p_rowPlayer); st1++) {
      cont[p_rowPlayer] = st1;
      theHtml += "<tr>";
      theHtml += wxString::Format("<td align=center><b>%s</b></td>",
				  (char *) p_nfg.GetPlayer(p_rowPlayer).GetStrategy(st1).GetLabel());
      for (int st2 = 1; st2 <= p_nfg.NumStrats(p_colPlayer); st2++) {
	cont[p_colPlayer] = st2;
	theHtml += "<td align=center>";
	for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
	  theHtml += wxString::Format("%s", (char *) ToText(p_nfg.Payoff(p_nfg.GetOutcome(cont), pl)));
	  if (pl < p_nfg.NumPlayers()) {
	    theHtml += ",";
	  }
	}
	theHtml += "</td>";
      }
      theHtml += "</tr>";
    } 

    theHtml += "</table>";
  } while (iter.NextContingency());

  return theHtml;
}

NfgPrintout::NfgPrintout(const Nfg &p_nfg,
			 int p_rowPlayer, int p_colPlayer,
			 const wxString &p_title)
  : wxHtmlPrintout(p_title)
{
  SetHtmlText(gbtBuildHtml(p_nfg, p_rowPlayer, p_colPlayer));
}
