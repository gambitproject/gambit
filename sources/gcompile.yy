%{
//
// FILE: gcompile.yy -- yaccer/compiler for the GCL
//
// This parser/compiler is dedicated to the memory of
// Jan L. A. van de Snepscheut, who wrote a program after which
// this code is modeled.
//
// $Id$
//

#include <stdlib.h>
#include <ctype.h>
#include "gmisc.h"

#include "gambitio.h"

#include "gstring.h"
#include "rational.h"
#include "glist.h"
#include "gstack.h"
#include "gsm.h"
#include "gsminstr.h"
#include "gsmfunc.h"
#include "portion.h"

#include "system.h"


#include "gstack.imp"


template class gStack<gString>;
template class gStack<int>;
template class gStack<char>;
template class gStack<gInput *>;
template class gStack<unsigned int>;

#include "glist.imp"

template class gList<bool>;
template class gNode<bool>;

template class gList<gclExpression *>;
template class gNode<gclExpression *>;

extern GSM* _gsm;  // defined at the end of gsm.cc
gStack<gString> GCL_InputFileNames(4);

%}

%name GCLCompiler

%define MEMBERS   \
  GSM& gsm; \
  bool record_funcbody, in_funcdecl; \
  int current_char, current_line; \
  gString current_expr, current_file; \
  gString funcbody, funcname, funcdesc, paramtype, functype; \
  gList<gString> formals, types; \
  gList<Portion *> portions; \
  gList<bool> refs; \
  gStack<gString> funcnames; \
  gString tval; \
  gclExpression *exprtree; \
  bool bval; \
  double dval; \
  int ival; \
  \
  char nextchar(void); \
  void ungetchar(char c); \
  \
  gclExpression *DefineFunction(gclExpression *expr); \
  bool DeleteFunction(void); \
  void RecoverFromError(void); \
  \
  int Parse(const gString &line, const gString &file, int lineno); \
  int Execute(void); 

%define CONSTRUCTOR_INIT     : gsm(*_gsm), in_funcdecl(false)

%define CONSTRUCTOR_CODE     

%union  {
  gclExpression *eval;
  gclParameterList *pval;
  gclReqParameterList *rpval; 
  gclOptParameterList *opval;
  gclListConstant *lcval;
}

%type <eval> expression constant function parameter
%type <pval> parameterlist
%type <rpval> reqparameterlist 
%type <opval> optparameterlist
%type <lcval> list listels

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
%token DBLLBRACK
%token RBRACK
%token LBRACE
%token RBRACE
%token RARROW
%token LARROW
%token DBLARROW
%token COMMA
%token HASH
%token DOT
%token CARET
%token UNDERSCORE
%token AMPER
%token WRITE
%token READ

%token PERCENT
%token DIV
%token LPAREN
%token RPAREN

%token IF
%token WHILE
%token FOR
%token QUIT
%token DEFFUNC
%token DELFUNC
%token TYPEDEF
%token INCLUDE

%token NAME
%token BOOLEAN
%token INTEGER
%token FLOAT
%token TEXT
%token STDIN
%token STDOUT
%token gNULL
%token MACHINEPREC
%token RATIONALPREC

%token CRLF
%token EOC

%right  SEMI
%left  UWRITE
%right  ASSIGN
%left  WRITE  READ
%left  LNOT
%left  LOR
%left  LAND
%nonassoc  EQU  NEQ  LTN  LEQ  GTN  GEQ
%left  PLUS  MINUS  AMPER
%left  STAR  SLASH  PERCENT  DIV  DOT  CARET
%left  UMINUS
%left  HASH  UNDERSCORE


%%

program: expression  EOC  { exprtree = $1; return 0; }
       | error EOC    { RecoverFromError(); return 1; }
       | error CRLF   { RecoverFromError(); return 1; }
       ;
 
expression:      constant
          |      function
          |      LPAREN expression RPAREN   { $$ = $2; }
          |      expression SEMI expression
              { $$ = new gclSemiExpr($1, $3); }
          |      expression SEMI
	      { $$ = $1; }
          |      expression ASSIGN expression 
              { $$ = new gclAssignment($1, $3); }
	  |      expression ASSIGN
              { $$ = new gclUnAssignment($1); }
          |      WRITE expression   %prec UWRITE
              { $$ = new gclFunctionCall("Print", $2); } 
          |      expression HASH expression
              { $$ = new gclFunctionCall("NthChild", $1, $3); }
          |      expression UNDERSCORE expression
              { $$ = new gclFunctionCall("NthElement", $1, $3); }
          |      expression PLUS expression
              { $$ = new gclFunctionCall("Plus", $1, $3); }
          |      expression MINUS expression
              { $$ = new gclFunctionCall("Minus", $1, $3); }
          |      expression AMPER expression  
              { $$ = new gclFunctionCall("Concat", $1, $3); }
          |      PLUS expression    %prec UMINUS
              { $$ = $2; }
          |      MINUS expression   %prec UMINUS
              { $$ = new gclFunctionCall("Negate", $2); }
          |      expression STAR expression
              { $$ = new gclFunctionCall("Times", $1, $3); }
          |      expression SLASH expression
              { $$ = new gclFunctionCall("Divide", $1, $3); }
          |      expression PERCENT expression
              { $$ = new gclFunctionCall("Modulus", $1, $3); }
          |      expression DIV expression
              { $$ = new gclFunctionCall("IntegerDivide", $1, $3); }
          |      expression DOT expression
              { $$ = new gclFunctionCall("Dot", $1, $3); }
          |      expression CARET expression
              { $$ = new gclFunctionCall("Power", $1, $3); }
          |      expression EQU expression
              { $$ = new gclFunctionCall("Equal", $1, $3); }
          |      expression NEQ expression
              { $$ = new gclFunctionCall("NotEqual", $1, $3); }
          |      expression LTN expression
              { $$ = new gclFunctionCall("Less", $1, $3); }
          |      expression LEQ expression
              { $$ = new gclFunctionCall("LessEqual", $1, $3); }
          |      expression GTN expression
              { $$ = new gclFunctionCall("Greater", $1, $3); }
          |      expression GEQ expression
              { $$ = new gclFunctionCall("GreaterEqual", $1, $3); }
          |      LNOT expression
              { $$ = new gclFunctionCall("Not", $2); }
          |      expression LAND expression
              { $$ = new gclFunctionCall("And", $1, $3); }
          |      expression LOR expression
              { $$ = new gclFunctionCall("Or", $1, $3); }
	  |      expression WRITE expression
              { $$ = new gclFunctionCall("Write", $1, $3); }
          |      expression READ expression
              { $$ = new gclFunctionCall("Read", $1, $3); }
          ;

function:        IF LBRACK expression COMMA expression COMMA
                           expression RBRACK
              { $$ = new gclConditional($3, $5, $7); } 
        |        IF LBRACK expression COMMA expression RBRACK
              { $$ = new gclConditional($3, $5, 
				new gclConstExpr(new BoolPortion(false))); }
        |        WHILE LBRACK expression COMMA expression RBRACK
              { $$ = new gclWhileExpr($3, $5); }
	|        FOR LBRACK expression COMMA expression COMMA
                            expression COMMA expression RBRACK
              { $$ = new gclForExpr($3, $5, $7, $9); }
        |        DEFFUNC { if (in_funcdecl) YYERROR;  in_funcdecl = true; }
                  LBRACK signature COMMA 
                  { funcbody = ""; record_funcbody = true; }
                  expression RBRACK
                  { record_funcbody = false; in_funcdecl = false;
                    $$ = DefineFunction($7); }
        |        NAME LBRACK  { funcnames.Push(tval); } parameterlist RBRACK
              { $$ = new gclFunctionCall(funcnames.Pop(), $4); }

parameterlist:     { $$ = new gclParameterList; }
             |     reqparameterlist  { $$ = new gclParameterList($1); }
             |     optparameterlist  { $$ = new gclParameterList($1); }
             |     reqparameterlist COMMA optparameterlist
                        { $$ = new gclParameterList($1, $3); }

reqparameterlist:  parameter  { $$ = new gclReqParameterList($1); }
             |     reqparameterlist COMMA parameter  { $1->Append($3); }

parameter:       expression

optparameterlist:  NAME  { funcnames.Push(tval); } arrow expression
                         { $$ = new gclOptParameterList(funcnames.Pop(), $4); }
                |  optparameterlist COMMA NAME  { funcnames.Push(tval); }
	              arrow expression
                         { $1->Append(funcnames.Pop(), $6); }

arrow:         RARROW | DBLARROW

constant:        BOOLEAN 
          { $$ = new gclConstExpr(new BoolPortion(bval)); }
        |        INTEGER   
          { $$ = new gclConstExpr(new IntPortion(ival)); }
        |        FLOAT
          { $$ = new gclConstExpr(new NumberPortion(dval)); }
        |        TEXT
          { $$ = new gclConstExpr(new TextPortion(tval)); }
        |        STDIN
          { $$ = new gclConstExpr(new InputPortion(gin)); }
        |        STDOUT
          { $$ = new gclConstExpr(new OutputPortion(gout)); }
        |        gNULL
          { $$ = new gclConstExpr(new OutputPortion(gnull)); }
        |        MACHINEPREC
          { $$ = new gclConstExpr(new PrecisionPortion(precDOUBLE)); }
        |        RATIONALPREC
          { $$ = new gclConstExpr(new PrecisionPortion(precRATIONAL)); }
        |        NAME
          { $$ = new gclVarName(tval); }
        |        list   { $$ = $1; }
        |        QUIT
          { $$ = new gclQuitExpression; }
        ;

list:            LBRACE RBRACE  { $$ = new gclListConstant; }
    |            LBRACE listels RBRACE  { $$ = $2; }
    ;

listels:         expression   { $$ = new gclListConstant($1); }
       |         listels COMMA expression  { $1->Append($3); }
       ;


signature:       NAME  { funcname = tval; }   LBRACK
                   formallist RBRACK TYPEopt

TYPEopt:         { functype = "ANYTYPE"; }
       |         TYPEDEF  { paramtype = ""; } typename
                 { functype = paramtype; }

typename:        starname
        |        NAME  { paramtype += tval; } optparen

optparen:        
        |        LPAREN { paramtype += '('; } typename
                 RPAREN { paramtype += ')'; }

starname:        NAME  { paramtype += tval; } STAR { paramtype += '*'; }

formallist:      
          |      formalparams

formalparams:    formalparam
            |    formalparams COMMA formalparam

formalparam:     NAME  { formals.Append(tval); }  binding
                 { paramtype = ""; }  typename
                 { types.Append(paramtype); portions.Append(REQUIRED); }
           |     LBRACE NAME  { formals.Append(tval); }  binding
                 { paramtype = ""; types.Append(paramtype); }
                 expression RBRACE
                 { {
                   Portion *_p_ = $6->Evaluate();
                   if (_p_->Spec().Type != porREFERENCE)
                     portions.Append(_p_);
                   else  {
                     delete _p_;
	             portions.Append(REQUIRED);
                   }
                   delete $6;
                 } }    

binding:         RARROW    { refs.Append(false); }
       |         DBLARROW  { refs.Append(true); }


%%


const char CR = (char) 10;

char GCLCompiler::nextchar(void)
{	
  return current_expr[current_char++];
}

void GCLCompiler::ungetchar(char /*c*/)
{
  current_char--;
}

typedef struct tokens  { long tok; char *name; } TOKENS_T;

void GCLCompiler::yyerror(char *s)
{
static struct tokens toktable[] =
{ { LOR, "OR or ||" },  { LAND, "AND or &&" }, { LNOT, "NOT or !" },
    { EQU, "=" }, { NEQ, "!=" }, { LTN, "<" }, { LEQ, "<=" },
    { GTN, ">" }, { GEQ, ">=" }, { PLUS, "+" }, { MINUS, "-" },
    { STAR, "*" }, { SLASH, "/" }, { ASSIGN, ":=" }, { SEMI, ";" },
    { LBRACK, "[" }, { DBLLBRACK, "[[" }, { RBRACK, "]" },
    { LBRACE, "{" }, { RBRACE, "}" }, { RARROW, "->" },
    { LARROW, "<-" }, { DBLARROW, "<->" }, { COMMA, "," }, { HASH, "#" },
    { DOT, "." }, { CARET, "^" }, { UNDERSCORE, "_" },
    { AMPER, "&" }, { WRITE, "<<" }, { READ, ">>" },
    { IF, "If" }, { WHILE, "While" }, { FOR, "For" },
    { QUIT, "Quit" }, 
    { DEFFUNC, "NewFunction" }, 
    { DELFUNC, "DeleteFunction" },
    { TYPEDEF, "=:" },
    { INCLUDE, "Include" },
    { PERCENT, "%" }, { DIV, "DIV" }, { LPAREN, "(" }, { RPAREN, ")" },
    { CRLF, "carriage return" }, { EOC, "carriage return" }, { 0, 0 }
};

  gerr << s << " at line " << current_line << " in file " << current_file
       << ": ";

  for (int i = 0; toktable[i].tok != 0; i++)
    if (toktable[i].tok == yychar)   {
      gerr << toktable[i].name << '\n';
      return;
    }

  switch (yychar)   {
    case NAME:
      gerr << "identifier " << tval << '\n';
      break;
    case BOOLEAN:
      gerr << ((bval) ? "True" : "False") << '\n';
      break;
    case FLOAT:
      gerr << "floating-point constant " << dval << '\n';
      break;
    case INTEGER:
      gerr << "integer constant " << ival << '\n';
      break;
    case TEXT:
      gerr << "text string " << tval << '\n';
      break;
    case STDIN:
      gerr << "StdIn\n";
      break;
    case STDOUT:
      gerr << "StdOut\n";
      break;
    case gNULL:
      gerr << "NullOut\n";
      break;
    default:
      if (isprint(yychar) && !isspace(yychar))
        gerr << ((char) yychar) << '\n';
      else 
        gerr << "nonprinting character " << yychar << '\n';
      break;
  }    
}

int GCLCompiler::yylex(void)
{
  char c;
  
  do  {
    c = nextchar();
  }  while (isspace(c) || c == '\r' || c == '\n');

  if (isalpha(c))  {
    gString s(c);
    c = nextchar();
    while (isalpha(c) || isdigit(c))   {
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
    else if (s == "StdIn")  return STDIN;
    else if (s == "StdOut") return STDOUT;
    else if (s == "NullOut")   return gNULL;
    else if (s == "AND")    return LAND;
    else if (s == "OR")     return LOR;
    else if (s == "NOT")    return LNOT;
    else if (s == "DIV")    return DIV;
    else if (s == "MOD")    return PERCENT;
    else if (s == "If")     return IF;
    else if (s == "While")  return WHILE;
    else if (s == "For")    return FOR;
    else if (s == "Quit")   return QUIT;
    else if (s == "NewFunction")   return DEFFUNC;
    else if (s == "DeleteFunction")   return DELFUNC;
    else if (s == "Machine")   return MACHINEPREC;
    else if (s == "Rational")  return RATIONALPREC;
    else if (s == "Include")   return INCLUDE;
    else  { tval = s; return NAME; }
  }

  if (c == '"')   {
    tval = "";
    bool quote = true;
    bool check_digraph = true;
    while( quote )
    {
      c = nextchar();
      tval += c;
      
      if( check_digraph && 
          tval.length() >= 2 && 
          tval[ tval.length() - 2 ] == '\\' )
      {
        switch( c )
        {
	case '\'':
	case '\"':
	case '\?':
	case '\\':
          tval = tval.left( tval.length() - 2 ) + gString(c);
          check_digraph = false;
          break;
        case 'a':
          tval = tval.left( tval.length() - 2 ) + gString('\a');
          check_digraph = false;
          break;            
        case 'b':
          tval = tval.left( tval.length() - 2 ) + gString('\b');
          check_digraph = false;
          break;            
        case 'f':
          tval = tval.left( tval.length() - 2 ) + gString('\f');
          check_digraph = false;
          break;            
        case 'n':
          tval = tval.left( tval.length() - 2 ) + gString('\n');
          check_digraph = false;
          break;            
        case 'r':
          tval = tval.left( tval.length() - 2 ) + gString('\r');
          check_digraph = false;
          break;            
        case 't':
          tval = tval.left( tval.length() - 2 ) + gString('\t');
          check_digraph = false;
          break;            
        case 'v':
          tval = tval.left( tval.length() - 2 ) + gString('\v');
          check_digraph = false;
          break;            
        } // switch( c )
      }
      else
      {
        check_digraph = true;
        if( c == '\"' )
        {
          tval = tval.left( tval.length() - 1 );
          quote = false;
        }
      }
    } // while( quote )
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
    case '.':   c = nextchar();
      if (c < '0' || c > '9')  { ungetchar(c);  return DOT; }
      else  {
	gString s(".");
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

    case ';':   return SEMI;
    case '_':   return UNDERSCORE;
    case '(':   return LPAREN;
    case ')':   return RPAREN;
    case '{':   return LBRACE;
    case '}':   return RBRACE;
    case '+':   return PLUS;
    case '-':   c = nextchar();
                if (c == '>')  return RARROW;
                else  { ungetchar(c);  return MINUS; }
    case '*':   return STAR;
    case '/':   return SLASH;
    case '%':   return PERCENT;
    case '=':   c = nextchar();
                if (c == ':')  return TYPEDEF;
                else   { ungetchar(c);  return EQU; }  
    case '#':   return HASH;
    case '^':   return CARET;
    case '[':   c = nextchar();
                if (c == '[')  return DBLLBRACK;
                else   {
		  ungetchar(c);
		  return LBRACK;
		}
    case ']':   return RBRACK;
    case ':':   c = nextchar();
                if (c == '=')  return ASSIGN;
                else   { ungetchar(c);  return ':'; }  
    case '!':   c = nextchar();
                if (c == '=')  return NEQ;
		else   { ungetchar(c);  return LNOT; }
    case '<':   c = nextchar();
                if (c == '=')  return LEQ;
	        else if (c == '<')  return WRITE; 
                else if (c != '-')  { ungetchar(c);  return LTN; }
                else   { 
		  c = nextchar();
		  if (c == '>')   return DBLARROW;
		  ungetchar(c);
		  return LARROW;
		}
    case '>':   c = nextchar();
                if (c == '=')  return GEQ;
                else if (c == '>')  return READ;
                else   { ungetchar(c);  return GTN; }
    case '&':   c = nextchar();
                if (c == '&')  return LAND;
                else   { ungetchar(c);  return AMPER; }
    case '|':   c = nextchar();
                if (c == '|')  return LOR;
                else   { ungetchar(c);  return '|'; }
    case '\0':  return EOC;
    case CR:    assert(0);
    default:    return c;
  }
}

int GCLCompiler::Parse(const gString &line, const gString &file, int lineno)
{
  current_expr = line;
  current_char = 0;
  current_file = file;
  current_line = lineno;

  for (int i = 0; i < line.length(); i++)   {
    if (!isspace(line[i]))  {	
      if (!yyparse())  {	
        Execute();
        if (exprtree)   delete exprtree;
      }

      return 1;
    }
  }

  return 0;
}


void GCLCompiler::RecoverFromError(void)
{
  in_funcdecl = false;
  formals.Flush();
  types.Flush();
  refs.Flush();
  portions.Flush();
}
    

gclExpression *GCLCompiler::DefineFunction(gclExpression *expr)
{
  FuncDescObj *func = new FuncDescObj(funcname, 1);
  bool error = false;

  PortionSpec funcspec;

  funcspec = TextToPortionSpec(functype);
  if (funcspec.Type != porERROR) {
    FuncInfoType funcinfo = 
      FuncInfoType(expr, funcspec, formals.Length());
    funcinfo.Desc = funcbody;
    if( funcdesc.length() > 0 )
      funcinfo.Desc += "\n\n" + funcdesc;
    funcdesc = "";
    func->SetFuncInfo(0, funcinfo);
  }
  else {
    error = true;
    gerr << "Error: Unknown type " << functype << ", " << 
      PortionSpecToText(funcspec) << " as return type in declaration of " << 
      funcname << "[]\n";
  }

  for (int i = 1; i <= formals.Length(); i++)   {
    PortionSpec spec;
    if(portions[i])
      spec = portions[i]->Spec();
    else
      spec = TextToPortionSpec(types[i]);

    if (spec.Type != porERROR)   {
      if (refs[i])
	func->SetParamInfo(0, i - 1, 
                          ParamInfoType(formals[i], spec,
			                portions[i], BYREF));
      else
	func->SetParamInfo(0, i - 1, 
                          ParamInfoType(formals[i], spec,
			                portions[i], BYVAL));
    }
    else   {
      error = true;
      gerr << "Error: Unknown type " << types[i] << ", " << 
	PortionSpecToText(spec) << " for parameter " << formals[i] <<
	 " in declaration of " << funcname << "[]\n";
      break;
    }
  }


  formals.Flush();
  types.Flush();
  refs.Flush();
  portions.Flush();
//  function = 0;
  if (!error)
    return new gclFunctionDef(func, expr);
  else
    return new gclConstExpr(new BoolPortion(false));;
}


bool GCLCompiler::DeleteFunction(void)
{
/*
  FuncDescObj *func = new FuncDescObj(funcname, 1);
  bool error = false;

	PortionSpec funcspec;

  funcspec = TextToPortionSpec(functype);
  if (funcspec.Type != porERROR) {
    func->SetFuncInfo(0, FuncInfoType(function, funcspec, formals.Length()));
  }
  else {
    error = true;
    gerr << "Error: Unknown type " << functype << ", " << 
      PortionSpecToText(funcspec) << " as return type in declaration of " << 
      funcname << "[]\n";
  }

//  function->Dump(gout);

  for (int i = 1; i <= formals.Length(); i++)   {
    PortionSpec spec;
    if(portions[i])
      spec = portions[i]->Spec();
    else
      spec = TextToPortionSpec(types[i]);

    if (spec.Type != porERROR)   {
      if (refs[i])
	func->SetParamInfo(0, i - 1, 
                          ParamInfoType(formals[i], spec,
			                portions[i], BYREF));
      else
	func->SetParamInfo(0, i - 1, 
                          ParamInfoType(formals[i], spec,
											portions[i], BYVAL));
    }
    else   {
      error = true;
      gerr << "Error: Unknown type " << types[i] << ", " << 
	PortionSpecToText(spec) << " for parameter " << formals[i] <<
	 " in declaration of " << funcname << "[]\n";
      break;
    }
  }

  if (!error)  gsm.DeleteFunction(func);
  formals.Flush();
  types.Flush();
  refs.Flush();
  portions.Flush();
  function = 0;
  return !error;
*/
  return false;
}


int GCLCompiler::Execute(void)
{
  Portion *result = gsm.Execute(exprtree); 
  if (result->Spec().Type == porERROR)
    gout << "GCL: " << result << '\n';

  delete result;
  return rcSUCCESS;
}

/*
void GCLCompiler::LoadInputs( const char* name )
{
  extern char* _SourceDir;
  const char* SOURCE = _SourceDir; 
  assert( SOURCE );

#ifdef __GNUG__
  const char SLASH = '/';
  const char SLASH1 = '/';
#elif defined __BORLANDC__
  const char * SLASH = "\\";
  const char  SLASH1 = '\\';
#endif   // __GNUG__

  bool search = false;
  bool ini_found = false;
  if( strchr( name, SLASH1 ) == NULL )
    search = true;
  gString IniFileName;

  IniFileName = (gString) name;
  inputs.Push( new gFileInput( IniFileName ) );
  if (!inputs.Peek()->IsValid())
    delete inputs.Pop();
  else
  {  
    GCL_InputFileNames.Push( IniFileName );
    ini_found = true;
  }

  if( search )
  {

    if( !ini_found && (System::GetEnv( "HOME" ) != NULL) )
    {
      IniFileName = (gString) System::GetEnv( "HOME" ) + SLASH + name;
      inputs.Push( new gFileInput( IniFileName ) );
      if (!inputs.Peek()->IsValid())
        delete inputs.Pop();
      else
      {  
        GCL_InputFileNames.Push( IniFileName );
        ini_found = true;
      }
    }

    if( !ini_found && (System::GetEnv( "GCLLIB" ) != NULL) )
    {
      IniFileName = (gString) System::GetEnv( "GCLLIB" ) + SLASH + name;
      inputs.Push( new gFileInput( IniFileName ) );
      if (!inputs.Peek()->IsValid())
        delete inputs.Pop();
      else
      {
        GCL_InputFileNames.Push( IniFileName );
        ini_found = true;
      }
    }

    if( !ini_found && (SOURCE != NULL) )
    {
      IniFileName = (gString) SOURCE + SLASH + name;
      inputs.Push( new gFileInput( IniFileName ) );
      if (!inputs.Peek()->IsValid())
        delete inputs.Pop();
      else
      {  
        GCL_InputFileNames.Push( IniFileName );
        ini_found = true;
      }
    }

  }

  if( ini_found )
    lines.Push(1);
  else
    gerr << "GCL Warning: " << name << " not found.\n";
}
*/
