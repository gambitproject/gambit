//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via enumerating extreme points, lrslib version
//
// This file is part of Gambit
// Copyright (c) 2006, The Gambit Project
// Based on the implementation in lrslib 4.2b, which is
// Copyright (c) 1995-2005, David Avis
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


//
// This file is based heavily on nash.c distributed with lrslib 4.2b.
// I have tried where possible to maintain as much of the original, and
// make a minimal set of changes to interface with Gambit's game
// representation library.  This way, hopefully, if future versions of lrslib
// fix bugs or create enhancements that are relevant, it will be fairly
// easy to port them to this program.    -- TLT, 23.viii.2006
//

#include <iostream>
#include <cstdio>
#include <cstring>

// The order of these next includes is important, because of macro definitions
#include "libgambit/libgambit.h"
using namespace Gambit;

extern "C" {
#include "lrslib.h"
}

//==========================================================================
//                   Building the problem representations
//==========================================================================

//
// These two functions are based upon the program setupnash.c from the
// lrslib distribution, and the user's guide documentation.
// There are two separate functions, one for each player's problem.
// According to the user's guide, the ordering of the constraint rows
// is significant, and differs between the players; for player 1's problem
// the nonnegativity constraints come first, whereas for player 2's problem
// they appear later.  Experiments suggest this is in fact true, and
// reversing them breaks something.
//

void FillNonnegativityRows(lrs_dic *P, lrs_dat *Q, 
			   int firstRow, int lastRow, int n)
{
  const int MAXCOL = 1000;     /* maximum number of columns */
  long num[MAXCOL], den[MAXCOL];

  for (long row = firstRow; row <= lastRow; row++) {
    num[0] = 0;
    den[0] = 1;

    for (long col = 1; col < n; col++) {
      num[col] = (row-firstRow+1 == col) ? 1 : 0;
      den[col] = 1;
    }

    lrs_set_row(P, Q, row, num, den, GE);
  }
}

void FillConstraintRows(lrs_dic *P, lrs_dat *Q,
			const StrategySupport &p_support,
			int p1, int p2, int firstRow)
{
  const int MAXCOL = 1000;     /* maximum number of columns */
  long num[MAXCOL], den[MAXCOL];

  Game game = p_support.GetGame();
  Rational min = game->GetMinPayoff() - Rational(1);
  PureStrategyProfile cont = game->NewPureStrategyProfile();

  for (long row = firstRow; row < firstRow + p_support.NumStrategies(p1); 
       row++) {
    num[0] = 0;
    den[0] = 1;

    cont->SetStrategy(p_support.GetStrategy(p1, row - firstRow + 1));

    for (long st = 1; st <= p_support.NumStrategies(p2); st++) {
      cont->SetStrategy(p_support.GetStrategy(p2, st));
      Rational x = cont->GetPayoff(p1) - min;

      num[st] = -x.numerator().as_long();
      den[st] = x.denominator().as_long();
    }

    num[p_support.NumStrategies(p2)+1] = 1;
    den[p_support.NumStrategies(p2)+1] = 1;
    lrs_set_row(P, Q, row, num, den, GE);
  }
}

void FillLinearityRow(lrs_dic *P, lrs_dat *Q, int m, int n)
{
  const int MAXCOL = 1000;     /* maximum number of columns */
  long num[MAXCOL], den[MAXCOL];

  num[0] = -1;
  den[0] = 1;

  for (int i = 1; i < n-1; i++) {
    num[i] = 1;
    den[i] = 1;
  }

  num[n-1] = 0;
  den[n-1] = 1;

  lrs_set_row(P, Q, m, num, den, EQ);
}

//
// Build the H-representation for player p1
//
void BuildRep(lrs_dic *P, lrs_dat *Q, const StrategySupport &p_support,
	      int p1, int p2)
{
  long m=Q->m;       /* number of inequalities      */
  long n=Q->n;       

  if (p1 == 1) {
    FillConstraintRows(P, Q, p_support, p1, p2, 1);
    FillNonnegativityRows(P, Q, p_support.NumStrategies(p1) + 1,
			  p_support.MixedProfileLength(), n);
  }
  else {
    FillNonnegativityRows(P, Q, 1, p_support.NumStrategies(p2), n);
    FillConstraintRows(P, Q, p_support, p1, p2,
		       p_support.NumStrategies(p2) + 1);
  }
  FillLinearityRow(P, Q, m, n);
}

//========================================================================
//               Forward declarations of useful functions
//========================================================================

/* lrs driver, argv[2]= 2nd input file for nash equilibria */
long nash2_main (lrs_dic *P1, lrs_dat *Q1, lrs_dic *P2orig,
		 lrs_dat *Q2, long *numequilib, 
		 lrs_mp_vector output1, lrs_mp_vector output2,
		 const StrategySupport &p_support);

long lrs_getfirstbasis2 (lrs_dic ** D_p, lrs_dat * Q, lrs_dic *P2orig,
			 lrs_mp_matrix * Lin, long no_output);

long getabasis2 (lrs_dic * P, lrs_dat * Q, lrs_dic * P2orig, long order[]);

// This is a modified version of lrs_output from the original, in which
// we output the equilibria found in Gambit format.
void nashoutput(lrs_dat *Q1, lrs_mp_vector output1,
		lrs_dat *Q2, lrs_mp_vector output2,
		const StrategySupport &p_support);



//
// This is the main function, based on main() from lrslib's 'nash' driver.
//
void LrsSolve(const StrategySupport &p_support)
{
  lrs_dic *P1,*P2; /* structure for holding current dictionary and indices */
  lrs_dat *Q1,*Q2; /* structure for holding static problem data            */

  lrs_mp_vector output1; /* holds one line of output; ray,vertex,facet,linearity */
  lrs_mp_vector output2; /* holds one line of output; ray,vertex,facet,linearity */
  lrs_mp_matrix Lin;	/* holds input linearities if any are found             */

  lrs_dic *P2orig;  /* we will save player 2's dictionary in getabasis      */

  long col;	    /* output column index for dictionary                   */
  long startcol = 0;
  long prune = FALSE;		/* if TRUE, getnextbasis will prune tree and backtrack  */
  long numequilib=0;            /* number of nash equilibria found                      */
  long oldnum=0;                                                                            
/* global variables lrs_ifp and lrs_ofp are file pointers for input and output   */
/* they default to stdin and stdout, but may be overidden by command line parms. */

/***************************************************
 Step 0: 
  Do some global initialization that should only be done once,
  no matter how many lrs_dat records are allocated. db

***************************************************/

  if (!lrs_init("")) {
    return;
  }

/*********************************************************************************/
/* Step 1: Allocate lrs_dat, lrs_dic and set up the problem                      */
/*********************************************************************************/


  Q1 = lrs_alloc_dat ("LRS globals");	/* allocate and init structure for static problem data */
  if (Q1 == NULL) {
    return;
  }

  Q1->nash=TRUE;
  Q1->n = p_support.NumStrategies(1) + 2;   
  Q1->m = p_support.MixedProfileLength() + 1;

  P1 = lrs_alloc_dic (Q1);	/* allocate and initialize lrs_dic */
  if (P1 == NULL) {
    return;
  }

  BuildRep(P1, Q1, p_support, 2, 1);

  output1 = lrs_alloc_mp_vector (Q1->n + Q1->m);   /* output holds one line of output from dictionary     */

  /* allocate and init structure for player 2's problem data */
  Q2 = lrs_alloc_dat ("LRS globals"); 
  if (Q2 == NULL) {
    return;
  }

  Q2->nash=TRUE;
  Q2->n = p_support.NumStrategies(2) + 2;   
  Q2->m = p_support.MixedProfileLength() + 1;

  P2 = lrs_alloc_dic (Q2);	/* allocate and initialize lrs_dic */
  if (P2 == NULL) {
    return;
  }
  BuildRep(P2, Q2, p_support, 1, 2);

  output2 = lrs_alloc_mp_vector (Q2->n + Q2->m);   /* output holds one line of output from dictionary     */

  P2orig = lrs_getdic(Q2);  	     /* allocate and initialize lrs_dic                     */
  if (P2orig == NULL)
    return;
  copy_dict(Q2,P2orig,P2);

/*********************************************************************************/
/* Step 2: Find a starting cobasis from default of specified order               */
/*         P1 is created to hold  active dictionary data and may be cached       */
/*         Lin is created if necessary to hold linearity space                   */
/*         Print linearity space if any, and retrieve output from first dict.    */
/*********************************************************************************/

  if (!lrs_getfirstbasis (&P1, Q1, &Lin, TRUE))
    return;

  if (Q1->dualdeg)
     {
      printf("\n*Warning! Dual degenerate, ouput may be incomplete");
      printf("\n*Recommendation: Add dualperturb option before maximize in first input file\n");
     }

  if (Q1->unbounded)
     {
      printf("\n*Warning! Unbounded starting dictionary for p1, output may be incomplete");
      printf("\n*Recommendation: Change/remove maximize option, or include bounds \n");
     }

  /* Pivot to a starting dictionary                      */
  /* There may have been column redundancy               */
  /* If so the linearity space is obtained and redundant */
  /* columns are removed. User can access linearity space */
  /* from lrs_mp_matrix Lin dimensions nredundcol x d+1  */



  if (Q1->homogeneous && Q1->hull)
    startcol++;			/* col zero not treated as redundant   */

  for (col = startcol; col < Q1->nredundcol; col++)	/* print linearity space               */
    lrs_printoutput (Q1, Lin[col]);	/* Array Lin[][] holds the coeffs.     */

/*********************************************************************************/
/* Step 3: Terminate if lponly option set, otherwise initiate a reverse          */
/*         search from the starting dictionary. Get output for each new dict.    */
/*********************************************************************************/

  /* We initiate reverse search from this dictionary       */
  /* getting new dictionaries until the search is complete */
  /* User can access each output line from output which is */
  /* vertex/ray/facet from the lrs_mp_vector output         */
  /* prune is TRUE if tree should be pruned at current node */
  do
    {
      prune=lrs_checkbound(P1,Q1);
      if (!prune && lrs_getsolution (P1, Q1, output1, col))
	{ 
           oldnum=numequilib;
           nash2_main(P1,Q1,P2orig,Q2,&numequilib,output1,output2,p_support);
	   if (numequilib > oldnum || Q1->verbose)
	      {
                if(Q1->verbose)
                  prat(" \np2's obj value: ",P1->objnum,P1->objden);
       	        //fprintf (lrs_ofp, "\n");
	      }
	}
    }
  while (lrs_getnextbasis (&P1, Q1, prune));

  lrs_clear_mp_vector(output1, Q1->m + Q1->n);
  lrs_clear_mp_vector(output2, Q2->m + Q2->n);

  
  Q2->Qhead = P2;                /* reset this or you crash free_dic */
  lrs_free_dic (P2,Q2);          /* deallocate lrs_dic */
  lrs_free_dat (Q2);             /* deallocate lrs_dat */

  lrs_free_dic (P1,Q1);          /* deallocate lrs_dic */
  lrs_free_dat (Q1);             /* deallocate lrs_dat */


  lrs_close ("");
  fclose(lrs_ofp);
}
/*********************************************/
/* end of nash driver                        */
/*********************************************/


/**********************************************************/
/* nash2_main is a second driver used in computing nash   */
/* equilibria on a second polytope interleaved with first */
/**********************************************************/

long nash2_main (lrs_dic *P1, lrs_dat *Q1, lrs_dic *P2orig, 
		 lrs_dat *Q2, long *numequilib, 
		 lrs_mp_vector output1, lrs_mp_vector output2,
		 const StrategySupport &p_support)


{

  lrs_dic *P2;                  /* This can get resized, cached etc. Loaded from P2orig */
  lrs_mp_matrix Lin;		/* holds input linearities if any are found             */
  long col;			/* output column index for dictionary                   */
  long startcol = 0;
  long prune = FALSE;		/* if TRUE, getnextbasis will prune tree and backtrack  */
  long nlinearity;
  long *linearity;
  static long firstwarning=TRUE;    /* FALSE if dual deg warning for Q2 already given     */
  static long firstunbounded=TRUE;  /* FALSE if dual deg warning for Q2 already given     */

  long i,j;

/* global variables lrs_ifp and lrs_ofp are file pointers for input and output   */
/* they default to stdin and stdout, but may be overidden by command line parms. */


/*********************************************************************************/
/* Step 1: Allocate lrs_dat, lrs_dic and set up the problem                      */
/*********************************************************************************/


  P2=lrs_getdic(Q2);
  copy_dict(Q2,P2,P2orig);

/* Here we take the linearities generated by the current vertex of player 1*/
/* and append them to the linearity in player 2's input matrix             */ 
/* next is the key magic linking player 1 and 2 */
/* be careful if you mess with this!            */

  linearity=Q2->linearity;
  nlinearity=0;
       for(i=Q1->lastdv+1;i <= P1->m; i++)
        {
           if (!zero(P1->A[P1->Row[i]][0]))
           {
             j =  Q1->inequality[P1->B[i]-Q1->lastdv];
             if (j < Q1->linearity[0])
	         linearity[nlinearity++]= j;
	   }
         }
/* add back in the linearity for probs summing to one */
       linearity[nlinearity++]= Q1->linearity[0];


/*sort linearities */
  for (i = 1; i < nlinearity; i++)	
    reorder (linearity, nlinearity);

  if(Q2->verbose)
  {
       fprintf(lrs_ofp,"\np2: linearities %ld",nlinearity);
       for (i=0;i < nlinearity; i++)
	       fprintf(lrs_ofp," %ld",linearity[i]);
  }    

  Q2->nlinearity = nlinearity;
  Q2->polytope = FALSE;


/*********************************************************************************/
/* Step 2: Find a starting cobasis from default of specified order               */
/*         P2 is created to hold  active dictionary data and may be cached        */
/*         Lin is created if necessary to hold linearity space                   */
/*         Print linearity space if any, and retrieve output from first dict.    */
/*********************************************************************************/

  if (!lrs_getfirstbasis2 (&P2, Q2, P2orig, &Lin, TRUE))
    goto sayonara;
  if (firstwarning && Q2->dualdeg)
     {
      firstwarning=FALSE;
      printf("\n*Warning! Dual degenerate, ouput may be incomplete");
      printf("\n*Recommendation: Add dualperturb option before maximize in second input file\n");
     }
  if (firstunbounded && Q2->unbounded)
     {
      firstunbounded=FALSE;
      printf("\n*Warning! Unbounded starting dictionary for p2, output may be incomplete");
      printf("\n*Recommendation: Change/remove maximize option, or include bounds \n");
     }

  /* Pivot to a starting dictionary                      */
  /* There may have been column redundancy               */
  /* If so the linearity space is obtained and redundant */
  /* columns are removed. User can access linearity space */
  /* from lrs_mp_matrix Lin dimensions nredundcol x d+1  */



  if (Q2->homogeneous && Q2->hull)
    startcol++;                 /* col zero not treated as redundant   */


  /* for (col = startcol; col < Q2->nredundcol; col++)*/	/* print linearity space               */
    /*lrs_printoutput (Q2, Lin[col]);*/	/* Array Lin[][] holds the coeffs.     */



/*********************************************************************************/
/* Step 3: Terminate if lponly option set, otherwise initiate a reverse          */
/*         search from the starting dictionary. Get output for each new dict.    */
/*********************************************************************************/



  /* We initiate reverse search from this dictionary       */
  /* getting new dictionaries until the search is complete */
  /* User can access each output line from output which is */
  /* vertex/ray/facet from the lrs_mp_vector output         */
  /* prune is TRUE if tree should be pruned at current node */
  do
    {
        prune=lrs_checkbound(P2,Q2);
        col=0;
	if (!prune && lrs_getsolution (P2, Q2, output2, col))
	{
	    (*numequilib)++;
             if (Q2->verbose)
                  prat(" \np1's obj value: ",P2->objnum,P2->objden);
	     nashoutput(Q1, output1, Q2, output2, p_support);
	}
    }
  while (lrs_getnextbasis (&P2, Q2, prune));

sayonara:
  lrs_free_dic(P2,Q2);
  return 0;

}
/*********************************************/
/* end of nash2_main                          */
/*********************************************/


/* In lrs_getfirstbasis and lrs_getnextbasis we use D instead of P */
/* since the dictionary P may change, ie. &P in calling routine    */

#define D (*D_p)

long 
lrs_getfirstbasis2 (lrs_dic ** D_p, lrs_dat * Q, lrs_dic * P2orig, lrs_mp_matrix * Lin, long no_output)
/* gets first basis, FALSE if none              */
/* P may get changed if lin. space Lin found    */
/* no_output is TRUE supresses output headers   */
{
  long i, j, k;

/* assign local variables to structures */

  lrs_mp_matrix A;
  long *B, *C, *Row, *Col;
  long *inequality;
  long *linearity;
  long hull = Q->hull;
  long m, d, lastdv, nlinearity, nredundcol;

  static long ocount=0;


  m = D->m;
  d = D->d;
  lastdv = Q->lastdv;

  nredundcol = 0L;		/* will be set after getabasis        */
  nlinearity = Q->nlinearity;	/* may be reset if new linearity read */
  linearity = Q->linearity;

  A = D->A;
  B = D->B;
  C = D->C;
  Row = D->Row;
  Col = D->Col;
  inequality = Q->inequality;

/* default is to look for starting cobasis using linearies first, then     */
/* filling in from last rows of input as necessary                         */
/* linearity array is assumed sorted here                                  */
/* note if restart/given start inequality indices already in place         */
/* from nlinearity..d-1                                                    */

  for (i = 0; i < nlinearity; i++)      /* put linearities first in the order */
    inequality[i] = linearity[i];


  k = 0;			/* index for linearity array   */

  if (Q->givenstart)
    k = d;
  else
    k = nlinearity;
  for (i = m; i >= 1; i--)
    {
      j = 0;
      while (j < k && inequality[j] != i)
	j++;			/* see if i is in inequality  */
      if (j == k)
	inequality[k++] = i;
    }
  if (Q->debug)
    {
      fprintf (lrs_ofp, "\n*Starting cobasis uses input row order");
      for (i = 0; i < m; i++)
	fprintf (lrs_ofp, " %ld", inequality[i]);
    }

  if (!Q->maximize && !Q->minimize)
    for (j = 0; j <= d; j++)
      itomp (ZERO, A[0][j]);

/* Now we pivot to standard form, and then find a primal feasible basis       */
/* Note these steps MUST be done, even if restarting, in order to get         */
/* the same index/inequality correspondance we had for the original prob.     */
/* The inequality array is used to give the insertion order                   */
/* and is defaulted to the last d rows when givenstart=FALSE                  */

  if (!getabasis2 (D, Q,P2orig, inequality))
          return FALSE;

  if(Q->debug)
  {
    fprintf(lrs_ofp,"\nafter getabasis2");
    printA(D, Q);
  }
  nredundcol = Q->nredundcol;
  lastdv = Q->lastdv;
  d = D->d;

/********************************************************************/
/* now we start printing the output file  unless no output requested */
/********************************************************************/
  if (!no_output || Q->debug)
    {
      fprintf (lrs_ofp, "\nV-representation");

/* Print linearity space                 */
/* Don't print linearity if first column zero in hull computation */

      k = 0;

     if (nredundcol > k)
	{
	  fprintf (lrs_ofp, "\nlinearity %ld ", nredundcol - k);	/*adjust nredundcol for homog. */
	  for (i = 1; i <= nredundcol - k; i++)
	    fprintf (lrs_ofp, " %ld", i);
	}			/* end print of linearity space */

      fprintf (lrs_ofp, "\nbegin");
      fprintf (lrs_ofp, "\n***** %ld rational", Q->n);

    }				/* end of if !no_output .......   */

/* Reset up the inequality array to remember which index is which input inequality */
/* inequality[B[i]-lastdv] is row number of the inequality with index B[i]              */
/* inequality[C[i]-lastdv] is row number of the inequality with index C[i]              */

  for (i = 1; i <= m; i++)
    inequality[i] = i;
  if (nlinearity > 0)		/* some cobasic indices will be removed */
    {
      for (i = 0; i < nlinearity; i++)	/* remove input linearity indices */
	inequality[linearity[i]] = 0;
      k = 1;			/* counter for linearities         */
      for (i = 1; i <= m - nlinearity; i++)
	{
	  while (k <= m && inequality[k] == 0)
	    k++;		/* skip zeroes in corr. to linearity */
	  inequality[i] = inequality[k++];
	}
    }				/* end if linearity */
  if (Q->debug)
    {
      fprintf (lrs_ofp, "\ninequality array initialization:");
      for (i = 1; i <= m - nlinearity; i++)
	fprintf (lrs_ofp, " %ld", inequality[i]);
    }
  if (nredundcol > 0)
    {
      *Lin = lrs_alloc_mp_matrix (nredundcol, Q->n);

      for (i = 0; i < nredundcol; i++)
	{
	  if (!(Q->homogeneous && Q->hull && i == 0))	/* skip redund col 1 for homog. hull */
	    {
	      lrs_getray (D, Q, Col[0], D->C[0] + i - hull, (*Lin)[i]);		/* adjust index for deletions */
	    }

	  if (!removecobasicindex (D, Q, 0L))
	    return FALSE;
	}
    }				/* end if nredundcol > 0 */

      if (Q->verbose)
      {
      fprintf (lrs_ofp, "\nNumber of pivots for starting dictionary: %ld",Q->count[3]);
      ocount=Q->count[3];
      }

/* Do dual pivots to get primal feasibility */
  if (!primalfeasible (D, Q))
    {
     if ( Q->verbose )
      {
          fprintf (lrs_ofp, "\nNumber of pivots for feasible solution: %ld",Q->count[3]);
          fprintf (lrs_ofp, " - No feasible solution");
          ocount=Q->count[3];
      }
      return FALSE;
    }

    if (Q->verbose)
     {
      fprintf (lrs_ofp, "\nNumber of pivots for feasible solution: %ld",Q->count[3]);
      ocount=Q->count[3];
     }


/* Now solve LP if objective function was given */
  if (Q->maximize || Q->minimize)
    {
      Q->unbounded = !lrs_solvelp (D, Q, Q->maximize);

      /* check to see if objective is dual degenerate */
      j = 1;
      while (j <= d && !zero (A[0][j]))
      j++;
      if (j <= d)
	    Q->dualdeg = TRUE;
    }
  else
/* re-initialize cost row to -det */
    {
      for (j = 1; j <= d; j++)
	{
	  copy (A[0][j], D->det);
	  storesign (A[0][j], NEG);
	}

      itomp (ZERO, A[0][0]);	/* zero optimum objective value */
    }


/* reindex basis to 0..m if necessary */
/* we use the fact that cobases are sorted by index value */
  if (Q->debug)
    printA (D, Q);
  while (C[0] <= m)
    {
      i = C[0];
      j = inequality[B[i] - lastdv];
      inequality[B[i] - lastdv] = inequality[C[0] - lastdv];
      inequality[C[0] - lastdv] = j;
      C[0] = B[i];
      B[i] = i;
      reorder1 (C, Col, ZERO, d);
    }

  if (Q->debug)
    {
      fprintf (lrs_ofp, "\n*Inequality numbers for indices %ld .. %ld : ", lastdv + 1, m + d);
      for (i = 1; i <= m - nlinearity; i++)
	fprintf (lrs_ofp, " %ld ", inequality[i]);
      printA (D, Q);
    }



  if (Q->restart)
    {
      if (Q->debug)
	fprintf (lrs_ofp, "\nPivoting to restart co-basis");
      if (!restartpivots (D, Q))
	return FALSE;
      D->lexflag = lexmin (D, Q, ZERO);		/* see if lexmin basis */
      if (Q->debug)
	printA (D, Q);
    }
/* Check to see if necessary to resize */
  if (Q->inputd > D->d)
    *D_p = resize (D, Q);

  return TRUE;
}
/********* end of lrs_getfirstbasis  ***************/
long 
getabasis2 (lrs_dic * P, lrs_dat * Q, lrs_dic * P2orig, long order[])

/* Pivot Ax<=b to standard form */
/*Try to find a starting basis by pivoting in the variables x[1]..x[d]        */
/*If there are any input linearities, these appear first in order[]           */
/* Steps: (a) Try to pivot out basic variables using order                    */
/*            Stop if some linearity cannot be made to leave basis            */
/*        (b) Permanently remove the cobasic indices of linearities           */
/*        (c) If some decision variable cobasic, it is a linearity,           */
/*            and will be removed.                                            */

{
  long i, j, k;
/* assign local variables to structures */
  lrs_mp_matrix A = P->A;
  long *B = P->B;
  long *C = P->C;
  long *Row = P->Row;
  long *Col = P->Col;
  long *linearity = Q->linearity;
  long *redundcol = Q->redundcol;
  long m, d, nlinearity;
  long nredundcol = 0L;		/* will be calculated here */

  static long firsttime=TRUE;
  static long *linindex;

  m = P->m;
  d = P->d;
  nlinearity = Q->nlinearity;

  if(firsttime)
  {
    firsttime = FALSE;
    linindex = (long int *) calloc ((m + d + 2), sizeof (long));
  }
  else     /* after first time we update the change in linearities from the last time, saving many pivots */
  {
    for(i=1;i<=m+d;i++)
	  linindex[i]=FALSE;
    if(Q->debug)
        fprintf(lrs_ofp,"\nlindex =");
    for(i=0;i<nlinearity;i++)
    {
       	  linindex[d+linearity[i]]=TRUE;
	  if(Q->debug)
             fprintf(lrs_ofp,"  %ld",d+linearity[i]);		   
    }
	  
    for(i=1;i<=m;i++)
    {
	  if(linindex[B[i]])  /* pivot out unwanted linearities */
	  {
		  k=0;
		  while(k<d && (linindex[C[k]] ||  zero (A[Row[i]][Col[k]])))
			  k++;

                  if (k < d)
                  {
	            j=i;   /* note this index changes in update, cannot use i!)*/

		    if(C[k] > B[j])  /* decrease i or we may skip a linearity */
		       i--;
	            pivot (P, Q, j, k);
		    update (P, Q, &j, &k);
                   }
		   else
                     if(Q->debug || Q->verbose)
		        fprintf(lrs_ofp,"\n*Couldn't remove linearity i=%ld B[i]=%ld",i,B[i]);		   
                     /* this is not necessarily an error, eg. two identical rows/cols in payoff matrix */
                   }
           }
   goto hotstart;
  }

/* standard lrs processing is done on only the first call to getabasis2 */

  if (Q->debug)
    {
      fprintf (lrs_ofp, "\ngetabasis from inequalities given in order");
      for (i = 0; i < m; i++)
	fprintf (lrs_ofp, " %ld", order[i]);
    }
  for (j = 0; j < m; j++)
    {
      i = 0;
      while (i <= m && B[i] != d + order[j])
	i++;			/* find leaving basis index i */
      if (j < nlinearity && i > m)	/* cannot pivot linearity to cobasis */
	{
	  if (Q->debug)
	    printA (P, Q);
#ifndef LRS_QUIET
	  fprintf (lrs_ofp, "\nCannot find linearity in the basis");
#endif
	  return FALSE;
	}
      if (i <= m)
	{			/* try to do a pivot */
	  k = 0;
	  while (C[k] <= d && zero (A[Row[i]][Col[k]]))
	    k++;

	  if (C[k] <= d)
	    {
	      pivot (P, Q, i, k);
	      update (P, Q, &i, &k);
	    }
	  else if (j < nlinearity)
	    {			/* cannot pivot linearity to cobasis */
	      if (zero (A[Row[i]][0]))
		{
#ifndef LRS_QUIET
		  fprintf (lrs_ofp, "\n*Input linearity in row %ld is redundant--skipped\n", order[j]);
#endif
		  linearity[j] = 0;
		}
	      else
		{
		  if (Q->debug)
		    printA (P, Q);
		  if (Q->verbose)
		    fprintf (lrs_ofp, "\nInconsistent linearities");
		  return FALSE;
		}
	    }			/* end if j < nlinearity */

	}			/* end of if i <= m .... */
    }				/* end of for   */

/* update linearity array to get rid of redundancies */
  i = 0;
  k = 0;			/* counters for linearities         */
  while (k < nlinearity)
    {
      while (k < nlinearity && linearity[k] == 0)
	k++;
      if (k < nlinearity)
	linearity[i++] = linearity[k++];
    }

  nlinearity = i;

/* column dependencies now can be recorded  */
/* redundcol contains input column number 0..n-1 where redundancy is */
  k = 0;
  while (k < d && C[k] <= d)
    {
      if (C[k] <= d)		/* decision variable still in cobasis */
	redundcol[nredundcol++] = C[k] - Q->hull;	/* adjust for hull indices */
      k++;
    }

/* now we know how many decision variables remain in problem */
  Q->nredundcol = nredundcol;
  Q->lastdv = d - nredundcol;

  /* if not first time we continue from here after loading dictionary */

hotstart:

  if (Q->debug)
    {
      fprintf (lrs_ofp, "\nend of first phase of getabasis2: ");
      fprintf (lrs_ofp, "lastdv=%ld nredundcol=%ld", Q->lastdv, Q->nredundcol);
      fprintf (lrs_ofp, "\nredundant cobases:");
      for (i = 0; i < nredundcol; i++)
	fprintf (lrs_ofp, " %ld", redundcol[i]);
      printA (P, Q);
    }

/* here we save dictionary for use next time, *before* we resize */

  copy_dict(Q,P2orig,P);

/* Remove linearities from cobasis for rest of computation */
/* This is done in order so indexing is not screwed up */

  for (i = 0; i < nlinearity; i++)
    {				/* find cobasic index */
      k = 0;
      while (k < d && C[k] != linearity[i] + d)
	k++;
      if (k >= d)
	{
          if(Q->debug || Q->verbose)
	    fprintf (lrs_ofp, "\nCould not remove cobasic index");
          /* not neccesarily an error as eg., could be repeated row/col in payoff */
	}
      else
         { 
              removecobasicindex (P, Q, k);
              d = P->d;
         }
    }
  if (Q->debug && nlinearity > 0)
    printA (P, Q);
/* set index value for first slack variable */

/* Check feasability */
  if (Q->givenstart)
    {
      i = Q->lastdv + 1;
      while (i <= m && !negative (A[Row[i]][0]))
	i++;
      if (i <= m)
	fprintf (lrs_ofp, "\n*Infeasible startingcobasis - will be modified");
    }
  return TRUE;
}				/*  end of getabasis2 */

//==========================================================================
//                         Outputting equilibria
//==========================================================================

// This is just a modified 'prat' from lrsmp.c
void 
printrat (const char *name, lrs_mp Nin, lrs_mp Din)	/*reduce and print Nin/Din  */
{
  lrs_mp Nt, Dt;
  long i;
  fprintf (stdout, "%s", name);
/* reduce fraction */
  copy (Nt, Nin);
  copy (Dt, Din);
  reduce (Nt, Dt);
/* print out       */
  if (sign (Nin) * sign (Din) == NEG)
    fprintf (stdout, "-");
  //  else
  //  fprintf (stdout, "");
  fprintf (stdout, "%lu", Nt[length (Nt) - 1]);
  for (i = length (Nt) - 2; i >= 1; i--)
    fprintf (stdout, FORMAT, Nt[i]);
  if (!(Dt[0] == 2 && Dt[1] == 1))	/* rational */
    {
      fprintf (stdout, "/");
      fprintf (stdout, "%lu", Dt[length (Dt) - 1]);
      for (i = length (Dt) - 2; i >= 1; i--)
	fprintf (stdout, FORMAT, Dt[i]);
    }
}

void
nashoutput(lrs_dat *Q1, lrs_mp_vector output1,
	   lrs_dat *Q2, lrs_mp_vector output2,
	   const StrategySupport &p_support)
{
  std::cout << "NE";
  // The -1 is because the last entry in the vector is the payoff
  // of the other player
  long i = 1;

  GamePlayer player1 = p_support.GetGame()->GetPlayer(1);
  for (int j = 1; j <= player1->NumStrategies(); j++) {
    if (p_support.Contains(player1->GetStrategy(j))) {
      std::cout << ",";
      printrat("", output1[i++], output1[0]);
    }
    else {
      std::cout << ",0";
    }
  }

  i = 1;
  GamePlayer player2 = p_support.GetGame()->GetPlayer(2);
  for (int j = 1; j <= player2->NumStrategies(); j++) {
    if (p_support.Contains(player2->GetStrategy(j))) {
      std::cout << ",";
      printrat("", output2[i++], output2[0]);
    }
    else {
      std::cout << ",0";
    }
  }
  std::cout << std::endl;
  fflush(stdout);
}

