/*******************************************************/
/* lrsnashlib is a library of routines for computing   */
/* computing all nash equilibria for two person games  */
/* given by mxn payoff matrices A,B                    */
/*                                                     */
/*                                                     */
/* Main user callable function is                      */
/*         lrs_solve_nash(game *g)                     */
/*                                                     */
/* Requires lrsnashlib.h lrslib.h lrslib.c             */
/*                                                     */
/* Sample driver: lrsnash.c                            */
/* Derived from nash.c in lrslib-060                   */
/* by Terje Lensberg, October 26, 2015:                */
/*******************************************************/

#include <stdio.h>
#include <string.h>
#include "lrslib.h"
#include "lrsnashlib.h"

//========================================================================
// Standard solver. Modified version of main() from lrsNash
//========================================================================
int lrs_solve_nash(game *g)
{
  lrs_dic *P1 /*, *P2*/; /* structure for holding current dictionary and indices */
  lrs_dat *Q1, *Q2;      /* structure for holding static problem data            */

  lrs_mp_vector output1; /* holds one line of output; ray,vertex,facet,linearity */
  lrs_mp_vector output2; /* holds one line of output; ray,vertex,facet,linearity */
  lrs_mp_matrix Lin;     /* holds input linearities if any are found             */
  lrs_mp_matrix A2orig;
  lrs_dic *P2orig; /* we will save player 2's dictionary in getabasis      */

  long *linindex; /* for faster restart of player 2                       */

  long col; /* output column index for dictionary                   */
  long startcol = 0;
  long prune = FALSE;  /* if TRUE, getnextbasis will prune tree and backtrack  */
  long numequilib = 0; /* number of nash equilibria found                      */
  long oldnum = 0;

  /* global variables lrs_ifp and lrs_ofp are file pointers for input and output   */
  /* they default to stdin and stdout, but may be overidden by command line parms. */

  /*********************************************************************************/
  /* Step 1: Allocate lrs_dat, lrs_dic and set up the problem                      */
  /*********************************************************************************/
  FirstTime = TRUE; /* This is done for each new game */

  Q1 = lrs_alloc_dat("LRS globals"); /* allocate and init structure for static problem data */
  if (Q1 == NULL) {
    return 1;
  }

  Q1->nash = TRUE;
  Q1->n = g->nstrats[ROW] + 2;
  Q1->m = g->nstrats[ROW] + g->nstrats[COL] + 1;

  Q1->debug = Debug_flag;
  Q1->verbose = Verbose_flag;

  P1 = lrs_alloc_dic(Q1); /* allocate and initialize lrs_dic */
  if (P1 == NULL) {
    return 1;
  }

  BuildRep(P1, Q1, g, 1, 0);

  output1 =
      lrs_alloc_mp_vector(Q1->n + Q1->m); /* output holds one line of output from dictionary     */

  /* allocate and init structure for player 2's problem data */
  Q2 = lrs_alloc_dat("LRS globals");
  if (Q2 == NULL) {
    return 1;
  }

  Q2->debug = Debug_flag;
  Q2->verbose = Verbose_flag;

  Q2->nash = TRUE;
  Q2->n = g->nstrats[COL] + 2;
  Q2->m = g->nstrats[ROW] + g->nstrats[COL] + 1;

  P2orig = lrs_alloc_dic(Q2); /* allocate and initialize lrs_dic */
  if (P2orig == NULL) {
    return 1;
  }
  BuildRep(P2orig, Q2, g, 0, 1);
  A2orig = P2orig->A;

  output2 =
      lrs_alloc_mp_vector(Q1->n + Q1->m); /* output holds one line of output from dictionary     */

  linindex = calloc((P2orig->m + P2orig->d + 2), sizeof(long)); /* for next time */

  fprintf(lrs_ofp, "\n");
  //  fprintf (lrs_ofp, "***** %ld %ld rational\n", Q1->n, Q2->n);

  /*********************************************************************************/
  /* Step 2: Find a starting cobasis from default of specified order               */
  /*         P1 is created to hold  active dictionary data and may be cached       */
  /*         Lin is created if necessary to hold linearity space                   */
  /*         Print linearity space if any, and retrieve output from first dict.    */
  /*********************************************************************************/

  if (!lrs_getfirstbasis(&P1, Q1, &Lin, TRUE)) {
    return 1;
  }

  if (Q1->dualdeg) {
    printf("\n*Warning! Dual degenerate, ouput may be incomplete");
    printf("\n*Recommendation: Add dualperturb option before maximize in first input file\n");
  }

  if (Q1->unbounded) {
    printf("\n*Warning! Unbounded starting dictionary for p1, output may be incomplete");
    printf("\n*Recommendation: Change/remove maximize option, or include bounds \n");
  }

  /* Pivot to a starting dictionary                      */
  /* There may have been column redundancy               */
  /* If so the linearity space is obtained and redundant */
  /* columns are removed. User can access linearity space */
  /* from lrs_mp_matrix Lin dimensions nredundcol x d+1  */

  if (Q1->homogeneous && Q1->hull) {
    startcol++; /* col zero not treated as redundant   */
  }

  for (col = startcol; col < Q1->nredundcol; col++) { /* print linearity space               */
    lrs_printoutput(Q1, Lin[col]);                    /* Array Lin[][] holds the coeffs.     */
  }

  /*********************************************************************************/
  /* Step 3: Terminate if lponly option set, otherwise initiate a reverse          */
  /*         search from the starting dictionary. Get output for each new dict.    */
  /*********************************************************************************/

  /* We initiate reverse search from this dictionary       */
  /* getting new dictionaries until the search is complete */
  /* User can access each output line from output which is */
  /* vertex/ray/facet from the lrs_mp_vector output         */
  /* prune is TRUE if tree should be pruned at current node */
  do {
    prune = lrs_checkbound(P1, Q1);
    if (!prune && lrs_getsolution(P1, Q1, output1, col)) {
      oldnum = numequilib;
      nash2_main(P1, Q1, P2orig, Q2, &numequilib, output2, linindex);
      if (numequilib > oldnum || Q1->verbose) {
        if (Q1->verbose) {
          prat(" \np2's obj value: ", P1->objnum, P1->objden);
        }
        lrs_nashoutput(Q1, output1, 1L);
        fprintf(lrs_ofp, "\n");
      }
    }
  } while (lrs_getnextbasis(&P1, Q1, prune));

  fprintf(lrs_ofp, "*Number of equilibria found: %ld", numequilib);
  fprintf(lrs_ofp, "\n*Player 1: vertices=%ld bases=%ld pivots=%ld", Q1->count[1], Q1->count[2],
          Q1->count[3]);
  fprintf(lrs_ofp, "\n*Player 2: vertices=%ld bases=%ld pivots=%ld", Q2->count[1], Q2->count[2],
          Q2->count[3]);

  lrs_clear_mp_vector(output1, Q1->m + Q1->n);
  lrs_clear_mp_vector(output2, Q1->m + Q1->n);

  lrs_free_dic(P1, Q1); /* deallocate lrs_dic */
  lrs_free_dat(Q1);     /* deallocate lrs_dat */

  /* 2015.10.10  new code to clear P2orig */
  Q2->Qhead = P2orig; /* reset this or you crash free_dic */
  P2orig->A = A2orig; /* reset this or you crash free_dic */

  lrs_free_dic(P2orig, Q2); /* deallocate lrs_dic */
  lrs_free_dat(Q2);         /* deallocate lrs_dat */

  free(linindex);

  //  lrs_close("nash:");
  fprintf(lrs_ofp, "\n");
  return 0;
}

/*********************************************/
/* end of nash driver                        */
/*********************************************/

/**********************************************************/
/* nash2_main is a second driver used in computing nash   */
/* equilibria on a second polytope interleaved with first */
/**********************************************************/

long nash2_main(lrs_dic *P1, lrs_dat *Q1, lrs_dic *P2orig, lrs_dat *Q2, long *numequilib,
                lrs_mp_vector output, long linindex[])
{

  lrs_dic *P2;       /* This can get resized, cached etc. Loaded from P2orig */
  lrs_mp_matrix Lin; /* holds input linearities if any are found             */
  long col;          /* output column index for dictionary                   */
  long startcol = 0;
  long prune = FALSE; /* if TRUE, getnextbasis will prune tree and backtrack  */
  long nlinearity;
  long *linearity;
  static long firstwarning = TRUE;   /* FALSE if dual deg warning for Q2 already given     */
  static long firstunbounded = TRUE; /* FALSE if dual deg warning for Q2 already given     */

  long i, j;

  /* global variables lrs_ifp and lrs_ofp are file pointers for input and output   */
  /* they default to stdin and stdout, but may be overidden by command line parms. */

  /*********************************************************************************/
  /* Step 1: Allocate lrs_dat, lrs_dic and set up the problem                      */
  /*********************************************************************************/

  P2 = lrs_getdic(Q2);
  copy_dict(Q2, P2, P2orig);

  /* Here we take the linearities generated by the current vertex of player 1*/
  /* and append them to the linearity in player 2's input matrix             */
  /* next is the key magic linking player 1 and 2 */
  /* be careful if you mess with this!            */

  linearity = Q2->linearity;
  nlinearity = 0;
  for (i = Q1->lastdv + 1; i <= P1->m; i++) {
    if (!zero(P1->A[P1->Row[i]][0])) {
      j = Q1->inequality[P1->B[i] - Q1->lastdv];
      if (Q1->nlinearity == 0 || j < Q1->linearity[0]) {
        linearity[nlinearity++] = j;
      }
    }
  }
  /* add back in the linearity for probs summing to one */
  if (Q1->nlinearity > 0) {
    linearity[nlinearity++] = Q1->linearity[0];
  }

  /*sort linearities */
  for (i = 1; i < nlinearity; i++) {
    reorder(linearity, nlinearity);
  }

  if (Q2->verbose) {
    fprintf(lrs_ofp, "\np2: linearities %ld", nlinearity);
    for (i = 0; i < nlinearity; i++) {
      fprintf(lrs_ofp, " %ld", linearity[i]);
    }
  }

  Q2->nlinearity = nlinearity;
  Q2->polytope = FALSE;

  /*********************************************************************************/
  /* Step 2: Find a starting cobasis from default of specified order               */
  /*         P2 is created to hold  active dictionary data and may be cached        */
  /*         Lin is created if necessary to hold linearity space                   */
  /*         Print linearity space if any, and retrieve output from first dict.    */
  /*********************************************************************************/

  if (!lrs_getfirstbasis2(&P2, Q2, P2orig, &Lin, TRUE, linindex)) {
    goto sayonara;
  }
  if (firstwarning && Q2->dualdeg) {
    firstwarning = FALSE;
    printf("\n*Warning! Dual degenerate, ouput may be incomplete");
    printf("\n*Recommendation: Add dualperturb option before maximize in second input file\n");
  }
  if (firstunbounded && Q2->unbounded) {
    firstunbounded = FALSE;
    printf("\n*Warning! Unbounded starting dictionary for p2, output may be incomplete");
    printf("\n*Recommendation: Change/remove maximize option, or include bounds \n");
  }

  /* Pivot to a starting dictionary                      */
  /* There may have been column redundancy               */
  /* If so the linearity space is obtained and redundant */
  /* columns are removed. User can access linearity space */
  /* from lrs_mp_matrix Lin dimensions nredundcol x d+1  */

  if (Q2->homogeneous && Q2->hull) {
    startcol++; /* col zero not treated as redundant   */
  }

  /* for (col = startcol; col < Q2->nredundcol; col++) */ /* print linearity space               */
  /*lrs_printoutput (Q2, Lin[col]); */                    /* Array Lin[][] holds the coeffs.     */

  /*********************************************************************************/
  /* Step 3: Terminate if lponly option set, otherwise initiate a reverse          */
  /*         search from the starting dictionary. Get output for each new dict.    */
  /*********************************************************************************/

  /* We initiate reverse search from this dictionary       */
  /* getting new dictionaries until the search is complete */
  /* User can access each output line from output which is */
  /* vertex/ray/facet from the lrs_mp_vector output         */
  /* prune is TRUE if tree should be pruned at current node */
  do {
    prune = lrs_checkbound(P2, Q2);
    col = 0;
    if (!prune && lrs_getsolution(P2, Q2, output, col)) {
      if (Q2->verbose) {
        prat(" \np1's obj value: ", P2->objnum, P2->objden);
      }
      if (lrs_nashoutput(Q2, output, 2L)) {
        (*numequilib)++;
      }
    }
  } while (lrs_getnextbasis(&P2, Q2, prune));

sayonara:
  lrs_free_dic(P2, Q2);
  return 0;
}

/*********************************************/
/* end of nash2_main                          */
/*********************************************/

/* In lrs_getfirstbasis and lrs_getnextbasis we use D instead of P */
/* since the dictionary P may change, ie. &P in calling routine    */

#define D (*D_p)

long lrs_getfirstbasis2(lrs_dic **D_p, lrs_dat *Q, lrs_dic *P2orig, lrs_mp_matrix *Lin,
                        long no_output, long linindex[])
/* gets first basis, FALSE if none              */
/* P may get changed if lin. space Lin found    */
/* no_output is TRUE supresses output headers   */
{
  long i, j, k;

  /* assign local variables to structures */

  lrs_mp_matrix A;
  long *B, *C, /* *Row, */ *Col;
  long *inequality;
  long *linearity;
  long hull = Q->hull;
  long m, d, lastdv, nlinearity, nredundcol;

  // static long ocount = 0;

  m = D->m;
  d = D->d;
  lastdv = Q->lastdv;

  nredundcol = 0L;            /* will be set after getabasis        */
  nlinearity = Q->nlinearity; /* may be reset if new linearity read */
  linearity = Q->linearity;

  A = D->A;
  B = D->B;
  C = D->C;
  // Row = D->Row;
  Col = D->Col;
  inequality = Q->inequality;

  /* default is to look for starting cobasis using linearies first, then     */
  /* filling in from last rows of input as necessary                         */
  /* linearity array is assumed sorted here                                  */
  /* note if restart/given start inequality indices already in place         */
  /* from nlinearity..d-1                                                    */

  for (i = 0; i < nlinearity; i++) { /* put linearities first in the order */
    inequality[i] = linearity[i];
  }

  k = 0; /* index for linearity array   */

  if (Q->givenstart) {
    k = d;
  }
  else {
    k = nlinearity;
  }
  for (i = m; i >= 1; i--) {
    j = 0;
    while (j < k && inequality[j] != i) {
      j++; /* see if i is in inequality  */
    }
    if (j == k) {
      inequality[k++] = i;
    }
  }
  if (Q->debug) {
    fprintf(lrs_ofp, "\n*Starting cobasis uses input row order");
    for (i = 0; i < m; i++) {
      fprintf(lrs_ofp, " %ld", inequality[i]);
    }
  }

  if (!Q->maximize && !Q->minimize) {
    for (j = 0; j <= d; j++) {
      itomp(ZERO, A[0][j]);
    }
  }

  /* Now we pivot to standard form, and then find a primal feasible basis       */
  /* Note these steps MUST be done, even if restarting, in order to get         */
  /* the same index/inequality correspondance we had for the original prob.     */
  /* The inequality array is used to give the insertion order                   */
  /* and is defaulted to the last d rows when givenstart=FALSE                  */

  if (!getabasis2(D, Q, P2orig, inequality, linindex)) {
    return FALSE;
  }

  if (Q->debug) {
    fprintf(lrs_ofp, "\nafter getabasis2");
    printA(D, Q);
  }
  nredundcol = Q->nredundcol;
  lastdv = Q->lastdv;
  d = D->d;

  /********************************************************************/
  /* now we start printing the output file  unless no output requested */
  /********************************************************************/
  if (!no_output || Q->debug) {
    fprintf(lrs_ofp, "\nV-representation");

    /* Print linearity space                 */
    /* Don't print linearity if first column zero in hull computation */

    k = 0;

    if (nredundcol > k) {
      fprintf(lrs_ofp, "\nlinearity %ld ", nredundcol - k); /*adjust nredundcol for homog. */
      for (i = 1; i <= nredundcol - k; i++) {
        fprintf(lrs_ofp, " %ld", i);
      }
    } /* end print of linearity space */

    fprintf(lrs_ofp, "\nbegin");
    fprintf(lrs_ofp, "\n***** %ld rational", Q->n);

  } /* end of if !no_output .......   */

  /* Reset up the inequality array to remember which index is which input inequality */
  /* inequality[B[i]-lastdv] is row number of the inequality with index B[i]              */
  /* inequality[C[i]-lastdv] is row number of the inequality with index C[i]              */

  for (i = 1; i <= m; i++) {
    inequality[i] = i;
  }
  if (nlinearity > 0) {                /* some cobasic indices will be removed */
    for (i = 0; i < nlinearity; i++) { /* remove input linearity indices */
      inequality[linearity[i]] = 0;
    }
    k = 1; /* counter for linearities         */
    for (i = 1; i <= m - nlinearity; i++) {
      while (k <= m && inequality[k] == 0) {
        k++; /* skip zeroes in corr. to linearity */
      }
      inequality[i] = inequality[k++];
    }
  } /* end if linearity */
  if (Q->debug) {
    fprintf(lrs_ofp, "\ninequality array initialization:");
    for (i = 1; i <= m - nlinearity; i++) {
      fprintf(lrs_ofp, " %ld", inequality[i]);
    }
  }
  if (nredundcol > 0) {
    const unsigned int Qn = Q->n;
    *Lin = lrs_alloc_mp_matrix(nredundcol, Qn);

    for (i = 0; i < nredundcol; i++) {
      if (!(Q->homogeneous && Q->hull && i == 0)) { /* skip redund col 1 for homog. hull */
        lrs_getray(D, Q, Col[0], D->C[0] + i - hull, (*Lin)[i]); /* adjust index for deletions */
      }

      if (!removecobasicindex(D, Q, 0L)) {
        lrs_clear_mp_matrix(*Lin, nredundcol, Qn);
        return FALSE;
      }
    }
  } /* end if nredundcol > 0 */

  if (Q->verbose) {
    fprintf(lrs_ofp, "\nNumber of pivots for starting dictionary: %ld", Q->count[3]);
    // ocount = Q->count[3];
  }

  /* Do dual pivots to get primal feasibility */
  if (!primalfeasible(D, Q)) {
    if (Q->verbose) {
      fprintf(lrs_ofp, "\nNumber of pivots for feasible solution: %ld", Q->count[3]);
      fprintf(lrs_ofp, " - No feasible solution");
      // ocount = Q->count[3];
    }
    return FALSE;
  }

  if (Q->verbose) {
    fprintf(lrs_ofp, "\nNumber of pivots for feasible solution: %ld", Q->count[3]);
    // ocount = Q->count[3];
  }

  /* Now solve LP if objective function was given */
  if (Q->maximize || Q->minimize) {
    Q->unbounded = !lrs_solvelp(D, Q, Q->maximize);

    /* check to see if objective is dual degenerate */
    j = 1;
    while (j <= d && !zero(A[0][j])) {
      j++;
    }
    if (j <= d) {
      Q->dualdeg = TRUE;
    }
  }
  else
  /* re-initialize cost row to -det */
  {
    for (j = 1; j <= d; j++) {
      copy(A[0][j], D->det);
      storesign(A[0][j], NEG);
    }

    itomp(ZERO, A[0][0]); /* zero optimum objective value */
  }

  /* reindex basis to 0..m if necessary */
  /* we use the fact that cobases are sorted by index value */
  if (Q->debug) {
    printA(D, Q);
  }
  while (C[0] <= m) {
    i = C[0];
    // j = inequality[B[i] - lastdv];
    // inequality[B[i] - lastdv] = inequality[C[0] - lastdv];
    // inequality[C[0] - lastdv] = j;
    C[0] = B[i];
    B[i] = i;
    reorder1(C, Col, ZERO, d);
  }

  if (Q->debug) {
    fprintf(lrs_ofp, "\n*Inequality numbers for indices %ld .. %ld : ", lastdv + 1, m + d);
    for (i = 1; i <= m - nlinearity; i++) {
      fprintf(lrs_ofp, " %ld ", inequality[i]);
    }
    printA(D, Q);
  }

  if (Q->restart) {
    if (Q->debug) {
      fprintf(lrs_ofp, "\nPivoting to restart co-basis");
    }
    if (!restartpivots(D, Q)) {
      return FALSE;
    }
    D->lexflag = lexmin(D, Q, ZERO); /* see if lexmin basis */
    if (Q->debug) {
      printA(D, Q);
    }
  }
  /* Check to see if necessary to resize */
  if (Q->inputd > D->d) {
    *D_p = resize(D, Q);
  }

  return TRUE;
}

/********* end of lrs_getfirstbasis  ***************/
long getabasis2(lrs_dic *P, lrs_dat *Q, lrs_dic *P2orig, long order[], long linindex[])

/* Pivot Ax<=b to standard form */
/*Try to find a starting basis by pivoting in the variables x[1]..x[d]        */
/*If there are any input linearities, these appear first in order[]           */
/* Steps: (a) Try to pivot out basic variables using order                    */
/*            Stop if some linearity cannot be made to leave basis            */
/*        (b) Permanently remove the cobasic indices of linearities           */
/*        (c) If some decision variable cobasic, it is a linearity,           */
/*            and will be removed.                                            */
{
  /* 2015.10.10 linindex now preallocated and received as parameter so we can free it */

  //  static long firsttime = TRUE; /* stays true until first valid dictionary built */

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
  long nredundcol = 0L; /* will be calculated here */

  m = P->m;
  d = P->d;
  nlinearity = Q->nlinearity;
  // 2015.9.15
  /* after first time we update the change in linearities from the last time, saving many pivots */
  if (!FirstTime) {
    for (i = 1; i <= m + d; i++) {
      linindex[i] = FALSE;
    }
    if (Q->debug) {
      fprintf(lrs_ofp, "\nlindex =");
    }
    for (i = 0; i < nlinearity; i++) {
      linindex[d + linearity[i]] = TRUE;
      if (Q->debug) {
        fprintf(lrs_ofp, "  %ld", d + linearity[i]);
      }
    }

    for (i = 1; i <= m; i++) {
      if (linindex[B[i]]) { /* pivot out unwanted linearities */
        k = 0;
        while (k < d && (linindex[C[k]] || zero(A[Row[i]][Col[k]]))) {
          k++;
        }

        if (k < d) {
          j = i; /* note this index changes in update, cannot use i!) */

          if (C[k] > B[j]) { /* decrease i or we may skip a linearity */
            i--;
          }
          pivot(P, Q, j, k);
          update(P, Q, &j, &k);
        }
        else {
          /* this is not necessarily an error, eg. two identical rows/cols in payoff matrix */
          if (!zero(A[Row[i]][0])) { /* error condition */
            if (Q->debug || Q->verbose) {
              fprintf(lrs_ofp, "\n*Infeasible linearity i=%ld B[i]=%ld", i, B[i]);
              if (Q->debug) {
                printA(P, Q);
              }
            }
            return (FALSE);
          }
          if (Q->debug || Q->verbose) {
            fprintf(lrs_ofp, "\n*Couldn't remove linearity i=%ld B[i]=%ld", i, B[i]);
          }
        }

      } /* if linindex */
    }   /* for i   .. */
  }
  else { /* we have not had a successful dictionary built from the given linearities */

    /* standard lrs processing is done on only the first call to getabasis2 */

    if (Q->debug) {
      fprintf(lrs_ofp, "\ngetabasis from inequalities given in order");
      for (i = 0; i < m; i++) {
        fprintf(lrs_ofp, " %ld", order[i]);
      }
    }
    for (j = 0; j < m; j++) {
      i = 0;
      while (i <= m && B[i] != d + order[j]) {
        i++; /* find leaving basis index i */
      }
      if (j < nlinearity && i > m) { /* cannot pivot linearity to cobasis */
        if (Q->debug) {
          printA(P, Q);
        }
#ifndef LRS_QUIET
        fprintf(lrs_ofp, "\nCannot find linearity in the basis");
#endif
        return FALSE;
      }
      if (i <= m) { /* try to do a pivot */
        k = 0;
        while (C[k] <= d && zero(A[Row[i]][Col[k]])) {
          k++;
        }

        if (C[k] <= d) {
          pivot(P, Q, i, k);
          update(P, Q, &i, &k);
        }
        else if (j < nlinearity) { /* cannot pivot linearity to cobasis */
          if (zero(A[Row[i]][0])) {
#ifndef LRS_QUIET
            fprintf(lrs_ofp, "\n*Input linearity in row %ld is redundant--skipped", order[j]);
#endif
            linearity[j] = 0;
          }
          else {
            if (Q->debug) {
              printA(P, Q);
            }
            if (Q->debug || Q->verbose) {
              fprintf(lrs_ofp, "\nInconsistent linearities");
            }
            return FALSE;
          }
        } /* end if j < nlinearity */

      } /* end of if i <= m .... */
    }   /* end of for   */

    /* update linearity array to get rid of redundancies */
    i = 0;
    k = 0; /* counters for linearities         */
    while (k < nlinearity) {
      while (k < nlinearity && linearity[k] == 0) {
        k++;
      }
      if (k < nlinearity) {
        linearity[i++] = linearity[k++];
      }
    }

    nlinearity = i;
    /* lrs bug fix, 2009.6.27, nash 2015.9.16 */
    Q->nlinearity = i;

    /* column dependencies now can be recorded  */
    /* redundcol contains input column number 0..n-1 where redundancy is */
    k = 0;
    while (k < d && C[k] <= d) {
      if (C[k] <= d) {                            /* decision variable still in cobasis */
        redundcol[nredundcol++] = C[k] - Q->hull; /* adjust for hull indices */
      }
      k++;
    }

    /* now we know how many decision variables remain in problem */
    Q->nredundcol = nredundcol;
    Q->lastdv = d - nredundcol;
    /* 2015.9.15 bug fix : we needed first *successful* time */
    FirstTime = FALSE;
  } /* else firsttime ... we have built a dictionary from the given linearities */

  /* we continue from here after loading dictionary */

  if (Q->debug) {
    fprintf(lrs_ofp, "\nend of first phase of getabasis2: ");
    fprintf(lrs_ofp, "lastdv=%ld nredundcol=%ld", Q->lastdv, Q->nredundcol);
    fprintf(lrs_ofp, "\nredundant cobases:");
    for (i = 0; i < nredundcol; i++) {
      fprintf(lrs_ofp, " %ld", redundcol[i]);
    }
    printA(P, Q);
  }

  /* here we save dictionary for use next time, *before* we resize */

  copy_dict(Q, P2orig, P);

  /* Remove linearities from cobasis for rest of computation */
  /* This is done in order so indexing is not screwed up */

  for (i = 0; i < nlinearity; i++) { /* find cobasic index */
    k = 0;
    while (k < d && C[k] != linearity[i] + d) {
      k++;
    }
    if (k >= d) {
      if (Q->debug || Q->verbose) {
        fprintf(lrs_ofp, "\nCould not remove cobasic index");
      }
      /* not neccesarily an error as eg., could be repeated row/col in payoff */
    }
    else {
      removecobasicindex(P, Q, k);
      d = P->d;
    }
  }
  if (Q->debug && nlinearity > 0) {
    printA(P, Q);
  }
  /* set index value for first slack variable */

  /* Check feasability */
  if (Q->givenstart) {
    i = Q->lastdv + 1;
    while (i <= m && !negative(A[Row[i]][0])) {
      i++;
    }
    if (i <= m) {
      fprintf(lrs_ofp, "\n*Infeasible startingcobasis - will be modified");
    }
  }
  return TRUE;
} /*  end of getabasis2 */

long lrs_nashoutput(lrs_dat *Q, lrs_mp_vector output, long player)
{
  long i;
  long origin = TRUE;

  /* do not print the origin for either player */
  for (i = 1; i < Q->n; i++) {
    if (!zero(output[i])) {
      origin = FALSE;
    }
  }

  if (origin) {
    return FALSE;
  }

  fprintf(lrs_ofp, "%ld ", player);
  for (i = 1; i < Q->n; i++) {
    prat("", output[i], output[0]);
  }
  fprintf(lrs_ofp, "\n");
  fflush(lrs_ofp);
  return TRUE;
} /* end lrs_nashoutput */

//========================================================================
// Old style solver. Included for backward compatibility
//========================================================================
int lrs_solve_nash_legacy(int argc, char *argv[])
// Handles legacy input files
{
  lrs_dic *P1 /*,*P2*/; /* structure for holding current dictionary and indices */
  lrs_dat *Q1, *Q2;     /* structure for holding static problem data            */

  lrs_mp_vector output1; /* holds one line of output; ray,vertex,facet,linearity */
  lrs_mp_vector output2; /* holds one line of output; ray,vertex,facet,linearity */
  lrs_mp_matrix Lin;     /* holds input linearities if any are found             */
  lrs_mp_matrix A2orig;
  lrs_dic *P2orig; /* we will save player 2's dictionary in getabasis      */

  long *linindex; /* for faster restart of player 2                       */

  long col; /* output column index for dictionary                   */
  long startcol = 0;
  long prune = FALSE;  /* if TRUE, getnextbasis will prune tree and backtrack  */
  long numequilib = 0; /* number of nash equilibria found                      */
  long oldnum = 0;

  /* global variables lrs_ifp and lrs_ofp are file pointers for input and output   */
  /* they default to stdin and stdout, but may be overidden by command line parms. */

  if (argc <= 2) {
    printf("Usage: %s input1 input2 [outputfile]     \n", argv[0]);
    return 1;
  }

  /***************************************************
   Step 0:
    Do some global initialization that should only be done once,
    no matter how many lrs_dat records are allocated. db

  ***************************************************/

  if (!lrs_init("\n*nash:")) {
    return 1;
  }
  printf("\n");
  printf(AUTHOR);

  /*********************************************************************************/
  /* Step 1: Allocate lrs_dat, lrs_dic and set up the problem                      */
  /*********************************************************************************/

  Q1 = lrs_alloc_dat("LRS globals"); /* allocate and init structure for static problem data */

  if (Q1 == NULL) {
    return 1;
  }
  Q1->nash = TRUE;

  if (!lrs_read_dat(Q1, argc, argv)) { /* read first part of problem data to get dimensions   */
    return 1;                          /* and problem type: H- or V- input representation     */
  }

  P1 = lrs_alloc_dic(Q1); /* allocate and initialize lrs_dic                     */
  if (P1 == NULL) {
    return 1;
  }

  if (!lrs_read_dic(P1, Q1)) { /* read remainder of input to setup P1 and Q1           */
    return 1;
  }

  output1 =
      lrs_alloc_mp_vector(Q1->n + Q1->m); /* output holds one line of output from dictionary     */

  fclose(lrs_ifp);

  /* allocate and init structure for player 2's problem data                                   */

  printf("\n*Second input taken from file %s\n", argv[2]);
  Q2 = lrs_alloc_dat("LRS globals");
  if (Q2 == NULL) {
    return 1;
  }

  Q2->nash = TRUE;

  if (!lrs_read_dat(Q2, 2, argv)) { /* read first part of problem data to get dimensions   */
    return 1;                       /* and problem type: H- or V- input representation     */
  }

  if (Q2->nlinearity > 0) {
    free(Q2->linearity); /* we will start again */
  }
  Q2->linearity = CALLOC((Q2->m + 2), sizeof(long));

  P2orig = lrs_alloc_dic(Q2); /* allocate and initialize lrs_dic                     */
  if (P2orig == NULL) {
    return 1;
  }
  if (!lrs_read_dic(P2orig, Q2)) { /* read remainder of input to setup P2 and Q2          */
    return 1;
  }
  A2orig = P2orig->A;

  output2 =
      lrs_alloc_mp_vector(Q1->n + Q1->m); /* output holds one line of output from dictionary     */

  linindex = calloc((P2orig->m + P2orig->d + 2), sizeof(long)); /* for next time*/

  fprintf(lrs_ofp, "\n***** %ld %ld rational\n", Q1->n, Q2->n);

  /*********************************************************************************/
  /* Step 2: Find a starting cobasis from default of specified order               */
  /*         P1 is created to hold  active dictionary data and may be cached       */
  /*         Lin is created if necessary to hold linearity space                   */
  /*         Print linearity space if any, and retrieve output from first dict.    */
  /*********************************************************************************/

  if (!lrs_getfirstbasis(&P1, Q1, &Lin, TRUE)) {
    return 1;
  }

  if (Q1->dualdeg) {
    printf("\n*Warning! Dual degenerate, ouput may be incomplete");
    printf("\n*Recommendation: Add dualperturb option before maximize in first input file\n");
  }

  if (Q1->unbounded) {
    printf("\n*Warning! Unbounded starting dictionary for p1, output may be incomplete");
    printf("\n*Recommendation: Change/remove maximize option, or include bounds \n");
  }

  /* Pivot to a starting dictionary                      */
  /* There may have been column redundancy               */
  /* If so the linearity space is obtained and redundant */
  /* columns are removed. User can access linearity space */
  /* from lrs_mp_matrix Lin dimensions nredundcol x d+1  */

  if (Q1->homogeneous && Q1->hull) {
    startcol++; /* col zero not treated as redundant   */
  }

  for (col = startcol; col < Q1->nredundcol; col++) { /* print linearity space               */
    lrs_printoutput(Q1, Lin[col]);                    /* Array Lin[][] holds the coeffs.     */
  }

  /*********************************************************************************/
  /* Step 3: Terminate if lponly option set, otherwise initiate a reverse          */
  /*         search from the starting dictionary. Get output for each new dict.    */
  /*********************************************************************************/

  /* We initiate reverse search from this dictionary       */
  /* getting new dictionaries until the search is complete */
  /* User can access each output line from output which is */
  /* vertex/ray/facet from the lrs_mp_vector output         */
  /* prune is TRUE if tree should be pruned at current node */

  do {
    prune = lrs_checkbound(P1, Q1);
    if (!prune && lrs_getsolution(P1, Q1, output1, col)) {
      oldnum = numequilib;
      nash2_main(P1, Q1, P2orig, Q2, &numequilib, output2, linindex);
      if (numequilib > oldnum || Q1->verbose) {
        if (Q1->verbose) {
          prat(" \np2's obj value: ", P1->objnum, P1->objden);
        }
        lrs_nashoutput(Q1, output1, 1L);
        fprintf(lrs_ofp, "\n");
      }
    }
  } while (lrs_getnextbasis(&P1, Q1, prune));

  fprintf(lrs_ofp, "\n*Number of equilibria found: %ld", numequilib);
  fprintf(lrs_ofp, "\n*Player 1: vertices=%ld bases=%ld pivots=%ld", Q1->count[1], Q1->count[2],
          Q1->count[3]);
  fprintf(lrs_ofp, "\n*Player 2: vertices=%ld bases=%ld pivots=%ld", Q2->count[1], Q2->count[2],
          Q2->count[3]);

  lrs_clear_mp_vector(output1, Q1->m + Q1->n);
  lrs_clear_mp_vector(output2, Q1->m + Q1->n);

  lrs_free_dic(P1, Q1); /* deallocate lrs_dic */
  lrs_free_dat(Q1);     /* deallocate lrs_dat */

  /* 2015.10.10  new code to clear P2orig */
  Q2->Qhead = P2orig; /* reset this or you crash free_dic */
  P2orig->A = A2orig; /* reset this or you crash free_dic */

  lrs_free_dic(P2orig, Q2); /* deallocate lrs_dic */
  lrs_free_dat(Q2);         /* deallocate lrs_dat */

  free(linindex);

  lrs_close("nash:");

  return 0;
}

/*********************************************/
/* end of nash driver                        */
/*********************************************/

//==========================================================================
//   Building the problem representations (adapted from Gambit-enummixed)
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

//----------------------------------------------------------------------------------------//
void FillNonnegativityRows(lrs_dic *P, lrs_dat *Q, int firstRow, int lastRow, int n)
{
  const int MAXCOL = 1000; /* maximum number of columns */
  long num[1000], den[1000];
  long row, col;

  for (row = firstRow; row <= lastRow; row++) {
    num[0] = 0;
    den[0] = 1;

    for (col = 1; col < n; col++) {
      num[col] = (row - firstRow + 1 == col) ? 1 : 0;
      den[col] = 1;
    }
    lrs_set_row(P, Q, row, num, den, GE);
  }
}

//----------------------------------------------------------------------------------------//
void FillConstraintRows(lrs_dic *P, lrs_dat *Q, const game *g, int p1, int p2, int firstRow)
{
  const int MAXCOL = 1000; /* maximum number of columns */
  long num[1000], den[1000];
  ratnum x;
  int row, s, t;

  for (row = firstRow; row < firstRow + g->nstrats[p1]; row++) {
    num[0] = 0;
    den[0] = 1;
    s = row - firstRow;
    for (t = 0; t < g->nstrats[p2]; t++) {
      x = p1 == ROW ? g->payoff[s][t][p1] : g->payoff[t][s][p1];
      num[t + 1] = -x.num;
      den[t + 1] = x.den;
    }
    num[g->nstrats[p2] + 1] = 1;
    den[g->nstrats[p2] + 1] = 1;
    lrs_set_row(P, Q, row, num, den, GE);
  }
}

//----------------------------------------------------------------------------------------//
void FillLinearityRow(lrs_dic *P, lrs_dat *Q, int m, int n)
{
  const int MAXCOL = 1000; /* maximum number of columns */
  long num[1000], den[1000];
  int i;

  num[0] = -1;
  den[0] = 1;

  for (i = 1; i < n - 1; i++) {
    num[i] = 1;
    den[i] = 1;
  }

  num[n - 1] = 0;
  den[n - 1] = 1;

  lrs_set_row(P, Q, m, num, den, EQ);
}

//
// TL added this to get first row of ones. Don't know if it's needed
//----------------------------------------------------------------------------------------//
void FillFirstRow(lrs_dic *P, lrs_dat *Q, int n)
{
  const int MAXCOL = 1000; /* maximum number of columns */
  long num[1000], den[1000];
  int i;

  for (i = 0; i < n; i++) {
    num[i] = 1;
    den[i] = 1;
  }
  lrs_set_row(P, Q, 0, num, den, GE);
}

//
// Build the H-representation for player p1
//----------------------------------------------------------------------------------------//
void BuildRep(lrs_dic *P, lrs_dat *Q, const game *g, int p1, int p2)
{
  long m = Q->m; /* number of inequalities      */
  long n = Q->n;

  if (p1 == 0) {
    FillConstraintRows(P, Q, g, p1, p2, 1);
    FillNonnegativityRows(P, Q, g->nstrats[p1] + 1, g->nstrats[ROW] + g->nstrats[COL], n);
  }
  else {
    FillNonnegativityRows(P, Q, 1, g->nstrats[p2], n);
    FillConstraintRows(P, Q, g, p1, p2, g->nstrats[p2] + 1); // 1 here
  }
  FillLinearityRow(P, Q, m, n);

  // TL added this to get first row of ones. (Is this necessary?)
  FillFirstRow(P, Q, n);
}

//----------------------------------------------------------------------------------------//
void printGame(game *g)
{
  int s, t;
  char out[2][MAXINPUT];
  fprintf(lrs_ofp,
          "\n--------------------------------------------------------------------------------\n");
  fprintf(lrs_ofp, "%s payoff matrix:\n", ((gInfo *)g->aux)->name);
  for (s = 0; s < g->nstrats[ROW]; s++) {
    for (t = 0; t < g->nstrats[COL]; t++) {
      if (g->payoff[s][t][ROW].den == 1) {
        sprintf(out[ROW], "%ld,", g->payoff[s][t][ROW].num);
      }
      else {
        sprintf(out[ROW], "%ld/%ld,", g->payoff[s][t][ROW].num, g->payoff[s][t][ROW].den);
      }
      if (g->payoff[s][t][COL].den == 1) {
        sprintf(out[COL], "%ld", g->payoff[s][t][COL].num);
      }
      else {
        sprintf(out[COL], "%ld/%ld", g->payoff[s][t][COL].num, g->payoff[s][t][COL].den);
      }
      fprintf(lrs_ofp, "%*s%-*s  ", ((gInfo *)g->aux)->fwidth[t][ROW] + 1, out[ROW],
              ((gInfo *)g->aux)->fwidth[t][COL], out[COL]);
    }
    fprintf(lrs_ofp, "\n");
  }
  fprintf(lrs_ofp, "\nNash equilibria:\n");
  fflush(lrs_ofp);
}

// Functions to set field widths for pretty printing of payoff matrices
void setFwidth(game *g, int len)
{
  int pos, t;
  for (t = 0; t < g->nstrats[COL]; t++) {
    for (pos = 0; pos < 2; pos++) {
      ((gInfo *)g->aux)->fwidth[t][pos] = len;
    }
  }
}

void initFwidth(game *g)
{
  int pos, t;
  for (t = 0; t < g->nstrats[COL]; t++) {
    for (pos = 0; pos < 2; pos++) {
      ((gInfo *)g->aux)->fwidth[t][pos] = 0;
    }
  }
}

void updateFwidth(game *g, int col, int pos, char *str)
{
  int len = strlen(str);
  if (len > ((gInfo *)g->aux)->fwidth[col][pos]) {
    ((gInfo *)g->aux)->fwidth[col][pos] = len;
  }
}

/******************** end of lrsnashlib.c ***************************/
