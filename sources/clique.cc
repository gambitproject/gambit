//
// FILE: clique.cc -- Maximal cliques and connected components
//                    via von Stengel's algorithm
//
// $Id$
//

/*  connected components and their maximal cliques in bipartite graphs 
    8 March 1998
    Bernhard von Stengel

    For a bipartite graph given as a set of pairs  (i,j), it outputs 
    - the connected components of that graph, and for each component
    - the maximal product sets   U x V
      so that all  (i,j)  in U x V  are edges of the graph
      (so these are the maximal complete bipartite subgraphs or CLIQUES).

    INPUT:
    The edges (i, j) are given by pairs of nonnegative integers separated
    by blanks on standard input.

    OUTPUT:
    On standard output,
    a headline for each connected component, then
    the cliques  U x V  listing  U  and  V  separately as lists
    of integers, separated by  "x"  and each set enclosed in braces, 
    one clique per line.

    METHOD:
    Connected components by a primitive version of union-find,
    cliques with a variant of the algorithm by
    [BK] C. Bron and J. Kerbosch, Finding all cliques of an undirected
         graph, Comm. ACM 16:9 (1973), 575-577.

    APPROXIMATE STORAGE REQUIREMENTS:
    for integer arrays, 4 bytes per integer, using constants
    MAXINP1, MAXINP2  max. node indices in input
    MAXEDGES          max. no. edges in input
    MAXM, MAXN        max. dimension of incidence matrix
   	              per connected component
    2 x MAXM x MAXN  integers for incidence matrix and stack
	   [2 MB  if MAXM = MAXN = 700 ]
    3 x MAXEDGES  integers for edge list
	   [0.6 MB  if  MAXEDGES = 50000 ]
    3 x MAXINP1  integers for input nodes and list of components
	   [60 kB   if  MAXINP1 = MAXINP2 = 5000 ]
    If these constants are exceeded certain edges will be rejected
    from the input with an error message.  Program shouldn't crash.
    No error value is returned by  main().

    DETAILS OF METHODS:

    a) Connected components

    Designed for minimum storage requirement, running time
    possibly quadratic in number of edges.
    For each node that is read, a component  co1[i]  resp.  co2[j]
    ( i  left node,  j  right node) is kept, initially  0  if node
    is not yet input.  (Isolated nodes are treated as absent.)
    For an edge  (i, j),  i and j  must be put in the same component.
    Each component  co  points to the first edge in  edgelist,
    where the edges are linked.  Merging two components is done
    by traversing the edgelist with the higher number, updating
    the component number of the nodes therein, and prepending
    it to the list of the other component.
    Components and edges are numbered starting with 1,  so "no
    component" and the end of an edgelist is represented by 0.

    Sets are represented by C arrays, if starting with 0
    (as usually in C),  then the elements of a  k-set are the
    array elements [0..k) i.e. [0..k-1],  if starting with 1
    they are [1..k].

    A possible improvement is to keep extra lists of the
    equivalence classes for the nodes for each component so only
    these have to be updated, which makes it faster.  
    
    b) Clique enumeration

    The procedure  extend  recursively extends a current set of pairs
    clique1, clique2  that eventually will form a maximal clique.
    In [BK], this is only a single set  COMPSUB (here called CLIQUE),
    here two sets are used since the graph is bipartite.  
    Cliques of a bipartite graph are equivalent to the cliques of
    the ordinary graph obtained by connecting all left nodes by
    themselves and all right nodes by themselves, except for
    the cliques consisting exclusively of left or right points.

    The recursive calls use a self-made stack  stk  containing
    local small arrays of variable size.  Intervals of this stack
    are indicated by their endpoints which ARE local variables
    to the recursive call.  The top of the stack  tos  is passed
    as a parameter.

    The extension is done by adding points from a set CAND of 
    candidates to CLIQUE.  Throughout, the points in CAND are
    connected to all points in  CLIQUE,  which holds at initialization
    when CAND contains all points and CLIQUE is empty.  

    Traversing the backtracking tree:  Extending its depth is
    done by picking  c  (cand  in the code below) from CAND,
    adding  c  to CLIQUE, removing all points not connected to  c 
    from CAND, and handing the new sets CLIQUE and CAND
    to the recursive call.
    For extending the backtracking tree in its breadth, this is
    done in a loop (called backtracking cycle in [BK]) where repeatedly
    different candidates  c  are added to CLIQUE (after the respective
    return from the recursive call).  In order to avoid the output
    of cliques that are not maximal, an additional set NOT is passed
    down as a parameter to the recursive call of  extend.
    This set NOT contains candidates  c  that 
    - are all connected to the elements in CLIQUE but
    - have already been tried out, that is, all extensions of CLIQUE
      containing any point in NOT have already been generated [BK, p.577].  

    Hence, the recursive call proceeds downwards by
    - removing  c  from CAND and adding it to CLIQUE
    - removing all points disconnected from  c  from the new
      sets  NOT  and  CAND  used in the recursive call.
    After extension,  c  is then moved to  NOT  and the next
    candidate is tried.

    To reduce the breadth of the backtracking tree, the first
    candidate (or the subsequent ones) are chosen such that
    as early as possible there is a node in NOT connected to all
    remaining candidates.  Then NOT will never become empty and
    hence no clique will be output, so the backtracking tree can
    be pruned here.  This is done by choosing first a  fixpoint  fixp
    in the set  NOT  or  CAND, such that after extension, when
    fixp  is definitely in  NOT,  only points disconnected to  fixp
    are added.  Their number is the smallest possible.

    This is version2 of the algorithm in [BK]:
    a - pick  fixp  in NOT or CAND  with the smallest number of 
        disconnections to the other nodes in CAND,
    b - if  fixp  is a candidate, try it out as a candidate, i.e.
        extend  CLIQUE  with  fixp  (procedures  candtry  below),
	and then move   fixp  to  NOT after extension.
    c - then try out only points disconnected to  fixp,  as
	determined in a.  (In contrast to [BK], we compute
	a local list of these disconnected points while looking
	for the smallest number of disconnections.)

    Amendments for the bipartite graph are here:  a  is done
    by inspecting both sides of the graph.
    For the single extension in  b  (if  fixp  is a candidate)
    and the extensions in  c , only the sets NOT and CAND
    on the other side of the candidate used for extension
    have to be updated.  Hence,  NOT and CAND are kept as
    separate sets  NOT1,  NOT2  and CAND1, CAND2.

*/


#include <stdio.h>
#define BOOLEAN int
#define FALSE 0
#define TRUE  1
#define MAX(A,B)  ((A) > (B) ? (A) : (B))
#define MIN(A,B)  ((A) < (B) ? (A) : (B))

#define MAXM 700     /* max. no of left nodes for incidence matrix */
#define MAXN MAXM  /* max. no of right nodes for incidence matrix */

#define MAXINP1 5000        /* max. no of left nodes in input */
#define MAXINP2 MAXINP1  /* max. no of right nodes in input */
#define MAXEDGES   50000       /* max. no of edges in input */
#define MAXCO  MIN(MAXINP1, MAXINP2) + 1
        /* max. no of connected components;  on the smaller side,
           each node could be in different component */
#define STKSIZE  (MAXM + 1) * (MAXN + 1)  
	/* largest stack usage for full graph */
			           
struct edge {
       int node1;
       int node2;
       int nextedge ;
       };

void candtry1 (int stk[], /* stack */
     BOOLEAN connected[MAXM][MAXN],
     int cand,  /* the candidate from NODES1  to be added to CLIQUE */
     int poscand,  /* its stack position */
     int clique1[], int cliqsize1,  /* CLIQUE so far in NODES1 */
     int clique2[], int cliqsize2,  /* CLIQUE so far in NODES2 */
     int sn1, int *sc1, int ec1,   /* start NOT1, start CAND1, end CAND1 */
     int sn2, int sc2, int ec2,   /* start NOT2, start CAND2, end CAND2 */
     int tos,  /* top of stack */
     int orignode1[MAXM],
     int orignode2[MAXN]
     )
     ;
void candtry2 (int stk[], /* stack */
     BOOLEAN connected[MAXM][MAXN],
     int cand,  /* the candidate from NODES2  to be added to CLIQUE */
     int poscand,  /* its stack position */
     int clique1[], int cliqsize1,  /* CLIQUE so far in NODES1 */
     int clique2[], int cliqsize2,  /* CLIQUE so far in NODES2 */
     int sn1, int sc1, int ec1,   /* start NOT1, start CAND1, end CAND1 */
     int sn2, int *sc2, int ec2,   /* start NOT2, start CAND2, end CAND2 */
     int tos,  /* top of stack */
     int orignode1[MAXM],
     int orignode2[MAXN]
     )
     ;
void extend (int stk[], /* stack */
     BOOLEAN connected[MAXM][MAXN],
     int clique1[], int cliqsize1,  /* CLIQUE so far in NODES1 */
     int clique2[], int cliqsize2,  /* CLIQUE so far in NODES2 */
     int sn1, int sc1, int ec1,   /* start NOT1, start CAND1, end CAND1 */
     int sn2, int sc2, int ec2,   /* start NOT2, start CAND2, end CAND2 */
     int tos,   /* top of stack,   tos >= ec1, ec2  */
     int orignode1[MAXM],   /* original node numbers as input */
     int orignode2[MAXN]
     )
     ;
void findfixpoint(int stk[], /* stack */
         BOOLEAN connected[MAXM][MAXN],
	 int *savelist,      /* position of savelist on the stack */
	 int *tmplist,       /* position of tmplist on the stack */
	 int *minnod,        /* currently lowest no. of disconnections */
	 int sninspect, int ecinspect,
	 int scother, int ecother,
	 BOOLEAN binspect1,  /* inspected nodes are in class1, o/w class2 */
	 BOOLEAN *bfound,  /* a new lower no. of disconnections was found */
	 int *fixp,     /* the new fixpoint, if *bfound = TRUE  */
	 int *posfix    /* position of fixpoint on the stack, if *bfound */
	 )
	 ;

void genincidence(int e,
     struct edge edgelist[MAXEDGES],
     int orignode1[MAXM],
     int orignode2[MAXN],
     BOOLEAN connected[MAXM][MAXN],
     int *m,
     int *n
     )
     ;
int getconnco(int firstedge[MAXCO],
    struct edge edgelist[MAXEDGES]
    )
    ;
void outCLIQUE(int clique1[], int cliqsize1, 
               int clique2[], int cliqsize2,
               int orignode1[MAXM],
               int orignode2[MAXN]
	       ) 
	       ;
void workonco(int numco,
     int firstedge[MAXCO],
     struct edge edgelist[MAXEDGES]
     )
     ;

/* --- the following are unused TEST ROUTINES --- */
void getgraph(BOOLEAN connected[MAXM][MAXN], int *m, int *n)
     ;
void outgraph(
     int orignode1[MAXM],
     int orignode2[MAXN],
     BOOLEAN connected[MAXM][MAXN],
     int m,
     int n
     )
     ;

/* =============== main =============== */
int main()
{
int  firstedge[MAXCO];
struct edge edgelist[MAXEDGES];

int numco;

numco = getconnco(firstedge, edgelist);

workonco(numco, firstedge, edgelist) ;

return 0 ;
}
/* =============== end main =============== */

/* -------------------------------------------------- */
void candtry1 (int stk[], /* stack */
     BOOLEAN connected[MAXM][MAXN],
     int cand,  /* the candidate from NODES1  to be added to CLIQUE */
     int poscand,  /* its stack position */
     int clique1[], int cliqsize1,  /* CLIQUE so far in NODES1 */
     int clique2[], int cliqsize2,  /* CLIQUE so far in NODES2 */
     int sn1, int *sc1, int ec1,   /* start NOT1, start CAND1, end CAND1 */
     int sn2, int sc2, int ec2,   /* start NOT2, start CAND2, end CAND2 */
     int tos,  /* top of stack */
     int orignode1[MAXM],
     int orignode2[MAXN]
     )
/* recurses down by moving  cand  from  CAND1  to  clique1  and
   then to NOT1  after extension.
   clique1  is extended by  cand  where all points in  NOT2 and CAND2 
	     relate to  cand.
   pre:  cand  is in CAND1
   post: cand  is moved from  CAND1  to  NOT1
	 CAND1 may be shuffled,  o/w stack unchanged
*/
{
int j, snnew, scnew, ecnew;

clique1[cliqsize1++] = cand ;
/* remove  cand  from CAND1 by replacing it with the last element of CAND1 */
stk[poscand] = stk[--ec1] ;
/* stk[ec1] is free now but will after extension be needed again */
/* fill new sets NOT2, CAND2 */
snnew = tos ;
for (j=sn2; j<sc2; j++) 
	   if (connected[cand][stk[j]])
	      stk[tos++] = stk[j] ;
scnew = tos ;
for (j=sc2; j<ec2; j++) 
	   if (connected[cand][stk[j]])
	      stk[tos++] = stk[j] ;
ecnew = tos ;
  
extend(stk, connected, clique1, cliqsize1, clique2, cliqsize2,
       sn1, *sc1, ec1, snnew, scnew, ecnew, tos, orignode1, orignode2);

/* remove  cand  from  clique1,
   put  cand  into  NOT1  by increasing  *sc1  and moving
   the node at position  *sc1  to the end of CAND1 */
cliqsize1-- ;
stk[ec1++] = stk[*sc1] ;
stk[*sc1] = cand ;
(*sc1)++ ;
}

/* -------------------------------------------------- */
void candtry2 (int stk[], /* stack */
     BOOLEAN connected[MAXM][MAXN],
     int cand,  /* the candidate from NODES2  to be added to CLIQUE */
     int poscand,  /* its stack position */
     int clique1[], int cliqsize1,  /* CLIQUE so far in NODES1 */
     int clique2[], int cliqsize2,  /* CLIQUE so far in NODES2 */
     int sn1, int sc1, int ec1,   /* start NOT1, start CAND1, end CAND1 */
     int sn2, int *sc2, int ec2,   /* start NOT2, start CAND2, end CAND2 */
     int tos,  /* top of stack */
     int orignode1[MAXM],
     int orignode2[MAXN]
     )
/* recurses down by moving  cand  from  CAND2  to  clique2  and
   then to NOT2  after extension;
   clique2  is extended by  cand  where all points in  NOT1 and CAND1 
	     relate to  cand.
   pre:  cand  is in CAND2
   post: cand  is moved from  CAND2  to  NOT2
	 CAND2 may be shuffled,  o/w stack unchanged
*/
{
int i, snnew, scnew, ecnew;

clique2[cliqsize2++] = cand ;
/* remove  cand  from CAND2 by replacing it with the last element of CAND2 */
stk[poscand] = stk[--ec2] ;
/* stk[ec2] is free now but will after extension be needed again */
/* fill new sets NOT1, CAND1 */
snnew = tos ;
for (i=sn1; i<sc1; i++) 
	   if (connected[stk[i]][cand])
	      stk[tos++] = stk[i] ;
scnew = tos ;
for (i=sc1; i<ec1; i++) 
	   if (connected[stk[i]][cand])
	      stk[tos++] = stk[i] ;
ecnew = tos ;
  
extend(stk, connected, clique1, cliqsize1, clique2, cliqsize2,
	      snnew, scnew, ecnew, sn2, *sc2, ec2, tos, orignode1, orignode2);

/* remove  cand  from  clique2,
   put  cand  into  NOT2  by increasing  *sc2  and moving
   the node at position  sc2  to the end of CAND1 */
cliqsize2-- ;
stk[ec2++] = stk[*sc2] ;
stk[*sc2] = cand ;
(*sc2)++ ;
}

/* -------------------------------------------------- */
void extend (int stk[], /* stack */
     BOOLEAN connected[MAXM][MAXN],
     int clique1[], int cliqsize1,  /* CLIQUE so far in NODES1 */
     int clique2[], int cliqsize2,  /* CLIQUE so far in NODES2 */
     int sn1, int sc1, int ec1,   /* start NOT1, start CAND1, end CAND1 */
     int sn2, int sc2, int ec2,   /* start NOT2, start CAND2, end CAND2 */
     int tos,   /* top of stack,   tos >= ec1, ec2  */
     int orignode1[MAXM],   /* original node numbers as input */
     int orignode2[MAXN]
     )
/* extends the current set CLIQUE or outputs it if
   NOT and CAND are empty.

   pre:  CLIQUE = clique1[0, cliqsize1], clique2[0, cliqsize2]
	 NOT1 = stk[sn1, sc1],  CAND1= stk[sc1, ec1]   
	 NOT2 = stk[sn2, sc2],  CAND2= stk[sc2, ec2]   
	 sn1 <= sc1 <= ec1, sn2 <= sc2 <= ec2 
	 all cliques extending  CLIQUE
	 containing a node in  NOT1  or  NOT2  have already been generated
   post: output of all maximal cliques extending  CLIQUE  with
         candidates from  CAND1  or  CAND2  but not from NOT1, NOT2.
*/
{
/* if no further extension is possible then  
   output the current CLIQUE if applicable, and return */

if (sc1 == ec1 && sc2 == ec2) {
   /*  CAND is empty  */
   if (sn1 == sc1 && sn2 == sc2)
      /*  NOT is empty, otherwise do nothing  */
      outCLIQUE(clique1, cliqsize1, clique2, cliqsize2,
                orignode1, orignode2) ;
   }

else {  /*  CAND not empty */
int cmax;  /* maximal number of candidates on either side  */
int firstlist, savelist, tmplist, posfix ; /* stack positions */
int fixp;  /* the fixpoint */
int minnod ;
BOOLEAN bfound, bfixin1, bcandfix;

cmax = MAX(ec1-sc1, ec2-sc2);  /* the larger of |CAND1|, |CAND2|  */

/* reserve two arrays of size cmax on the stack */
firstlist = tmplist = tos;  tos += cmax;
savelist = tos;  tos += cmax;

/* find fixpoint  fixp (a node of the graph) in  NOT  or  CAND
   which has the smallest possible number of disconnections  minnod 
   to CAND  */
minnod = cmax + 1 ;

/* look for  fixp  in NODES1  */
findfixpoint(stk, connected, &savelist, &tmplist, &minnod,
	     sn1, ec1, sc2, ec2, 1, &bfixin1, &fixp, &posfix) ;
bcandfix = (posfix >= sc1);

/* look for  fixp  in nodes2  */
findfixpoint(stk, connected, &savelist, &tmplist, &minnod,
	     sn2, ec2, sc1, ec1, 0, &bfound, &fixp, &posfix) ;

if (bfound) {
   bfixin1 = FALSE ;
   bcandfix = (posfix >= sc2);
   }

/* now:  fixp     = the node that is the fixpoint,
	 posfix   = its position on the stack,
	 bfixin1  = fixp  is in NODES1
	 bcandfix = fixp  is a candidate
	 stk[savelist, +minnod] = stack indices (!) of nodes
	 disconnected to fixp which are all either in CAND1 or in CAND2;
*/
/*    top of stack can be reset to  savelist+minnod  where
      if savelist  is the second of the two lists, recopy it
      to avoid that stk[firstlist, +cmax] is wasted
*/
if (savelist != firstlist) {int i;
   for (i=0; i < minnod; i++) 
       stk[firstlist + i] = stk[savelist + i];
   savelist = firstlist ;
   }
tos = savelist + minnod;

if (bfixin1) {int j;  /* fixpoint in NODES1  */
   if (bcandfix)      /* fixpoint is a candidate */
       candtry1(stk, connected, fixp, posfix,
                clique1, cliqsize1, clique2, cliqsize2,
	        sn1, &sc1, ec1, sn2, sc2, ec2, tos, orignode1, orignode2);
   /* fixpoint is now in NOT1, try all the nodes disconnected to it */
   for (j=0; j<minnod;  j++) 
       candtry2(stk, connected, stk[stk[savelist+j]], stk[savelist+j],
                clique1, cliqsize1, clique2, cliqsize2,
	        sn1, sc1, ec1, sn2, &sc2, ec2, tos, orignode1, orignode2);
   }
else {int j;          /* fixpoint in NODES2  */
   if (bcandfix)      /* fixpoint is a candidate */
       candtry2(stk, connected, fixp, posfix,
                clique1, cliqsize1, clique2, cliqsize2,
	        sn1, sc1, ec1, sn2, &sc2, ec2, tos, orignode1, orignode2);
   /* fixpoint is now in NOT2, try all the nodes disconnected to it */
   for (j=0; j<minnod;  j++) 
       candtry1(stk, connected, stk[stk[savelist+j]], stk[savelist+j],
                clique1, cliqsize1, clique2, cliqsize2,
	        sn1, &sc1, ec1, sn2, sc2, ec2, tos, orignode1, orignode2);
   }
}  /* end candidates not empty */
} 

/* -------------------------------------------------- */
void findfixpoint(int stk[], /* stack */
         BOOLEAN connected[MAXM][MAXN],
	 int *savelist,      /* position of savelist on the stack */
	 int *tmplist,       /* position of tmplist on the stack */
			     /* might be swapped afterwards */
	 int *minnod,        /* currently lowest no. of disconnections */
	 int sninspect, int ecinspect, /* range of stack positions
	        containing the nodes inspected as possible fixpoints */
	 int scother, int ecother,
	        /* range of corresponding candidates on the other side  */
	 BOOLEAN binspect1,  /* inspected nodes are in class1, o/w class2 */
	 BOOLEAN *bfound,  /* a new lower no. of disconnections was found */
	 int *fixp,     /* the new fixpoint, if *bfound = TRUE  */
	 int *posfix    /* position of fixpoint on the stack, if *bfound */
	 )
/* pre:  enough space on stack for the two lists  savelist,  tmplist
   post: *minnod contains the new minimum no. of disconnections
	 stk[*savelist, +*minnod] contains the candidates disconnected to
	 the fixpoint
*/
{
int i,j, p ;
int count ;
int tmp ;
*bfound = FALSE ;

for (i=sninspect; i<ecinspect; i++) {
    p = stk[i] ;
    count = 0;
    /* count number of disconnections to  p,  
       building up stk[tmplist+count] containing the indices (!)
       of disconnected points */
    for (j=scother; (j<ecother) && (count < *minnod); j++) {
	if (!( binspect1 ? connected[p][stk[j]] : connected[stk[j]][p] )) {
	   stk[(*tmplist) + count] = j ;
	   count ++ ;
	   }
        }  /* end loop j, comparing to other side */
    /* check if new minimum found, in that case update fixpoint */
    if (count < *minnod) {
       *fixp = p ;
       *posfix = i;
       *minnod = count ;
       /* save tmplist by making it the new savelist */
       tmp = *savelist ;
       *savelist = *tmplist;
       *tmplist  = tmp ;
       *bfound = TRUE ;
       }
    }  /* end loop  i,  inspecting nodes for fixpoint  */
}

/* -------------------------------------------------- */
void genincidence(
     int e,
     struct edge edgelist[MAXEDGES],
     int orignode1[MAXM],
     int orignode2[MAXN],
     BOOLEAN connected[MAXM][MAXN],
     int *m,
     int *n
     )
/* generates the incidence matrix  connected from the edgelist
   starting with edgelist[e]
pre:  all nodes in edgelist < MAXINP1,2
post: orignode1[0..*m) contains the original node1 numbers
      orignode2[0..*n) contains the original node2 numbers
      connected[][] == TRUE if edge, o/w FALSE
      *m == number of rows 
      *n == number of columns
*/
{
int newnode1[MAXINP1] ;
int newnode2[MAXINP2] ;
int i,j, newi, newj ;

/* init newnode */
for (i=0; i<MAXINP1; i++)  newnode1[i] = -1;
for (j=0; j<MAXINP2; j++)  newnode2[j] = -1;

/* init connected for test ; note different dimension 
for (i=0; i<MAXM; i++)
    for (j=0; j<MAXN; j++)  connected[i][j] = 2;    
*/

*m = *n = 0;

while (e) { /* process the edge list with edge index e */
   i= edgelist[e].node1;
   j= edgelist[e].node2;
   newi = newnode1[i] ;
   newj = newnode2[j] ;
   if (newi == -1) {
      if (*m >= MAXM) { /* out of bounds for connected, reject */
	 printf("Left bound %d for incidence matrix ", MAXM ) ;
	 printf("reached, edge (%d, %d) rejected\n", i, j);
	 goto KEEPGOING ;
	 }
      else {
         int k;
         newi = (*m) ++ ;
         /* init connected on the fly */
	 for (k=0; k<MAXN; k++)  connected[newi][k] = FALSE;
	 newnode1[i] = newi ;
	 orignode1[newi] = i ;
	 }
      }
   if (newj == -1) {
      if (*n >= MAXN) { /* out of bounds for connected, reject */
	 printf("Right bound %d for incidence matrix ", MAXN ) ;
	 printf("reached, edge (%d, %d) rejected\n", i, j);
	 goto KEEPGOING ;
	 }
      else {
         newj = (*n) ++ ;
	 newnode2[j] = newj ;
	 orignode2[newj] = j ;
	 }
      }
   connected[newi][newj] = TRUE ;

   KEEPGOING:
   e = edgelist[e].nextedge ;
   }
}

/* -------------------------------------------------- */
int getconnco(int firstedge[MAXCO],
    struct edge edgelist[MAXEDGES]
    )

/* reads edges of bipartite graph from input, puts them in disjoint
   lists of edges representing its connected components 
   pre:  nodes are nonzero integers < MAXINP1,2
	 other edges are rejected, and so are edges starting
	 from the MAXEDGEth edge on and larger, each with a warning msg.
   post: return value == numco  (largest index of a connected component)
	 where  numco < MAXCO,  and for   1 <= co <= numco:
	 edgelist[co].firstedge == 0    if  co  is not a component
		       == edgeindex  e  otherwise where  e > 0  and
         edgelist[e].node1, .node2[e] are endpoints of edge,
	 edgelist[e].nextedge == next edgeindex of component,
	                         zero if  e  is index to the last edge
*/
{
int numco, newedge ;
int co1[MAXINP1], co2[MAXINP2];   /* components of node1,2  */
int i, j;  /* indices to left and right nodes */

/* initialize  component indices of left and right nodes */
for (i=0; i<MAXINP1; i++)
    co1[i] = 0;
for (j=0; j<MAXINP2; j++)
    co2[j] = 0;

numco = 0;
newedge = 0;

while (scanf("%d %d", &i, &j) != EOF) {
   if (i < 0 || i>= MAXINP1 || j<0 || j>=MAXINP2)
      printf("Edge (%d, %d) not in admitted range (0..%d, 0..%d), rejected\n",
              i,j, MAXINP1-1, MAXINP2-1) ;
   else if (newedge >= MAXEDGES-1)
      printf("max no. %d of edges exceeded, edge (%d, %d) rejected\n",
              MAXEDGES-1, i,j) ;
   else { 
      /* add edge (i,j) to current componentlist */
      int  ico, jco ;  /* current components of i,j  */
      newedge ++;
      edgelist[newedge].node1 = i;
      edgelist[newedge].node2 = j;
      ico = co1[i] ;
      jco = co2[j] ;

      if (ico == 0) {
	 /*  i  has not yet been in a component before  */
         if (jco == 0) {
	    /*  j  has not yet been in a component before  */
	    /*  add a new component  */
	    numco ++;
	    co1[i] = co2[j] = numco ;
	    firstedge[numco] = newedge ;
            edgelist[newedge].nextedge  = 0;
	    }
	 else { /* j  is already in a component: add  i  to j's
	           component, adding list elements in front */
	    co1[i] = jco ;
            edgelist[newedge].nextedge  = firstedge[jco];
	    firstedge[jco] = newedge;
	    }
	 }
      else { /* i  is already in a component */
         if (jco == 0) {
	    /*  j  has not yet been in a component before  */
	    /*  add  j  to  i's component  */
	    co2[j] = ico ;
            edgelist[newedge].nextedge  = firstedge[ico];
	    firstedge[ico] = newedge;
	    }
	 else { /* i  and  j  are already in components  */
	    if (ico == jco) {
	       /* i, j  in same component: just add the current edge */
               edgelist[newedge].nextedge  = firstedge[ico];
	       firstedge[ico] = newedge;
	       }
            else { /*  i  and  j  in different components:  
	               merge these by traversing the edgelists
		       and updating components of all incident nodes
		       (this is wasteful since only nodes need be
		       updated, not edges)  */
	       int e, newco, oldco ;
	       if (ico < jco) { newco = ico; oldco = jco; }
	       else           { newco = jco; oldco = ico; }
	       /* insert the current edge */
	       edgelist[newedge].nextedge= firstedge[oldco] ;
	       e = newedge ; 
	       while (1) {
		  co1[edgelist[e].node1] = co2[edgelist[e].node2] = newco ;
		  if (edgelist[e].nextedge == 0) break;
		  e = edgelist[e].nextedge;
		  }
	       /*  e  is now the last edge in the updated list  */
	       edgelist[e].nextedge = firstedge[newco] ;
	       firstedge[newco] = newedge ;
	       /* oldco is unused now: reuse it if it was the 
		  last component, otherwise just leave empty */
	       if (oldco == numco) numco-- ;
	       firstedge[oldco] = 0;
	       }
	    }
      }
   }
}  /* end scanning input */
return numco;
}


/* -------------------------------------------------- */
void outCLIQUE(int clique1[], int cliqsize1, 
               int clique2[], int cliqsize2,
               int orignode1[MAXM],
               int orignode2[MAXN]
	       ) 
/* outputs  CLIQUE  using the original node numbers in  orignode
*/
{
if (cliqsize1>0 && cliqsize2>0) {
   int i;
   printf("{") ;
   for (i=0; i<cliqsize1; i++) {
       printf("%d", orignode1[clique1[i]]);
       if (i<cliqsize1-1) printf(", "); 
       }
   printf("}  x  {") ;
   /*  the  "x"  in the output symbolizes the product set */

   for (i=0; i<cliqsize2; i++) {
       printf("%d", orignode2[clique2[i]]);
       if (i<cliqsize2-1) printf(", "); 
       }
   printf("}\n");
   }
}

/* -------------------------------------------------- */
void workonco(int numco,
     int firstedge[MAXCO],
     struct edge edgelist[MAXEDGES]
     )
/* works on the edgelists as generated by  getconnco
   it processes each component by computing its maximal cliques 
pre : firstedge[1..numco], if nonzero, points to a connected component
      in edgelist
post: all components are processed
*/
{
int orignode1[MAXM];
int orignode2[MAXN];
BOOLEAN connected[MAXM][MAXN];
int m; int n;   /* graph dimensions */

int stk[STKSIZE];  /* stack */
int tos;  /* top of stack */
int clique1[MAXM], clique2[MAXN];
    /* CLIQUE for first and second node class  */

int co;
int countco = 0;

for (co=1; co <= numco; co++) 
   if (firstedge[co]) {
   /* found a nonzero component list */
   countco ++ ;
   printf("\nConnected component %d:\n", countco) ;

   genincidence(firstedge[co], edgelist,
       orignode1, orignode2, connected, &m, &n);

   /* compute the cliques of the component via  extend;
      initialize stack with the full sets of nodes
      and empty sets CAND and NOT  */
   tos = 0;
   {
   int i ;
   for (i=0; i<m; i++)  stk[tos++] = i;   /* CAND1 = NODES1 */
   for (i=0; i<n; i++)  stk[tos++] = i;   /* CAND2 = NODES2 */
   }
   extend(stk, connected, clique1, 0, clique2, 0,
          0, 0, m, m, m, m+n, tos, orignode1, orignode2);
   }
}


/* ----- the following are unused TEST ROUTINES ----- */
/* -------------------------------------------------- */
void getgraph(BOOLEAN connected[MAXM][MAXN], int *m, int *n)
/* pre:  graph edges as pairs of nonnegative integers on standard input
	 edges with nodes <0 or >=MAXM/2  will be rejected
	 with a warning message
   post: connected[i][j] = TRUE for the edges (i,j) of the bipartite
         graph, nodes numbered  0..*m-1  and  0..*n-1
	 m <= MAXM,  n <= MAXN
*/
{ 
int i, j;
for (i=0; i<MAXM; i++)
    for (j=0; j<MAXN; j++)
	connected[i][j] = FALSE;
*m = *n = 0;
while (scanf("%d %d", &i, &j) != EOF) {
   if (i < 0 || i>= MAXM || j<0 || j>=MAXN)
      printf("Edge (%d, %d) not in admitted range (0..%d, 0..%d), rejected\n",
	      i,j, MAXM-1, MAXN-1) ;
   else {
      connected[i][j] = TRUE ;
      *m = MAX(*m, i+1) ;
      *n = MAX(*n, j+1) ;
      }
   }
}
/* -------------------------------------------------- */
void outgraph(
     int orignode1[MAXM],
     int orignode2[MAXN],
     BOOLEAN connected[MAXM][MAXN],
     int m,
     int n
     )
/* prints the graph incidence matrix on standard output
   including information about original node numbers */
{
int i, j;
printf("\nConnected component: ") ;
printf("%d columns of right hand side nodes are\n       ", n) ;
for (j=0; j<n; j++)
        printf("%d ", orignode2[j]);
printf("\nincidence matrix with %d rows:\n", m) ;
for (i=0; i<m; i++){
    printf("%5d:", orignode1[i]);
    for (j=0; j<n; j++)
           printf("%2d",  connected[i][j]);
    printf("\n");
    }
}


