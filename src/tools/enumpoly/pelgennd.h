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

#include "pelconv.h"

Gen_node gen_node();

Gen_node free_Gen_node(Gen_node a);

Gen_node free_Gen_list(Gen_node a);

void print_Gen_node(Gen_node g);

void silent_print_Gen_list(Gen_node g);

void print_Gen_list(Gen_node g);

Gen_node Cat(Gen_node g1,Gen_node g2);

Gen_node copy_Gen_list(Gen_node a);
     
Gen_node copy_Gen_node(Gen_node a);


char *Gen_idval(Gen_node g);

Gen_node Gen_set_next(Gen_node g,Gen_node h);

Gen_node Gen_lval(Gen_node g);

Gen_node Gen_set_lval(Gen_node g,Gen_node g1);

Gen_node Gen_next(Gen_node g);

int Gen_type(Gen_node g);

int Gen_set_int(Gen_node g,int i);

int Gen_int(Gen_node g);

int Gen_length(Gen_node g);

Gen_node Gen_elt(Gen_node g, int idx);

polynomial1 Gen_poly(Gen_node g);

node Gen_aset(Gen_node g);
   
void print_Proc(Gen_node (*p)(Gen_node));

/*used for reader.lex-- 
takes a string in quotes and removes outside quotes*/
char *Copy_String_NQ(char *s);

char *Copy_String(const char *s);

Gen_node IDND(const char *s);

Gen_node ASTND(node n);

Gen_node INTND(int n);

Gen_node DBLND(double d);

Gen_node CPXND(fcomplex c);

Gen_node PLYND(polynomial1 p);

Gen_node Rerror(const char *s,Gen_node g);

Gen_node G_Print(Gen_node g);
