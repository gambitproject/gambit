//
// FILE: funcmisc.cc -- Arithmetic functions for GEL
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
#include "nfg.h"

//----------
// Input
//----------

DECLARE_UNARY(gelfuncInput, gText *, gInput *)

gInput *gelfuncInput::EvalItem(gText *filename) const
{
  if (!filename)  return 0;
  return new gFileInput(*filename);
}

//-----------
// Output
//-----------

DECLARE_UNARY(gelfuncOutput, gText *, gOutput *)

gOutput *gelfuncOutput::EvalItem(gText *filename) const
{
  if (!filename)   return 0;
  return new gFileOutput(*filename);
} 

//-------------
// Print
//-------------

DECLARE_UNARY_LIST(gelfuncPrintNumber, gNumber *, gNumber *)
DECLARE_UNARY_LIST(gelfuncPrintBoolean, gTriState *, gTriState *)
DECLARE_UNARY_LIST(gelfuncPrintText, gText *, gText *)
DECLARE_UNARY(gelfuncPrintEfg, Efg *, Efg *)
DECLARE_UNARY(gelfuncPrintNode, Node *, Node *)
DECLARE_UNARY(gelfuncPrintAction, Action *, Action *)
DECLARE_UNARY(gelfuncPrintInfoset, Infoset *, Infoset *)
DECLARE_UNARY(gelfuncPrintEFPlayer, EFPlayer *, EFPlayer *)
DECLARE_UNARY(gelfuncPrintEFOutcome, EFOutcome *, EFOutcome *)
DECLARE_UNARY(gelfuncPrintEFSupport, EFSupport *, EFSupport *)
DECLARE_UNARY(gelfuncPrintBehav, BehavSolution *, BehavSolution *)
DECLARE_UNARY(gelfuncPrintNfg, Nfg *, Nfg *)
DECLARE_UNARY(gelfuncPrintStrategy, Strategy *, Strategy *)
DECLARE_UNARY(gelfuncPrintNFPlayer, NFPlayer *, NFPlayer *)
DECLARE_UNARY(gelfuncPrintNFOutcome, NFOutcome *, NFOutcome *)
DECLARE_UNARY(gelfuncPrintNFSupport, NFSupport *, NFSupport *)
DECLARE_UNARY(gelfuncPrintMixed, MixedSolution *, MixedSolution *)
DECLARE_UNARY(gelfuncPrintInput, gInput *, gInput *)
DECLARE_UNARY(gelfuncPrintOutput, gOutput *, gOutput *) 

gNestedList<gNumber *> gelfuncPrintNumber::Evaluate(gelVariableTable *vt) const
{
  gNestedList<gNumber *> ret = op1->Evaluate(vt);
  gout << ret;
  return ret;
}

gNestedList<gText *> gelfuncPrintText::Evaluate(gelVariableTable *vt) const
{
  gNestedList<gText *> ret = op1->Evaluate(vt);
  gout << ret;
  return ret;
}

gNestedList<gTriState *> gelfuncPrintBoolean::Evaluate(gelVariableTable *vt) const
{
  gNestedList<gTriState *> ret = op1->Evaluate(vt);
//  ret.Output(gout, gTriState_Output);
  gout << ret;
  return ret;
}

Efg *gelfuncPrintEfg::EvalItem(Efg *E) const
{
  if (E)
    gout << E->GetTitle();
  else
    gout << "(null TEXT)";
  return E;
}

Node *gelfuncPrintNode::EvalItem(Node *n) const
{
  gout << n;
  return n;
}

Action *gelfuncPrintAction::EvalItem(Action *a) const
{
  gout << a;
  return a;
}

Infoset *gelfuncPrintInfoset::EvalItem(Infoset *s) const
{
  gout << s;
  return s;
}

EFPlayer *gelfuncPrintEFPlayer::EvalItem(EFPlayer *p) const
{
  gout << p;
  return p;
}

EFOutcome *gelfuncPrintEFOutcome::EvalItem(EFOutcome *c) const
{
  gout << c;
  return c;
}

EFSupport *gelfuncPrintEFSupport::EvalItem(EFSupport *s) const
{
  gout << s;
  return s;
}

BehavSolution *gelfuncPrintBehav::EvalItem(BehavSolution *b) const
{
  gout << *b;
  return b;
}

Nfg *gelfuncPrintNfg::EvalItem(Nfg *N) const
{
  gout << N->GetTitle();
  return N;
}

Strategy *gelfuncPrintStrategy::EvalItem(Strategy *s) const
{
  gout << s;
  return s;
}

NFPlayer *gelfuncPrintNFPlayer::EvalItem(NFPlayer *p) const
{
  gout << p;
  return p;
}

NFOutcome *gelfuncPrintNFOutcome::EvalItem(NFOutcome *c) const
{
  gout << c;
  return c;
}

NFSupport *gelfuncPrintNFSupport::EvalItem(NFSupport *s) const
{
  gout << s;
  return s;
}

MixedSolution *gelfuncPrintMixed::EvalItem(MixedSolution *m) const
{
  gout << m;
  return m;
}

gInput *gelfuncPrintInput::EvalItem(gInput *f) const
{
  gout << f;
  return f;
}

gOutput *gelfuncPrintOutput::EvalItem(gOutput *f) const
{
  gout << f;
  return f;
}

//----------
// Semi
//----------

template <class T> class gelfuncSemi : public gelExpression<T>
{
private:
  gelExpr *op1;
  gelExpression<T> *op2;

public:
  gelfuncSemi(gelExpr *, gelExpression<T> *);
  ~gelfuncSemi();

  gNestedList<T> Evaluate(gelVariableTable *vt) const;
};

template <class T>
gelfuncSemi<T>::gelfuncSemi(gelExpr *x, gelExpression<T> *y)
  : op1(x), op2(y)
{ }

template <class T> gelfuncSemi<T>::~gelfuncSemi()
{ delete op1;  delete op2; }

template <class T>
gNestedList<T> gelfuncSemi<T>::Evaluate(gelVariableTable *vt) const
{
  op1->Execute(vt);
  return op2->Evaluate(vt);
}

template class gelfuncSemi<gTriState *>;
template class gelfuncSemi<gNumber *>;
template class gelfuncSemi<gText *>;
template class gelfuncSemi<Efg *>;
template class gelfuncSemi<Node *>;
template class gelfuncSemi<Action *>;
template class gelfuncSemi<Infoset *>;
template class gelfuncSemi<EFPlayer *>;
template class gelfuncSemi<EFOutcome *>;
template class gelfuncSemi<EFSupport *>;
template class gelfuncSemi<BehavSolution *>;
template class gelfuncSemi<Nfg *>;
template class gelfuncSemi<Strategy *>;
template class gelfuncSemi<NFPlayer *>;
template class gelfuncSemi<NFOutcome *>;
template class gelfuncSemi<NFSupport *>;
template class gelfuncSemi<MixedSolution *>;
template class gelfuncSemi<gInput *>;
template class gelfuncSemi<gOutput *>;


#include "gwatch.h"

gWatch _gelStopwatch(false);

//---------------
// ElapsedTime
//---------------

class gelfuncElapsedTime : public gelExpression<gNumber *>
{
public:
  gelfuncElapsedTime(void) { }
  virtual ~gelfuncElapsedTime() { }
  gNestedList<gNumber *> Evaluate(gelVariableTable *) const;
};

gNestedList<gNumber *> gelfuncElapsedTime::Evaluate(gelVariableTable *) const
{
  gNestedList<gNumber *> ret;
  ret[1] = new gNumber(_gelStopwatch.Elapsed());
  return ret;
}

//------------------
// IsWatchRunning
//------------------

class gelfuncIsWatchRunning : public gelExpression<gTriState *>
{
public:
  gelfuncIsWatchRunning(void) { }
  virtual ~gelfuncIsWatchRunning() { }
  gNestedList<gTriState *> Evaluate(gelVariableTable *) const;
};

gNestedList<gTriState *> gelfuncIsWatchRunning::Evaluate(gelVariableTable *) const
{
  gNestedList<gTriState *> ret;
  ret[1] = new gTriState((_gelStopwatch.IsRunning()) ? triTRUE : triFALSE);
  return ret;
}

//---------------
// StartWatch
//---------------

class gelfuncStartWatch : public gelExpression<gNumber *>
{
public:
  gelfuncStartWatch(void) { }
  virtual ~gelfuncStartWatch() { }
  gNestedList<gNumber *> Evaluate(gelVariableTable *) const;
};

gNestedList<gNumber *> gelfuncStartWatch::Evaluate(gelVariableTable *) const
{
  _gelStopwatch.Start();
  gNestedList<gNumber *> ret;
  ret[1] = new gNumber(0);
  return ret;
}

//--------------
// StopWatch
//--------------

class gelfuncStopWatch : public gelExpression<gNumber *>
{
public:
  gelfuncStopWatch(void) { }
  virtual ~gelfuncStopWatch() { }
  gNestedList<gNumber *> Evaluate(gelVariableTable *) const;
};

gNestedList<gNumber *> gelfuncStopWatch::Evaluate(gelVariableTable *) const
{
  _gelStopwatch.Stop();
  gNestedList<gNumber *> ret;
  ret[1] = new gNumber(_gelStopwatch.Elapsed());
  return ret;
}


gelExpr *GEL_ElapsedTime(const gArray<gelExpr *> &)
{
  return new gelfuncElapsedTime();
}

gelExpr *GEL_Input(const gArray<gelExpr *> &params)
{
  return new gelfuncInput((gelExpression<gText *> *) params[1]);
}

gelExpr *GEL_IsWatchRunning(const gArray<gelExpr *> &)
{
  return new gelfuncIsWatchRunning();
}

gelExpr *GEL_Output(const gArray<gelExpr *> &params)
{
  return new gelfuncOutput((gelExpression<gText *> *) params[1]);
}

gelExpr *GEL_PrintBoolean(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintBoolean((gelExpression<gTriState *> *) params[1]);
}

gelExpr *GEL_PrintNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintNumber((gelExpression<gNumber *> *) params[1]);
}

gelExpr *GEL_PrintText(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintText((gelExpression<gText *> *) params[1]);
}

gelExpr *GEL_PrintEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintEfg((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_PrintNode(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintNode((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_PrintAction(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintAction((gelExpression<Action *> *) params[1]);
}

gelExpr *GEL_PrintInfoset(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintInfoset((gelExpression<Infoset *> *) params[1]);
}

gelExpr *GEL_PrintEFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintEFPlayer((gelExpression<EFPlayer *> *) params[1]);
}

gelExpr *GEL_PrintEFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintEFOutcome((gelExpression<EFOutcome *> *) params[1]);
}

gelExpr *GEL_PrintEFSupport(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintEFSupport((gelExpression<EFSupport *> *) params[1]);
}

gelExpr *GEL_PrintBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintBehav((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_PrintNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintNfg((gelExpression<Nfg *> *) params[1]);
}

gelExpr *GEL_PrintStrategy(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintStrategy((gelExpression<Strategy *> *) params[1]);
}

gelExpr *GEL_PrintNFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintNFPlayer((gelExpression<NFPlayer *> *) params[1]);
}

gelExpr *GEL_PrintNFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintNFOutcome((gelExpression<NFOutcome *> *) params[1]);
}

gelExpr *GEL_PrintNFSupport(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintNFSupport((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_PrintMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintMixed((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_PrintInput(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintInput((gelExpression<gInput *> *) params[1]);
}

gelExpr *GEL_PrintOutput(const gArray<gelExpr *> &params)
{
  return new gelfuncPrintOutput((gelExpression<gOutput *> *) params[1]);
}

gelExpr *GEL_SemiBoolean(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<gTriState *>(params[1],
				    ((gelExpression<gTriState *> *) params[2]));
}

gelExpr *GEL_SemiNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<gNumber *>(params[1],
				  ((gelExpression<gNumber *> *) params[2]));
}

gelExpr *GEL_SemiText(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<gText *>(params[1],
				((gelExpression<gText *> *) params[2]));
}

gelExpr *GEL_SemiEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<Efg *>(params[1],
				((gelExpression<Efg *> *) params[2]));
}

gelExpr *GEL_SemiNode(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<Node *>(params[1],
				((gelExpression<Node *> *) params[2]));
}

gelExpr *GEL_SemiAction(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<Action *>(params[1],
				((gelExpression<Action *> *) params[2]));
}

gelExpr *GEL_SemiInfoset(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<Infoset *>(params[1],
				((gelExpression<Infoset *> *) params[2]));
}

gelExpr *GEL_SemiEFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<EFPlayer *>(params[1],
				((gelExpression<EFPlayer *> *) params[2]));
}

gelExpr *GEL_SemiEFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<EFOutcome *>(params[1],
				((gelExpression<EFOutcome *> *) params[2]));
}

gelExpr *GEL_SemiEFSupport(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<EFSupport *>(params[1],
				((gelExpression<EFSupport *> *) params[2]));
}

gelExpr *GEL_SemiBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<BehavSolution *>(params[1],
				((gelExpression<BehavSolution *> *) params[2]));
}

gelExpr *GEL_SemiNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<Nfg *>(params[1],
				((gelExpression<Nfg *> *) params[2]));
}

gelExpr *GEL_SemiStrategy(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<Strategy *>(params[1],
				((gelExpression<Strategy *> *) params[2]));
}

gelExpr *GEL_SemiNFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<NFPlayer *>(params[1],
				((gelExpression<NFPlayer *> *) params[2]));
}

gelExpr *GEL_SemiNFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<NFOutcome *>(params[1],
				((gelExpression<NFOutcome *> *) params[2]));
}

gelExpr *GEL_SemiNFSupport(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<NFSupport *>(params[1],
				((gelExpression<NFSupport *> *) params[2]));
}

gelExpr *GEL_SemiMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<MixedSolution *>(params[1],
				((gelExpression<MixedSolution *> *) params[2]));
}

gelExpr *GEL_SemiInput(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<gInput *>(params[1],
                             (gelExpression<gInput *> *) params[2]);
}

gelExpr *GEL_SemiOutput(const gArray<gelExpr *> &params)
{
  return new gelfuncSemi<gOutput *>(params[1],
				    (gelExpression<gOutput *> *) params[2]);
}
 
gelExpr *GEL_StartWatch(const gArray<gelExpr *> &)
{
  return new gelfuncStartWatch();
}

gelExpr *GEL_StopWatch(const gArray<gelExpr *> &)
{
  return new gelfuncStopWatch();
}


#include "match.h"

void gelMiscInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_ElapsedTime, "ElapsedTime[] =: NUMBER" },
    { GEL_Input, "Input[filename->TEXT] =: INPUT" },
    { GEL_IsWatchRunning, "IsWatchRunning[] =: BOOLEAN" },
    { GEL_Output, "Output[filename->TEXT] =: OUTPUT" },
    { GEL_PrintBoolean, "Print[x->BOOLEAN] =: BOOLEAN" },
    { GEL_PrintEfg, "Print[x->EFG] =: EFG" },
    { GEL_PrintNumber, "Print[x->NUMBER] =: NUMBER" },
    { GEL_PrintText, "Print[x->TEXT] =: TEXT" },
    { GEL_PrintNode, "Print[x->NODE] =: NODE" },
    { GEL_PrintInfoset, "Print[x->INFOSET] =: INFOSET" },
    { GEL_PrintAction, "Print[x->ACTION] =: ACTION" },
    { GEL_PrintEFPlayer, "Print[x->EFPLAYER] =: EFPLAYER" },
    { GEL_PrintEFOutcome, "Print[x->EFOUTCOME] =: EFOUTCOME" },
    { GEL_PrintEFSupport, "Print[x->EFSUPPORT] =: EFSUPPORT" },
    { GEL_PrintBehav, "Print[x->BEHAV] =: BEHAV" },
    { GEL_PrintNfg, "Print[x->NFG] =: NFG" },
    { GEL_PrintStrategy, "Print[x->STRATEGY] =: STRATEGY" },
    { GEL_PrintNFPlayer, "Print[x->NFPLAYER] =: NFPLAYER" },
    { GEL_PrintNFOutcome, "Print[x->NFOUTCOME] =: NFOUTCOME" },
    { GEL_PrintNFSupport, "Print[x->NFSUPPORT] =: NFSUPPORT" },
    { GEL_PrintMixed, "Print[x->MIXED] =: MIXED" },
    { GEL_PrintInput, "Print[x->INPUT] =: INPUT" },
    { GEL_PrintOutput, "Print[x->OUTPUT] =: OUTPUT" },
    { GEL_SemiBoolean, "Semi[x->ANYTYPE, y->BOOLEAN] =: BOOLEAN" },
    { GEL_SemiNumber, "Semi[x->ANYTYPE, y->NUMBER] =: NUMBER" },
    { GEL_SemiText, "Semi[x->ANYTYPE, y->TEXT] =: TEXT" },
    { GEL_SemiEfg, "Semi[x->ANYTYPE, y->EFG] =: EFG" },
    { GEL_SemiNode, "Semi[x->ANYTYPE, y->NODE] =: NODE" },
    { GEL_SemiAction, "Semi[x->ANYTYPE, y->ACTION] =: ACTION" },
    { GEL_SemiInfoset, "Semi[x->ANYTYPE, y->INFOSET] =: INFOSET" },
    { GEL_SemiEFPlayer, "Semi[x->ANYTYPE, y->EFPLAYER] =: EFPLAYER" },
    { GEL_SemiEFOutcome, "Semi[x->ANYTYPE, y->EFOUTCOME] =: EFOUTCOME" },
    { GEL_SemiEFSupport, "Semi[x->ANYTYPE, y->EFSUPPORT] =: EFSUPPORT" },
    { GEL_SemiBehav, "Semi[x->ANYTYPE, y->BEHAV] =: BEHAV" },
    { GEL_SemiNfg, "Semi[x->ANYTYPE, y->NFG] =: NFG" },
    { GEL_SemiStrategy, "Semi[x->ANYTYPE, y->STRATEGY] =: STRATEGY" },
    { GEL_SemiNFPlayer, "Semi[x->ANYTYPE, y->NFPLAYER] =: NFPLAYER" },
    { GEL_SemiNFOutcome, "Semi[x->ANYTYPE, y->NFOUTCOME] =: NFOUTCOME" },
    { GEL_SemiNFSupport, "Semi[x->ANYTYPE, y->NFSUPPORT] =: NFSUPPORT" },
    { GEL_SemiMixed, "Semi[x->ANYTYPE, y->MIXED] =: MIXED" },
    { GEL_SemiInput, "Semi[x->ANYTYPE, y->INPUT] =: INPUT" },
    { GEL_SemiOutput, "Semi[x->ANYTYPE, y->OUTPUT] =: OUTPUT" },
    { GEL_StartWatch, "StartWatch[] =: NUMBER" },
    { GEL_StopWatch, "StopWatch[] =: NUMBER" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)  
    env->Register(sigarray[i].func, sigarray[i].sig);
}












