/* before anything */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
#ifdef __cplusplus
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#else
#endif
#include <stdio.h>
$ /* %{ and %header{ and %union, during decl */
#ifndef YY_@_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_@_COMPATIBILITY 1
#else
#define  YY_@_COMPATIBILITY 0
#endif
#endif

#if YY_@_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_@_LTYPE
#define YY_@_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_@_STYPE 
#define YY_@_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_@_DEBUG
#define  YY_@_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_@_STYPE
#ifndef yystype
#define yystype YY_@_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_@_USE_GOTO
#define YY_@_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_@_USE_GOTO
#define YY_@_USE_GOTO 0
#endif

#ifndef YY_@_PURE
$/* YY_@_PURE */
#endif
$/* prefix */
#ifndef YY_@_DEBUG
$/* YY_@_DEBUG */
#endif
#ifndef YY_@_LSP_NEEDED
$ /* YY_@_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_@_LSP_NEEDED
#ifndef YY_@_LTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YY_@_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_@_STYPE
#define YY_@_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_@_PARSE
#define YY_@_PARSE yyparse
#endif
#ifndef YY_@_LEX
#define YY_@_LEX yylex
#endif
#ifndef YY_@_LVAL
#define YY_@_LVAL yylval
#endif
#ifndef YY_@_LLOC
#define YY_@_LLOC yylloc
#endif
#ifndef YY_@_CHAR
#define YY_@_CHAR yychar
#endif
#ifndef YY_@_NERRS
#define YY_@_NERRS yynerrs
#endif
#ifndef YY_@_DEBUG_FLAG
#define YY_@_DEBUG_FLAG yydebug
#endif
#ifndef YY_@_ERROR
#define YY_@_ERROR yyerror
#endif

#ifndef YY_@_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_@_PARSE_PARAM
#ifndef YY_@_PARSE_PARAM_DEF
#define YY_@_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_@_PARSE_PARAM
#define YY_@_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_@_PURE
extern YY_@_STYPE YY_@_LVAL;
#endif

$ /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_@_CLASS
#define YY_@_CLASS @
#endif

#ifndef YY_@_INHERIT
#define YY_@_INHERIT
#endif
#ifndef YY_@_MEMBERS
#define YY_@_MEMBERS 
#endif
#ifndef YY_@_LEX_BODY
#define YY_@_LEX_BODY  
#endif
#ifndef YY_@_ERROR_BODY
#define YY_@_ERROR_BODY  
#endif
#ifndef YY_@_CONSTRUCTOR_PARAM
#define YY_@_CONSTRUCTOR_PARAM
#endif
/* choose between enum and const */
#ifndef YY_@_USE_CONST_TOKEN
#define YY_@_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */ 
#endif
#if YY_@_USE_CONST_TOKEN != 0
#ifndef YY_@_ENUM_TOKEN
#define YY_@_ENUM_TOKEN yy_@_enum_token
#endif
#endif

class YY_@_CLASS YY_@_INHERIT
{
public: 
#if YY_@_USE_CONST_TOKEN != 0
/* static const int token ... */
$ /* decl const */
#else
enum YY_@_ENUM_TOKEN { YY_@_NULL_TOKEN=0
$ /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_@_PARSE(YY_@_PARSE_PARAM);
 virtual void YY_@_ERROR(char *msg) YY_@_ERROR_BODY;
#ifdef YY_@_PURE
#ifdef YY_@_LSP_NEEDED
 virtual int  YY_@_LEX(YY_@_STYPE *YY_@_LVAL,YY_@_LTYPE *YY_@_LLOC) YY_@_LEX_BODY;
#else
 virtual int  YY_@_LEX(YY_@_STYPE *YY_@_LVAL) YY_@_LEX_BODY;
#endif
#else
 virtual int YY_@_LEX() YY_@_LEX_BODY;
 YY_@_STYPE YY_@_LVAL;
#ifdef YY_@_LSP_NEEDED
 YY_@_LTYPE YY_@_LLOC;
#endif
 int YY_@_NERRS;
 int YY_@_CHAR;
#endif
#if YY_@_DEBUG != 0
public:
 int YY_@_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_@_CLASS(YY_@_CONSTRUCTOR_PARAM);
public:
 YY_@_MEMBERS 
};
/* other declare folow */
#endif


#if YY_@_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_@_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_@_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_@_DEBUG 
#define YYDEBUG YY_@_DEBUG
#endif
#endif

#endif
/* END */
$ /* section 3 %header{ */
 /* AFTER END , NEVER READ !!! */



