//
// FILE: dllegends.h -- Extensive form legend selection dialog
//
// $Id$
//

#ifndef DLLEGENDS_H
#define DLLEGENDS_H

#include "treedraw.h"

class dialogLegends : public guiAutoDialog {
private:
  wxChoice *m_nodeAbove, *m_nodeBelow, *m_nodeAfter;
  wxChoice *m_branchAbove, *m_branchBelow;

  const char *HelpString(void) const { return "Legends Dialog"; }

public:
  dialogLegends(wxWindow *, const TreeDrawSettings &);
  virtual ~dialogLegends() { }

  int GetNodeAbove(void) const { return m_nodeAbove->GetSelection(); }
  int GetNodeBelow(void) const { return m_nodeBelow->GetSelection(); }
  int GetNodeAfter(void) const { return m_nodeAfter->GetSelection(); }

  int GetBranchAbove(void) const { return m_branchAbove->GetSelection(); }
  int GetBranchBelow(void) const { return m_branchBelow->GetSelection(); }
};

#endif // DLLEGENDS_H
