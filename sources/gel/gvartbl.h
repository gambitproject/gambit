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

class gelVariableTable   {
  private:
    gList<gText> m_NumberNames;
    gList<gNumber> m_NumberValues;

    gList<gText> m_BooleanNames;
    gList<gTriState> m_BooleanValues;
    
    gList<gText> m_TextNames;
    gList<gText> m_TextValues;

  public:
    gelVariableTable(void);

    bool IsDefined(const gText &name) const;
    void Define(const gText &name, gelType type);
    gelType Type(const gText &name) const;

    void Value(const gText &name, gNumber &) const;
    void Value(const gText &name, gTriState &) const;
    void Value(const gText &name, gText &) const;
    void SetValue(const gText &name, const gNumber &value);
    void SetValue(const gText &name, const gTriState &value);
    void SetValue(const gText &name, const gText &value);
};




#endif  // GVARTBL_H
