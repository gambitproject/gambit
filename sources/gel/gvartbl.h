//
// FILE: gvartbl.h -- Variable table for GEL
//
// $Id$
//

#ifndef GVARTBL_H
#define GVARTBL_H


#include "tristate.h"
#include "gnumber.h"
#include "gtext.h"
#include "glist.h"
#include "exprtree.h"

class gelVariableTable   
{
private:
  gList<gText> m_NumberNames;
  gList< gNestedList<gNumber> > m_NumberValues;
  
  gList<gText> m_BooleanNames;
  gList< gNestedList<gTriState> > m_BooleanValues;
  
  gList<gText> m_TextNames;
  gList< gNestedList<gText> > m_TextValues;

  gList<gText> m_EfgNames;
  gList<gNestedList<Efg *> > m_EfgValues;

  gList<gText> m_NodeNames;
  gList<gNestedList<Node *> > m_NodeValues;

  gList<gText> m_ActionNames;
  gList<gNestedList<Action *> > m_ActionValues;

  gList<gText> m_InfosetNames;
  gList<gNestedList<Infoset *> > m_InfosetValues;

  gList<gText> m_EFPlayerNames;
  gList<gNestedList<EFPlayer *> > m_EFPlayerValues;

  gList<gText> m_EFOutcomeNames;
  gList<gNestedList<EFOutcome *> > m_EFOutcomeValues;

  gList<gText> m_NfgNames;
  gList<gNestedList<Nfg *> > m_NfgValues;

  gList<gText> m_NFPlayerNames;
  gList<gNestedList<NFPlayer *> > m_NFPlayerValues;

  gList<gText> m_NFOutcomeNames;
  gList<gNestedList<NFOutcome *> > m_NFOutcomeValues;

  gList<gText> m_StrategyNames;
  gList<gNestedList<Strategy *> > m_StrategyValues;

public:
  gelVariableTable(void);

  bool IsDefined(const gText& name) const;
  void Define(const gText& name, gelType type);
  gelType Type(const gText& name) const;

  void Value(const gText &name, gNestedList<gNumber> &) const;
  void Value(const gText &name, gNestedList<gTriState> &) const;
  void Value(const gText &name, gNestedList<gText> &) const;
  void Value(const gText &name, gNestedList<Efg *> &) const;
  void Value(const gText &name, gNestedList<Node *> &) const;
  void Value(const gText &name, gNestedList<Action *> &) const;
  void Value(const gText &name, gNestedList<Infoset *> &) const;
  void Value(const gText &name, gNestedList<EFPlayer *> &) const;
  void Value(const gText &name, gNestedList<EFOutcome *> &) const;
  void Value(const gText &name, gNestedList<Nfg *> &) const;
  void Value(const gText &name, gNestedList<Strategy *> &) const;
  void Value(const gText &name, gNestedList<NFPlayer *> &) const;
  void Value(const gText &name, gNestedList<NFOutcome *> &) const;

  void SetValue(const gText &name, const gNestedList<gNumber> &value);
  void SetValue(const gText &name, const gNestedList<gTriState> &value);
  void SetValue(const gText &name, const gNestedList<gText> &value);
  void SetValue(const gText &name, const gNestedList<Efg *> &value);
  void SetValue(const gText &name, const gNestedList<Node *> &value);
  void SetValue(const gText &name, const gNestedList<Action *> &value);
  void SetValue(const gText &name, const gNestedList<Infoset *> &value);
  void SetValue(const gText &name, const gNestedList<EFPlayer *> &value);
  void SetValue(const gText &name, const gNestedList<EFOutcome *> &value);
  void SetValue(const gText &name, const gNestedList<Nfg *> &value);
  void SetValue(const gText &name, const gNestedList<Strategy *> &value);
  void SetValue(const gText &name, const gNestedList<NFPlayer *> &value);
  void SetValue(const gText &name, const gNestedList<NFOutcome *> &value);
};




#endif  // GVARTBL_H
