
%{
/* $Id$ */
#include <stdlib.h>
#include <ctype.h>
#include "gstring.h"
#include "gametree.h"
#include "extform.h"
#include "outcome.h"
#include "rational.h"

%}

%name EfgFileReader

%define MEMBERS   gInput &infile;  BaseExtForm *E;  \
                  gString last_name;  int last_int;  double last_double; \
                  gRational last_rational; \
                  int playerNo, gameNo, isetNo, actNo, outcNo, errCode; \
                  Node node; \
                  struct nodeinfo info; \
                  gVector<double> *outc_dbl; \
                  gVector<gRational> *outc_rat; \
                  gTuple<double> *prob_dbl;  gTuple<gRational> *prob_rat; \
                  gBlock<struct nodeinfo *> *nodelist; \
                            \
                  BaseExtForm *ReadEfgFile(void);

%define CONSTRUCTOR_PARAM  gInput &f

%define CONSTRUCTOR_INIT   : infile(f)

%token LPAREN    257
%token RPAREN    258
%token COMMA     259
%token COLON     260
%token LBRACE    261
%token RBRACE    262
%token SLASH     263

%token TYPEFLOAT 264
%token TYPERAT   265

%token NAME      270
%token INTEGER   271
%token FLOAT     272
%token RAT       273

%%

efgfile:    LBRACE NAME type { E->SetTitle(last_name); } 
            players outcomes games RBRACE  { return 0; }

type:           { E = new ExtForm<double>(1); }
    |       TYPEFLOAT    { E = new ExtForm<double>(1); }
    |       TYPERAT      { E = new ExtForm<gRational>(1); }

/* Parsing the player list */

players:    LBRACE chance_name RBRACE
       |    LBRACE chance_name player_names RBRACE

chance_name:  NAME   { E->players[0] = last_name; }

player_names: player_name
            | player_names player_name

player_name:  NAME 
           { E->players.Expand(1);  E->players[E->players.Last()] = last_name; }

/* Parsing the outcome list */
           
outcomes:   LBRACE RBRACE 
        |   LBRACE outcome_list RBRACE 

outcome_list:  outcome
            |  outcome_list outcome

outcome:    LBRACE INTEGER  { E->CreateOutcome(outcNo = last_int);
            switch (E->Type())   {
              case DOUBLE:
                outc_dbl = new gVector<double>(E->NumPlayers());
		break;
	      case RATIONAL:
		outc_rat = new gVector<gRational>(E->NumPlayers());
		break;
	    }
            playerNo = 1; }
            outcome_vector
            { switch (E->Type())   {
   	        case DOUBLE:
	          ((ExtForm<double> *) E)->SetOutcomeValues(outcNo, *outc_dbl);  delete outc_dbl;
		  break;
		case RATIONAL:
		  ((ExtForm<gRational> *) E)->SetOutcomeValues(outcNo, *outc_rat);
		  delete outc_rat;
		  break;
		}
	    }
            NAME  { E->LabelOutcome(outcNo, last_name); }
            RBRACE

outcome_vector:  LBRACE RBRACE
              |  LBRACE outcome_values RBRACE

outcome_values:  outcome_value
              |  outcome_values outcome_value

outcome_value:   FLOAT  
          { switch (E->Type())   {
              case DOUBLE:
	        (*outc_dbl)[playerNo++] = last_double;
	        break;
	      case RATIONAL:
	        (*outc_rat)[playerNo++] = last_double;
	        break;
            }
	  }
             |   INTEGER  
          { switch (E->Type())   {
   	      case DOUBLE:
	        (*outc_dbl)[playerNo++] = last_int;
		break;
	      case RATIONAL:
		(*outc_rat)[playerNo++] = last_int;
		break;
	      }
	  }
             |   RAT 
          { switch (E->Type())   {
 	      case DOUBLE:
	        (*outc_dbl)[playerNo++] = (double) last_rational;
		break;
	      case RATIONAL:
		(*outc_rat)[playerNo++] = last_rational;
		break;
	      }
	  }

/* Parsing the game list */

games:      game 
     |      games game

game:       LBRACE INTEGER  { gameNo = last_int;
                              E->CreateGame(gameNo, 1); }
            NAME  { E->LabelGame(gameNo, last_name); }
            infosets nodes
            RBRACE

infosets:   LBRACE { playerNo = 0; } players_isets RBRACE

players_isets:  player_isets
             |  players_isets player_isets


player_isets:   LBRACE { isetNo = 1; } RBRACE   { playerNo++; }
            |   LBRACE { isetNo = 1; } iset_list RBRACE   { playerNo++; }

iset_list:      infoset
         |      iset_list infoset

infoset:        LBRACE { E->CreateInfoset(playerNo, gameNo, 0); }
                NAME { E->LabelInfoset(gameNo, playerNo, isetNo, last_name); } action_data RBRACE  { isetNo++; }

action_data:    action_list prob_list
           |    action_list

action_list:    LBRACE { actNo = 1; } actions RBRACE

actions:        action
       |        actions action

action:         NAME
             { E->AppendAction(gameNo, playerNo, isetNo);
               E->LabelAction(gameNo, playerNo, isetNo, actNo++, last_name); }

prob_list:      LBRACE
            { switch (E->Type())   {
	        case DOUBLE:
	          prob_dbl = new gTuple<double>(1, actNo - 1);
		  break;
		case RATIONAL:
		  prob_rat = new gTuple<gRational>(1, actNo - 1);
		  break;
	      }
	      actNo = 1;
	    }
                probs RBRACE
            { switch (E->Type())   {
	        case DOUBLE:
	          ((ExtForm<double> *) E)->SetActionProbs(gameNo, isetNo, *prob_dbl);
		  delete prob_dbl;
		  break;
		case RATIONAL:
		  ((ExtForm<gRational> *) E)->SetActionProbs(gameNo, isetNo, *prob_rat);
		  delete prob_rat;
		  break;
                }
	    }

probs:          prob
     |          probs prob

prob:           FLOAT  { 
                  switch (E->Type())  {
		    case DOUBLE:
		      (*prob_dbl)[actNo++] = last_double;
		      break;
		    case RATIONAL:
		      (*prob_rat)[actNo++] = last_double;
		      break;
                  }
		}
    |           RAT   {
                  switch (E->Type())  {
		    case DOUBLE:
		      (*prob_dbl)[actNo++] = (double) last_rational;
		      break;
		    case RATIONAL:
		      (*prob_rat)[actNo++] = last_rational;
		      break;
		    }
		}

nodes:          LBRACE { nodelist = new gBlock<struct nodeinfo *>; } node_list
                RBRACE
      { errCode = E->nodes(gameNo)->InputFromFile(*nodelist);
        while (nodelist->Length())  delete nodelist->Remove(1); 
        delete nodelist;
        if (errCode)   YYERROR;  }

node_list:      node
         |      node_list node

node:           node_ID   { info.my_ID = node; } COLON
                node_ID   { info.parent_ID = node; }
                INTEGER   { info.child_no = last_int; }
                INTEGER   { info.nextgame = last_int; }
                INTEGER   { info.outcome = (last_int) ? E->outcomes[last_int] : 0; }
                NAME      { info.name = last_name;
                            nodelist->Append(new struct nodeinfo(info)); }

node_ID:        LPAREN INTEGER { node[1] = last_int; } COMMA
                INTEGER { node[2] = last_int; } COMMA 
                INTEGER { node[3] = last_int; } RPAREN


%%

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
  
  if (isalpha(c) || c == '"')  {
    infile.unget(c);
    gString s;
    infile >> s;

    if (s == "FLOAT")    return TYPEFLOAT;
    else if (s == "RATIONAL")   return TYPERAT;

    last_name = s;

    return NAME;
  }
  else if (isdigit(c) || c == '-')   {
    gString s(c);

    infile >> c;

    if (!isdigit(c) && s == "-")  { infile.unget(c);  return '-';  }

    while (isdigit(c))   {
      s += c;
      infile >> c;
    }

    if (c == '.')   {    // floating-point number
      s += c;

      infile >> c;

      while (isdigit(c))   {
	s += c;
	infile >> c;
      }
      infile.unget(c);
      last_double = atof((const char *) s);
      return FLOAT;
    }
    else if (c == '/')  {    // rational number
      int num = atoi((const char *) s);
   
      s = "";
      
      infile >> c;
      while (isdigit(c))   {
	s += c;
	infile >> c;
      }
      infile.unget(c);
      last_int = atoi((const char *) s);
      last_rational = gRational(num, last_int);
      return RAT;
    }
    else   {   // integer number
      infile.unget(c);
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

void EfgFileReader::yyerror(char *s)
{
  fprintf(stderr, "Error: %s.\n", s);
}

BaseExtForm *EfgFileReader::ReadEfgFile(void)
{
  if (yyparse())  {
    delete E;
    return 0;
  }
  return E;
}
