%{
/* $Id$ */
#include <stdlib.h>
#include <ctype.h>
#include "base/gstream.h"
#include "base/gtext.h"
#include "math/rational.h"
#include "math/gnumber.h"
#include "base/gstack.h"
#include "base/glist.h"
#include "efg.h"


#include "base/gstack.imp"

template class gStack<Node *>;

static gInput *infile;
static gText last_name;
static gNumber last_number;
static int last_int, iset_idx;
static FullEfg *E;
static gText title, comment;
static gStack<Node *> path;
static gList<gText> actions, players;
static gList<gNumber> values;
static EFPlayer *player;
static Infoset *infoset;
static EFOutcome *outcome;
static int i;
static gText iset_name, outc_name;

static void SetOutcome(EFOutcome *, const gList<gNumber> &);
static void SetActionProbs(Infoset *, const gList<gNumber> &); 
static bool CheckActionProbs(Infoset *, const gList<gNumber > &);
static bool CheckOutcome(EFOutcome *, const gList<gNumber> &);
static int Parse(void);
static void CreateEfg(void);

void efg_yyerror(char *);
int efg_yylex(void);

%}

%token LBRACE
%token RBRACE
%token SLASH

%token NAME
%token VARNAME
%token NUMBER

%%

efgfile:           header  { CreateEfg(); path.Push(E->RootNode()); }
                   body    { E->Reindex();  return 0; }
       ;

header:            NAME 
                   { title = last_name; players.Flush(); path.Flush(); } 
                   playerlist commentopt
      ;

playerlist:        LBRACE RBRACE
          |        LBRACE players RBRACE
          ;
players:           player
       |           players player
       ;

player:            NAME   { players.Append(last_name); }
      ;

commentopt:        
          |        NAME   { comment = last_name; }
          ;

body:              node
    |              body node
    ;

node:              person_node | chance_node | terminal_node
    ;

person_node:       'p' node_name player_number infoset_number infoset_info
                   outcome_number outcome_info
                   { path.Push(path.Peek()->GetChild(1)); }
           ;

chance_node:       'c' node_name { player = E->GetChance(); }
                   infoset_number chance_infoset_info
                   outcome_number outcome_info
                   { path.Push(path.Peek()->GetChild(1)); }
           ;

terminal_node:     't' node_name outcome_number outcome_info
                   { path.Push(path.Pop()->NextSibling());
		     while (path.Depth() > 1 && !path.Peek())  {
		       path.Pop();
		       path.Push(path.Pop()->NextSibling());
		     }
		   }
             ;

node_name:         NAME     { path.Peek()->SetName(last_name); }
         ;

player_number:     NUMBER
                   { if (last_number.operator gRational().denominator() != 1)  YYERROR;
		     last_int = (last_number.operator gRational()).numerator().as_long();
		     if (last_int <= 0 || last_int > E->NumPlayers()) YYERROR;
		     player = E->Players()[last_int];
		   }
             ;

infoset_number:    NUMBER
                   { if (last_number.operator gRational().denominator() != 1)  YYERROR;
		     iset_idx = last_number.operator gRational().numerator().as_long();
		     infoset = E->GetInfosetByIndex(player, iset_idx);
		   }
              ; 

infoset_info:      { if (!infoset)   YYERROR; }
            |      infoset_name LBRACE { actions.Flush(); } actionlist RBRACE
                   { if (!infoset)   {
		       if (actions.Length() == 0)   YYERROR;
		       infoset = E->CreateInfoset(iset_idx, player,
						  actions.Length());
		       infoset->SetName(iset_name);
		       for (i = 1; i <= actions.Length(); i++)
			 infoset->SetActionName(i, actions[i]);
		     }
		     else   {
		       if (infoset->NumActions() != actions.Length())
			 YYERROR;
		       for (i = 1; i <= actions.Length(); i++)
			 if (actions[i] != infoset->GetActionName(i))
			   YYERROR;
		     }
		     E->AppendNode(path.Peek(), infoset);
		   }
            ;

infoset_name:      NAME
                   { if (infoset && infoset->GetName() != last_name) YYERROR;
		     else if (!infoset)   iset_name = last_name;
                   }
            ;

actionlist:        action
          |        actionlist action
          ;

action:            NAME   { actions.Append(last_name); }
      ;

chance_infoset_info:   { if (!infoset)  YYERROR; }
                   |   infoset_name LBRACE
                       { actions.Flush(); values.Flush(); }
                       actionproblist RBRACE
                       { if (!infoset)  {
			   if (actions.Length() == 0)  YYERROR;
			   infoset = E->CreateInfoset(iset_idx, player,
						      actions.Length());
			   infoset->SetName(iset_name);
			   for (i = 1; i <= actions.Length(); i++)
			     infoset->SetActionName(i, actions[i]);
			   SetActionProbs(infoset, values);
			 }
			 else  {
			   if (infoset->NumActions() != actions.Length())
			     YYERROR;
			   for (i = 1; i <= actions.Length(); i++)
			     if (actions[i] != infoset->GetActionName(i))
			       YYERROR;
			   if (!CheckActionProbs(infoset, values))  YYERROR;
			 }
			 E->AppendNode(path.Peek(), infoset);
		       }
                   ;

actionproblist:    actionprob
              |    actionproblist actionprob
              ;

actionprob:        NAME NUMBER
                   { actions.Append(last_name); 
                     values.Append(last_number); }
          ;
 
outcome_number:    NUMBER
                   { if (last_number.operator gRational().denominator() != 1)  YYERROR;
		     last_int = last_number.operator gRational().numerator().as_long();
		     if (last_int > 0)  
		       outcome = E->GetOutcomeByIndex(last_int);
		   }
              ;

outcome_info:      { if (!outcome && last_int != 0)  YYERROR; }
            |      outcome_name LBRACE 
                   { values.Flush(); } payofflist
                   RBRACE
                   { if (values.Length() != E->NumPlayers())   YYERROR;
		     if (!outcome)   {
		       outcome = E->CreateOutcomeByIndex(last_int);
		       outcome->SetName(outc_name);
		       SetOutcome(outcome, values);
		     }
		     else if (!CheckOutcome(outcome, values))  YYERROR;

		     path.Peek()->SetOutcome(outcome);
		   }
            ;

outcome_name:      NAME
                   { if (outcome && outcome->GetName() != last_name) YYERROR;
		     else if (!outcome)   outc_name = last_name; }

payofflist:        payoff
          |        payofflist commaopt payoff
          ;

commaopt:          
        |          ','
        ;

payoff:            NUMBER    { values.Append(last_number); }

%%

void efg_yyerror(char *s)   { gout << s << '\n'; }

int efg_yylex(void)
{
  char c, d;

  while (1)  {
    do  {
      *infile >> c;
    }  while (isspace(c));
 
    if (c == '/')   {
      *infile >> d;
      if (d == '/')  {
	do  {
	  *infile >> d;
	}  while (d != '\n');
      }
      else if (d == '*')  {
	int done = 0;
	while (!done)  {
	  do {
	    *infile >> d;
	  }  while (d != '*');
	  *infile >> d;
	  if (d == '/')   done = 1;
	}
      }
      else  {
	infile->unget(d);
	return SLASH;
      }
    }
    else
      break;
  }

  if (isalpha(c))   {
    return c;	
  }   

  if (c == '"')  {
    infile->unget(c);
    *infile >> last_name;

    return NAME;
  }
  else if (isdigit(c) || c == '-')   {
    infile->unget(c);
    *infile >> last_number;
    return NUMBER;
  }
  
  switch (c)   {
    case '-':  *infile >> c;
               if (isdigit(c))  {
                 infile->unget(c);
                 *infile >> last_number;
		 last_number = -last_number;
                 return NUMBER;
               }
               else  {
                 infile->unget(c);
                 return '-';
               }
    case '{':  return LBRACE;
    case '}':  return RBRACE;
    default:   return c;
  }
}

void SetOutcome(EFOutcome *c, const gList<gNumber> &p)
{
  for (int i = 1; i <= p.Length(); i++)
    E->SetPayoff(c, i, p[i]);
}

void SetActionProbs(Infoset *s, const gList<gNumber> &p)
{
  for (int i = 1; i <= p.Length(); i++)
    E->SetChanceProb(s, i, p[i]);
}

bool CheckActionProbs(Infoset *s, const gList<gNumber> &p)
{
  for (int i = 1; i <= p.Length(); i++)
    if (E->GetChanceProb(s, i) != p[i])  return false;
  return true;
}

bool CheckOutcome(EFOutcome *c, const gList<gNumber> &p)
{
  for (int i = 1; i <= p.Length(); i++)
    if (E->Payoff(c, E->Players()[i]) != p[i])   return false;
  return true;
}

int Parse(void)
{
  infile->seekp(0);
  static char *prologue = { "EFG 2 " };
  char c;
  for (unsigned int i = 0; i < strlen(prologue); i++)   {
    infile->get(c);
    if (c != prologue[i])   return 1;
  }

  infile->get(c);
  if (c != 'D' && c != 'R')   return 1;

  int ret = efg_yyparse();
  E->SetIsDirty(false);
  return ret;
}

void CreateEfg(void)
{
  E = new FullEfg;

  E->SetTitle(title);
  E->SetComment(comment);
  for (int pl = 1; pl <= players.Length(); pl++)
    E->NewPlayer()->SetName(players[pl]);
}	

FullEfg *ReadEfgFile(gInput &p_file)
{
  infile = &p_file;
  E = 0;  

  if (Parse())  {
    if (E)  {
      delete E;
    }
    return 0;
  }

  return E;
}




