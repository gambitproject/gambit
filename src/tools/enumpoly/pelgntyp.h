/* *****************************************************************

   This file, and all other pel*.h and pel*.cc files in the Gambit
source code, are derived from the source code for Pelican, an
implementation of the Huber-Sturmfels algorithm for solving sparse
systems of polynomials written by Birk Huber.  That code in turn
incorporated the source code for HOMPACK and Qhull, which are included
here after being modified in the construction of both Pelican and
Gambit.  We are grateful to the authors of Pelican, Qhull, and
HOMPACK, for code that has been enormously useful in Gambit.
    More information about Qhull can be obtained from:

http://www.geom.edu/software/qhull/

    For more information concerning HOMPACK see:

http://netlib2.cs.utk.edu/hompack/

***************************************************************** */

/*
**    copyright (c) 1995  Birk Huber
*/

#ifndef GENTYPE_H
#define GENTYPE_H

#include "pelpsys.h"

/* define basic data types to be manipulated */
#define INVALID_T 0
/*
** Basic Numerical Types
*/
#define Int_T 2
#define Dbl_T 4
#define Cpx_T 6
#define Ply_T 8
#define Mtx_T 10

/*
** Other Types
*/
#define Str_T 50
#define Idf_T 60
#define Lst_T 70
#define Ast_T 80
#define Prc_T 90
#define Rng_T 1000
#define Err_T 1100
/* 
** Alias Types
*/
#define Sys_T 100
#define Xpl_T 110
#define Npl_T 120

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL (void*)0
#endif
#endif

typedef struct Gen_node_tag *Gen_node;

struct Gen_node_tag {
	int type;
	Gen_node next;
	union {
	 int ival;
         double dval;
	 fcomplex cval;
	 polynomial1 pval; 
         char   *gval;
         char   *idval;
	 Gen_node  (*proc)(Gen_node); 
	 struct Gen_node_tag *lval;
       } Genval;
     };
extern Pring Def_Ring;
extern int N;

Gen_node gen_node(); /* constructor for Gen_node */
Gen_node free_Gen_node(Gen_node); 
Gen_node free_Gen_list(Gen_node); 
Gen_node copy_Gen_node(Gen_node);
Gen_node copy_Gen_list(Gen_node);
void print_Gen_list(Gen_node); 
void print_Gen_node(Gen_node);
Gen_node GMND();
Gen_node INTND(int);
Gen_node DBLND(double);
Gen_node CPXND(fcomplex);
Gen_node PLYND(polynomial1);
/* Gen_node SYSND(GMatrix); */
Gen_node XPLND();
Gen_node IDND(const char *s);
Gen_node PND(Gen_node p(Gen_node));
Gen_node Rerror(const char *s, Gen_node g);

/* accessors for Gen_nodes*/
Gen_node Gen_next(Gen_node); 
Gen_node Gen_set_next(Gen_node,Gen_node); 
int Gen_set_int(Gen_node g,int i);
int Gen_int(Gen_node g);          
char    *Gen_idval(Gen_node); 
Gen_node Gen_lval(Gen_node); 
Gen_node Gen_set_lval(Gen_node,Gen_node); 
int      Gen_type(Gen_node); 

/* operations on Gen_Lists */
int Gen_length(Gen_node g);
Gen_node Gen_elt(Gen_node g, int idx);
Gen_node Gen_node_to_List(Gen_node);       /*enclose a Gen_node in a list */
Gen_node Link();       /*link two gen nodes by their next pointers*/ 
Gen_node Cat();        


/* miscalaneous */
char *Copy_String(const char *);
char *Copy_String_NQ(char *);

Gen_node ASTND(node n); 
node Gen_aset(Gen_node g);
polynomial1 Gen_poly(Gen_node g);

#endif /* GENTYPE_H */
