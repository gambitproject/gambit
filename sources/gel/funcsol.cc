//
// FILE: funcsol.cc -- Efg functions for GEL
//
// $Id$
//

#include "gmisc.h"
#include "exprtree.h"
#include "funcmisc.h"
#include "glist.h"
#include "tristate.h"

#include "efg.h"
#include "behavsol.h"
#include "efgutils.h"
#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "mixedsol.h"

//-------------
// ActionProb
//-------------

DECLARE_BINARY(gelfuncActionProb, BehavSolution *, Action *, gNumber *)

gNumber *gelfuncActionProb::EvalItem(BehavSolution *b, Action *a) const
{
  if (!b || !a)  return 0;
  if (&b->Game() != a->BelongsTo()->Game())
    throw gelGameMismatchError("ActionProb");
  if (a->BelongsTo()->GetPlayer()->IsChance())
    return new gNumber(b->Game().GetChanceProb(a->BelongsTo(),
                                               a->GetNumber()));
  else if (b->Support().Find(a))
    return new gNumber((*b)(a->BelongsTo()->GetPlayer()->GetNumber(),
                            a->BelongsTo()->GetNumber(),
                            b->Support().Find(a)));
  else
    return new gNumber(0);
}

//------------------
// ActionValue
//------------------

DECLARE_BINARY(gelfuncActionValue, BehavSolution *, Action *, gNumber *)

gNumber *gelfuncActionValue::EvalItem(BehavSolution *b, Action *a) const
{
  if (!b || !a)   return 0;
  if (&b->Game() != a->BelongsTo()->Game())
    throw gelGameMismatchError("ActionValue");
  if (a->BelongsTo()->GetPlayer()->IsChance())
    return 0;
  else if (b->Support().Find(a))  {
    Efg *efg = &b->Game();

    gDPVector<gNumber> values(efg->NumActions());
    gPVector<gNumber> probs(efg->NumInfosets());

    b->CondPayoff(values, probs);
  
    if (probs(a->BelongsTo()->GetPlayer()->GetNumber(),
	            a->BelongsTo()->GetNumber()) > gNumber(0.0))
      return new gNumber(values(a->BelongsTo()->GetPlayer()->GetNumber(),
				a->BelongsTo()->GetNumber(),
				b->Support().Find(a)));
    else
      return 0;
  }
  else
    return 0;
}

//---------
// Behav
//---------

DECLARE_UNARY(gelfuncBehav, EFSupport *, BehavSolution *)

BehavSolution *gelfuncBehav::EvalItem(EFSupport *S) const
{
  return (S) ? new BehavSolution(BehavProfile<gNumber>(*S)) : 0;
}

//----------
// Belief
//----------

DECLARE_BINARY(gelfuncBelief, BehavSolution *, Node *, gNumber *)

gNumber *gelfuncBelief::EvalItem(BehavSolution *b, Node *n) const
{
  if (!b || !n)  return 0;
  if (&b->Game() != n->Game())
    throw gelGameMismatchError("Belief");
  const gDPVector<gNumber> &values(b->Beliefs());
  Infoset *s = n->GetInfoset();
  const gArray<Node *> &members = s->Members();

  if (s->IsChanceInfoset() || n->NumChildren() == 0)
    return 0;

  int index;
  for (index = 1; members[index] != n; index++);
  return new gNumber(values(s->GetPlayer()->GetNumber(),
				                    s->GetNumber(), index));
}

//--------
// Game
//--------

DECLARE_UNARY(gelfuncGameBehav, BehavSolution *, Efg *)

Efg *gelfuncGameBehav::EvalItem(BehavSolution *b) const
{
  return (b) ? &b->Game() : 0;
}

DECLARE_UNARY(gelfuncGameEFSupport, EFSupport *, Efg *)

Efg *gelfuncGameEFSupport::EvalItem(EFSupport *s) const
{
  return (s) ? (Efg *) &s->Game() : 0;
}

DECLARE_UNARY(gelfuncGameMixed, MixedSolution *, Nfg *)

Nfg *gelfuncGameMixed::EvalItem(MixedSolution *m) const
{
  return (m) ? &m->Game() : 0;
}

DECLARE_UNARY(gelfuncGameNFSupport, NFSupport *, Nfg *)

Nfg *gelfuncGameNFSupport::EvalItem(NFSupport *s) const
{
  return (s) ? (Nfg *) &s->Game() : 0;
}

//---------------
// GobitLambda
//---------------

DECLARE_UNARY(gelfuncGobitLambdaBehav, BehavSolution *, gNumber *)

gNumber *gelfuncGobitLambdaBehav::EvalItem(BehavSolution *b) const
{
  if (!b || b->Creator() != EfgAlg_GOBIT)
    return 0;
  else
    return new gNumber(b->GobitLambda());
}

DECLARE_UNARY(gelfuncGobitLambdaMixed, MixedSolution *, gNumber *)

gNumber *gelfuncGobitLambdaMixed::EvalItem(MixedSolution *m) const
{
  if (!m || m->Creator() != NfgAlg_GOBIT)
    return 0;
  else
    return new gNumber(m->GobitLambda());
}

//--------------
// GobitValue
//--------------

DECLARE_UNARY(gelfuncGobitValueBehav, BehavSolution *, gNumber *)

gNumber *gelfuncGobitValueBehav::EvalItem(BehavSolution *b) const
{
  if (!b || b->Creator() != EfgAlg_GOBIT)
    return 0;
  else
    return new gNumber(b->GobitValue());
}

DECLARE_UNARY(gelfuncGobitValueMixed, MixedSolution *, gNumber *)

gNumber *gelfuncGobitValueMixed::EvalItem(MixedSolution *m) const
{
  if (!m || m->Creator() != NfgAlg_GOBIT)
    return 0;
  else
    return new gNumber(m->GobitValue());
}

//----------------
// InfosetProb
//----------------

DECLARE_BINARY(gelfuncInfosetProb, BehavSolution *, Infoset *, gNumber *)

gNumber *gelfuncInfosetProb::EvalItem(BehavSolution *b, Infoset *s) const
{
  if (!b || !s)   return 0;
  if (&b->Game() != s->Game())
    throw gelGameMismatchError("InfosetProb");
  if (s->IsChanceInfoset())   return 0;

  Efg *E = &b->Game();

  gDPVector<gNumber> values(E->NumActions());
  gPVector<gNumber> probs(E->NumInfosets());

  b->CondPayoff(values, probs);

  return new gNumber(probs(s->GetPlayer()->GetNumber(),
			   s->GetNumber()));
}

//---------
// IsNash
//---------

DECLARE_UNARY(gelfuncIsNashBehav, BehavSolution *, gTriState *)

gTriState *gelfuncIsNashBehav::EvalItem(BehavSolution *b) const
{
  if (!b)  return 0;
  switch (b->IsNash())   {
    case T_YES:  return new gTriState(triTRUE);
    case T_NO:   return new gTriState(triFALSE);
    default:     return new gTriState(triMAYBE);
  }
}

DECLARE_UNARY(gelfuncIsNashMixed, MixedSolution *, gTriState *)

gTriState *gelfuncIsNashMixed::EvalItem(MixedSolution *m) const
{
  if (!m)  return 0;
  switch (m->IsNash())   {
    case T_YES:  return new gTriState(triTRUE);
    case T_NO:   return new gTriState(triFALSE);
    default:     return new gTriState(triMAYBE);
  }
}

//-------------
// IsPerfect
//-------------

DECLARE_UNARY(gelfuncIsPerfect, MixedSolution *, gTriState *)

gTriState *gelfuncIsPerfect::EvalItem(MixedSolution *m) const
{
  if (!m)  return 0;
  switch (m->IsPerfect())   {
    case T_YES:  return new gTriState(triTRUE);
    case T_NO:   return new gTriState(triFALSE);
    default:     return new gTriState(triMAYBE);
  }
}

//----------------
// IsSequential
//----------------

DECLARE_UNARY(gelfuncIsSequential, BehavSolution *, gTriState *)

gTriState *gelfuncIsSequential::EvalItem(BehavSolution *b) const
{
  if (!b)  return 0;
  switch (b->IsSequential())   {
    case T_YES:  return new gTriState(triTRUE);
    case T_NO:   return new gTriState(triFALSE);
    default:     return new gTriState(triMAYBE);
  }
}

//--------------------
// IsSubgamePerfect
//--------------------

DECLARE_UNARY(gelfuncIsSubgamePerfect, BehavSolution *, gTriState *)

gTriState *gelfuncIsSubgamePerfect::EvalItem(BehavSolution *b) const
{
  if (!b)  return 0;
  switch (b->IsSubgamePerfect())   {
    case T_YES:  return new gTriState(triTRUE);
    case T_NO:   return new gTriState(triFALSE);
    default:     return new gTriState(triMAYBE);
  }
}

//-------------
// LiapValue
//-------------

DECLARE_UNARY(gelfuncLiapValueBehav, BehavSolution *, gNumber *)

gNumber *gelfuncLiapValueBehav::EvalItem(BehavSolution *b) const
{
  if (!b)  return 0;
  return new gNumber(b->LiapValue());
}

DECLARE_UNARY(gelfuncLiapValueMixed, MixedSolution *, gNumber *)

gNumber *gelfuncLiapValueMixed::EvalItem(MixedSolution *m) const
{
  if (!m)  return 0;
  return new gNumber(m->LiapValue());
}

//---------
// Mixed
//---------

DECLARE_UNARY(gelfuncMixed, NFSupport *, MixedSolution *)

MixedSolution *gelfuncMixed::EvalItem(NFSupport *S) const
{
  return (S) ? new MixedSolution(MixedProfile<gNumber>(*S)) : 0;
}

//--------------
// RealizProb
//--------------

DECLARE_BINARY(gelfuncRealizProb, BehavSolution *, Node *, gNumber *)

gNumber *gelfuncRealizProb::EvalItem(BehavSolution *b, Node *n) const
{
  if (!b || !n)  return 0;
  if (&b->Game() != n->Game())
    throw gelGameMismatchError("RealizProb");

  // don't nodes have numbers now? -- check on this
  Efg* E = &b->Game();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  for (i = 1; i <= list.Length(); i++)
    if (n == list[i])  break;

  return new gNumber(b->NodeRealizProbs()[i]);
}  

//-----------
// Regret
//-----------

DECLARE_BINARY(gelfuncRegretBehav, BehavSolution *, Action *, gNumber *)

gNumber *gelfuncRegretBehav::EvalItem(BehavSolution *b, Action *a) const
{
  if (!b || !a)  return 0;
  if (&b->Game() != a->BelongsTo()->Game())
    throw gelGameMismatchError("Regret");
  Infoset* s = a->BelongsTo();
  EFPlayer* p = s->GetPlayer();

  if (s->IsChanceInfoset())
    return 0;
  
  return new gNumber(b->Regret()(p->GetNumber(), s->GetNumber(),
				 a->GetNumber()));
}

DECLARE_BINARY(gelfuncRegretMixed, MixedSolution *, Strategy *, gNumber *)

gNumber *gelfuncRegretMixed::EvalItem(MixedSolution *m, Strategy *s) const
{
  if (!m || !s)  return 0;
  if (&m->Game() != &s->nfp->Game())
    throw gelGameMismatchError("Regret");
  NFPlayer* p = s->nfp;
  Nfg &n = p->Game();

  gPVector<gNumber> v(n.NumStrats());
  m->Regret(v);

  return new gNumber(v(p->GetNumber(), s->number));
}

//----------------
// StrategyProb
//----------------

DECLARE_BINARY(gelfuncStrategyProb, MixedSolution *, Strategy *, gNumber *)

gNumber *gelfuncStrategyProb::EvalItem(MixedSolution *m, Strategy *s) const
{
  if (!m || !s)  return 0;
  if (&m->Game() != &s->nfp->Game())
    throw gelGameMismatchError("StrategyProb");
  NFPlayer* player = s->nfp;
  
  if (m->Support().Find(s))
    return new gNumber((*m)(player->GetNumber(), m->Support().Find(s)));
  else
    return new gNumber(0.0);
}

//-----------------
// StrategyValue
//-----------------

DECLARE_BINARY(gelfuncStrategyValue, MixedSolution *, Strategy *, gNumber *)

gNumber *gelfuncStrategyValue::EvalItem(MixedSolution *m, Strategy *s) const
{
  if (!m || !s)  return 0;
  if (&m->Game() != &s->nfp->Game())
    throw gelGameMismatchError("StrategyValue");

  if (m->Support().Find(s))
    return new gNumber(m->Payoff(s->nfp->GetNumber(), s));
  else
    return 0;
}

//----------
// Support
//----------

DECLARE_UNARY(gelfuncSupportBehav, BehavSolution *, EFSupport *)

EFSupport *gelfuncSupportBehav::EvalItem(BehavSolution *b) const
{
  return (b) ? new EFSupport(b->Support()) : 0;
}

DECLARE_UNARY(gelfuncSupportMixed, MixedSolution *, NFSupport *)

NFSupport *gelfuncSupportMixed::EvalItem(MixedSolution *m) const
{
  return (m) ? new NFSupport(m->Support()) : 0;
}


gelExpr *GEL_ActionProb(const gArray<gelExpr *> &params)
{
  return new gelfuncActionProb((gelExpression<BehavSolution *> *) params[1],
                               (gelExpression<Action *> *) params[2]);
}

gelExpr *GEL_ActionValue(const gArray<gelExpr *> &params)
{
  return new gelfuncActionValue((gelExpression<BehavSolution *> *) params[1],
                                (gelExpression<Action *> *) params[2]);
}

gelExpr *GEL_Behav(const gArray<gelExpr *> &params)
{
  return new gelfuncBehav((gelExpression<EFSupport *> *) params[1]);
}

gelExpr *GEL_Belief(const gArray<gelExpr *> &params)
{
  return new gelfuncBelief((gelExpression<BehavSolution *> *) params[1],
                           (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_GameEFSupport(const gArray<gelExpr *> &params)
{
  return new gelfuncGameEFSupport((gelExpression<EFSupport *> *) params[1]);
}

gelExpr *GEL_GameBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncGameBehav((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_GameNFSupport(const gArray<gelExpr *> &params)
{
  return new gelfuncGameNFSupport((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_GameMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncGameMixed((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_GobitLambdaBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncGobitLambdaBehav((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_GobitLambdaMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncGobitLambdaMixed((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_GobitValueBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncGobitValueBehav((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_GobitValueMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncGobitValueMixed((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_InfosetProb(const gArray<gelExpr *> &params)
{
  return new gelfuncInfosetProb((gelExpression<BehavSolution *> *) params[1],
                                (gelExpression<Infoset *> *) params[2]);
}

gelExpr *GEL_IsNashBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncIsNashBehav((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_IsNashMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncIsNashMixed((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_IsPerfect(const gArray<gelExpr *> &params)
{
  return new gelfuncIsPerfect((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_IsSequential(const gArray<gelExpr *> &params)
{
  return new gelfuncIsSequential((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_IsSubgamePerfect(const gArray<gelExpr *> &params)
{
  return new gelfuncIsSubgamePerfect((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_LiapValueBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncLiapValueBehav((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_LiapValueMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncLiapValueMixed((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_Mixed(const gArray<gelExpr *> &params)
{
  return new gelfuncMixed((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_RealizProb(const gArray<gelExpr *> &params)
{
  return new gelfuncRealizProb((gelExpression<BehavSolution *> *) params[1],
                               (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_RegretBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncRegretBehav((gelExpression<BehavSolution *> *) params[1],
                                (gelExpression<Action *> *) params[2]);
}

gelExpr *GEL_RegretMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncRegretMixed((gelExpression<MixedSolution *> *) params[1],
                                (gelExpression<Strategy *> *) params[2]);
}

gelExpr *GEL_StrategyProb(const gArray<gelExpr *> &params)
{
  return new gelfuncStrategyProb((gelExpression<MixedSolution *> *) params[1],
                                 (gelExpression<Strategy *> *) params[2]);
}

gelExpr *GEL_StrategyValue(const gArray<gelExpr *> &params)
{
  return new gelfuncStrategyValue((gelExpression<MixedSolution *> *) params[1],
                                  (gelExpression<Strategy *> *) params[2]);
}

gelExpr *GEL_SupportBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncSupportBehav((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_SupportMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncSupportMixed((gelExpression<MixedSolution *> *) params[1]);
}



#include "match.h"

void gelSolInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_ActionProb, "ActionProb[profile->BEHAV, action->ACTION] =: NUMBER" },
    { GEL_ActionValue, "ActionValue[profile->BEHAV, action->ACTION] =: NUMBER" },
    { GEL_Behav, "Behav[support->EFSUPPORT] =: BEHAV" },
    { GEL_Belief, "Belief[profile->BEHAV, node->NODE] =: NODE" },
    { GEL_GameEFSupport, "Game[support->EFSUPPORT] =: EFG" },
    { GEL_GameBehav, "Game[profile->BEHAV] =: EFG" },
    { GEL_GameNFSupport, "Game[support->NFSUPPORT] =: NFG" },
    { GEL_GameMixed, "Game[profile->MIXED] =: NFG" },
    { GEL_GobitLambdaBehav, "GobitLambda[profile->BEHAV] =: NUMBER" },
    { GEL_GobitLambdaMixed, "GobitLambda[profile->MIXED] =: NUMBER" },
    { GEL_GobitValueBehav, "GobitValue[profile->BEHAV] =: NUMBER" },
    { GEL_GobitValueMixed, "GobitValue[profile->MIXED] =: NUMBER" },
    { GEL_InfosetProb, "InfosetProb[profile->BEHAV, infoset->INFOSET] =: NUMBER" },
    { GEL_IsNashBehav, "IsNash[profile->BEHAV] =: BOOLEAN" },
    { GEL_IsNashMixed, "IsNash[profile->MIXED] =: BOOLEAN" },
    { GEL_IsPerfect, "IsPerfect[profile->MIXED] =: BOOLEAN" },
    { GEL_IsSequential, "IsSequential[profile->BEHAV] =: BOOLEAN" },
    { GEL_IsSubgamePerfect, "IsSubgamePerfect[profile->BEHAV] =: BOOLEAN" },
    { GEL_LiapValueBehav, "LiapValue[profile->BEHAV] =: NUMBER" },
    { GEL_LiapValueMixed, "LiapValue[profile->MIXED] =: NUMBER" },
    { GEL_Mixed, "Mixed[support->NFSUPPORT] =: MIXED" },
    { GEL_RealizProb, "RealizProb[profile->BEHAV, node->NODE] =: NUMBER" },
    { GEL_RegretBehav, "Regret[profile->BEHAV, action->ACTION] =: NUMBER" },
    { GEL_RegretMixed, "Regret[profile->MIXED, strategy->STRATEGY] =: NUMBER" },
    { GEL_StrategyProb, "StrategyProb[profile->MIXED, strategy->STRATEGY] =: NUMBER" },
    { GEL_StrategyValue, "StrategyValue[profile->MIXED, strategy->STRATEGY] =: NUMBER" },
    { GEL_SupportBehav, "Support[profile->BEHAV] =: EFSUPPORT" },
    { GEL_SupportMixed, "Support[profile->MIXED] =: NFSUPPORT" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++) 
    env->Register(sigarray[i].func, sigarray[i].sig);
}
