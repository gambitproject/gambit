
%{
#include <stdlib.h>
#include <ctype.h>
#include "basic.h"
#include "gambitio.h"
#include "gstring.h"
#include "rational.h"
#include "glist.h"
#include "gsm.h"
#include "gsminstr.h"

%}

%name GCLCompiler

%define MEMBERS   \
  int index; \
  gString input_text; \
  bool bval; \
  gInteger ival; \
  double dval; \
  gRational rval; \
  gString tval, formal;  \
  gList<Instruction *> program; \
  GSM gsm; \
  \
  char nextchar(void); \
  void ungetchar(char c); \
  \
  void emit(Instruction *); \
  \
  int Parse(const gString &s); \
  void Execute(void);

%define CONSTRUCTOR_INIT     : index(0), gsm(256)

%token LOR
%token LAND
%token LNOT
%token EQU
%token NEQ
%token LTN
%token LEQ
%token GTN
%token GEQ
%token PLUS
%token MINUS
%token STAR
%token SLASH
%token ASSIGN
%token SEMI
%token LBRACK
%token RBRACK
%token RARROW
%token COMMA

%token NAME
%token BOOLEAN
%token FLOAT
%token INTEGER
%token RATIONAL
%token PERCENT
%token DIV
%token LPAREN
%token RPAREN

%token BOOLEAN
%token INTEGER
%token FLOAT
%token RATCONST
%token TEXT
%token NAME

%%

program:      statements   { emit(new Output); }

statements:   statement
          |   statements SEMI statement

statement:
         |    expression
         |    lvalue ASSIGN expression   { emit(new Assign); }

lvalue:       NAME     { emit(new PushRef(tval)); }

expression:   E1
          |   expression LOR E1  { emit(new OR); }
          ;
E1:           E2
  |           E1 LAND E2  { emit(new AND); } 
  ;

E2:           E3
  |           LNOT E2     { emit(new NOT); }
  ;

E3:           E4       
  |           E3 EQU E4    { emit(new Equ); } 
  |           E3 NEQ E4    { emit(new Neq); }
  |           E3 LTN E4    { emit(new Ltn); }
  |           E3 LEQ E4    { emit(new Leq); }
  |           E3 GTN E4    { emit(new Gtn); } 
  |           E3 GEQ E4    { emit(new Geq); }
  ;

E4:           E5
  |           E4 PLUS E5   { emit(new Add); }
  |           E4 MINUS E5  { emit(new Sub); }
  ;

E5:           E6
  |           E5 STAR E6    { emit(new Mul); }
  |           E5 SLASH E6   { emit(new Div); }
  |           E5 PERCENT E6 { emit(new Mod); }
  |           E5 DIV E6     { emit(new IntDiv); }
  ;

E6:           PLUS E7
  |           MINUS E7      { emit(new Neg); }
  |           E7

E7:           E8
  ;

E8:           BOOLEAN       { emit(new Push<bool>(bval)); }
  |           INTEGER       { emit(new Push<gInteger>(ival)); }
  |           FLOAT         { emit(new Push<double>(dval)); }
  |           RATCONST      { emit(new Push<gRational>(rval)); }
  |           TEXT          { emit(new Push<gString>(tval)); }
  |           LPAREN expression RPAREN
  |           NAME          { emit(new PushRef(tval)); }
  |           function      { emit(new CallFunction()); }
  ;

function:     NAME LBRACK  { emit(new InitCallFunction(tval)); } arglist RBRACK

arglist:
       |      unnamed_args
       |      unnamed_args COMMA named_args
       |      named_args

unnamed_args: unnamed_arg
            | unnamed_args COMMA unnamed_arg

unnamed_arg:  expression  { emit(new Bind()); }

named_args:   named_arg
          |   named_args COMMA named_arg

named_arg:
             NAME RARROW { formal = tval; } expression
                           { emit(new Bind(formal)); }

%%


char GCLCompiler::nextchar(void)
{
  return input_text[index++];
}

void GCLCompiler::ungetchar(char c)
{
  index--;
}

void GCLCompiler::yyerror(char *s)
{
  gerr << "Error: " << s << '\n';
}

int GCLCompiler::yylex(void)
{
  char c;

  c = nextchar();
  while (isspace(c))   c = nextchar();

  if (isalpha(c))  {
    gString s(c);
    c = nextchar();
    while (isalpha(c) || isdigit(c) || c == '_')   {
      s += c;
      c = nextchar();
    }
    ungetchar(c);

    if (s == "True")   {
      bval = true;
      return BOOLEAN;
    }
    else if (s == "False")  {
      bval = false;
      return BOOLEAN;
    }
    else if (s == "AND")    return LAND;
    else if (s == "OR")     return LOR;
    else if (s == "NOT")    return LNOT;
    else if (s == "DIV")    return DIV;
    else if (s == "MOD")    return PERCENT;
    else  { tval = s; return NAME; }
  }

  if (c == '"')   {
    tval = "";
    c = nextchar();
    while (c != '"')   {
      tval += c;
      c = nextchar();
    }
    return TEXT;
  }

  if (isdigit(c))   {
    gString s(c);
    c = nextchar();
    while (isdigit(c))   {
      s += c;
      c = nextchar();
    }

    if (c == '.')   {
      s += c;
      c = nextchar();
      while (isdigit(c))  {
	s += c;
	c = nextchar();
      }

      ungetchar(c);
      dval = atof((char *) s);
      return FLOAT;
    }
    else  {
      ungetchar(c);
      ival = atoi((char *) s);
      return INTEGER;
    }
  }

  switch (c)  {
    case ',':   return COMMA;
    case ';':   return SEMI;
    case '(':   return LPAREN;
    case ')':   return RPAREN;
    case '+':   return PLUS;
    case '-':   c = nextchar();
                if (c == '>')  return RARROW;
                else  { ungetchar(c);  return MINUS; }
    case '*':   return STAR;
    case '/':   return SLASH;
    case '%':   return PERCENT;
    case '=':   return EQU;
    case '[':   return LBRACK;
    case ']':   return RBRACK;
    case ':':   c = nextchar();
                if (c == '=')  return ASSIGN;
                else   { ungetchar(c);  return ':'; }  
    case '!':   c = nextchar();
                if (c == '=')  return NEQ;
		else   { ungetchar(c);  return LNOT; }
    case '<':   c = nextchar();
                if (c == '=')  return LEQ;
                else   { ungetchar(c);  return LTN; }
    case '>':   c = nextchar();
                if (c == '>')  return GEQ;
                else   { ungetchar(c);  return GTN; }
    case '&':   c = nextchar();
                if (c == '&')  return LAND;
                else   { ungetchar(c);  return '&'; }
    case '|':   c = nextchar();
                if (c == '|')  return LOR;
                else   { ungetchar(c);  return '|'; }
    default:    return c;
  }
}

int GCLCompiler::Parse(const gString &s)
{
  input_text = s;
  index = 0;
  if (!yyparse())  {
    Execute();
    return 0;
  }
  else  {
    while (program.Length() > 0)   delete program.Remove(1);
    return 1;
  }
}


void GCLCompiler::emit(Instruction *op)
{
  program.Append(op);
}

void GCLCompiler::Execute(void)
{
  gsm.Execute(program);
  gsm.Flush();
}

