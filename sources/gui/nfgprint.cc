//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of class to print out normal forms
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "game/nfg.h"
#include "game/nfgciter.h"
#include "nfgprint.h"

wxString NfgPrintout::BuildHtml(const Nfg &p_nfg,
				int p_rowPlayer, int p_colPlayer)
{
  wxString theHtml;
  NFSupport support(p_nfg);
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
				  (char *) p_nfg.Strategies(p_colPlayer)[st]->Name());
    } 
    theHtml += "</tr>";
    for (int st1 = 1; st1 <= p_nfg.NumStrats(p_rowPlayer); st1++) {
      cont[p_rowPlayer] = st1;
      theHtml += "<tr>";
      theHtml += wxString::Format("<td align=center><b>%s</b></td>",
				  (char *) p_nfg.Strategies(p_rowPlayer)[st1]->Name());
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
  SetHtmlText(BuildHtml(p_nfg, p_rowPlayer, p_colPlayer));
}
