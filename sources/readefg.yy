
%{
/* $Id$ */
#include <stdlib.h>
#include <ctype.h>
#include "gambitio.h"
#include "gstring.h"
#include "rational.h"
#include "ggrstack.h"
#include "glist.h"
#include "efg.h"

%}

%name EfgFileReader

%define MEMBERS    gInput &infile; \
                   gString last_name;  gRational last_rational;  \
                   int last_int, iset_idx; \
                   BaseEfg *& E; \
                   gGrowableStack<Node *> path; \
                   gList<gString> actions; \
                   gList<gRational> values; \
                   EFPlayer *player; Infoset *infoset; Outcome *outcome; \
                   int i;  gString iset_name, outc_name; \
                   virtual ~EfgFileReader(); \
                   virtual Outcome *NewOutcome(void) = 0; \
                   virtual void SetOutcome(Outcome *, \
					   const gList<gRational> &) = 0; \
                   virtual void SetActionProbs(Infoset *, \
					       const gList<gRational> &) = 0; \
                   virtual bool CheckActionProbs(Infoset *, \
						 const gList<gRational> &)=0;\
                   virtual bool CheckOutcome(Outcome *, \
					     const gList<gRational> &) = 0;


%define CONSTRUCTOR_PARAM    gInput &f, BaseEfg *& e

%define CONSTRUCTOR_INIT     : infile(f), E(e), path(32)

%token LBRACE
%token RBRACE
%token SLASH

%token NAME
%token NUMBER

%%

efgfile:           header  { path.Push(E->RootNode()); }
                   body    { E->Reindex();  return 0; }
       ;

header:            NAME  { E->SetTitle(last_name); }  playerlist
      ;

playerlist:        LBRACE RBRACE
          |        LBRACE players RBRACE
          ;
players:           player
       |           players player
       ;

player:            NAME   { E->NewPlayer()->SetName(last_name); }
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
                   { if (last_rational.denominator() != 1)  YYERROR;
		     last_int = last_rational.numerator().as_long();
		     if (last_int <= 0 || last_int > E->NumPlayers()) YYERROR;
		     player = E->PlayerList()[last_int];
		   }
             ;

infoset_number:    NUMBER
                   { if (last_rational.denominator() != 1)  YYERROR;
		     iset_idx = last_rational.numerator().as_long();
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
                   { actions.Append(last_name); values.Append(last_rational); }
          ;
 
outcome_number:    NUMBER
                   { if (last_rational.denominator() != 1)  YYERROR;
		     last_int = last_rational.numerator().as_long();
		     if (last_int > 0)  
		       outcome = E->GetOutcomeByIndex(last_int);
		   }
              ;

outcome_info:      { if (!outcome && last_int != 0)  YYERROR; }
            |      outcome_name LBRACE { values.Flush(); } payofflist RBRACE
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
          |        payofflist payoff
          ;

payoff:            NUMBER  { values.Append(last_rational); }   
      ;

%%

void EfgFileReader::yyerror(char *s)   { gout << s << '\n'; }

int EfgFileReader::yylex(void)
{
  char c, d;

  while (1)  {
    do  {
      infile >> c;
    }  while (isspace(c));
 
    if (c == '/')   {
      infile >> d;
      if (d == '/')  {
	do  {
	  infile >> d;
	}  while (d != '\n');
      }
      else if (d == '*')  {
	int done = 0;
	while (!done)  {
	  do {
	    infile >> d;
	  }  while (d != '*');
	  infile >> d;
	  if (d == '/')   done = 1;
	}
      }
      else  {
	infile.unget(d);
	return SLASH;
      }
    }
    else
      break;
  }

  if (isalpha(c))   return c;

  if (c == '"')  {
    infile.unget(c);
    infile >> last_name;

    return NAME;
  }
  else if (isdigit(c) || c == '-')   {
    infile.unget(c);
    infile >> last_rational;
    return NUMBER;
  }
  
  switch (c)   {
    case '{':  return LBRACE;
    case '}':  return RBRACE;
    default:   return c;
  }
}

EfgFileReader::~EfgFileReader()   { }

void EfgFileType(gInput &f, bool &valid, DataType &type)
{
  f.seekp(0);
  static char *prologue = { "EFG 2 " };
  char c;
  for (int i = 0; i < strlen(prologue); i++)  {
    f.get(c);
    if (c != prologue[i])   {
      valid = false;
      return;
    }
  }

  f.get(c);
  switch (c)   {
    case 'D':
      valid = true;
      type = DOUBLE;
      return;
    case 'R':
      valid = true;
      type = RATIONAL;
      return;
    default:
      valid = false;
      return;
  }
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "gstack.imp"
#include "ggrstack.imp"

TEMPLATE class gStack<Node *>;
TEMPLATE class gGrowableStack<Node *>;


#include "glist.imp"

TEMPLATE class gList<gString>;
TEMPLATE class gNode<gString>;

TEMPLATE class gList<gRational>;
TEMPLATE class gNode<gRational>;


