//
// FILE: funcmath.cc -- Arithmetic functions for GEL
//
// $Id$
//

#include "gmisc.h"
#include "exprtree.h"
#include "funcmisc.h"
#include "glist.h"
#include "tristate.h"

#include "efg.h"

//------------
// LoadEfg
//------------

DECLARE_UNARY(gelfuncLoadEfg, gText, Efg *)

Efg *gelfuncLoadEfg::EvalItem(gText filename) const
{
  gFileInput f(filename);
  Efg *E = 0;
  ReadEfgFile(f, E);
  return E;
}

//------------
// RootNode
//------------

DECLARE_UNARY(gelfuncRootNode, Efg *, Node *)

Node *gelfuncRootNode::EvalItem(Efg *E) const
{
  return E->RootNode();
}

//------------
// SaveEfg
//------------

DECLARE_BINARY(gelfuncSaveEfg, Efg *, gText, Efg *)

Efg *gelfuncSaveEfg::EvalItem(Efg *E, gText filename) const
{
  gFileOutput f(filename);
  E->WriteEfgFile(f);
  return E;
}


gelExpr *GEL_LoadEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLoadEfg((gelExpression<gText> *) params[1]);
}

gelExpr *GEL_RootNode(const gArray<gelExpr *> &params)
{
  return new gelfuncRootNode((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_SaveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSaveEfg((gelExpression<Efg *> *) params[1],
                            (gelExpression<gText> *) params[2]);
}

#include "match.h"

void gelEfgInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_LoadEfg, "LoadEfg[file->TEXT] =: EFG" },
    { GEL_RootNode, "RootNode[efg->EFG] =: NODE" },
    { GEL_SaveEfg, "SaveEfg[efg->EFG, file->TEXT] =: EFG" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)
    env->Register(sigarray[i].func, sigarray[i].sig);
}












