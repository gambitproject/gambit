//
// FILE: efgcursor.cc -- Extensive form cursor property window
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgcursor.h"


const int EfgCursorWindow::s_numFeatures = 8;
static const int s_numFeatures = 8;
char *EfgCursorWindow::s_featureNames[9] = 
{ "BlankEntry", "Infoset", "RealizProb", "IsetProb", "BeliefProb", "NodeValue",
  "IsetValue", "BranchProb", "BranchValue" };

EfgCursorWindow::EfgCursorWindow(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_efgShow)
{
  Show(false);
  CreateGrid(s_numFeatures, 1);
  SetEditable(false);

  for (int i = 1; i <= s_numFeatures; i++) {
    SetLabelValue(wxVERTICAL, s_featureNames[i], i - 1);
  }

  SetLabelSize(wxHORIZONTAL, 0);
  SetLabelSize(wxVERTICAL, 100);
  AdjustScrollbars();
}

void EfgCursorWindow::Set(const Node *p_cursor) 
{
  m_cursor = p_cursor;
  
  if (!m_cursor) { // no data available
    for (int i = 1; i <= s_numFeatures; i++) { 
      SetCellValue("N/A", i - 1, 0);
    }
    return;
  }

  // if we got here, the node is valid.
  try {
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
	  
    SetCellValue((char *) tmpstr, 0, 0);

    SetCellValue((char *) m_parent->AsString(tRealizProb, m_cursor), 1, 0);
    SetCellValue((char *) m_parent->AsString(tIsetProb, m_cursor), 2, 0);
    SetCellValue((char *) m_parent->AsString(tBeliefProb, m_cursor), 3, 0);
    SetCellValue((char *) m_parent->AsString(tNodeValue, m_cursor), 4, 0);
    SetCellValue((char *) m_parent->AsString(tIsetValue, m_cursor), 5, 0);
	
    Node *p = m_cursor->GetParent();
    if (p) {
      int branch = 0;
      for (branch = 1; p->GetChild(branch) != m_cursor; branch++);
      SetCellValue((char *) m_parent->AsString(tBranchProb, p, branch), 6, 0);
    }
    else {
      SetCellValue("1", 6, 0);
    }
	
    if (p) {
      int branch = 0;
      for (branch = 1; p->GetChild(branch) != m_cursor; branch++);
      SetCellValue((char *) m_parent->AsString(tBranchVal, p, branch), 7, 0);
    }
    else {
      SetCellValue("N/A", 7, 0);
    }
  }	
  catch (gException &) { }
}

