
%{
/* $Id$ */
#include <ctype.h>
#include "gmisc.h"
#include "gambitio.h"
#include "glist.h"
#include "rational.h"
#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"


%}

%name NfgFileReader

%define MEMBERS      gInput &infile;  \
                     gString last_name;  gNumber last_number; \
                     gString title, comment; \
                     Nfg *& N; \
                     int ncont, pl, cont; \
		     gString last_poly; \
                     gList<gString> names, params; \
                     gList<gNumber> numbers; \
                     gList<gString> stratnames; \
                     NFOutcome *outcome; \
                     \
                     bool CreateNfg(const gList<gString> &, \
					    const gList<gNumber> &, \
					    const gList<gString> &); \
                     void SetPayoff(int cont, int pl, \
				    const gPoly<gNumber> &); \
  	             bool ExistsVariable(const gString &); \
		     int Parse(void); \
                     virtual ~NfgFileReader();

%define CONSTRUCTOR_PARAM     gInput &f, Nfg *& nfg

%define CONSTRUCTOR_INIT      : infile(f), N(nfg)

%token LBRACE
%token RBRACE
%token SLASH
%token NAME
%token VARNAME
%token NUMBER

%%

nfgfile:      header 
              { if (!CreateNfg(names, numbers, stratnames))  return 1;
		names.Flush();  numbers.Flush();  stratnames.Flush();
	        N->SetTitle(title); N->SetComment(comment);
              }              
              body  { return 0; }

header:       NAME { title = last_name; pl = 0; }  playerlist 
              stratlist paramsopt commentopt
 
playerlist:   LBRACE players RBRACE

players:      player
       |      players player

player:       NAME   { names.Append(last_name); }

stratlist:    dimensionality
         |    stratnamelist

stratnamelist:  LBRACE playerstrlist RBRACE

playerstrlist:   playerstrats
             |   playerstrlist playerstrats

playerstrats:  LBRACE { pl++; numbers.Append(0); } stratnames RBRACE

stratnames:   stratname
          |   stratnames stratname

stratname:    NAME  { stratnames.Append(last_name); numbers[pl] += 1; }

paramsopt:        
         |         LBRACE paramlist RBRACE
         ;

paramlist:         NAME  { params.Append(last_name); }
         |         paramlist NAME  { params.Append(last_name); }
         ;

commentopt:
          |   NAME   { comment = last_name; }


dimensionality:   LBRACE intlist RBRACE

intlist:      integer
       |      intlist integer

integer:      NUMBER  { numbers.Append(last_number); }

body:         payoffbody | outcomebody


payoffbody:         { cont = 1;
                pl = 1; }
              payofflist

payofflist:   payoff
          |   payofflist payoff

payoff:       NUMBER
                {  if (pl > N->NumPlayers())   {
		    cont++;
		    pl = 1;
		  }	
		if (cont > ncont)  YYERROR;
		SetPayoff(cont, pl, gPoly<gNumber>(N->Parameters(),
                                                   last_number,
                                                   N->ParamOrder()));
		pl++;
	      }

outcomebody:   outcomelist { cont = 1; } contingencylist

outcomelist:   LBRACE RBRACE
           |   LBRACE outcomes RBRACE

outcomes:      outcome
        |      outcomes outcome

outcome:       LBRACE NAME
                 { outcome = N->NewOutcome();
                   outcome->SetName(last_name);  pl = 1; }
               outcpaylist RBRACE

outcpaylist:   outcpay
           |   outcpaylist commaopt outcpay

outcpay:       NUMBER   
                 { if (pl > N->NumPlayers())  YYERROR;
                   N->SetPayoff(outcome, pl++, 
	                        gPoly<gNumber>(N->Parameters(),
                                               last_number,
					       N->ParamOrder())); } 
       |       polynomial
                 { if (pl > N->NumPlayers())  YYERROR;
                   N->SetPayoff(outcome, pl++, 
	                        gPoly<gNumber>(N->Parameters(),
                                               last_poly,
					       N->ParamOrder()));
                   last_poly = ""; } 
       |       NUMBER '+' { last_poly = ToString(last_number) + " + "; }
               polynomial
                 { if (pl > N->NumPlayers())  YYERROR;
                   N->SetPayoff(outcome, pl++, 
	                        gPoly<gNumber>(N->Parameters(),
                                               last_poly,
					       N->ParamOrder()));
                   last_poly = ""; } 
       |       NUMBER '-' { last_poly = ToString(last_number) + " - "; }
               polynomial
                 { if (pl > N->NumPlayers())  YYERROR;
                   N->SetPayoff(outcome, pl++, 
	                        gPoly<gNumber>(N->Parameters(),
                                               last_poly,
					       N->ParamOrder())); 
	           last_poly = ""; } 
 


commaopt:    | ','   
      

contingencylist:  contingency
               |  contingencylist contingency

contingency:   NUMBER
                { if (cont > ncont)  YYERROR;
                  if (last_number != gNumber(0))
                    N->SetOutcome(cont++, N->Outcomes()[last_number]); }
              
polynomial:        polyterm
          |        polynomial '+' { last_poly += "+ "; } polyterm
          |        polynomial '-' { last_poly += "- "; } polyterm
          ;

polyterm:          NUMBER { last_poly += ToString(last_number) + " "; }
                   variables
        ;

variables:         variable
         |         variables variable
         ;

variable:          varname
                   { last_poly += " "; }
        |          varname '^' NUMBER
                   { last_poly += "^" + ToString(last_number) + " "; }
        ;

varname:           VARNAME
                   { if (!ExistsVariable(last_name))  YYERROR;
                     last_poly += last_name; }


%%

void NfgFileReader::yyerror(char *)    { }

int NfgFileReader::yylex(void)
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


NfgFileReader::~NfgFileReader()   { }


bool NfgFileReader::CreateNfg(const gList<gString> &players,
			      const gList<gNumber> &dims,
			      const gList<gString> &strats)
{
  if (players.Length() != dims.Length())   return false;

  gArray<int> dim(dims.Length());
  ncont = 1;
  int i;
  for (i = 1; i <= dim.Length(); i++)  {
    dim[i] = (int) dims[i];
    ncont *= dim[i];
    if (dim[i] <= 0)   return false;
  }
  
  ORD_PTR ord = &lex;
  gSpace *space = new gSpace(params.Length());
  for (int var = 1; var <= params.Length(); var++)
    space->SetVariableName(var, params[var]);
  N = new Nfg(dim, space, new term_order(space, ord));
  int strat = 1;
  for (i = 1; i <= dim.Length(); i++)  {
    N->Players()[i]->SetName(players[i]);
    if (strats.Length() > 0)
      for (int j = 1; j <= dim[i]; j++)
	N->Strategies(i)[j]->name = strats[strat++];
  }

  return true;
}

void NfgFileReader::SetPayoff(int cont, int pl, 
                              const gPoly<gNumber> &value)
{
  if (pl == 1)
    N->SetOutcome(cont, N->NewOutcome());
  N->SetPayoff(N->GetOutcome(cont), pl, value);
}

bool NfgFileReader::ExistsVariable(const gString &varname)
{
  for (int var = 1; var <= N->Parameters()->Dmnsn(); var++)
    if (N->Parameters()->GetVariableName(var) == varname)
      return true;
  return false;
}

int NfgFileReader::Parse(void)
{
  infile.seekp(0);
  static char *prologue = { "NFG 1 " };
  char c;
  for (unsigned int i = 0; i < strlen(prologue); i++)  {
    infile.get(c);
    if (c != prologue[i])  return 1;
  }

  infile.get(c);
  switch (c)   {
    case 'D':
      break;
    case 'R':
      break;
    default:
      return 1;
  }
  return yyparse();
}


int ReadNfgFile(gInput &f, Nfg *& Nrat)
{
  assert(!Nrat);

  NfgFileReader R(f, Nrat);

  if (R.Parse())   {
    if (Nrat)   { delete Nrat;  Nrat = 0; }
    return 0;
  }
   
  return 1;
}

