
%{
/* $Id$ */
#include <ctype.h>
#include "gmisc.h"
#include "gambitio.h"
#include "glist.h"
#include "rational.h"
#include "nfg.h"

%}

%name NfgFileReader

%define MEMBERS      gInput &infile;  \
                     gString last_name;  gRational last_rational; \
                     gString title; \
                     Nfg *& Nrat; \
                     int ncont, pl, cont; \
                     gList<gString> names; \
                     gList<gRational> numbers; \
                     gList<gString> stratnames; \
                     \
                     virtual bool CreateNfg(const gList<gString> &, \
					    const gList<gRational> &, \
					    const gList<gString> &) = 0; \
                     virtual void SetPayoff(int cont, int pl, \
					    const gRational &) = 0; \
                     virtual ~NfgFileReader();

%define CONSTRUCTOR_PARAM     gInput &f, Nfg *& Nr

%define CONSTRUCTOR_INIT      : infile(f), Nrat(Nr)

%token LBRACE
%token RBRACE
%token SLASH
%token NAME
%token NUMBER

%%

nfgfile:      header 
              { if (!CreateNfg(names, numbers, stratnames))  return 1;
		names.Flush();  numbers.Flush();  stratnames.Flush();
	        Nrat->SetTitle(title);
              }              
              body  { return 0; }

header:       NAME { title = last_name; pl = 0; }  playerlist  stratlist

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


dimensionality:   LBRACE intlist RBRACE

intlist:      integer
       |      intlist integer

integer:      NUMBER  { numbers.Append(last_rational); }


body:         { cont = 1;
                pl = 1; }
              payofflist

payofflist:   payoff
          |   payofflist payoff

payoff:       NUMBER
                {  if (pl > Nrat->NumPlayers())   {
		    cont++;
		    pl = 1;
		  }	
		if (cont > ncont)  YYERROR;
		SetPayoff(cont, pl, last_rational);
		pl++;
	      }

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


NfgFileReader::~NfgFileReader()   { }

#include "nfplayer.h"
#include "nfstrat.h"

class NfgFile : public NfgFileReader   {
  private:
    Nfg *fooR;

  public:
    NfgFile(gInput &, Nfg *&);
    virtual ~NfgFile();

    int Parse(void);
    bool CreateNfg(const gList<gString> &, const gList<gRational> &,
		   const gList<gString> &);
    void SetPayoff(int cont, int pl, const gRational &);
};

NfgFile::NfgFile(gInput &f, Nfg *& N)
  : NfgFileReader(f, N), fooR(N)
{ }

NfgFile::~NfgFile()
{ }

bool NfgFile::CreateNfg(const gList<gString> &players,
			const gList<gRational> &dims,
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
  
  Nrat = new Nfg(dim);
  int strat = 1;
  for (i = 1; i <= dim.Length(); i++)  {
    Nrat->Players()[i]->SetName(players[i]);
    if (strats.Length() > 0)
      for (int j = 1; j <= dim[i]; j++)
	Nrat->Strategies(i)[j]->name = strats[strat++];
  }

  return true;
}

void NfgFile::SetPayoff(int cont, int pl,
			const gRational &value)
{
  if (pl == 1)
    Nrat->SetOutcome(cont, Nrat->NewOutcome());
  Nrat->SetPayoff(Nrat->GetOutcome(cont), pl, value);
}


int NfgFile::Parse(void)
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

  NfgFile R(f, Nrat);

  if (R.Parse())   {
    if (Nrat)   { delete Nrat;  Nrat = 0; }
    return 0;
  }
   
  return 1;
}

