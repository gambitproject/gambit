
%{
/* $Id$ */
#include <stdlib.h>
#include <ctype.h>
#include "gambitio.h"
#include "gstring.h"
#include "rational.h"
#include "ggrstack.h"
#include "glist.h"
#include "extform.h"

%}

%name EfgFileReader

%define MEMBERS    gInput &infile; \
                   gString last_name;  gRational last_rational;  \
                   BaseExtForm *& E; \
                   gGrowableStack<Node *> path; \
                   gList<gString> actions; \
                   gList<gRational> values; \
                   Player *player; Infoset *infoset; Outcome *outcome; \
                   int i;  gString iset_name; \
                   virtual ~EfgFileReader(); \
                   virtual Outcome *NewOutcome(void) = 0; \
                   virtual Outcome *GetOutcome(const gString &) = 0; \
                   virtual void SetOutcome(Outcome *, \
					   const gList<gRational> &) = 0; \
                   virtual void SetActionProbs(Infoset *, \
					       const gList<gRational> &) = 0; \
                   virtual bool CheckActionProbs(Infoset *, \
						 const gList<gRational> &) = 0;


%define CONSTRUCTOR_PARAM    gInput &f, BaseExtForm *& e

%define CONSTRUCTOR_INIT     : infile(f), E(e), path(32)

%token LBRACE
%token RBRACE
%token SLASH

%token NAME
%token NUMBER

%%

efgfile:    header { path.Push(E->RootNode()); } body

header:     NAME { E->SetTitle(last_name); } playerlist

playerlist:   LBRACE RBRACE
          |   LBRACE players RBRACE

players:      NAME  { E->NewPlayer()->SetName(last_name); }
       |      players NAME  { E->NewPlayer()->SetName(last_name); }

body:       node
    |       body node

node:       person_node | chance_node | terminal_node

person_node:
            'p' NAME  { path.Peek()->SetName(last_name); }
                NAME  { player = E->GetPlayer(last_name);
			if (!player)   YYERROR; }
                NAME  { infoset = player->GetInfoset(iset_name = last_name);
		        actions.Flush(); }
                actionlistopt 
                      { if (!infoset)   {
			  if (actions.Length() == 0)   YYERROR;
			  infoset = E->AppendNode(path.Peek(),
						  player, actions.Length());
			  infoset->SetName(iset_name);
			  for (i = 1; i <= actions.Length(); i++)
			    infoset->SetActionName(i, actions[i]);
			}
			else   {
			  if (actions.Length() > 0 && actions.Length() != infoset->NumActions())
			    YYERROR;
			  for (i = 1; i <= actions.Length(); i++)
			    if (actions[i] != infoset->GetActionName(i))
			      YYERROR;
			  E->AppendNode(path.Peek(), infoset);
			}
		      }
                outcome   { path.Push(path.Peek()->GetChild(1)); }

actionlistopt:  { if (!infoset)   YYERROR; }
          | LBRACE actionlist RBRACE

actionlist:  NAME   { actions.Append(last_name); }
          |  actionlist NAME  { actions.Append(last_name); }

chance_node: 'c' NAME  { path.Peek()->SetName(last_name); }
                 NAME  { player = E->GetChance();
			 infoset = player->GetInfoset(iset_name = last_name);
			 actions.Flush();  values.Flush();
		       }
                 actionproblistopt 
                       { if (!infoset)   {
			   if (actions.Length() == 0)  YYERROR;
			   infoset = E->AppendNode(path.Peek(),
						   player, actions.Length());
			   infoset->SetName(iset_name);
			   for (i = 1; i <= actions.Length(); i++) 
			     infoset->SetActionName(i, actions[i]);
			   SetActionProbs(infoset, values);
			 }
			 else   {
			   if (actions.Length() > 0 && actions.Length() != infoset->NumActions())
			     YYERROR;
			   for (i = 1; i <= actions.Length(); i++) 
			     if (actions[i] != infoset->GetActionName(i))
			       YYERROR;
			   if (!CheckActionProbs(infoset, values))   YYERROR;
			   E->AppendNode(path.Peek(), infoset);
			 }
		       }
               outcome  { path.Push(path.Peek()->GetChild(1)); }

actionproblistopt:   { if (!infoset)  YYERROR; }
          | LBRACE actionproblist RBRACE

actionproblist:   actionprob
              |   actionproblist actionprob

actionprob:     NAME NUMBER   { actions.Append(last_name);
				values.Append(last_rational); }

terminal_node:
             't' NAME  { path.Peek()->SetName(last_name); }
                 outcome
              { path.Push(path.Pop()->NextSibling());
		while (path.Depth() > 1 && !path.Peek())   {
		  path.Pop();
		  path.Push(path.Pop()->NextSibling());
		}
	      }

outcome:     NAME   {
               outcome = GetOutcome(last_name);
	       if (!outcome && last_name != "")
		 (outcome = NewOutcome())->SetName(last_name);
	       path.Peek()->SetOutcome(outcome);
	     }
             payofflistopt

payofflistopt:
             | LBRACE  { values.Flush(); } payofflist RBRACE
             { if (values.Length() != E->NumPlayers())   YYERROR;
	       SetOutcome(outcome, values);
	     }

payofflist:    NUMBER   { values.Append(last_rational); }
          |    payofflist NUMBER  { values.Append(last_rational); }

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
  static char *prologue = { "EFG 1 " };
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
