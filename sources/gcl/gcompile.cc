
/*  A Bison parser, made from gcompile.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse gcl_yyparse
#define yylex gcl_yylex
#define yyerror gcl_yyerror
#define yylval gcl_yylval
#define yychar gcl_yychar
#define yydebug gcl_yydebug
#define yynerrs gcl_yynerrs
#define	LOR	257
#define	LAND	258
#define	LNOT	259
#define	EQU	260
#define	NEQ	261
#define	LTN	262
#define	LEQ	263
#define	GTN	264
#define	GEQ	265
#define	PLUS	266
#define	MINUS	267
#define	STAR	268
#define	SLASH	269
#define	ASSIGN	270
#define	SEMI	271
#define	LBRACK	272
#define	DBLLBRACK	273
#define	RBRACK	274
#define	LBRACE	275
#define	RBRACE	276
#define	RARROW	277
#define	LARROW	278
#define	DBLARROW	279
#define	COMMA	280
#define	HASH	281
#define	DOT	282
#define	CARET	283
#define	UNDERSCORE	284
#define	AMPER	285
#define	WRITE	286
#define	READ	287
#define	PERCENT	288
#define	DIV	289
#define	LPAREN	290
#define	RPAREN	291
#define	DOLLAR	292
#define	IF	293
#define	WHILE	294
#define	FOR	295
#define	NEWFUNC	296
#define	DELFUNC	297
#define	TYPEDEF	298
#define	INCLUDE	299
#define	NAME	300
#define	BOOLEAN	301
#define	INTEGER	302
#define	FLOAT	303
#define	TEXT	304
#define	STDOUT	305
#define	gNULL	306
#define	FLOATPREC	307
#define	RATIONALPREC	308
#define	CRLF	309
#define	EOC	310
#define	UWRITE	311
#define	UMINUS	312

#line 1 "gcompile.yy"

//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Parser input file for GCL
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <stdlib.h>
#include <ctype.h>
#include "base/gmisc.h"
#include "base/gstream.h"
#include "base/gtext.h"
#include "math/rational.h"
#include "base/glist.h"
#include "base/gstack.h"
#include "gsm.h"
#include "gsminstr.h"
#include "gsmfunc.h"
#include "portion.h"

#include "base/system.h"

gStack<gText> GCL_InputFileNames(4);

static GSM *gsm; \
static bool record_funcbody, in_funcdecl;
static unsigned int current_char, current_line;
static gText current_expr, current_file, current_rawline;
static gText funcbody, funcname, funcdesc, paramtype, functype; 
static gList<gText> formals, types; 
static gList<Portion *> portions;
static gList<bool> refs;
static gStack<gText> funcnames;
static gText tval;
static gclExpression *exprtree;
static gTriState bval;
static double dval;
static gInteger ival;

static char nextchar(void);
static void ungetchar(char c);

static gclExpression *NewFunction(gclExpression *expr);
static gclExpression *DeleteFunction(void);
static void RecoverFromError(void);

int GCLParse(const gText& line, const gText &file,
             int lineno, const gText& rawline); 
int Execute(void); 

void gcl_yyerror(char *s);
int gcl_yylex(void);


#line 76 "gcompile.yy"
typedef union  {
  gclExpression *eval;
  gclParameterList *pval;
  gclReqParameterList *rpval; 
  gclOptParameterList *opval;
  gclListConstant *lcval;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		189
#define	YYFLAG		-32768
#define	YYNTBASE	59

#define YYTRANSLATE(x) ((unsigned)(x) <= 312 ? yytranslate[x] : 95)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     6,     9,    11,    13,    17,    21,    24,    28,
    31,    34,    38,    42,    46,    50,    54,    57,    60,    64,
    68,    72,    76,    80,    84,    88,    92,    96,   100,   104,
   108,   111,   115,   119,   123,   127,   136,   143,   150,   161,
   162,   163,   172,   173,   179,   180,   186,   188,   190,   192,
   193,   195,   197,   201,   203,   207,   209,   210,   215,   216,
   223,   225,   227,   229,   231,   233,   235,   237,   239,   241,
   243,   245,   248,   252,   254,   257,   261,   263,   267,   268,
   275,   276,   277,   281,   283,   284,   288,   289,   290,   295,
   296,   300,   301,   303,   305,   309,   310,   311,   317,   318,
   319,   327,   329
};

static const short yyrhs[] = {    60,
    56,     0,     1,    56,     0,     1,    55,     0,    74,     0,
    61,     0,    36,    60,    37,     0,    60,    17,    60,     0,
    60,    17,     0,    60,    16,    60,     0,    60,    16,     0,
    32,    60,     0,    60,    27,    60,     0,    60,    30,    60,
     0,    60,    12,    60,     0,    60,    13,    60,     0,    60,
    31,    60,     0,    12,    60,     0,    13,    60,     0,    60,
    14,    60,     0,    60,    15,    60,     0,    60,    34,    60,
     0,    60,    35,    60,     0,    60,    28,    60,     0,    60,
    29,    60,     0,    60,     6,    60,     0,    60,     7,    60,
     0,    60,     8,    60,     0,    60,     9,    60,     0,    60,
    10,    60,     0,    60,    11,    60,     0,     5,    60,     0,
    60,     4,    60,     0,    60,     3,    60,     0,    60,    32,
    60,     0,    60,    33,    60,     0,    39,    18,    60,    26,
    60,    26,    60,    20,     0,    39,    18,    60,    26,    60,
    20,     0,    40,    18,    60,    26,    60,    20,     0,    41,
    18,    60,    26,    60,    26,    60,    26,    60,    20,     0,
     0,     0,    42,    62,    18,    77,    26,    63,    60,    20,
     0,     0,    43,    64,    18,    77,    20,     0,     0,    66,
    18,    65,    67,    20,     0,    46,     0,    53,     0,    54,
     0,     0,    68,     0,    70,     0,    68,    26,    70,     0,
    69,     0,    68,    26,    69,     0,    60,     0,     0,    46,
    71,    73,    60,     0,     0,    70,    26,    46,    72,    73,
    60,     0,    23,     0,    25,     0,    47,     0,    48,     0,
    49,     0,    50,     0,    51,     0,    52,     0,    53,     0,
    54,     0,    46,     0,    38,    46,     0,    38,    38,    46,
     0,    75,     0,    21,    22,     0,    21,    76,    22,     0,
    60,     0,    76,    26,    60,     0,     0,    66,    78,    18,
    87,    20,    79,     0,     0,     0,    44,    80,    81,     0,
    85,     0,     0,    46,    82,    83,     0,     0,     0,    36,
    84,    81,    37,     0,     0,    46,    86,    14,     0,     0,
    88,     0,    89,     0,    88,    26,    89,     0,     0,     0,
    46,    90,    94,    91,    81,     0,     0,     0,    21,    46,
    92,    94,    93,    60,    22,     0,    23,     0,    25,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   165,   166,   167,   170,   171,   172,   173,   175,   177,   179,
   181,   184,   187,   190,   193,   196,   199,   201,   204,   207,
   210,   213,   216,   219,   222,   225,   228,   231,   234,   237,
   240,   243,   246,   249,   252,   257,   260,   263,   265,   268,
   269,   271,   274,   275,   277,   277,   281,   282,   283,   286,
   287,   288,   289,   292,   293,   295,   297,   297,   299,   300,
   303,   303,   305,   307,   309,   311,   313,   315,   317,   319,
   321,   323,   325,   327,   330,   331,   334,   335,   339,   340,
   342,   343,   344,   346,   347,   349,   349,   350,   351,   353,
   353,   355,   356,   358,   359,   361,   362,   363,   364,   365,
   366,   378,   379
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","LOR","LAND",
"LNOT","EQU","NEQ","LTN","LEQ","GTN","GEQ","PLUS","MINUS","STAR","SLASH","ASSIGN",
"SEMI","LBRACK","DBLLBRACK","RBRACK","LBRACE","RBRACE","RARROW","LARROW","DBLARROW",
"COMMA","HASH","DOT","CARET","UNDERSCORE","AMPER","WRITE","READ","PERCENT","DIV",
"LPAREN","RPAREN","DOLLAR","IF","WHILE","FOR","NEWFUNC","DELFUNC","TYPEDEF",
"INCLUDE","NAME","BOOLEAN","INTEGER","FLOAT","TEXT","STDOUT","gNULL","FLOATPREC",
"RATIONALPREC","CRLF","EOC","UWRITE","UMINUS","program","expression","function",
"@1","@2","@3","@4","funcname","parameterlist","reqparameterlist","parameter",
"optparameterlist","@5","@6","arrow","constant","list","listels","signature",
"@7","TYPEopt","@8","typename","@9","optparen","@10","starname","@11","formallist",
"formalparams","formalparam","@12","@13","@14","@15","binding", NULL
};
#endif

static const short yyr1[] = {     0,
    59,    59,    59,    60,    60,    60,    60,    60,    60,    60,
    60,    60,    60,    60,    60,    60,    60,    60,    60,    60,
    60,    60,    60,    60,    60,    60,    60,    60,    60,    60,
    60,    60,    60,    60,    60,    61,    61,    61,    61,    62,
    63,    61,    64,    61,    65,    61,    66,    66,    66,    67,
    67,    67,    67,    68,    68,    69,    71,    70,    72,    70,
    73,    73,    74,    74,    74,    74,    74,    74,    74,    74,
    74,    74,    74,    74,    75,    75,    76,    76,    78,    77,
    79,    80,    79,    81,    82,    81,    83,    84,    83,    86,
    85,    87,    87,    88,    88,    90,    91,    89,    92,    93,
    89,    94,    94
};

static const short yyr2[] = {     0,
     2,     2,     2,     1,     1,     3,     3,     2,     3,     2,
     2,     3,     3,     3,     3,     3,     2,     2,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     2,     3,     3,     3,     3,     8,     6,     6,    10,     0,
     0,     8,     0,     5,     0,     5,     1,     1,     1,     0,
     1,     1,     3,     1,     3,     1,     0,     4,     0,     6,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     2,     3,     1,     2,     3,     1,     3,     0,     6,
     0,     0,     3,     1,     0,     3,     0,     0,     4,     0,
     3,     0,     1,     1,     3,     0,     0,     5,     0,     0,
     7,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    40,    43,    71,    63,    64,    65,    66,    67,    68,
    69,    70,     0,     5,     0,     4,    74,     3,     2,    31,
    17,    18,    75,    77,     0,    11,     0,     0,    72,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    10,     8,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     1,    45,    76,
     0,     6,    73,     0,     0,     0,     0,     0,    33,    32,
    25,    26,    27,    28,    29,    30,    14,    15,    19,    20,
     9,     7,    12,    23,    24,    13,    16,    34,    35,    21,
    22,    50,    78,     0,     0,     0,    47,    48,    49,    79,
     0,     0,    71,    56,     0,    51,    54,    52,     0,     0,
     0,     0,    41,    44,     0,    46,     0,     0,    37,     0,
    38,     0,    92,     0,    61,    62,     0,    55,    53,    59,
     0,     0,     0,    96,     0,    93,    94,     0,    58,     0,
    36,     0,    99,     0,    81,     0,    42,     0,     0,     0,
   102,   103,    97,    82,    80,    95,    60,    39,   100,     0,
     0,     0,    85,    98,    84,    83,     0,    87,     0,   101,
    88,    86,    91,     0,     0,    89,     0,     0,     0
};

static const short yydefgoto[] = {   187,
   114,    24,    43,   134,    44,   102,    25,   115,   116,   117,
   118,   125,   150,   137,    26,    27,    35,   111,   122,   165,
   171,   174,   178,   182,   184,   175,   179,   145,   146,   147,
   154,   170,   160,   172,   163
};

static const short yypact[] = {   128,
   -32,   228,   228,   228,   178,   228,   228,   -17,     7,    12,
    25,-32768,-32768,    26,-32768,-32768,-32768,-32768,-32768,-32768,
    50,    69,    66,-32768,    73,-32768,-32768,-32768,-32768,   804,
    59,    59,-32768,   728,    62,   761,   330,    56,-32768,   228,
   228,   228,    85,    90,   228,   228,   228,   228,   228,   228,
   228,   228,   228,   228,   228,   228,   228,   228,   228,   228,
   228,   228,   228,   228,   228,   228,   228,-32768,-32768,-32768,
   228,-32768,-32768,   398,   431,   464,   -26,   -26,     4,   804,
   834,   834,   834,   834,   834,   834,   174,   174,    80,    80,
   761,   728,-32768,    80,    59,-32768,   174,   794,   794,    80,
    80,   278,   728,   228,   228,   228,-32768,-32768,-32768,-32768,
    91,    95,    67,   728,    96,    92,-32768,    93,   365,   497,
   530,   102,-32768,-32768,    88,-32768,   278,    75,-32768,   228,
-32768,   228,   -20,   228,-32768,-32768,   228,-32768,    93,-32768,
   563,   596,    77,-32768,   104,    99,-32768,   629,   728,    88,
-32768,   228,-32768,    89,    82,   -20,-32768,   228,   662,    89,
-32768,-32768,-32768,-32768,-32768,-32768,   728,-32768,-32768,    81,
    81,   228,   114,-32768,-32768,-32768,   695,   100,   117,-32768,
-32768,-32768,-32768,    81,    98,-32768,   138,   139,-32768
};

static const short yypgoto[] = {-32768,
     0,-32768,-32768,-32768,-32768,-32768,   -41,-32768,-32768,    15,
    16,-32768,-32768,    -6,-32768,-32768,-32768,    68,-32768,-32768,
-32768,  -162,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -11,
-32768,-32768,-32768,-32768,   -13
};


#define	YYLAST		869


static const short yytable[] = {    23,
   143,    30,    31,    32,    34,    36,    37,    46,   176,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    56,   107,
    38,   185,    28,    29,    40,   144,   108,   109,    39,    41,
    59,    60,    61,    62,    63,   110,   110,    66,    67,    74,
    75,    76,    42,   -47,    79,    80,    81,    82,    83,    84,
    85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
    95,    96,    97,    98,    99,   100,   101,   -48,    45,    46,
   103,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    70,   -47,    59,   -49,    71,    62,   -57,
    69,   -57,    59,    60,    61,    62,    63,    64,    65,    66,
    67,    73,    77,   119,   120,   121,    59,    78,    61,    62,
   135,   161,   136,   162,   124,   126,   123,   127,   128,   133,
   140,    68,   153,   155,   156,   164,   173,   -90,     1,   141,
   183,   142,     2,   148,   186,   181,   149,   188,   189,     3,
     4,   138,   139,   158,   166,   112,   169,     0,     5,     0,
     0,   159,     0,     0,     0,     0,     0,   167,     0,     6,
     0,     0,     0,     7,     0,     8,     9,    10,    11,    12,
    13,   177,     0,    14,    15,    16,    17,    18,    19,    20,
    21,    22,     2,     0,     0,     0,     0,    55,    56,     3,
     4,     0,     0,     0,     0,     0,     0,     0,     5,    33,
    59,    60,    61,    62,     0,     0,     0,    66,    67,     6,
     0,     0,     0,     7,     0,     8,     9,    10,    11,    12,
    13,     0,     0,    14,    15,    16,    17,    18,    19,    20,
    21,    22,     2,     0,     0,     0,     0,     0,     0,     3,
     4,     0,     0,     0,     0,     0,     0,     0,     5,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     6,
     0,     0,     0,     7,     0,     8,     9,    10,    11,    12,
    13,     0,     0,    14,    15,    16,    17,    18,    19,    20,
    21,    22,     2,     0,     0,     0,     0,     0,     0,     3,
     4,     0,     0,     0,     0,     0,     0,     0,     5,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     6,
     0,     0,     0,     7,     0,     8,     9,    10,    11,    12,
    13,     0,     0,   113,    15,    16,    17,    18,    19,    20,
    21,    22,    45,    46,     0,    47,    48,    49,    50,    51,
    52,    53,    54,    55,    56,    57,    58,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    59,    60,    61,    62,
    63,    64,    65,    66,    67,     0,    72,    45,    46,     0,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58,     0,     0,   129,     0,     0,     0,     0,     0,
   130,    59,    60,    61,    62,    63,    64,    65,    66,    67,
    45,    46,     0,    47,    48,    49,    50,    51,    52,    53,
    54,    55,    56,    57,    58,     0,     0,     0,     0,     0,
     0,     0,     0,   104,    59,    60,    61,    62,    63,    64,
    65,    66,    67,    45,    46,     0,    47,    48,    49,    50,
    51,    52,    53,    54,    55,    56,    57,    58,     0,     0,
     0,     0,     0,     0,     0,     0,   105,    59,    60,    61,
    62,    63,    64,    65,    66,    67,    45,    46,     0,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
    58,     0,     0,     0,     0,     0,     0,     0,     0,   106,
    59,    60,    61,    62,    63,    64,    65,    66,    67,    45,
    46,     0,    47,    48,    49,    50,    51,    52,    53,    54,
    55,    56,    57,    58,     0,     0,   131,     0,     0,     0,
     0,     0,     0,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    45,    46,     0,    47,    48,    49,    50,    51,
    52,    53,    54,    55,    56,    57,    58,     0,     0,     0,
     0,     0,     0,     0,     0,   132,    59,    60,    61,    62,
    63,    64,    65,    66,    67,    45,    46,     0,    47,    48,
    49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
     0,     0,   151,     0,     0,     0,     0,     0,     0,    59,
    60,    61,    62,    63,    64,    65,    66,    67,    45,    46,
     0,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,     0,     0,     0,     0,     0,     0,     0,
     0,   152,    59,    60,    61,    62,    63,    64,    65,    66,
    67,    45,    46,     0,    47,    48,    49,    50,    51,    52,
    53,    54,    55,    56,    57,    58,     0,     0,   157,     0,
     0,     0,     0,     0,     0,    59,    60,    61,    62,    63,
    64,    65,    66,    67,    45,    46,     0,    47,    48,    49,
    50,    51,    52,    53,    54,    55,    56,    57,    58,     0,
     0,   168,     0,     0,     0,     0,     0,     0,    59,    60,
    61,    62,    63,    64,    65,    66,    67,    45,    46,     0,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58,     0,     0,     0,     0,   180,     0,     0,     0,
     0,    59,    60,    61,    62,    63,    64,    65,    66,    67,
    45,    46,     0,    47,    48,    49,    50,    51,    52,    53,
    54,    55,    56,    57,    58,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    59,    60,    61,    62,    63,    64,
    65,    66,    67,    45,    46,     0,    47,    48,    49,    50,
    51,    52,    53,    54,    55,    56,    57,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
    62,    63,    64,    65,    66,    67,    45,    46,     0,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    56,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    56,     0,
    59,    60,    61,    62,    63,     0,     0,    66,    67,     0,
    59,    60,    61,    62,    63,     0,     0,    66,    67,-32768,
-32768,-32768,-32768,-32768,-32768,    53,    54,    55,    56,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    59,    60,    61,    62,    63,     0,     0,    66,    67
};

static const short yycheck[] = {     0,
    21,     2,     3,     4,     5,     6,     7,     4,   171,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    46,
    38,   184,    55,    56,    18,    46,    53,    54,    46,    18,
    27,    28,    29,    30,    31,    77,    78,    34,    35,    40,
    41,    42,    18,    18,    45,    46,    47,    48,    49,    50,
    51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
    61,    62,    63,    64,    65,    66,    67,    18,     3,     4,
    71,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    22,    18,    27,    18,    26,    30,    23,
    18,    25,    27,    28,    29,    30,    31,    32,    33,    34,
    35,    46,    18,   104,   105,   106,    27,    18,    29,    30,
    23,    23,    25,    25,    20,    20,    26,    26,    26,    18,
    46,    56,    46,    20,    26,    44,    46,    14,     1,   130,
    14,   132,     5,   134,    37,    36,   137,     0,     0,    12,
    13,   127,   127,   150,   156,    78,   160,    -1,    21,    -1,
    -1,   152,    -1,    -1,    -1,    -1,    -1,   158,    -1,    32,
    -1,    -1,    -1,    36,    -1,    38,    39,    40,    41,    42,
    43,   172,    -1,    46,    47,    48,    49,    50,    51,    52,
    53,    54,     5,    -1,    -1,    -1,    -1,    14,    15,    12,
    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,
    27,    28,    29,    30,    -1,    -1,    -1,    34,    35,    32,
    -1,    -1,    -1,    36,    -1,    38,    39,    40,    41,    42,
    43,    -1,    -1,    46,    47,    48,    49,    50,    51,    52,
    53,    54,     5,    -1,    -1,    -1,    -1,    -1,    -1,    12,
    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
    -1,    -1,    -1,    36,    -1,    38,    39,    40,    41,    42,
    43,    -1,    -1,    46,    47,    48,    49,    50,    51,    52,
    53,    54,     5,    -1,    -1,    -1,    -1,    -1,    -1,    12,
    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
    -1,    -1,    -1,    36,    -1,    38,    39,    40,    41,    42,
    43,    -1,    -1,    46,    47,    48,    49,    50,    51,    52,
    53,    54,     3,     4,    -1,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    30,
    31,    32,    33,    34,    35,    -1,    37,     3,     4,    -1,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
     3,     4,    -1,     6,     7,     8,     9,    10,    11,    12,
    13,    14,    15,    16,    17,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    26,    27,    28,    29,    30,    31,    32,
    33,    34,    35,     3,     4,    -1,     6,     7,     8,     9,
    10,    11,    12,    13,    14,    15,    16,    17,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    28,    29,
    30,    31,    32,    33,    34,    35,     3,     4,    -1,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,     3,
     4,    -1,     6,     7,     8,     9,    10,    11,    12,    13,
    14,    15,    16,    17,    -1,    -1,    20,    -1,    -1,    -1,
    -1,    -1,    -1,    27,    28,    29,    30,    31,    32,    33,
    34,    35,     3,     4,    -1,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    26,    27,    28,    29,    30,
    31,    32,    33,    34,    35,     3,     4,    -1,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    -1,    27,
    28,    29,    30,    31,    32,    33,    34,    35,     3,     4,
    -1,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
    35,     3,     4,    -1,     6,     7,     8,     9,    10,    11,
    12,    13,    14,    15,    16,    17,    -1,    -1,    20,    -1,
    -1,    -1,    -1,    -1,    -1,    27,    28,    29,    30,    31,
    32,    33,    34,    35,     3,     4,    -1,     6,     7,     8,
     9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
    -1,    20,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
    29,    30,    31,    32,    33,    34,    35,     3,     4,    -1,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,
    -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
     3,     4,    -1,     6,     7,     8,     9,    10,    11,    12,
    13,    14,    15,    16,    17,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    27,    28,    29,    30,    31,    32,
    33,    34,    35,     3,     4,    -1,     6,     7,     8,     9,
    10,    11,    12,    13,    14,    15,    16,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    29,
    30,    31,    32,    33,    34,    35,     3,     4,    -1,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
    27,    28,    29,    30,    31,    -1,    -1,    34,    35,    -1,
    27,    28,    29,    30,    31,    -1,    -1,    34,    35,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    27,    28,    29,    30,    31,    -1,    -1,    34,    35
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 165 "gcompile.yy"
{ exprtree = yyvsp[-1].eval; return 0; ;
    break;}
case 2:
#line 166 "gcompile.yy"
{ RecoverFromError(); return 1; ;
    break;}
case 3:
#line 167 "gcompile.yy"
{ RecoverFromError(); return 1; ;
    break;}
case 6:
#line 172 "gcompile.yy"
{ yyval.eval = yyvsp[-1].eval; ;
    break;}
case 7:
#line 174 "gcompile.yy"
{ yyval.eval = new gclSemiExpr(yyvsp[-2].eval, yyvsp[0].eval); ;
    break;}
case 8:
#line 176 "gcompile.yy"
{ yyval.eval = yyvsp[-1].eval; ;
    break;}
case 9:
#line 178 "gcompile.yy"
{ yyval.eval = new gclAssignment(yyvsp[-2].eval, yyvsp[0].eval); ;
    break;}
case 10:
#line 180 "gcompile.yy"
{ yyval.eval = new gclUnAssignment(yyvsp[-1].eval); ;
    break;}
case 11:
#line 182 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Print", yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 12:
#line 185 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("NthChild", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 13:
#line 188 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("NthElement", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 14:
#line 191 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Plus", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 15:
#line 194 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Minus", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 16:
#line 197 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Concat", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 17:
#line 200 "gcompile.yy"
{ yyval.eval = yyvsp[0].eval; ;
    break;}
case 18:
#line 202 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Negate", yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 19:
#line 205 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Times", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 20:
#line 208 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Divide", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 21:
#line 211 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Modulus", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 22:
#line 214 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("IntegerDivide", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 23:
#line 217 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Dot", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 24:
#line 220 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Power", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 25:
#line 223 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Equal", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 26:
#line 226 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("NotEqual", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 27:
#line 229 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Less", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 28:
#line 232 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("LessEqual", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 29:
#line 235 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Greater", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 30:
#line 238 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("GreaterEqual", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 31:
#line 241 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Not", yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 32:
#line 244 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("And", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 33:
#line 247 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Or", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 34:
#line 250 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Write", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 35:
#line 253 "gcompile.yy"
{ yyval.eval = new gclFunctionCall("Read", yyvsp[-2].eval, yyvsp[0].eval,
					 current_line, current_file); ;
    break;}
case 36:
#line 259 "gcompile.yy"
{ yyval.eval = new gclConditional(yyvsp[-5].eval, yyvsp[-3].eval, yyvsp[-1].eval); ;
    break;}
case 37:
#line 261 "gcompile.yy"
{ yyval.eval = new gclConditional(yyvsp[-3].eval, yyvsp[-1].eval, 
				new gclConstExpr(new BoolPortion(false))); ;
    break;}
case 38:
#line 264 "gcompile.yy"
{ yyval.eval = new gclWhileExpr(yyvsp[-3].eval, yyvsp[-1].eval); ;
    break;}
case 39:
#line 267 "gcompile.yy"
{ yyval.eval = new gclForExpr(yyvsp[-7].eval, yyvsp[-5].eval, yyvsp[-3].eval, yyvsp[-1].eval); ;
    break;}
case 40:
#line 268 "gcompile.yy"
{ if (in_funcdecl) YYERROR;  in_funcdecl = true; ;
    break;}
case 41:
#line 270 "gcompile.yy"
{ funcbody = ""; record_funcbody = true; ;
    break;}
case 42:
#line 272 "gcompile.yy"
{ record_funcbody = false; in_funcdecl = false;
                    yyval.eval = NewFunction(yyvsp[-1].eval); ;
    break;}
case 43:
#line 274 "gcompile.yy"
{ if (in_funcdecl) YYERROR; ;
    break;}
case 44:
#line 276 "gcompile.yy"
{ yyval.eval = DeleteFunction(); ;
    break;}
case 45:
#line 277 "gcompile.yy"
{ funcnames.Push(tval); ;
    break;}
case 46:
#line 278 "gcompile.yy"
{ yyval.eval = new gclFunctionCall(funcnames.Pop(), yyvsp[-1].pval,
					 current_line, current_file); ;
    break;}
case 48:
#line 282 "gcompile.yy"
{ tval = "Float"; ;
    break;}
case 49:
#line 283 "gcompile.yy"
{ tval = "Rational"; ;
    break;}
case 50:
#line 286 "gcompile.yy"
{ yyval.pval = new gclParameterList; ;
    break;}
case 51:
#line 287 "gcompile.yy"
{ yyval.pval = new gclParameterList(yyvsp[0].rpval); ;
    break;}
case 52:
#line 288 "gcompile.yy"
{ yyval.pval = new gclParameterList(yyvsp[0].opval); ;
    break;}
case 53:
#line 290 "gcompile.yy"
{ yyval.pval = new gclParameterList(yyvsp[-2].rpval, yyvsp[0].opval); ;
    break;}
case 54:
#line 292 "gcompile.yy"
{ yyval.rpval = new gclReqParameterList(yyvsp[0].eval); ;
    break;}
case 55:
#line 293 "gcompile.yy"
{ yyvsp[-2].rpval->Append(yyvsp[0].eval); ;
    break;}
case 57:
#line 297 "gcompile.yy"
{ funcnames.Push(tval); ;
    break;}
case 58:
#line 298 "gcompile.yy"
{ yyval.opval = new gclOptParameterList(funcnames.Pop(), yyvsp[0].eval); ;
    break;}
case 59:
#line 299 "gcompile.yy"
{ funcnames.Push(tval); ;
    break;}
case 60:
#line 301 "gcompile.yy"
{ yyvsp[-5].opval->Append(funcnames.Pop(), yyvsp[0].eval); ;
    break;}
case 63:
#line 306 "gcompile.yy"
{ yyval.eval = new gclConstExpr(new BoolPortion(bval)); ;
    break;}
case 64:
#line 308 "gcompile.yy"
{ yyval.eval = new gclConstExpr(new NumberPortion(ival)); ;
    break;}
case 65:
#line 310 "gcompile.yy"
{ yyval.eval = new gclConstExpr(new NumberPortion(dval)); ;
    break;}
case 66:
#line 312 "gcompile.yy"
{ yyval.eval = new gclConstExpr(new TextPortion(tval)); ;
    break;}
case 67:
#line 314 "gcompile.yy"
{ yyval.eval = new gclConstExpr(new OutputPortion(gsm->OutputStream())); ;
    break;}
case 68:
#line 316 "gcompile.yy"
{ yyval.eval = new gclConstExpr(new OutputPortion(*new gNullOutput)); ;
    break;}
case 69:
#line 318 "gcompile.yy"
{ yyval.eval = new gclConstExpr(new PrecisionPortion(precDOUBLE)); ;
    break;}
case 70:
#line 320 "gcompile.yy"
{ yyval.eval = new gclConstExpr(new PrecisionPortion(precRATIONAL)); ;
    break;}
case 71:
#line 322 "gcompile.yy"
{ yyval.eval = new gclVarName(tval); ;
    break;}
case 72:
#line 324 "gcompile.yy"
{ yyval.eval = new gclVarName(gText("$") + tval); ;
    break;}
case 73:
#line 326 "gcompile.yy"
{ yyval.eval = new gclVarName(gText("$$") + tval); ;
    break;}
case 74:
#line 327 "gcompile.yy"
{ yyval.eval = yyvsp[0].lcval; ;
    break;}
case 75:
#line 330 "gcompile.yy"
{ yyval.lcval = new gclListConstant; ;
    break;}
case 76:
#line 331 "gcompile.yy"
{ yyval.lcval = yyvsp[-1].lcval; ;
    break;}
case 77:
#line 334 "gcompile.yy"
{ yyval.lcval = new gclListConstant(yyvsp[0].eval); ;
    break;}
case 78:
#line 335 "gcompile.yy"
{ yyvsp[-2].lcval->Append(yyvsp[0].eval); ;
    break;}
case 79:
#line 339 "gcompile.yy"
{ funcname = tval; ;
    break;}
case 81:
#line 342 "gcompile.yy"
{ functype = "ANYTYPE"; ;
    break;}
case 82:
#line 343 "gcompile.yy"
{ paramtype = ""; ;
    break;}
case 83:
#line 344 "gcompile.yy"
{ functype = paramtype; ;
    break;}
case 85:
#line 347 "gcompile.yy"
{ paramtype += tval; ;
    break;}
case 88:
#line 350 "gcompile.yy"
{ paramtype += '('; ;
    break;}
case 89:
#line 351 "gcompile.yy"
{ paramtype += ')'; ;
    break;}
case 90:
#line 353 "gcompile.yy"
{ paramtype += tval; ;
    break;}
case 91:
#line 353 "gcompile.yy"
{ paramtype += '*'; ;
    break;}
case 96:
#line 361 "gcompile.yy"
{ formals.Append(tval); ;
    break;}
case 97:
#line 362 "gcompile.yy"
{ paramtype = ""; ;
    break;}
case 98:
#line 363 "gcompile.yy"
{ types.Append(paramtype); portions.Append(REQUIRED); ;
    break;}
case 99:
#line 364 "gcompile.yy"
{ formals.Append(tval); ;
    break;}
case 100:
#line 365 "gcompile.yy"
{ paramtype = ""; types.Append(paramtype); ;
    break;}
case 101:
#line 367 "gcompile.yy"
{ {
                   Portion *_p_ = yyvsp[-1].eval->Evaluate(*gsm);
                   if (_p_->Spec().Type != porREFERENCE)
                     portions.Append(_p_);
                   else  {
                     delete _p_;
	             portions.Append(REQUIRED);
                   }
                   delete yyvsp[-1].eval;
                 } ;
    break;}
case 102:
#line 378 "gcompile.yy"
{ refs.Append(false); ;
    break;}
case 103:
#line 379 "gcompile.yy"
{ refs.Append(true); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 382 "gcompile.yy"



const char CR = (char) 10;

char nextchar(void)
{
  char c = current_expr[current_char];
  if( c == '\r' || c == '\n' )
    ++current_line;
  ++current_char;
  return c;
}

void ungetchar(char /*c*/)
{
  char c = current_expr[current_char-1];
  if( (current_char > 0) && (c == '\r' || c == '\n') )
    --current_line;
  --current_char;
}

typedef struct tokens  { long tok; char *name; } TOKENS_T;

void gcl_yyerror(char *s)
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
    { AMPER, "&" }, { WRITE, "<<" }, { READ, ">>" }, { DOLLAR, "$" },
    { IF, "If" }, { WHILE, "While" }, { FOR, "For" },
    { NEWFUNC, "NewFunction" }, { DELFUNC, "DeleteFunction" },
    { TYPEDEF, "=:" }, { INCLUDE, "Include" },
    { PERCENT, "%" }, { DIV, "DIV" }, { LPAREN, "(" }, { RPAREN, ")" },
    { CRLF, "carriage return" }, { EOC, "carriage return" },
    { FLOATPREC, "Float" }, { RATIONALPREC, "Rational" }, { 0, 0 }
};

  gsm->ErrorStream() << s << " at line " << current_line << " in file " << current_file
       << ": ";

  for (int i = 0; toktable[i].tok != 0; i++)
    if (toktable[i].tok == gcl_yychar)   {
      gsm->ErrorStream() << toktable[i].name << '\n';
      return;
    }

  switch (gcl_yychar)   {
    case NAME:
      gsm->ErrorStream() << "identifier " << tval << '\n';
      break;
    case BOOLEAN:
      if (bval == triTRUE)
     	gsm->ErrorStream() << "True\n";
      else if (bval == triFALSE)
        gsm->ErrorStream() << "False\n";
      else  /* (bval == triUNKNOWN) */
        gsm->ErrorStream() << "Unknown\n";
      break;
    case FLOAT:
      gsm->ErrorStream() << "floating-point constant " << dval << '\n';
      break;
    case INTEGER:
      gsm->ErrorStream() << "integer constant " << ival << '\n';
      break;
    case TEXT:
      gsm->ErrorStream() << "text string " << tval << '\n';
      break;
    case STDOUT:
      gsm->ErrorStream() << "StdOut\n";
      break;
    case gNULL:
      gsm->ErrorStream() << "NullOut\n";
      break;
    default:
      if (isprint(gcl_yychar) && !isspace(gcl_yychar))
        gsm->ErrorStream() << ((char) gcl_yychar) << '\n';
      else 
        gsm->ErrorStream() << "nonprinting character " << gcl_yychar << '\n';
      break;
  }    
}

int gcl_yylex(void)
{
  char c;
  
  do  {
    c = nextchar();
  }  while (isspace(c) || c == '\r' || c == '\n');

  if (isalpha(c))  {
    gText s(c);
    c = nextchar();
    while (isalpha(c) || isdigit(c))   {
      s += c;
      c = nextchar();
    }
    ungetchar(c);

    if (s == "True")   {
      bval = triTRUE;
      return BOOLEAN;
    }
    else if (s == "False")  {
      bval = triFALSE;
      return BOOLEAN;
    }
    else if (s == "Unknown") {
      bval = triUNKNOWN;
      return BOOLEAN;
    }
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
    else if (s == "NewFunction")   return NEWFUNC;
    else if (s == "DeleteFunction")   return DELFUNC;
    else if (s == "Float")   return FLOATPREC;
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
          tval.Length() >= 2 && 
          tval[ tval.Length() - 2 ] == '\\' )
      {
        switch( c )
        {
	case '\'':
	case '\"':
	case '\?':
	case '\\':
          tval = tval.Left( tval.Length() - 2 ) + gText(c);
          check_digraph = false;
          break;
        case 'a':
          tval = tval.Left( tval.Length() - 2 ) + gText('\a');
          check_digraph = false;
          break;            
        case 'b':
          tval = tval.Left( tval.Length() - 2 ) + gText('\b');
          check_digraph = false;
          break;            
        case 'f':
          tval = tval.Left( tval.Length() - 2 ) + gText('\f');
          check_digraph = false;
          break;            
        case 'n':
          tval = tval.Left( tval.Length() - 2 ) + gText('\n');
          check_digraph = false;
          break;            
        case 'r':
          tval = tval.Left( tval.Length() - 2 ) + gText('\r');
          check_digraph = false;
          break;            
        case 't':
          tval = tval.Left( tval.Length() - 2 ) + gText('\t');
          check_digraph = false;
          break;            
        case 'v':
          tval = tval.Left( tval.Length() - 2 ) + gText('\v');
          check_digraph = false;
          break;            
        } // switch( c )
      }
      else
      {
        check_digraph = true;
        if( c == '\"' )
        {
          tval = tval.Left( tval.Length() - 1 );
          quote = false;
        }
      }
    } // while( quote )
    return TEXT;
  }

  if (isdigit(c))   {
    gText s(c);
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
      ival = atoI((char *) s);
      return INTEGER;
    }
  }

  switch (c)  {
    case ',':   return COMMA;
    case '.':   c = nextchar();
      if (c < '0' || c > '9')  { ungetchar(c);  return DOT; }
      else  {
	gText s(".");
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
    case '$':   return DOLLAR;
    case '\0':  return EOC;
    case CR:    assert(0);
    default:    return c;
  }
}

int GCLParse(GSM *p_gsm,
	     const gText& line, const gText &file, int lineno,
             const gText& rawline)
{
  gsm = p_gsm;
  current_expr = line;
  current_char = 0;
  current_file = file;
  current_line = lineno;
  current_rawline = rawline;

  for (unsigned int i = 0; i < line.Length(); i++)   {
    if (!isspace(line[i]))  {	
      if (!gcl_yyparse())  {	
        Execute();
        if (exprtree)   delete exprtree;
      }

      return 1;
    }
  }

  return 0;
}


void RecoverFromError(void)
{
  in_funcdecl = false;
  formals.Flush();
  types.Flush();
  refs.Flush();
  portions.Flush();
}
    

gclExpression *NewFunction(gclExpression *expr)
{
  gclFunction *func = new gclFunction(*gsm, funcname, 1);
  PortionSpec funcspec;

  try {
    funcspec = TextToPortionSpec(functype);
  }
  catch (gclRuntimeError &)  {
    gsm->ErrorStream() << "Error: Unknown type " << functype << ", " << 
      " as return type in declaration of " << funcname << "[]\n";
    return new gclConstExpr(new BoolPortion(false));;
  }

  gclSignature funcinfo = 
    gclSignature(expr, funcspec, formals.Length());

  funcbody = current_rawline;
  if( !strstr((const char *) funcbody, "/*Private*/" ) )
    funcinfo.Desc = funcbody;
  else
    funcinfo.Desc = "/*Private*/";

  if( funcdesc.Length() > 0 )
    funcinfo.Desc += "\n\n" + funcdesc;
  funcdesc = "";
  
  func->SetFuncInfo(0, funcinfo);

  for (int i = 1; i <= formals.Length(); i++)   {
    PortionSpec spec;
    if(portions[i])
      spec = portions[i]->Spec();
    else {
      try {
	spec = TextToPortionSpec(types[i]);
      }
      catch (gclRuntimeError &) {
	gsm->ErrorStream() << "Error: Unknown type " << types[i] << ", " << 
	  PortionSpecToText(spec) << " for parameter " << formals[i] <<
	  " in declaration of " << funcname << "[]\n";
	return new gclConstExpr(new BoolPortion(false));;
      }
    }

    if (refs[i])
      func->SetParamInfo(0, i - 1, 
			 gclParameter(formals[i], spec,
				       portions[i], BYREF));
      else
	func->SetParamInfo(0, i - 1, 
			   gclParameter(formals[i], spec,
					 portions[i], BYVAL));
  }


  formals.Flush();
  types.Flush();
  refs.Flush();
  portions.Flush();
  
  return new gclFunctionDef(func, expr);
}


gclExpression *DeleteFunction(void)
{
  gclFunction *func = new gclFunction(*gsm, funcname, 1);

  PortionSpec funcspec;

  try  {
    funcspec = TextToPortionSpec(functype);
  }
  catch (gclRuntimeError &)  {
    gsm->ErrorStream() << "Error: Unknown type " << functype << ", " << 
      PortionSpecToText(funcspec) << " as return type in declaration of " << 
      funcname << "[]\n";
    return new gclConstExpr(new BoolPortion(false));
  }

  func->SetFuncInfo(0, gclSignature((gclExpression *) 0, funcspec, formals.Length()));

  for (int i = 1; i <= formals.Length(); i++)   {
    PortionSpec spec;

    try {
      if (portions[i])
	spec = portions[i]->Spec();
      else
	spec = TextToPortionSpec(types[i]);
      
      if (refs[i])
	func->SetParamInfo(0, i - 1, 
			   gclParameter(formals[i], spec,
			                portions[i], BYREF));
      else
	func->SetParamInfo(0, i - 1, 
			   gclParameter(formals[i], spec,
					portions[i], BYVAL));
    }
    catch (gclRuntimeError &) {
      gsm->ErrorStream() << "Error: Unknown type " << types[i] << ", " << 
	PortionSpecToText(spec) << " for parameter " << formals[i] <<
	" in declaration of " << funcname << "[]\n";
      return new gclConstExpr(new BoolPortion(false));
    }
  }

  formals.Flush();
  types.Flush();
  refs.Flush();
  portions.Flush();

  return new gclDeleteFunction(func);
}

#include "base/gstatus.h"
#include "gsm.h"

int Execute(void)
{
  try  {
    Portion *result = gsm->Execute(exprtree);
    if (result)  delete result;
  }
  catch (gclQuitOccurred &) {
    throw;
  }
  catch (gclRuntimeError &E) {
    gsm->OutputStream() << "ERROR: " << E.Description() << '\n';
  }
  catch (gException &E) {
    gsm->OutputStream() << "EXCEPTION: " << E.Description() << '\n';
  }

  return rcSUCCESS;
}
