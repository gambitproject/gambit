
%{
/* $Id$ */
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include "gstring.h"
#include "problem.h"

Problem *the_problem;
gString *last_name;
int last_int;
double last_double;

/* Here are some scratch variables... */

int playerNo, gameNo;
gTuple<gNumber> *v;

gInput *input_stream;

int efg_set_input(const gString &s)
{
  input_stream = new gInput((const char *) s);

  if (*input_stream == gInput((FILE *)0))  {
    delete input_stream;
    return 0;
  }

  last_name = new gString;

  return 1;
}

void efg_close_input(void)
{
  delete input_stream;
  input_stream = 0;
  delete last_name;
}

void efg_set_problem(Problem *p)
{
  the_problem = p;
}

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

efgfile:    LBRACE NAME 
            players outcomes games RBRACE  { return 0; }

/* Parsing the player list */

players:    LBRACE chance_name RBRACE
       |    LBRACE chance_name player_names RBRACE

chance_name:  NAME   { the_problem->SetPlayerName(0, *last_name); }

player_names: player_name
            | player_names player_name

player_name:  NAME   { the_problem->AppendPlayer(*last_name); }

/* Parsing the outcome list */
           
outcomes:   LBRACE RBRACE 
        |   LBRACE outcome_list RBRACE 

outcome_list:  outcome
            |  outcome_list outcome

outcome:    LBRACE INTEGER  { the_problem->CreateOutcome(last_int);
            v = new gTuple<gNumber>(1, the_problem->NumPlayers());
            playerNo = 1; }
            outcome_vector { the_problem->SetOutcomeValues(last_int, *v); }
            NAME  { the_problem->SetOutcomeName(last_int, *last_name); }
            RBRACE

outcome_vector:  LBRACE RBRACE
              |  LBRACE outcome_values RBRACE

outcome_values:  outcome_value
              |  outcome_values outcome_value

outcome_value:   FLOAT   { (*v)[playerNo++] = last_double; }

/* Parsing the game list */

games:      game 
     |      games game

game:       LBRACE INTEGER  { gameNo = last_int;
                              the_problem->CreateGame(gameNo); }
            NAME  { the_problem->SetGameName(gameNo, *last_name); }
            infosets nodes
            RBRACE

infosets:   LBRACE { playerNo = 0; } players_isets RBRACE

players_isets:  player_isets
             |  players_isets player_isets


player_isets:   LBRACE RBRACE   { playerNo++; }
            |   LBRACE iset_list RBRACE   { playerNo++; }

iset_list:      infoset
         |      iset_list infoset

infoset:        LBRACE NAME action_data RBRACE

action_data:    action_list prob_list
           |    action_list

action_list:    LBRACE actions RBRACE

actions:        action
       |        actions action

action:         NAME

prob_list:      LBRACE probs RBRACE

probs:          prob
     |          probs prob

prob:           FLOAT


nodes:          LBRACE node_list RBRACE

node_list:      node
         |      node_list node

node:           node_ID COLON node_ID INTEGER INTEGER INTEGER NAME

node_ID:        LPAREN INTEGER COMMA INTEGER COMMA INTEGER RPAREN

%%

int yylex(void)
{
  char c, d;

  while (1)  {
    do  {
      *input_stream >> c;
    }  while (isspace(c));
 
    if (c == '/')   {
      *input_stream >> d;
      if (d == '/')  {
	do  {
	  *input_stream >> d;
	}  while (d != '\n');
      }
      else if (d == '*')  {
	int done = 0;
	while (!done)  {
	  do {
	    *input_stream >> d;
	  }  while (d != '*');
	  *input_stream >> d;
	  if (d == '/')   done = 1;
	}
      }
      else  {
	input_stream->unget(d);
	return SLASH;
      }
    }
    else
      break;
  }
  
  if (isalpha(c) || c == '"')  {
    input_stream->unget(c);
    *input_stream >> *last_name;

    return NAME;
  }
  else if (isdigit(c) || c == '-')   {
    gString s(c);

    *input_stream >> c;

    if (!isdigit(c) && s == "-")  { input_stream->unget(c);  return '-';  }

    while (isdigit(c))   {
      s += c;
      *input_stream >> c;
    }

    if (c == '.')   {    // floating-point number
      s += c;

      *input_stream >> c;

      while (isdigit(c))   {
	s += c;
	*input_stream >> c;
      }
      input_stream->unget(c);
      last_double = atof((const char *) s);
      return FLOAT;
    }
    else   {   // integer number
      input_stream->unget(c);
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
