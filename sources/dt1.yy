
%{
/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include "gstring.h"
#include "dt1strct.h"
#include "dt1util.h"

gString last_name;
int last_int;
double last_double;
gInput *input_stream;
int i, j;
int cGames, cNodes, cISets, cPlayers, cOutcomes;
int numGames, numNodes, numISets, numPlayers, numOutcomes;
struct game *vv;
struct node *nn;
struct iset *ii;
struct plyr *pp;
struct outc *oo;
extern struct pblm *whichpblm;
extern struct node *NULLnode;
extern struct plyr *NULLplyr;

void yyerror(char *s)
{
  fprintf(stderr, "Error: %s\n", s);
}

void dt1_set_input(FILE *f)
{
  input_stream = new gInput(f);
}

void dt1_close_input(void)
{
  delete input_stream;
}

int yylex(void);

%}

%token EOLN      269
%token NAME      270
%token INTEGER   271
%token FLOAT     272

%%

dt1file:   NAME EOLN EOLN counts {cGames = 0; cNodes = 0; cISets = 0; cPlayers = 0;
                                  cOutcomes = 0;}
           EOLN games     { if (cGames != numGames) YYERROR; }
           EOLN nodes     { if (cNodes != numNodes) YYERROR; }
           EOLN infosets  { if (cISets != numISets) YYERROR; }
           EOLN players   { if (cPlayers != numPlayers) YYERROR;
                            oo = whichpblm->firstoutcome->nextoutcome;}
           EOLN outcomes   { if (cOutcomes != numOutcomes) YYERROR; return 0;}

counts:    INTEGER { for (i = 1; i < last_int; i++) newgame(); numGames = last_int;}
           INTEGER { for (i = 1; i < last_int; i++) newnode(NULLnode); numNodes = last_int;}
           INTEGER { for (i = 0; i < last_int; i++) newiset(NULLnode, NULLplyr, 0);
                     numISets = last_int;}
           INTEGER { for (i = 0; i < last_int; i++) newplayer(); numPlayers = last_int;}
           INTEGER { for (i = 0; i < last_int; i++) newoutcome(); numOutcomes = last_int;}
           EOLN

games:     game
     |     games game

game:      INTEGER { vv = findgame(last_int);} 
           INTEGER { vv->rootnode = findnode(last_int);}
           NAME    { strncpy(whichpblm->title, (char *) last_name, 80); }
           EOLN    { cGames++;}

nodes:     node 
     |     nodes node 

node:      INTEGER { nn = findnode(last_int);}
           INTEGER { nn->parent = findnode(last_int);}
           INTEGER { nn->firstbranch = findnode(last_int);}
           INTEGER { nn->nextbranch = findnode(last_int);}
           INTEGER { nn->infoset = findiset(last_int);}
           INTEGER { nn->nextmember = findnode(last_int);}
           INTEGER { nn->nextgame = findgame(last_int);}
           INTEGER { nn->outcome = findoutcome(last_int);}
           FLOAT   { nn->probability = last_double;}
           NAME    { strncpy(nn->branchname, (char *) last_name, NAMESIZE);}
           NAME    { strncpy(nn->nodename, (char *) last_name, NAMESIZE); }
           EOLN    { cNodes++;}

infosets:  infoset 
        |  infosets infoset

infoset:   INTEGER { ii = findiset(last_int);}
           INTEGER { ii->nextplyriset = findiset(last_int);}
           INTEGER { ii->firstmember = findnode(last_int);}
           INTEGER { ii->playr = findplyr(last_int);}
           INTEGER { ii->branches = last_int;}
           NAME    { strncpy(ii->isetname, (char *) last_name, NAMESIZE);}
           EOLN    { cISets++;}

players:   player
       |   players player

player:    INTEGER { pp = findplyr(last_int); } 
           INTEGER { pp->firstiset = findiset(last_int); }
           NAME    { strncpy(pp->plyrname, (char *) last_name, NAMESIZE);}
           EOLN    { cPlayers++;}

outcomes:  outcome 
        |  outcomes outcome

outcome:   INTEGER { j = 1;}
           outvector 
           NAME    { strncpy(oo->outname, (char *) last_name, NAMESIZE);} 
           EOLN    { oo = oo->nextoutcome; cOutcomes++; }
                    

outvector: outvalue
         | outvector outvalue

outvalue:  FLOAT   { if (j > whichpblm->nplayers)  YYERROR;
                     oo->component[j++] = last_double; }

%%

int yylex(void)
{
  char c, d;

  do  {
    *input_stream >> c;
  }  while (isspace(c) && c != '\n');
  
  if (c == '\n')    return EOLN;

  if (isalpha(c) || c == '"')  {
    input_stream->unget(c);
    *input_stream >> last_name;

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
      last_double = atof((char *) s);
      return FLOAT;
    }
    else   {   // integer number
      input_stream->unget(c);
      last_int = atoi((char *) s);
      return INTEGER;
    }
  }

  return c;
}
