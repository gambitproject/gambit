//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for setting legends for extensive form display
//

#ifndef DLEFGLEGEND_H
#define DLEFGLEGEND_H

#include "treedraw.h"

class dialogLegend : public wxDialog {
private:
  wxRadioBox *m_nodeAbove, *m_nodeBelow, *m_nodeAfter;
  wxRadioBox *m_branchAbove, *m_branchBelow;

public:
  // Lifecycle
  dialogLegend(wxWindow *, const TreeDrawSettings &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  int GetNodeAbove(void) const { return m_nodeAbove->GetSelection(); }
  int GetNodeBelow(void) const { return m_nodeBelow->GetSelection(); }
  int GetNodeAfter(void) const { return m_nodeAfter->GetSelection(); }

  int GetBranchAbove(void) const { return m_branchAbove->GetSelection(); }
  int GetBranchBelow(void) const { return m_branchBelow->GetSelection(); }
};

#endif // DLEFGLEGEND_H
