//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of extensive form navigation window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgnavigate.h"

EfgNavigateWindow::EfgNavigateWindow(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_efgShow)
{
  CreateGrid(10, 1);
  SetEditable(false);
  SetDefaultCellAlignment(wxCENTER, wxCENTER);

  SetLabelValue(wxVERTICAL, "Node label", 0);
  SetLabelValue(wxVERTICAL, "Pr(node reached)", 1);
  SetLabelValue(wxVERTICAL, "Node value", 2);
  SetLabelValue(wxVERTICAL, "Information set", 3);
  SetLabelValue(wxVERTICAL, "Pr(infoset reached)", 4);
  SetLabelValue(wxVERTICAL, "Belief", 5);
  SetLabelValue(wxVERTICAL, "Information set value", 6);
  SetLabelValue(wxVERTICAL, "Incoming action label", 7);
  SetLabelValue(wxVERTICAL, "Pr(incoming action)", 8);
  SetLabelValue(wxVERTICAL, "Incoming action value", 9);

  SetLabelSize(wxHORIZONTAL, 0);
  SetLabelSize(wxVERTICAL, 150);
  AdjustScrollbars();
  Show(true);
}

void EfgNavigateWindow::Set(const Node *p_cursor) 
{
  m_cursor = p_cursor;
  
  if (!m_cursor) { // no data available
    for (int i = 0; i < GetRows(); i++) { 
      SetCellValue("N/A", i, 0);
    }
    return;
  }

  // if we got here, the node is valid.
  try {
    SetCellValue((char *) m_cursor->GetName(), 0, 0);
    SetCellValue((char *) m_parent->AsString(tRealizProb, m_cursor), 1, 0);
    SetCellValue((char *) m_parent->AsString(tNodeValue, m_cursor), 2, 0);

    gText tmpstr;
  
    if (!m_cursor->GetPlayer()) {
      tmpstr = "TERMINAL";
    }
    else {
      if (m_cursor->GetPlayer()->IsChance())
	tmpstr = "CHANCE";
      else
	tmpstr = ("(" + ToText(m_cursor->GetPlayer()->GetNumber()) + "," +
		  ToText(m_cursor->GetInfoset()->GetNumber()) + ")");
    }
	  
    SetCellValue((char *) tmpstr, 3, 0);
    SetCellValue((char *) m_parent->AsString(tIsetProb, m_cursor), 4, 0);

    SetCellValue((char *) m_parent->AsString(tBeliefProb, m_cursor), 5, 0);
    SetCellValue((char *) m_parent->AsString(tIsetValue, m_cursor), 6, 0);
	
    Node *p = m_cursor->GetParent();

    if (p) {
      SetCellValue((char *) m_cursor->GetAction()->GetName(), 7, 0);
    }
    else {
      SetCellValue("N/A (root)", 7, 0);
    }

    if (p) {
      int branch = 0;
      for (branch = 1; p->GetChild(branch) != m_cursor; branch++);
      SetCellValue((char *) m_parent->AsString(tBranchProb, p, branch), 8, 0);
    }
    else {
      SetCellValue("1", 8, 0);
    }
	
    if (p) {
      int branch = 0;
      for (branch = 1; p->GetChild(branch) != m_cursor; branch++);
      SetCellValue((char *) m_parent->AsString(tBranchVal, p, branch), 9, 0);
    }
    else {
      SetCellValue("N/A", 9, 0);
    }
  }	
  catch (gException &) { }
}

