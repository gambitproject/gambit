//#
//# FILE: efgfunc.cc -- Extensive form editing builtins
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "extform.h"

Portion *GSM_NewEfg(Portion **param)
{
  ExtForm<double> *E = new ExtForm<double>;
  return new Efg_Portion<double>(*E);
}

Portion *GSM_ReadEfg(Portion **param)
{
  gFileInput f(((gString_Portion *) param[0])->Value());
  
  if (f.IsValid())   {
    DataType type;
    bool valid;

    EfgFileType(f, valid, type);
    if (!valid)   return 0;
    
    switch (type)   {
      case DOUBLE:  {
	ExtForm<double> *E = 0;
	ReadEfgFile((gInput &) f, E);

	if (E)
	  return new Efg_Portion<double>(*E);
	else
	  return 0;
      }
      case RATIONAL:   {
	ExtForm<gRational> *E = 0;
	ReadEfgFile((gInput &) f, E);
	
	if (E)
	  return new Efg_Portion<gRational>(*E);
	else
	  return 0;
      }
    }
  }
  else
    return 0;
}

//
// Utility functions for converting gArrays to List_Portions
// (perhaps these are more generally useful and should appear elsewhere?
//
template <class T> Portion *ArrayToList(const gArray<T> &A)
{
  List_Portion *ret = new List_Portion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new numerical_Portion<T>(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Player *> &A)
{
  List_Portion *ret = new List_Portion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new Player_Portion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Outcome *> &A)
{
  List_Portion *ret = new List_Portion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new Outcome_Portion(A[i]));
  return ret;
}

//
// Implementation of extensive form query functions, in alpha order
//

Portion *GSM_ChanceProbs(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value();
  if (!n->GetPlayer() || !n->GetPlayer()->IsChance())   return 0;
  switch (n->BelongsTo()->Type())   {
    case DOUBLE:
      return ArrayToList((gArray<double> &) ((ChanceInfoset<double> *) n->GetInfoset())->GetActionProbs());
    case RATIONAL:
      return ArrayToList((gArray<gRational> &) ((ChanceInfoset<gRational> *) n->GetInfoset())->GetActionProbs());
  }
}

Portion *GSM_Infoset(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value();
  return new Infoset_Portion(n->GetInfoset());
}

Portion *GSM_IsPredecessor(Portion **param)
{
  Node *n1 = ((Node_Portion *) param[0])->Value();
  Node *n2 = ((Node_Portion *) param[1])->Value();
  return new bool_Portion(n1->BelongsTo()->IsPredecessor(n1, n2));
}

Portion *GSM_IsSuccessor(Portion **param)
{
  Node *n1 = ((Node_Portion *) param[0])->Value();
  Node *n2 = ((Node_Portion *) param[1])->Value();
  return new bool_Portion(n1->BelongsTo()->IsSuccessor(n1, n2));
}

Portion *GSM_NameEfg(Portion **param)
{
  ExtForm<double> &E = ((Efg_Portion<double> *) param[0])->Value();
  return new gString_Portion(E.GetTitle());
}

Portion *GSM_NamePlayer(Portion **param)
{
  Player *p = ((Player_Portion *) param[0])->Value();
  return new gString_Portion(p->GetName());
}

Portion *GSM_NameNode(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value();
  return new gString_Portion(n->GetName());
}

Portion *GSM_NameInfoset(Portion **param)
{
  Infoset *s = ((Infoset_Portion *) param[0])->Value();
  return new gString_Portion(s->GetName());
}

Portion *GSM_NameOutcome(Portion **param)
{
  Outcome *c = ((Outcome_Portion *) param[0])->Value();
  return new gString_Portion(c->GetName());
}

Portion *GSM_NextSibling(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value()->NextSibling();
  if (n)   return new Node_Portion(n);
  else   return 0;
}

Portion *GSM_NthChild(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value();
  int child = ((numerical_Portion<gInteger> *) param[1])->Value().as_long();
  if (child < 1 || child >= n->NumChildren())   return 0;
  return new Node_Portion(n->GetChild(child));
}

Portion *GSM_NumActions(Portion **param)
{
  Infoset *s = ((Infoset_Portion *) param[0])->Value();
  return new numerical_Portion<gInteger>(s->NumActions());
}

Portion *GSM_NumChildren(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value();
  return new numerical_Portion<gInteger>(n->NumChildren());
}

Portion *GSM_NumInfosets(Portion **param)
{
  Player *p = ((Player_Portion *) param[0])->Value();
  return new numerical_Portion<gInteger>(p->NumInfosets());
}

Portion *GSM_NumMembers(Portion **param)
{
  Infoset *s = ((Infoset_Portion *) param[0])->Value();
  return new numerical_Portion<gInteger>(s->NumMembers());
}

Portion *GSM_NumOutcomes(Portion **param)
{
  ExtForm<double> &E = ((Efg_Portion<double> *) param[0])->Value();
  return new numerical_Portion<gInteger>(E.NumOutcomes());
}

Portion *GSM_NumPlayersEfg(Portion **param)
{
  ExtForm<double> &E = ((Efg_Portion<double> *) param[0])->Value();
  return new numerical_Portion<gInteger>(E.NumPlayers());
}

Portion *GSM_Outcome(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value();
  if (n->GetOutcome())
    return new Outcome_Portion(n->GetOutcome());
  else
    return 0;
}

Portion *GSM_Outcomes(Portion **param)
{
  ExtForm<double> *E = &((Efg_Portion<double> *) param[0])->Value();
  return ArrayToList(E->OutcomeList());
}

Portion *GSM_OutcomeVector(Portion **param)
{
  Outcome *c = ((Outcome_Portion *) param[0])->Value();
  switch (c->BelongsTo()->Type())   {
    case DOUBLE:
      return ArrayToList((gArray<double> &) (OutcomeVector<double> &) *c);
    case RATIONAL:
      return ArrayToList((gArray<gRational> &) (OutcomeVector<gRational> &) *c);
  }
}
Portion *GSM_Parent(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value();
  if (n->GetParent())
    return new Node_Portion(n->GetParent());
  else
    return 0;
}

Portion *GSM_Players(Portion **param)
{
  ExtForm<double> *E = &((Efg_Portion<double> *) param[0])->Value();
  return ArrayToList(E->PlayerList());
}

Portion *GSM_PriorSibling(Portion **param)
{
  Node *n = ((Node_Portion *) param[0])->Value()->PriorSibling();
  if (n)   return new Node_Portion(n);
  else   return 0;
}

Portion *GSM_RootNode(Portion **param)
{
  ExtForm<double> &E = ((Efg_Portion<double> *) param[0])->Value();
  return new Node_Portion(E.RootNode());
}

Portion *GSM_WriteEfg(Portion **param)
{
  gOutput &f = ((Output_Portion *) param[0])->Value();
  BaseExtForm *E = &((BaseEfg_Portion *) param[1])->Value();
  
  E->WriteEfgFile(f);
  return new Output_Portion(f);
}

void Init_efgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("NewEfg");
  FuncObj->SetFuncInfo(GSM_NewEfg, 0);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ReadEfg");
  FuncObj->SetFuncInfo(GSM_ReadEfg, 1);
  FuncObj->SetParamInfo(GSM_ReadEfg, 0, "f", porSTRING);
  gsm->AddFunction(FuncObj);

//-----------------------------------------------------------

  FuncObj = new FuncDescObj("ChanceProbs");
  FuncObj->SetFuncInfo(GSM_ChanceProbs, 1);
  FuncObj->SetParamInfo(GSM_ChanceProbs, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Infoset");
  FuncObj->SetFuncInfo(GSM_Infoset, 1);
  FuncObj->SetParamInfo(GSM_Infoset, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsPredecessor");
  FuncObj->SetFuncInfo(GSM_IsPredecessor, 2);
  FuncObj->SetParamInfo(GSM_IsPredecessor, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_IsPredecessor, 1, "of", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsSuccessor");
  FuncObj->SetFuncInfo(GSM_IsSuccessor, 2);
  FuncObj->SetParamInfo(GSM_IsSuccessor, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_IsSuccessor, 1, "from", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Name");
  FuncObj->SetFuncInfo(GSM_NameEfg, 1);
  FuncObj->SetParamInfo(GSM_NameEfg, 0, "efg", porEFG_DOUBLE);

  FuncObj->SetFuncInfo(GSM_NamePlayer, 1);
  FuncObj->SetParamInfo(GSM_NamePlayer, 0, "x", porPLAYER);

  FuncObj->SetFuncInfo(GSM_NameNode, 1);
  FuncObj->SetParamInfo(GSM_NameNode, 0, "x", porNODE);

  FuncObj->SetFuncInfo(GSM_NameInfoset, 1);
  FuncObj->SetParamInfo(GSM_NameInfoset, 0, "x", porINFOSET);

  FuncObj->SetFuncInfo(GSM_NameOutcome, 1);
  FuncObj->SetParamInfo(GSM_NameOutcome, 0, "x", porOUTCOME);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NextSibling");
  FuncObj->SetFuncInfo(GSM_NextSibling, 1);
  FuncObj->SetParamInfo(GSM_NextSibling, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NthChild");
  FuncObj->SetFuncInfo(GSM_NthChild, 2);
  FuncObj->SetParamInfo(GSM_NthChild, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_NthChild, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumActions");
  FuncObj->SetFuncInfo(GSM_NumActions, 1);
  FuncObj->SetParamInfo(GSM_NumActions, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumChildren");
  FuncObj->SetFuncInfo(GSM_NumChildren, 1);
  FuncObj->SetParamInfo(GSM_NumChildren, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("NumInfosets");
  FuncObj->SetFuncInfo(GSM_NumInfosets, 1);
  FuncObj->SetParamInfo(GSM_NumInfosets, 0, "player", porPLAYER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumMembers");
  FuncObj->SetFuncInfo(GSM_NumMembers, 1);
  FuncObj->SetParamInfo(GSM_NumMembers, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumOutcomes");
  FuncObj->SetFuncInfo(GSM_NumOutcomes, 1);
  FuncObj->SetParamInfo(GSM_NumOutcomes, 0, "efg", porEFG_DOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumPlayers");
  FuncObj->SetFuncInfo(GSM_NumPlayersEfg, 1);
  FuncObj->SetParamInfo(GSM_NumPlayersEfg, 0, "efg", porEFG_DOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Outcome");
  FuncObj->SetFuncInfo(GSM_Outcome, 1);
  FuncObj->SetParamInfo(GSM_Outcome, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Outcomes");
  FuncObj->SetFuncInfo(GSM_Outcomes, 1);
  FuncObj->SetParamInfo(GSM_Outcomes, 0, "efg", porEFG_DOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("OutcomeVector");
  FuncObj->SetFuncInfo(GSM_OutcomeVector, 1);
  FuncObj->SetParamInfo(GSM_OutcomeVector, 0, "outc", porOUTCOME);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Parent");
  FuncObj->SetFuncInfo(GSM_Parent, 1);
  FuncObj->SetParamInfo(GSM_Parent, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Players");
  FuncObj->SetFuncInfo(GSM_Players, 1);
  FuncObj->SetParamInfo(GSM_Players, 0, "efg", porEFG_DOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("PriorSibling");
  FuncObj->SetFuncInfo(GSM_PriorSibling, 1);
  FuncObj->SetParamInfo(GSM_PriorSibling, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RootNode");
  FuncObj->SetFuncInfo(GSM_RootNode, 1);
  FuncObj->SetParamInfo(GSM_RootNode, 0, "efg", porEFG_DOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("WriteEfg");
  FuncObj->SetFuncInfo(GSM_WriteEfg, 2);
  FuncObj->SetParamInfo(GSM_WriteEfg, 0, "output", porOUTPUT);
  FuncObj->SetParamInfo(GSM_WriteEfg, 1, "efg", porEFG);
  gsm->AddFunction(FuncObj);
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE Portion *ArrayToList(const gArray<double> &);
TEMPLATE Portion *ArrayToList(const gArray<gRational> &);

