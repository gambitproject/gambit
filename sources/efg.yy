
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

int foo, bar;


input *input_stream;

void efg_set_input(const gString &s)
{
  input_stream = new input((const char *) s);
  last_name = new gString;
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

void efg_yyerror(char *s)
{
  fprintf(stderr, "Error: %s.\n", s);
}

int efg_yylex(void);


// dirty nasty trick to have more than one yaccer in a binary
#define yyerror   efg_yyerror
#define yylex     efg_yylex
#define yyparse   efg_yyparse

%}


%token LPAREN    257
%token RPAREN    258
%token COMMA     259
%token COLON     260
%token LBRACE    261
%token RBRACE    262

%token NAME      270
%token INTEGER   271
%token FLOAT     272

%%

efgfile:    LBRACE NAME { the_problem->SetTitle(*last_name); }
            players outcomes games RBRACE  { return 0; }

/* Parsing the player list */

players:    LBRACE chance_name RBRACE
       |    LBRACE chance_name player_names RBRACE

chance_name:  NAME   { the_problem->SetPlayerName(0, *last_name); }

player_names: NAME   { the_problem->AppendPlayer(*last_name); }
            | player_names NAME   { the_problem->AppendPlayer(*last_name); }

/* Parsing the outcome list */
           
outcomes:   LBRACE RBRACE 
        |   LBRACE outcome_list RBRACE 

outcome_list:  outcome
            |  outcome_list outcome

outcome:    LBRACE INTEGER  { the_problem->CreateOutcome(bar = last_int); }
            outcome_vector
            NAME { the_problem->SetOutcomeName(bar, *last_name); }
            RBRACE

outcome_vector:  LBRACE RBRACE
              |  LBRACE { foo = 1; } outcome_values RBRACE

outcome_values:  outcome_value
              |  outcome_values outcome_value

outcome_value:   FLOAT
                { if (foo <= the_problem->NumPlayers())
                    the_problem->SetOutcomeValue(bar, foo++, last_double);
		  else  YYERROR;
                }


/* Parsing the game list */

games:      game 
     |      games game

game:       LBRACE INTEGER
            NAME   { the_problem->CreateGame(last_int, 1);
                     the_problem->SetGameName(last_int, *last_name);
                     if (the_problem->ReadExtForm(last_int))  return 1;  }
            RBRACE

%%

int yylex(void)
{
  char c, d;

  while (1)  {
    do  {
      *input_stream >> c;
    }  while (isspace(c));
    

    break;
/*
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
      else
	    // unget character d
	    // return SLASH
      }	
    */
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
