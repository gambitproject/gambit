
%{
/* $Id$ */
#include <stdlib.h>
#include <ctype.h>
#include "gambitio.h"
#include "gstring.h"
#include "rational.h"
#include "gnumber.h"
#include "gstack.h"
#include "glist.h"
#include "efg.h"


#include "gstack.imp"

template class gStack<Node *>;
%}

%name EfgFileReader

%define MEMBERS    gInput &infile; \
                   gString last_name, last_poly;  gNumber last_number;  \
                   int last_int, iset_idx; \
		   bool polymode; \
                   Efg *& E; \
		   gString title, comment; \
                   gStack<Node *> path; \
                   gList<gString> actions, players, params; \
                   gList<gPoly<gNumber> > values; \
                   EFPlayer *player; Infoset *infoset; EFOutcome *outcome; \
                   int i;  gString iset_name, outc_name; \
                   virtual ~EfgFileReader(); \
                   EFOutcome *NewOutcome(void); \
                   void SetOutcome(EFOutcome *, \
			           const gList<gPoly<gNumber> > &); \
                   void SetActionProbs(Infoset *, \
			               const gList<gPoly<gNumber> > &); \
                   bool CheckActionProbs(Infoset *, \
					 const gList<gPoly<gNumber> > &);\
                   bool CheckOutcome(EFOutcome *, \
				     const gList<gPoly<gNumber> > &); \
	           bool ExistsVariable(const gString &); \
		   int Parse(void); \
                   void CreateEfg(void);


%define CONSTRUCTOR_PARAM    gInput &f, Efg *& e

%define CONSTRUCTOR_INIT     : infile(f), polymode(false), E(e), path(32)

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

header:            NAME  { title = last_name; }  playerlist paramsopt commentopt
      ;

playerlist:        LBRACE RBRACE
          |        LBRACE players RBRACE
          ;
players:           player
       |           players player
       ;

player:            NAME   { players.Append(last_name); }
      ;

paramsopt:        
         |         LBRACE paramlist RBRACE
         ;

paramlist:         NAME   { params.Append(last_name); }
         |         paramlist NAME  { params.Append(last_name); }
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
                   { if (((gRational) last_number).denominator() != 1)  YYERROR;
		     last_int = ((gRational) last_number).numerator().as_long();
		     if (last_int <= 0 || last_int > E->NumPlayers()) YYERROR;
		     player = E->Players()[last_int];
		   }
             ;

infoset_number:    NUMBER
                   { if (((gRational) last_number).denominator() != 1)  YYERROR;
		     iset_idx = ((gRational) last_number).numerator().as_long();
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
                     values.Append(gPoly<gNumber>(E->Parameters(),
     						  last_number,
 						  E->ParamOrder())); }	
          ;
 
outcome_number:    NUMBER
                   { if (((gRational) last_number).denominator() != 1)  YYERROR;
		     last_int = ((gRational) last_number).numerator().as_long();
		     if (last_int > 0)  
		       outcome = E->GetOutcomeByIndex(last_int);
		   }
              ;

outcome_info:      { if (!outcome && last_int != 0)  YYERROR; }
            |      outcome_name LBRACE 
                   { values.Flush(); polymode = true; } payofflist
                   RBRACE
                   { polymode = false;
                     if (values.Length() != E->NumPlayers())   YYERROR;
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

payoff:            NUMBER 
    { values.Append(gPoly<gNumber>(E->Parameters(), last_number, E->ParamOrder())); }
      |            polynomial 
    { values.Append(gPoly<gNumber>(E->Parameters(), last_poly, E->ParamOrder())); gout << last_poly << '\n'; last_poly = ""; }
      |            NUMBER '+'
    { last_poly = ToString(last_number) + " + "; }
                  polynomial
    { values.Append(gPoly<gNumber>(E->Parameters(), last_poly, E->ParamOrder())); gout << last_poly << '\n'; last_poly = ""; }
      |            NUMBER '-'
    { last_poly = ToString(last_number) + " - "; }
                  polynomial
    { values.Append(gPoly<gNumber>(E->Parameters(), last_poly, E->ParamOrder())); gout << last_poly << '\n'; last_poly = ""; }
      ;

polynomial:        polyterm
          |        polynomial '+' { last_poly += "+ "; } polyterm
          |        polynomial '-' { last_poly += "- "; } polyterm
          ;

polyterm:          NUMBER { last_poly += ToString(last_number) + ' '; }
                   variables
        ;

variables:         variable
         |         variables variable
         ;

variable:          varname
                   { last_poly += ' '; }
        |          varname '^' NUMBER
                   { last_poly += "^" + ToString(last_number) + ' '; }
        ;

varname:           VARNAME
                   { if (!ExistsVariable(last_name))  YYERROR;
                     last_poly += last_name; }

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

  if (isalpha(c))   {
    if (!polymode) 
      return c;	
    last_name = c;
    infile >> c;
    while (isalpha(c))   {
      last_name += c;
      infile >> c;
    }  
    infile.unget(c);
    return VARNAME;
  }   

  if (c == '"')  {
    infile.unget(c);
    infile >> last_name;

    return NAME;
  }
  else if (isdigit(c) || c == '-')   {
    infile.unget(c);
    infile >> last_number;
    return NUMBER;
  }
  
  switch (c)   {
    case '-':  infile >> c;
               if (isdigit(c))  {
                 infile.unget(c);
                 infile >> last_number;
		 last_number = -last_number;
                 return NUMBER;
               }
               else  {
                 infile.unget(c);
                 return '-';
               }
    case '{':  return LBRACE;
    case '}':  return RBRACE;
    default:   return c;
  }
}

EfgFileReader::~EfgFileReader()   { }


EFOutcome *EfgFileReader::NewOutcome(void)
{
  return E->NewOutcome();
}

void EfgFileReader::SetOutcome(EFOutcome *c, const gList<gPoly<gNumber> > &p)
{
  for (int i = 1; i <= p.Length(); i++)
    E->SetPayoff(c, i, p[i]);
}

void EfgFileReader::SetActionProbs(Infoset *s,
	 			   const gList<gPoly<gNumber> > &p)
{
  gArray<gNumber> zeroes(E->Parameters()->Dmnsn());
  for (int i = 1; i <= p.Length(); i++)
    E->SetChanceProb(s, i, p[i].Evaluate(zeroes));
}

bool EfgFileReader::CheckActionProbs(Infoset *s, 
                                     const gList<gPoly<gNumber> > &p)
{
  gArray<gNumber> zeroes(E->Parameters()->Dmnsn());	
  for (int i = 1; i <= p.Length(); i++)
    if (E->GetChanceProb(s, i) != p[i].Evaluate(zeroes))  return false;
  return true;
}

bool EfgFileReader::CheckOutcome(EFOutcome *c, const gList<gPoly<gNumber> > &p)
{
  for (int i = 1; i <= p.Length(); i++)
    if (E->Payoff(c, i) != p[i])   return false;
  return true;
}

bool EfgFileReader::ExistsVariable(const gString &varname)
{
  for (int var = 1; var <= E->Parameters()->Dmnsn(); var++)
    if (E->Parameters()->GetVariableName(var) == varname)
      return true;
  return false;
}

int EfgFileReader::Parse(void)
{
  infile.seekp(0);
  static char *prologue = { "EFG 2 " };
  char c;
  for (unsigned int i = 0; i < strlen(prologue); i++)   {
    infile.get(c);
    if (c != prologue[i])   return 1;
  }

  infile.get(c);
  if (c != 'D' && c != 'R')   return 1;
  return yyparse();
}

void EfgFileReader::CreateEfg(void)
{
  gSpace *space = new gSpace(params.Length());
  for (int var = 1; var <= params.Length(); var++)
    space->SetVariableName(var, params[var]);
  ORD_PTR ord = &lex;
  E = new Efg(space, new term_order(space, ord));

  E->SetTitle(title);
  E->SetComment(comment);
  for (int pl = 1; pl <= players.Length(); pl++)
    E->NewPlayer()->SetName(players[pl]);
}	

int ReadEfgFile(gInput &f, Efg *& E)
{
  assert(!E);

  EfgFileReader R(f, E);
  
  if (R.Parse())  {
    if (E)  {  delete E;  E = 0; }
    return 0;
  }

  return 1;
}




