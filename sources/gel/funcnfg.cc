//
// FILE: funcnfg.cc -- Nfg functions for GEL
//
// $Id$
//

#include "gmisc.h"
#include "exprtree.h"
#include "funcmisc.h"
#include "glist.h"
#include "tristate.h"

#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"


//---------------
// AddStrategy
//---------------

DECLARE_BINARY(gelfuncAddStrategy, NFSupport *, Strategy *, NFSupport *)

NFSupport *gelfuncAddStrategy::EvalItem(NFSupport *S, Strategy *s) const
{
  NFSupport *T = new NFSupport(*S);
  T->AddStrategy(s);
  return T;
}

//-----------------
// DeleteOutcome
//-----------------

DECLARE_UNARY(gelfuncDeleteOutcomeNfg, NFOutcome *, gTriState)

gTriState gelfuncDeleteOutcomeNfg::EvalItem(NFOutcome *c) const
{
  c->Game()->DeleteOutcome(c);
  return triTRUE;
}

//---------
// Game
//---------

DECLARE_UNARY(gelfuncGameNFPlayer, NFPlayer *, Nfg *)

Nfg *gelfuncGameNFPlayer::EvalItem(NFPlayer *p) const
{
  return &p->Game();
}

DECLARE_UNARY(gelfuncGameStrategy, Strategy *, Nfg *)

Nfg *gelfuncGameStrategy::EvalItem(Strategy *s) const
{
  return &s->nfp->Game();
}

DECLARE_UNARY(gelfuncGameNFOutcome, NFOutcome *, Nfg *)

Nfg *gelfuncGameNFOutcome::EvalItem(NFOutcome *c) const
{
  return c->Game();
}


//-------------
// IsConstSum
//-------------

DECLARE_UNARY(gelfuncIsConstSumNfg, Nfg *, gTriState)

gTriState gelfuncIsConstSumNfg::EvalItem(Nfg *N) const
{
  return (IsConstSum(*N)) ? triTRUE : triFALSE;
}

//------------
// LoadEfg
//------------

DECLARE_UNARY(gelfuncLoadNfg, gText, Nfg *)

Nfg *gelfuncLoadNfg::EvalItem(gText filename) const
{
  gFileInput f(filename);
  Nfg *N = 0;
  ReadNfgFile(f, N);
  return N;
}

//--------
// Name
//--------

DECLARE_UNARY(gelfuncNameNfg, Nfg *, gText)

gText gelfuncNameNfg::EvalItem(Nfg *N) const
{
  return N->GetTitle();
}

DECLARE_UNARY(gelfuncNameNFPlayer, NFPlayer *, gText)

gText gelfuncNameNFPlayer::EvalItem(NFPlayer *p) const
{
  return p->GetName();
}

DECLARE_UNARY(gelfuncNameStrategy, Strategy *, gText)

gText gelfuncNameStrategy::EvalItem(Strategy *s) const
{
  return s->name;
}

DECLARE_UNARY(gelfuncNameNFOutcome, NFOutcome *, gText)

gText gelfuncNameNFOutcome::EvalItem(NFOutcome *c) const
{
  return c->GetName();
}


//-------------
// NewOutcome
//-------------

DECLARE_UNARY(gelfuncNewOutcomeNfg, Nfg *, NFOutcome *)

NFOutcome *gelfuncNewOutcomeNfg::EvalItem(Nfg *N) const
{
  return N->NewOutcome();
}


//----------
// Payoff
//----------

DECLARE_BINARY(gelfuncPayoffNFOutcome, NFOutcome *, NFPlayer *, gNumber)

gNumber gelfuncPayoffNFOutcome::EvalItem(NFOutcome *c, NFPlayer *p) const
{
  return c->Game()->Payoff(c, p->GetNumber());
}


//----------
// Player
//----------

DECLARE_UNARY(gelfuncPlayerStrategy, Strategy *, NFPlayer *)

NFPlayer *gelfuncPlayerStrategy::EvalItem(Strategy *s) const
{
  return s->nfp;
}


//------------------
// RemoveStrategy
//------------------

DECLARE_BINARY(gelfuncRemoveStrategy, NFSupport *, Strategy *, NFSupport *)

NFSupport *gelfuncRemoveStrategy::EvalItem(NFSupport *S, Strategy *s) const
{
  NFSupport *T = new NFSupport(*S);
  T->RemoveStrategy(s);
  return T;
}

//------------
// SaveEfg
//------------

DECLARE_BINARY(gelfuncSaveNfg, Nfg *, gText, Nfg *)

Nfg *gelfuncSaveNfg::EvalItem(Nfg *N, gText filename) const
{
  gFileOutput f(filename);
  N->WriteNfgFile(f);
  return N;
}

//-----------
// SetName
//-----------

DECLARE_BINARY(gelfuncSetNameNfg, Nfg *, gText, Nfg *)

Nfg *gelfuncSetNameNfg::EvalItem(Nfg *N, gText name) const
{
  N->SetTitle(name);
  return N;
}

DECLARE_BINARY(gelfuncSetNameNFPlayer, NFPlayer *, gText, NFPlayer *)

NFPlayer *gelfuncSetNameNFPlayer::EvalItem(NFPlayer *p, gText name) const
{
  p->SetName(name);
  return p;
}

DECLARE_BINARY(gelfuncSetNameStrategy, Strategy *, gText, Strategy *)

Strategy *gelfuncSetNameStrategy::EvalItem(Strategy *s, gText name) const
{
  s->name = name;
  return s;
}

DECLARE_BINARY(gelfuncSetNameNFOutcome, NFOutcome *, gText, NFOutcome *)

NFOutcome *gelfuncSetNameNFOutcome::EvalItem(NFOutcome *c, gText name) const
{
  c->SetName(name);
  return c;
}

//-----------
// Support
//-----------

DECLARE_UNARY(gelfuncSupportNfg, Nfg *, NFSupport *)

NFSupport *gelfuncSupportNfg::EvalItem(Nfg *N) const
{
  return new NFSupport(*N);
}


gelExpr *GEL_AddStrategy(const gArray<gelExpr *> &params)
{
  return new gelfuncAddStrategy((gelExpression<NFSupport *> *) params[1],
                                (gelExpression<Strategy *> *) params[2]);
}

gelExpr *GEL_DeleteOutcomeNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncDeleteOutcomeNfg((gelExpression<NFOutcome *> *) params[1]);
}

gelExpr *GEL_GameNFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncGameNFPlayer((gelExpression<NFPlayer *> *) params[1]);
}

gelExpr *GEL_GameStrategy(const gArray<gelExpr *> &params)
{
  return new gelfuncGameStrategy((gelExpression<Strategy *> *) params[1]);
}

gelExpr *GEL_GameNFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncGameNFOutcome((gelExpression<NFOutcome *> *) params[1]);
}

gelExpr *GEL_IsConstSumNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncIsConstSumNfg((gelExpression<Nfg *> *) params[1]);
}

gelExpr *GEL_LoadNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLoadNfg((gelExpression<gText> *) params[1]);
}

gelExpr *GEL_NameNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncNameNfg((gelExpression<Nfg *> *) params[1]);
}

gelExpr *GEL_NameNFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncNameNFPlayer((gelExpression<NFPlayer *> *) params[1]);
}

gelExpr *GEL_NameStrategy(const gArray<gelExpr *> &params)
{
  return new gelfuncNameStrategy((gelExpression<Strategy *> *) params[1]);
}

gelExpr *GEL_NameNFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncNameNFOutcome((gelExpression<NFOutcome *> *) params[1]);
}

gelExpr *GEL_NewOutcomeNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncNewOutcomeNfg((gelExpression<Nfg *> *) params[1]);
}

gelExpr *GEL_PlayerStrategy(const gArray<gelExpr *> &params)
{
  return new gelfuncPlayerStrategy((gelExpression<Strategy *> *) params[1]);
}

gelExpr *GEL_RemoveStrategy(const gArray<gelExpr *> &params)
{
  return new gelfuncRemoveStrategy((gelExpression<NFSupport *> *) params[1],
                                   (gelExpression<Strategy *> *) params[2]);
}

gelExpr *GEL_SaveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSaveNfg((gelExpression<Nfg *> *) params[1],
                            (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameNfg((gelExpression<Nfg *> *) params[1],
                               (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameStrategy(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameStrategy((gelExpression<Strategy *> *) params[1],
                                    (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameNFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameNFPlayer((gelExpression<NFPlayer *> *) params[1],
                                    (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameNFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameNFOutcome((gelExpression<NFOutcome *> *) params[1],
                                     (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SupportNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSupportNfg((gelExpression<Nfg *> *) params[1]);
}


#include "match.h"

void gelNfgInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_AddStrategy, "AddStrategy[support->NFSUPPORT, strategy->STRATEGY] =: NFSUPPORT" },
    { GEL_DeleteOutcomeNfg, "DeleteOutcome[outcome->NFOUTCOME] =: BOOLEAN" },
    { GEL_GameNFPlayer, "Game[player->NFPLAYER] =: NFG" },
    { GEL_GameStrategy, "Game[strategy->STRATEGY] =: NFG" },
    { GEL_GameNFOutcome, "Game[outcome->NFOUTCOME] =: NFG" },
    { GEL_IsConstSumNfg, "IsConstSum[nfg->NFG] =: BOOLEAN" },
    { GEL_LoadNfg, "LoadNfg[file->TEXT] =: NFG" },
    { GEL_NameNfg, "Name[nfg->NFG] =: TEXT" },
    { GEL_NameNFPlayer, "Name[player->NFPLAYER] =: TEXT" },
    { GEL_NameStrategy, "Name[strategy->STRATEGY] =: TEXT" },
    { GEL_NameNFOutcome, "Name[outcome->NFOUTCOME] =: TEXT" },
    { GEL_NewOutcomeNfg, "NewOutcome[nfg->NFG] =: NFOUTCOME" },
    { GEL_PlayerStrategy, "Player[strategy->STRATEGY] =: NFPLAYER" },
    { GEL_RemoveStrategy, "RemoveStrategy[support->NFSUPPORT, strategy->STRATEGY] =: NFSUPPORT" },
    { GEL_SaveNfg, "SaveNfg[nfg->NFG, file->TEXT] =: NFG" },
    { GEL_SetNameNfg, "SetName[nfg->NFG, name->TEXT] =: NFG" },
    { GEL_SetNameNFPlayer, "SetName[player->NFPLAYER, name->TEXT] =: NFPLAYER" },
    { GEL_SetNameStrategy, "SetName[strategy->STRATEGY, name->TEXT] =: STRATEGY" },
    { GEL_SetNameNFOutcome, "SetName[outcome->NFOUTCOME, name->TEXT] =: NFOUTCOME" },
    { GEL_SupportNfg, "Support[nfg->NFG] =: NFSUPPORT" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)
    env->Register(sigarray[i].func, sigarray[i].sig);
}












