//
// FILE: match.h -- Function matching structures and functions
//
// $Id$
//

#ifndef MATCH_H
#define MATCH_H

#include "exprtree.h"
#include "gnumber.h"
#include "glist.h"
#include "funcmisc.h"
#include "gvartbl.h"

class gelEnvironment   {
  private:
// Implementation of function table
    gList<gelSignature *> signatures;
    gList<gelAdapter *> functions;

// Implementation of variables
    gelVariableTable variables;

  public:
    gelEnvironment(void);
    ~gelEnvironment();

    void Register(gelAdapter *, const gText &);
    gelExpr *Match(const gText &, gelExpr *);
    gelExpr *Match(const gText &, gelExpr *, gelExpr *);
    gelExpr *Match(const gText &, const gArray<gelExpr *> &);
};



#endif    // MATCH_H
