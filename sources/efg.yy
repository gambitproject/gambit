
%{
/* $Id$ */
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include "gstring.h"
#include "extform.h"
#include "node.h"
#include "outcome.h"

gString last_name;
int last_int;
double last_double;

/* Here are some scratch variables... */

int playerNo, gameNo, isetNo, actNo, outcNo, errCode;
Node node;
struct nodeinfo info;
gTuple<gNumber> *v;
gBlock<struct nodeinfo *> *nodelist;

gInput *efg_input_stream;

void yyerror(char *s)
{
  fprintf(stderr, "Error: %s.\n", s);
}

int yylex(void);
%}


%token LPAREN    257
%token RPAREN    258
%token COMMA     259
%token COLON     260
%token LBRACE    261
%token RBRACE    262
%token SLASH     263

%token NAME      270
%token INTEGER   271
%token FLOAT     272

%%

efgfile:    LBRACE NAME  { title = last_name; } 
            players outcomes games RBRACE  { return 0; }

/* Parsing the player list */

players:    LBRACE chance_name RBRACE
       |    LBRACE chance_name player_names RBRACE

chance_name:  NAME   { players[0] = last_name; }

player_names: player_name
            | player_names player_name

player_name:  NAME 
           { players.Expand(1);  players[players.Last()] = last_name; }

/* Parsing the outcome list */
           
outcomes:   LBRACE RBRACE 
        |   LBRACE outcome_list RBRACE 

outcome_list:  outcome
            |  outcome_list outcome

outcome:    LBRACE INTEGER  { CreateOutcome(outcNo = last_int);
            v = new gTuple<gNumber>(1, NumPlayers());
            playerNo = 1; }
            outcome_vector
            { SetOutcomeValues(outcNo, *v);  delete v; }
            NAME  { LabelOutcome(outcNo, last_name); }
            RBRACE

outcome_vector:  LBRACE RBRACE
              |  LBRACE outcome_values RBRACE

outcome_values:  outcome_value
              |  outcome_values outcome_value

outcome_value:   FLOAT    { (*v)[playerNo++] = last_double; }
             |   INTEGER  { (*v)[playerNo++] = last_int; }

/* Parsing the game list */

games:      game 
     |      games game

game:       LBRACE INTEGER  { gameNo = last_int;
                              CreateSubgame(gameNo, 1); }
            NAME  { LabelSubgame(gameNo, last_name); }
            infosets nodes
            RBRACE

infosets:   LBRACE { playerNo = 0; } players_isets RBRACE

players_isets:  player_isets
             |  players_isets player_isets


player_isets:   LBRACE { isetNo = 1; } RBRACE   { playerNo++; }
            |   LBRACE { isetNo = 1; } iset_list RBRACE   { playerNo++; }

iset_list:      infoset
         |      iset_list infoset

infoset:        LBRACE { CreateInfoset(playerNo, gameNo, 0); }
                NAME { LabelInfoset(gameNo, playerNo, isetNo, last_name); } action_data RBRACE  { isetNo++; }

action_data:    action_list prob_list
           |    action_list

action_list:    LBRACE { actNo = 1; } actions RBRACE

actions:        action
       |        actions action

action:         NAME
             { AppendAction(gameNo, playerNo, isetNo);
               LabelAction(gameNo, playerNo, isetNo, actNo++, last_name); }

prob_list:      LBRACE
            { v = new gTuple<gNumber>(1, actNo - 1); actNo = 1; }
                probs RBRACE
            { SetActionProbs(gameNo, isetNo, *v);
              delete v;  }

probs:          prob
     |          probs prob

prob:           FLOAT  { (*v)[actNo++] = last_double; }

nodes:          LBRACE { nodelist = new gBlock<struct nodeinfo *>; } node_list
                RBRACE
      { errCode = nodes(gameNo)->InputFromFile(*nodelist);
        while (nodelist->Length())  delete nodelist->Remove(1); 
        delete nodelist;
        if (errCode)   YYERROR;  }

node_list:      node
         |      node_list node

node:           node_ID   { info.my_ID = node; } COLON
                node_ID   { info.parent_ID = node; }
                INTEGER   { info.child_no = last_int; }
                INTEGER   { info.nextgame = last_int; }
                INTEGER   { info.outcome = last_int; }
                NAME      { info.name = last_name;
                            nodelist->Append(new struct nodeinfo(info)); }

node_ID:        LPAREN INTEGER { node[1] = last_int; } COMMA
                INTEGER { node[2] = last_int; } COMMA 
                INTEGER { node[3] = last_int; } RPAREN


%%

int yylex(void)
{
  char c, d;

  while (1)  {
    do  {
      *efg_input_stream >> c;
    }  while (isspace(c));
 
    if (c == '/')   {
      *efg_input_stream >> d;
      if (d == '/')  {
	do  {
	  *efg_input_stream >> d;
	}  while (d != '\n');
      }
      else if (d == '*')  {
	int done = 0;
	while (!done)  {
	  do {
	    *efg_input_stream >> d;
	  }  while (d != '*');
	  *efg_input_stream >> d;
	  if (d == '/')   done = 1;
	}
      }
      else  {
	efg_input_stream->unget(d);
	return SLASH;
      }
    }
    else
      break;
  }
  
  if (isalpha(c) || c == '"')  {
    efg_input_stream->unget(c);
    *efg_input_stream >> last_name;

    return NAME;
  }
  else if (isdigit(c) || c == '-')   {
    gString s(c);

    *efg_input_stream >> c;

    if (!isdigit(c) && s == "-")  { efg_input_stream->unget(c);  return '-';  }

    while (isdigit(c))   {
      s += c;
      *efg_input_stream >> c;
    }

    if (c == '.')   {    // floating-point number
      s += c;

      *efg_input_stream >> c;

      while (isdigit(c))   {
	s += c;
	*efg_input_stream >> c;
      }
      efg_input_stream->unget(c);
      last_double = atof((const char *) s);
      return FLOAT;
    }
    else   {   // integer number
      efg_input_stream->unget(c);
      last_int = atoi((const char *) s);
      return INTEGER;
    }
  }
  
  switch (c)   {
    case '(':  return LPAREN;
    case ')':  return RPAREN;
    case ',':  return COMMA;
    case ':':  return COLON;
    case '{':  return LBRACE;
    case '}':  return RBRACE;
    default:   return c;
  }
}
