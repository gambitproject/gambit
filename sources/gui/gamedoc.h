//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of game document class
//

#ifndef GAMEDOC_H
#define GAMEDOC_H

#include "game/efg.h"
#include "nash/behavsol.h"
#include "game/nfg.h"

class EfgShow;
class NfgShow;

class gbtGameDocument {
public:
  gbtEfgGame *m_efg;
  EfgShow *m_efgShow;
  EFSupport *m_curEfgSupport;
  gList<EFSupport *> m_efgSupports;
  int m_curBehavProfile;
  gList<BehavSolution> m_behavProfiles;
  gbtEfgNode m_cursor, m_copyNode, m_cutNode;

  gbtNfgGame *m_nfg;
  NfgShow *m_nfgShow;

  gText m_fileName;

  gbtGameDocument(gbtEfgGame);
  gbtGameDocument(gbtNfgGame);
  ~gbtGameDocument();

  gbtEfgGame *GetEfg(void) const { return m_efg; }

  // MARKED NODES
  gbtEfgNode GetCursor(void) const { return m_cursor; }
  gbtEfgNode GetCopyNode(void) const { return m_copyNode; }
  gbtEfgNode GetCutNode(void) const { return m_cutNode; }
  
  // OUTCOMES
  gText UniqueEfgOutcomeName(void) const;
 
  // SUPPORTS
  gText UniqueEfgSupportName(void) const;
  void AddSupport(EFSupport *);
};

#endif  // GAMEDOC_H
