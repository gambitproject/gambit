%{
//#
//# FILE: gcompile.yy -- yaccer/compiler for the GCL
//#
//# This parser/compiler is dedicated to the memory of
//# Jan L. A. van de Snepscheut, who wrote a program after which
//# this code is modeled.
//#
//# $Id$
//#

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

%}

%name GCLCompiler

%define MEMBERS   \
  int index; \
  gString input_text; \
  bool bval, triv, semi; \
  int statementcount; \
  gInteger ival; \
  double dval; \
  gString tval, formal, funcname, paramtype;  \
  Portion* por; \
  gList<Instruction *> program, *function; \
  gList<gString> formals, types; \
  gList<bool> refs; \
  gList<Portion*> portions; \
  gStack<gString> formalstack; \
  gStack<int> labels, listlen; \
  gStack<char> matching; \
  gStack<gInput *> inputs; \
  gStack<gString> filenames; \
  gStack<int> lines; \
  GSM gsm; \
  bool quit; \
  \
  char nextchar(void); \
  void ungetchar(char c); \
  \
  void emit(Instruction *); \
  bool DefineFunction(void); \
  void RecoverFromError(void); \
  int ProgLength(void); \
  \
  int Parse(void); \
  void Execute(void);

%define CONSTRUCTOR_INIT     : por(0), function(0), formalstack(4), \
                               labels(4), \
                               listlen(4), matching(4), \
                               filenames(4), lines(4), \
                               gsm(256), quit(false)

%define CONSTRUCTOR_CODE       filenames.Push("stdin"); lines.Push(1); \
                               inputs.Push(new gFileInput("gclini.gcl")); \
			       if (!inputs.Peek()->IsValid())  \
		                 delete inputs.Pop();  \
			       else  {  \
                                 filenames.Push("gclini.gcl"); \
		                 lines.Push(1); \
	                       }

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
%token INCLUDE
%token ASSIGNFUNC
%token UNASSIGN
%token CLEAR
%token HELP

%token NAME
%token BOOLEAN
%token INTEGER
%token FLOAT
%token TEXT
%token STDIN
%token STDOUT
%token gNULL


%token CRLF
%token EOC

%%

program: 
              toplevel EOC 	
              { if (!triv || !semi) emit(new Display); emit(new Pop); return 0; }
       |      error EOC   { RecoverFromError();  return 1; }
       |      error CRLF  { RecoverFromError();  return 1; }

toplevel:     statements
        |     include toplevel
        |     funcdecl toplevel
        |     clearstmt toplevel

statements:   statement
          |   statements sep statement

sep:          SEMI    { semi = true; }
   |          CRLF    { semi = false; 
                        if (!triv)  { emit(new Display); } }

clearstmt:    CLEAR LBRACK RBRACK  { emit(new Clear); }


funcdecl:     DEFFUNC LBRACK NAME
              { funcname = tval; function = new gList<Instruction *>; 
                statementcount = 0; }
              LBRACK formallist RBRACK COMMA statements
              RBRACK   { if (!triv && !semi) emit(new Display);
			 if (!DefineFunction())  YYERROR; } 
		
formallist:
          |   formalparams

formalparams: formalparam
            | formalparams CRLFopt COMMA CRLFopt formalparam

formalparam:  NAME { formals.Append(tval); } binding 
              { paramtype = ""; } typename 
              { types.Append(paramtype); portions.Append(NO_DEFAULT_VALUE); } 
            | LBRACE NAME { formals.Append(tval); } binding
              { paramtype = ""; types.Append(paramtype); }
              E9 { portions.Append(por); por = 0; } 
              RBRACE

typename:     NAME { paramtype += tval; } optparen

optparen:
        |     LPAREN { paramtype += '('; }  typename
              RPAREN { paramtype += ')'; }

binding:      RARROW    { refs.Append(false); }
       |      DBLARROW  { refs.Append(true); }

statement:    { triv = true; statementcount++; }
         |    expression { triv = false; }
         |    conditional { triv = false; }
         |    whileloop { triv = false; }
         |    forloop   { triv = false; }
         |    assignment
         |    unassignment
         |    helpstmt
         |    QUIT     { triv = false; quit = true; emit(new Quit); }

assignment:   ASSIGNFUNC LBRACK NAME { emit(new PushRef(tval)); }
              COMMA expression RBRACK { emit(new Assign); }

unassignment: UNASSIGN LBRACK NAME RBRACK
              { emit(new PushRef(tval));
		emit(new UnAssign);
	      }

helpstmt:     HELP LBRACK text_or_name RBRACK
              { emit(new PushRef(tval));
		emit(new Help);
	      }

text_or_name: TEXT
         |    NAME

include:      INCLUDE LBRACK TEXT RBRACK
              { inputs.Push(new gFileInput(tval));
		if (!inputs.Peek()->IsValid())   {
		  gerr << "include file " << tval << " not found\n";
		  delete inputs.Pop();
		  YYERROR;
		}
		else { // not sure if making this part an else clause is right
		  filenames.Push(tval);
		  lines.Push(1);
		}
	      }

conditional:  IF LBRACK CRLFopt expression CRLFopt COMMA 
              { emit(new NOT); emit(0);
                labels.Push(ProgLength()); } statements 
              { emit(0);
		if (function)
		  (*function)[labels.Pop()] = new IfGoto(ProgLength() + 1);
		else
		  program[labels.Pop()] = new IfGoto(ProgLength() + 1);
		labels.Push(ProgLength());
	      }
              alternative RBRACK
              { emit(new NOP);
		if (function)
		  (*function)[labels.Pop()] = new Goto(ProgLength());
		else
		  program[labels.Pop()] = new Goto(ProgLength());
              } 

alternative:   
           |  COMMA statements

CRLFopt:    | CRLFs

CRLFs:     CRLF | CRLFs CRLF

whileloop:    WHILE LBRACK CRLFopt { labels.Push(ProgLength() + 1); }
              expression { emit(new NOT); emit(0);
			   labels.Push(ProgLength()); }
              CRLFopt COMMA statements RBRACK 
              { if (!triv && !semi)   emit(new Display);
                if (function)
		  (*function)[labels.Pop()] = new IfGoto(ProgLength() + 2);
		else
		  program[labels.Pop()] = new IfGoto(ProgLength() + 2);
		emit(new Goto(labels.Pop()));
		emit(new NOP);
	      }

forloop:      FOR LBRACK CRLFopt exprlist CRLFopt COMMA CRLFopt 
              { labels.Push(ProgLength() + 1); }
              expression CRLFopt COMMA CRLFopt
              {  index = labels.Pop();   // index is loc of begin of guard eval
                 emit(new NOT);
                 // slot for guard-false jump
                 emit(0); labels.Push(ProgLength());
                 // push location of increment 
                 labels.Push(ProgLength() + 2);
                 // slot for guard-true jump
                 emit(0); labels.Push(ProgLength()); labels.Push(index);
              }
              exprlist CRLFopt COMMA
              { // emit jump to beginning of guard eval
                emit(new Goto(labels.Pop())); 
                // link guard-true jump
                if (function)
                  (*function)[labels.Pop()] = new Goto(ProgLength() + 1);
		else
		  program[labels.Pop()] = new Goto(ProgLength() + 1);
                semi = false;
              }
              statements RBRACK
              { 
                if (!triv && !semi)  emit(new Display);
                // emit jump to beginning of increment step
                emit(new Goto(labels.Pop()));
		// link guard-false branch to end of code
                if (function)
		  (*function)[labels.Pop()] = new IfGoto(ProgLength() + 1);
		else
		  program[labels.Pop()] = new IfGoto(ProgLength() + 1);
		emit(new NOP);
	      }

exprlist:     expression  { emit(new Pop); }
        |     exprlist SEMI expression  { emit(new Pop); }

expression:   Ea
          |   Ea ASSIGN expression { emit(new Assign()); }
          ;

Ea:           E0
  |           Ea WRITE E0   { emit(new Write); }
  |           Ea READ E0    { emit(new Read); }
  ; 

E0:           E1
  |           E0 LOR E1  { emit(new OR); }
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
  |           E4 AMPER E5  { emit(new Concat); }
  ;

E5:           E6
  |           E5 STAR E6    { emit(new Mul); }
  |           E5 SLASH E6   { emit(new Div); }
  |           E5 PERCENT E6 { emit(new Mod); }
  |           E5 DIV E6     { emit(new IntDiv); }
  |           E5 DOT E6     { emit(new Dot); }
  |           E5 CARET E6   { emit(new Power); } 
  ;

E6:           PLUS E7
  |           MINUS E7      { emit(new Neg); }
  |           E7

E7:           E8
  |           E7 HASH E8   { emit(new Child); }
  |           E7 DBLLBRACK expression RBRACK RBRACK 
                 { emit(new Subscript); }
  ;

E8:           E9            { delete por; }
  |           LPAREN expression RPAREN
  |           NAME          { emit(new PushRef(tval)); }
  |           function      { emit(new CallFunction()); }
  |           list          { emit(new PushList(listlen.Pop())); }
  ;

E9:           BOOLEAN  { emit(new Push<bool>(bval));
                         por = new BoolValPortion(bval); }
  |           INTEGER  { emit(new Push<long>(ival.as_long()));
                         por = new IntValPortion(ival.as_long()); }
  |           FLOAT    { emit(new Push<double>(dval));
                         por = new FloatValPortion(dval); }
  |           TEXT     { emit(new Push<gString>(tval));
                         por = new TextValPortion(tval); }
  |           STDIN    { emit(new PushInput(gin));
                         por = new InputRefPortion(gin); }
  |           STDOUT   { emit(new PushOutput(gout));
                         por = new OutputRefPortion(gout); }
  |           gNULL    { emit(new PushOutput(gnull));
                         por = new OutputRefPortion(gnull); }
  ;

function:     NAME LBRACK { emit(new InitCallFunction(tval)); } arglist RBRACK

arglist:
       |      unnamed_args
       |      unnamed_args COMMA named_args
       |      named_args

unnamed_args: unnamed_arg
            | unnamed_args COMMA unnamed_arg

unnamed_arg:  expression  { emit(new Bind()); }

named_args:   named_arg
          |   named_args COMMA named_arg

named_arg:    NAME RARROW { formalstack.Push(tval); } expression
                           { emit(new BindVal(formalstack.Pop())); }
         |    NAME DBLARROW  { formalstack.Push(tval); } name_or_io
                           { emit(new BindRef(formalstack.Pop())); }

name_or_io:   NAME     { emit(new PushRef(tval)); }
         |    STDIN    { emit(new PushInput(gin)); }
         |    STDOUT   { emit(new PushOutput(gout)); }
         |    gNULL    { emit(new PushOutput(gnull)); }

list:         LBRACE CRLFopt  { listlen.Push(0); } listels CRLFopt RBRACE
    |         LBRACE CRLFopt  { listlen.Push(0); } RBRACE

listels:      listel
       |      listels CRLFopt COMMA CRLFopt listel

listel:       expression   { listlen.Push(listlen.Pop() + 1); }

%%


const char CR = (char) 10;

char GCLCompiler::nextchar(void)
{
  char c;

  while (inputs.Depth() && inputs.Peek()->eof())  {
    delete inputs.Pop();
    filenames.Pop();
    lines.Pop();
  }

  if (inputs.Depth() == 0)
    gin >> c;
  else
    *inputs.Peek() >> c;

  if (c == CR)
    lines.Peek()++;

  return c;
}

void GCLCompiler::ungetchar(char c)
{
  if (inputs.Depth() == 0)
    gin.unget(c);
  else
    inputs.Peek()->unget(c);

  if (c == CR)
    lines.Peek()--;
}

typedef struct tokens  { long tok; char *name; };

void GCLCompiler::yyerror(char *s)
{
static struct tokens toktable[] =
{ { LOR, "OR or ||" },  { LAND, "AND or &&" }, { LNOT, "NOT or !" },
    { EQU, "=" }, { NEQ, "!=" }, { LTN, "<" }, { LEQ, "<=" },
    { GTN, ">" }, { GEQ, ">=" }, { PLUS, "+" }, { MINUS, "-" },
    { STAR, "*" }, { SLASH, "/" }, { ASSIGN, ":=" }, { SEMI, ";" },
    { LBRACK, "[" }, { DBLLBRACK, "[[" }, { RBRACK, "]" },
    { LBRACE, "{" }, { RBRACE, "}" }, { RARROW, "->" },
    { LARROW, "<-" }, { COMMA, "," }, { HASH, "#" },
    { DOT, "." }, { CARET, "^" }, { AMPER, "&" }, { WRITE, "<<" }, { READ, ">>" },
    { IF, "If" }, { WHILE, "While" }, { FOR, "For" },
    { QUIT, "Quit" }, { DEFFUNC, "NewFunction" }, { INCLUDE, "Include" },
    { CLEAR, "Clear" }, { HELP, "Help" },
    { PERCENT, "%" }, { DIV, "DIV" }, { LPAREN, "(" }, { RPAREN, ")" },
    { CRLF, "carriage return" }, { EOC, "carriage return" }, { 0, 0 }
};


  gerr << s << ": " << filenames.Peek() << ':'
       << ((yychar == CRLF || yychar == EOC) ? lines.Peek() - 1 : lines.Peek()) << " at ";

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
      gerr << "Null\n";
      break;
    default:
      gerr << yychar << '\n';
      break;
  }    
}

int GCLCompiler::yylex(void)
{
  char c;

I_dont_believe_Im_doing_this:

  while (1)  {
    char d;
    do  {
      c = nextchar();
    }  while (isspace(c) && c != CR);
    if (c == '/')  {
      if ((d = nextchar()) == '/')  {
	while ((d = nextchar()) != CR);
	if (matching.Depth())
	  return CRLF;
	else
	  return EOC;
      }
      else if (d == '*')  {
	int done = 0;
	while (!done)  {
	  while ((d = nextchar()) != '*');
	  if ((d = nextchar()) == '/')  done = 1;
	}
      }
      else  {
	ungetchar(d);
	return SLASH;
      }
    }
    else
      break;
  }

  if (c == '\\')   {
    while (isspace(c = nextchar()) && c != CR);
    if (c == CR)
      goto I_dont_believe_Im_doing_this;
    else  {
      ungetchar(c);
      return '\\';
    }
  }

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
    else if (s == "StdIn")  return STDIN;
    else if (s == "StdOut") return STDOUT;
    else if (s == "Null")   return gNULL;
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
    else if (s == "Include")   return INCLUDE;
    else if (s == "Assign")  return ASSIGNFUNC;
    else if (s == "UnAssign")  return UNASSIGN;
    else if (s == "Help") return HELP;
    else if (s == "Clear")   return CLEAR;
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
    case '(':   matching.Push('(');  return LPAREN;
    case ')':   if (matching.Depth() > 0 && matching.Peek() == '(')
                  matching.Pop();
                return RPAREN;
    case '{':   matching.Push('{');  return LBRACE;
    case '}':   if (matching.Depth() > 0 && matching.Peek() == '{')
                  matching.Pop();
                return RBRACE;
    case '+':   return PLUS;
    case '-':   c = nextchar();
                if (c == '>')  return RARROW;
                else  { ungetchar(c);  return MINUS; }
    case '*':   return STAR;
    case '/':   return SLASH;
    case '%':   return PERCENT;
    case '=':   return EQU;
    case '#':   return HASH;
    case '^':   return CARET;
    case '[':   matching.Push('[');
                c = nextchar();
                if (c == '[')   {
		  matching.Push('[');
		  return DBLLBRACK;
		}
                else   {
		  ungetchar(c);
		  return LBRACK;
		}
    case ']':   if (matching.Depth() > 0 && matching.Peek() == '[')
                  matching.Pop();
                return RBRACK;
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
    case CR:    if (matching.Depth())
                  return CRLF;
    case EOF:   return EOC;
    default:    return c;
  }
}

int GCLCompiler::Parse(void)
{
  int command = 1;

  while (!quit && (inputs.Depth() > 0 || !gin.eof()))  {

    while (inputs.Depth() && inputs.Peek()->eof())  {
      delete inputs.Pop();
      filenames.Pop();
      lines.Pop();
    }

    if (inputs.Depth() == 0)
      gout << "GCL" << command << ": ";
    matching.Flush();
    if (!yyparse())  {
      Execute();
//      gsm.Dump();
      if (inputs.Depth() == 0) command++;
    }
    else 
      while (program.Length() > 0)   delete program.Remove(1);
  }
  return 1;
}


void GCLCompiler::emit(Instruction *op)
{
  // the encoding for the line number is decoded in GSM::ExecuteUserFunc()
  if(op) op->LineNumber() = statementcount * 65536 + lines.Peek();
  if (function)
    function->Append(op);
  else
    program.Append(op);
}

int GCLCompiler::ProgLength(void)
{
  return (function) ? function->Length() : program.Length();
}

void GCLCompiler::RecoverFromError(void)
{
  if (function)   {
    while (function->Length())   delete function->Remove(1);
    delete function;  function = 0;
    formals.Flush();
    types.Flush();
    refs.Flush();
    portions.Flush();
  }
  labels.Flush();
  listlen.Flush();

  while (inputs.Depth())   {
    delete inputs.Pop();
    filenames.Pop();
    lines.Pop();
  }
}
    

bool GCLCompiler::DefineFunction(void)
{
  FuncDescObj *func = new FuncDescObj(funcname);
  func->SetFuncInfo(function, formals.Length());
//  function->Dump(gout);

  bool error = false;
  for (int i = 1; i <= formals.Length(); i++)   {
    PortionType type;
    if(portions[i])
      type = portions[i]->Type();
    else
      type = TextToPortionType(types[i]);
    int listdepth = TextToPortionListDepth(types[i]);

    if (type != porERROR)   {
      if (refs[i])
	func->SetParamInfo(function, i - 1, formals[i], type,
			   portions[i], PASS_BY_REFERENCE, listdepth);
      else
	func->SetParamInfo(function, i - 1, formals[i], type,
			   portions[i], PASS_BY_VALUE, listdepth);
    }
    else   {
      error = true;
      gerr << "Error: Unknown type " << types[i] << " for parameter " << formals[i] << " in declaration of " << funcname << "[]\n";
      break;
    }
  }

  if (!error)  gsm.AddFunction(func);
  formals.Flush();
  types.Flush();
  refs.Flush();
  portions.Flush();
  function = 0;
  return !error;
}

void GCLCompiler::Execute(void)
{
#ifdef ASSEMBLY
  program.Dump(gout);   gout << '\n';
#endif   // ASSEMBLY
  gsm.Execute(program);
  gsm.Flush();
}

#include "gstack.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__

TEMPLATE class gStack<gString>;
TEMPLATE class gStack<int>;
TEMPLATE class gStack<char>;
TEMPLATE class gStack<gInput *>;

#include "glist.imp"

TEMPLATE class gList<bool>;
TEMPLATE class gNode<bool>;

