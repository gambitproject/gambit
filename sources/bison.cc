/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* HEADER SECTION */
#if defined( _MSDOS ) || defined(MSDOS) || defined(__MSDOS__) 
#define __MSDOS_AND_ALIKE
#endif
#if defined(_WINDOWS) && defined(_MSC_VER)
#define __HAVE_NO_ALLOCA
#define __MSDOS_AND_ALIKE
#endif

#ifndef alloca
#if defined( __GNUC__)
#define alloca __builtin_alloca

#elif (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)  || defined (__sgi)
#include <alloca.h>

#elif defined (__MSDOS_AND_ALIKE)
#include <malloc.h>
#ifndef __TURBOC__
/* MS C runtime lib */
#define alloca _alloca
#endif

#elif defined(_AIX)
#include <malloc.h>
#pragma alloca

#elif defined(__hpux)
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */

#endif /* not _AIX  not MSDOS, or __TURBOC__ or _AIX, not sparc.  */
#endif /* alloca not defined.  */
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
#ifndef __STDC__
#define const
#endif
#endif
#include <stdio.h>
#define YYBISON 1  
$/* %{ and %header{ and %union, during decl */
#define YY_@_BISON 1
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
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_@_STYPE 
#define YY_@_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_@_DEBUG
#define  YY_@_DEBUG YYDEBUG
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
$/*  YY_@_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */
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
      /* We used to use `unsigned long' as YY_@_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

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
#if YY_@_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_@_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_@_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_@_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_@_PURE
#ifndef YYPURE
#define YYPURE YY_@_PURE
#endif
#endif
#ifdef YY_@_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_@_DEBUG 
#endif
#endif
#ifndef YY_@_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_@_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_@_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_@_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */
$ /* #defines tokens */
#else
/* CLASS */
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
#ifndef YY_@_CONSTRUCTOR_CODE
#define YY_@_CONSTRUCTOR_CODE
#endif
#ifndef YY_@_CONSTRUCTOR_INIT
#define YY_@_CONSTRUCTOR_INIT
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
 int YY_@_PARSE (YY_@_PARSE_PARAM);
 virtual void YY_@_ERROR(char *msg) YY_@_ERROR_BODY;
#ifdef YY_@_PURE
#ifdef YY_@_LSP_NEEDED
 virtual int  YY_@_LEX (YY_@_STYPE *YY_@_LVAL,YY_@_LTYPE *YY_@_LLOC) YY_@_LEX_BODY;
#else
 virtual int  YY_@_LEX (YY_@_STYPE *YY_@_LVAL) YY_@_LEX_BODY;
#endif
#else
 virtual int YY_@_LEX() YY_@_LEX_BODY;
 YY_@_STYPE YY_@_LVAL;
#ifdef YY_@_LSP_NEEDED
 YY_@_LTYPE YY_@_LLOC;
#endif
 int   YY_@_NERRS;
 int    YY_@_CHAR;
#endif
#if YY_@_DEBUG != 0
 int YY_@_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_@_CLASS(YY_@_CONSTRUCTOR_PARAM);
public:
 YY_@_MEMBERS 
};
/* other declare folow */
#if YY_@_USE_CONST_TOKEN != 0
$ /* const YY_@_CLASS::token */
#endif
/*apres const  */
YY_@_CLASS::YY_@_CLASS(YY_@_CONSTRUCTOR_PARAM) YY_@_CONSTRUCTOR_INIT
{
#if YY_@_DEBUG != 0
YY_@_DEBUG_FLAG=0;
#endif
YY_@_CONSTRUCTOR_CODE;
};
#endif
$ /* fattrs + tables */

/* parser code folow  */


/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */ 

#if YY_@_USE_GOTO != 0
/* 
 SUPRESSION OF GOTO : on some C++ compiler (sun c++)
  the goto is strictly forbidden if any constructor/destructor
  is used in the whole function (very stupid isn't it ?)
 so goto are to be replaced with a 'while/switch/case construct'
 here are the macro to keep some apparent compatibility
*/
#define YYGOTO(lb) {yy_gotostate=lb;continue;}
#define YYBEGINGOTO  enum yy_labels yy_gotostate=yygotostart; \
                     for(;;) switch(yy_gotostate) { case yygotostart: {
#define YYLABEL(lb) } case lb: {
#define YYENDGOTO } } 
#define YYBEGINDECLARELABEL enum yy_labels {yygotostart
#define YYDECLARELABEL(lb) ,lb
#define YYENDDECLARELABEL  };
#else
/* macro to keep goto */
#define YYGOTO(lb) goto lb
#define YYBEGINGOTO 
#define YYLABEL(lb) lb:
#define YYENDGOTO
#define YYBEGINDECLARELABEL 
#define YYDECLARELABEL(lb)
#define YYENDDECLARELABEL 
#endif
/* LABEL DECLARATION */
YYBEGINDECLARELABEL
  YYDECLARELABEL(yynewstate)
  YYDECLARELABEL(yybackup)
/* YYDECLARELABEL(yyresume) */
  YYDECLARELABEL(yydefault)
  YYDECLARELABEL(yyreduce)
  YYDECLARELABEL(yyerrlab)   /* here on detecting error */
  YYDECLARELABEL(yyerrlab1)   /* here on error raised explicitly by an action */
  YYDECLARELABEL(yyerrdefault)  /* current state does not do anything special for the error token. */
  YYDECLARELABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */
  YYDECLARELABEL(yyerrhandle)  
YYENDDECLARELABEL
/* ALLOCA SIMULATION */
/* __HAVE_NO_ALLOCA */
#ifdef __HAVE_NO_ALLOCA
int __alloca_free_ptr(char *ptr,char *ref)
{if(ptr!=ref) free(ptr);
 return 0;}

#define __ALLOCA_alloca(size) malloc(size)
#define __ALLOCA_free(ptr,ref) __alloca_free_ptr((char *)ptr,(char *)ref)

#ifdef YY_@_LSP_NEEDED
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		    __ALLOCA_free(yyls,yylsa)+\
		   (num))
#else
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		   (num))
#endif
#else
#define __ALLOCA_return(num) return(num)
#define __ALLOCA_alloca(size) alloca(size)
#define __ALLOCA_free(ptr,ref) 
#endif

/* ENDALLOCA SIMULATION */

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_@_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        __ALLOCA_return(0)
#define YYABORT         __ALLOCA_return(1)
#define YYERROR         YYGOTO(yyerrlab1)
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          YYGOTO(yyerrlab)
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (YY_@_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_@_CHAR = (token), YY_@_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_@_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_@_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_@_PURE
/* UNPURE */
#define YYLEX           YY_@_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_@_CHAR;                      /*  the lookahead symbol        */
YY_@_STYPE      YY_@_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_@_NERRS;                 /*  number of parse errors so far */
#ifdef YY_@_LSP_NEEDED
YY_@_LTYPE YY_@_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_@_LSP_NEEDED
#define YYLEX           YY_@_LEX(&YY_@_LVAL, &YY_@_LLOC)
#else
#define YYLEX           YY_@_LEX(&YY_@_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_@_DEBUG != 0
int YY_@_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif
#endif



/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
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


#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)       __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */

#ifdef __cplusplus
static void __yy_bcopy (char *from, char *to, int count)
#else
#ifdef __STDC__
static void __yy_bcopy (char *from, char *to, int count)
#else
static void __yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
#endif
#endif
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}
#endif

int
#ifdef YY_USE_CLASS
 YY_@_CLASS::
#endif
     YY_@_PARSE(YY_@_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_@_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_@_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_@_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_@_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_@_LSP_NEEDED
  YY_@_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_@_LTYPE *yyls = yylsa;
  YY_@_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_@_PURE
  int YY_@_CHAR;
  YY_@_STYPE YY_@_LVAL;
  int YY_@_NERRS;
#ifdef YY_@_LSP_NEEDED
  YY_@_LTYPE YY_@_LLOC;
#endif
#endif

  YY_@_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_@_DEBUG != 0
  if (YY_@_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_@_NERRS = 0;
  YY_@_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_@_LSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
YYLABEL(yynewstate)

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YY_@_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_@_LSP_NEEDED
      YY_@_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_@_LSP_NEEDED
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
#ifdef YY_@_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_@_ERROR("parser stack overflow");
	  __ALLOCA_return(2);
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) __ALLOCA_alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      __ALLOCA_free(yyss1,yyssa);
      yyvs = (YY_@_STYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
      __ALLOCA_free(yyvs1,yyvsa);
#ifdef YY_@_LSP_NEEDED
      yyls = (YY_@_LTYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      __ALLOCA_free(yyls1,yylsa);
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_@_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_@_DEBUG != 0
      if (YY_@_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_@_DEBUG != 0
  if (YY_@_DEBUG_FLAG)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  YYGOTO(yybackup);
YYLABEL(yybackup)

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* YYLABEL(yyresume) */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    YYGOTO(yydefault);

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (YY_@_CHAR == YYEMPTY)
    {
#if YY_@_DEBUG != 0
      if (YY_@_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_@_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_@_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_@_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_@_DEBUG != 0
      if (YY_@_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_@_CHAR);

#if YY_@_DEBUG != 0
      if (YY_@_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_@_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_@_CHAR, YY_@_LVAL);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    YYGOTO(yydefault);

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
	YYGOTO(yyerrlab);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrlab);

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YY_@_DEBUG != 0
  if (YY_@_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_@_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_@_CHAR != YYEOF)
    YY_@_CHAR = YYEMPTY;

  *++yyvsp = YY_@_LVAL;
#ifdef YY_@_LSP_NEEDED
  *++yylsp = YY_@_LLOC;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  YYGOTO(yynewstate);

/* Do the default action for the current state.  */
YYLABEL(yydefault)

  yyn = yydefact[yystate];
  if (yyn == 0)
    YYGOTO(yyerrlab);

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
YYLABEL(yyreduce)
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YY_@_DEBUG != 0
  if (YY_@_DEBUG_FLAG)
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

$   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_@_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_@_DEBUG != 0
  if (YY_@_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_@_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_@_LLOC.first_line;
      yylsp->first_column = YY_@_LLOC.first_column;
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

  YYGOTO(yynewstate);

YYLABEL(yyerrlab)   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++YY_@_NERRS;

#ifdef YY_@_ERROR_VERBOSE
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
	      YY_@_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_@_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_@_ERROR_VERBOSE */
	YY_@_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_@_CHAR == YYEOF)
	YYABORT;

#if YY_@_DEBUG != 0
      if (YY_@_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_@_CHAR, yytname[yychar1]);
#endif

      YY_@_CHAR = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;              /* Each real token shifted decrements this */

  YYGOTO(yyerrhandle);

YYLABEL(yyerrdefault)  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) YYGOTO(yydefault);
#endif

YYLABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YY_@_LSP_NEEDED
  yylsp--;
#endif

#if YY_@_DEBUG != 0
  if (YY_@_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

YYLABEL(yyerrhandle)

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    YYGOTO(yyerrdefault);

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    YYGOTO(yyerrdefault);

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrpop);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrpop);

  if (yyn == YYFINAL)
    YYACCEPT;

#if YY_@_DEBUG != 0
  if (YY_@_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_@_LVAL;
#ifdef YY_@_LSP_NEEDED
  *++yylsp = YY_@_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */
$ /* section 3 */

/* AFTER END , NEVER READ !!! */
