//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of game document class
//

#include "base/base.h"
#include "gamedoc.h"
#include "game/efg.h"
#include "game/efstrat.h"
#include "game/nfg.h"

gbtGameDocument::gbtGameDocument(gbtEfgGame p_efg)
  : m_efg(new gbtEfgGame(p_efg)), m_efgShow(0),
    m_curEfgSupport(0), m_curBehavProfile(0),
    m_cursor(0), m_copyNode(0), m_cutNode(0),
    m_nfg(0), m_nfgShow(0)
{ }

gbtGameDocument::gbtGameDocument(gbtNfgGame p_nfg)
  : m_efg(0), m_efgShow(0),
    m_curEfgSupport(0), m_curBehavProfile(0),
    m_cursor(0), m_copyNode(0), m_cutNode(0),
    m_nfg(new gbtNfgGame(p_nfg)), m_nfgShow(0)
{ }

gbtGameDocument::~gbtGameDocument()
{
  if (m_efg) {
    for (int i = 1; i <= m_efgSupports.Length(); delete m_efgSupports[i++]);
    delete m_efg;
  }
  if (m_nfg) {
    delete m_nfg;
  }
}


//==========================================================================
//                 gbtGameDocument: Operations on outcomes
//==========================================================================

gText gbtGameDocument::UniqueEfgOutcomeName(void) const
{
  int number = m_efg->NumOutcomes() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_efg->NumOutcomes(); i++) {
      if (m_efg->GetOutcome(i).GetLabel() == "Outcome" + ToText(number)) {
	break;
      }
    }

    if (i > m_efg->NumOutcomes()) {
      return "Outcome" + ToText(number);
    }
    
    number++;
  }
}

//==========================================================================
//                 gbtGameDocument: Operations on supports
//==========================================================================

gText gbtGameDocument::UniqueEfgSupportName(void) const
{
  int number = m_efgSupports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_efgSupports.Length(); i++) {
      if (m_efgSupports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_efgSupports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

void gbtGameDocument::AddSupport(EFSupport *p_support)
{
  m_efgSupports.Append(p_support);
}
