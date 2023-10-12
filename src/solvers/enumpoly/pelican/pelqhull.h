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

/* qhull.h -- user-level header file for using qhull.a library
   
   see README, qhull_a.h

   copyright (c) 1993-1994, The Geometry Center

   defines qh_qh, global data structure for qhull.  
   
   NOTE: access to qh_qh is via the 'qh' macro.  This allows
   qh_qh to be either a pointer or a structure.  An example
   of using qh is "qh DROPdim" which accesses the DROPdim
   field of qh_qh.  Similarly, access to qh_qhstat is via
   the 'qhstat' macro.
   
   includes function prototypes for qhull.c, geom.c, global.c, io.c, user.c

   use mem.h for mem.c
   use set.h for set.c
   
   see unix.c for an example of using qhull.h
   
   recompile qhull if you change this file
*/


#include <cstdio>
#include <cstdlib>
#include <csetjmp>
#include <cstring>
#include <cmath>
#include <cfloat>    /* some compilers will not need float.h */
#include <climits>
#include <ctime>
#include <cctype>

#ifndef qhDEFqhull
#define qhDEFqhull 1

#include <csetjmp>
#include <cfloat>
#include <ctime>

/* ============ -types- ==================== */

#define pointT coordT                 /* array of hull_dim coordinates */
#define flagT unsigned int            /* Boolean flag */
typedef enum {False, True} boolT;     /* True=1, False= 0 */

/* -center type for facet->center and CENTERtype */
enum qh_CENTER {qh_none, qh_voronoi, qh_centrum};

/* -output formats for printing (qh PRINTout) */
enum qh_PRINT {qh_PRINTnone, qh_PRINTfacets, qh_PRINTgeom, qh_PRINTnormals, 
  qh_PRINTincidences, qh_PRINTmathematica, qh_PRINToff, qh_PRINTpoints,
  qh_PRINTEND};



/*---------------------------------
-realT -- select size of floating point numbers
   normally use 'float' because convex hulls are space limited in high-d
      this saves about 8% in time and 25% in space
   could use 'double' for internal calculations (realT vs. coordT)
        but this requires many type casts, and adjusted error bounds
   C compilers will often do expressions in double
   WARNING: on some machines, can not test for equality after setting REALmax.
                        Use REALmax/2 instead.
*/

#define REALfloat 1   /* use 0 for double precision */
#if REALfloat
#define realT float
#define REALmax FLT_MAX
#define REALmin FLT_MIN
#define REALepsilon FLT_EPSILON
#define qh_REALdigits 8   /* maximum number of significant digits */
#define qh_REAL_1 "%6.8g "
#define qh_REAL_2n "%6.8g %6.8g\n"
#define qh_REAL_3n "%6.8g %6.8g %6.8g\n"
#else
#define realT double
#define REALmax DBL_MAX
#define REALmin DBL_MIN
#define REALepsilon DBL_EPSILON
#define qh_REALdigits 16    /* maximum number of significant digits */
#define qh_REAL_1 "%6.16g "
#define qh_REAL_2n "%6.16g %6.16g\n"
#define qh_REAL_3n "%6.16g %6.16g %6.16g\n"
#endif
#define coordT realT  /* for stored coordinates and coefficients */

/*------------------------------------------
        constants
*/
#define qh_DEFAULTbox 0.5    /* default box size (Geomview expects 0.5) */
#define qh_INFINITE  -10.101 /* coordinates for Voronoi center at infinity */

#define qh_ALL 	    True     /* argument for printall and checkall parameters*/


/*------------------------------------
	Conditional compilation
	
-COMPUTEfurthest computing furthest saves 4% of memory and costs ca. 3% time
                         (about 40% more distance tests for partitioning)
-MAXoutside      keep maxoutside for each facet
		   this takes a realT per facet and slightly slows down qhull
		   it should speed up pre-merging with Wn or Qm
		   better outer planes for geomview output 
*/

#define qh_COMPUTEfurthest 0    /* 1 removes facet->furthestdist */
#define qh_MAXoutside 1         /* 0 removes facet->maxoutside */


/*-------------------------------------------
-ERR - qhull exit codes, for indicating errors
*/
#define qh_ERRnone  0    /* no error occurred during qhull */
#define qh_ERRinput 1    /* input inconsistency */
#define qh_ERRsingular 2 /* singular input data */
#define qh_ERRprec  3    /* precision error */
#define qh_ERRmem   4    /* insufficient memory, matches mem.h */
#define qh_ERRqhull 5    /* internal error detected, matches mem.h */

/* ============ -structures- ====================
   each of the following structures is defined by a typedef
   all realT and coordT fields occur at the beginning of a structure
        (otherwise space may be wasted due to alignment)
   define all flags together and pack into 32-bit number
*/

typedef struct vertexT vertexT;
typedef struct ridgeT ridgeT;
typedef struct facetT facetT;
#ifndef DEFsetT
#define DEFsetT 1
typedef struct setT setT;          /* defined in set.h */
#endif

/* ----------------------------------------------
-facetT- specifies a facet
*/

struct facetT {
#if !qh_COMPUTEfurthest
  coordT   furthestdist;/* distance to furthest point of outsideset */
#endif
#if qh_MAXoutside
  coordT   maxoutside;  /* max computed distance of point to facet
  			Before QHULLfinished this is an approximation
  			since maxdist not always set for mergefacet */
#endif
  coordT   offset;      /* offset of hyperplane from origin */
  coordT  *normal;      /* normal of hyperplane, hull_dim coefficients */
  coordT  *center;      /* centrum or Voronoi center, see CENTERtype */
  facetT  *previous;    /* previous facet in the facet_list */
  facetT  *next;        /* next facet in the facet_list */
  setT    *vertices;    /* vertices for this facet, inverse sorted by id */
  setT    *ridges;      /* explicit ridges for nonsimplicial facets.
  			   for simplicial facets, neighbors defines ridge */
  setT    *neighbors;   /* neighbors of the facet.
  			   if simplicial, kth neighbor skips kth vertex.
  			   if visible, 1st neighbor (if any) is newfacet 
  			         or deleted newfacet (visible) */
  setT    *outsideset;  /* set of points outside this facet
		           if non-empty, last point is furthest */
  setT    *coplanarset; /* set of points coplanar with this facet
  			   > min_vertex and <= facet->max_outside
                           a point is assigned to the furthest facet
		           if non-empty, last point is furthest away */
  unsigned visitid;     /* visit_id, for visiting all neighbors,
			   all uses must be independent */
  unsigned id:24;       /* unique identifier, =>room for 8 flags 
  			   new facet if >= qh newfacet_id */
  flagT    toporient:1; /* True if created with top orientation
			   after merging, use ridge orientation */
  flagT    simplicial:1;/* True if simplicial facet, ->ridges may be implicit */
  flagT	   good:1;      /* True if a good facet (for output) */
  flagT    tested:1;    /* True if facet changed due to merge */
  flagT    seen:1;      /* used to perform operations only once, like visitid */
  flagT	   flipped:1;   /* True (if !NEWmerges) if facet is flipped */
  flagT	   visible:1;   /* True if visible facet */
};
#define qh_isnewmerge_(facet) ((facet)->flipped)    /* if qh NEWmerges */
#define qh_newmerge_(facet) {(facet)->flipped= True; numnewmerges++;}
#define qh_clearnewmerge_(facet) (facet)->flipped= False;


/*----------------------------------------------
-ridgeT- specifies a ridge
*/

struct ridgeT {
  setT    *vertices;    /* vertices belonging to this ridge, inverse sorted by id 
                           NULL if a degen ridge (matchsame) */
  facetT  *top;         /* top facet this ridge is part of */
  facetT  *bottom;      /* bottom facet this ridge is part of */
  unsigned id:24;       /* unique identifier, =>room for 8 flags */
  flagT    seen:1;      /* used to perform operations only once */
  flagT    tested:1;    /* True when ridge is tested for convexity */
  flagT    nonconvex:1; /* True if getmergeset detected a non-convex neighbor */
  flagT    mergeridge:1; /* True if from a qh_MERGEridge (duplicates) */
};

/* ----------------------------------------------
-vertexT- specifies a vertex
*/

struct vertexT {
  vertexT *next;        /* next vertex in vertex_list */
  vertexT *previous;    /* previous vertex in vertex_list */
  pointT  *point;       /* hull_dim coordinates (coordT) */
  setT    *neighbors;   /* neighboring facets of vertex, if qh_vertexneighbors */
  unsigned visitid;     /* for use with qh vertex_visit */
  unsigned id:24;       /* unique identifier, =>room for 8 flags */
  flagT    seen:1;      /* used to perform operations only once */
  flagT    seen2:1;     /* another seen flag */
  flagT    delridge:1;  /* vertex was part of a deleted ridge */
  flagT	   deleted:1;   /* true if vertex on qh del_vertices */
  flagT    newlist:1;   /* true if vertex on qh newvertex_list */
};

/* ======= -global variables -qh ============================ 

   all global variables for qhull are in qh, qhmem, and qhstat
   
   qhmem is defined in mem.h and qhstat is defined in stat.h
   
   set qh_QHpointer 1 to enable qh_saveqhull() and qh_restoreqhull()
     this costs about 7% in time and space.
*/

typedef struct qhT qhT;    

#define qh_QHpointer 0  /* 1 for dynamic allocation, 0 for global structure */
#if qh_QHpointer
#define qh qh_qh->
extern qhT *qh_qh;     /* allocated in global.c */
#else
#define qh qh_qh.
extern qhT qh_qh;
#endif

struct qhT {

  /*-user flags */

  boolT ALLpoints;        /* true 'Qi' if search all points for initial simplex */
  boolT APPROXhull;       /* true 'W' if MINoutside set */
  realT MINoutside;       /*   min. distance for an outside point */
  boolT BESToutside;      /* true 'Qf' if partition points into best outsideset */
  boolT CHECKfrequently;  /* true 'Tc' if checking frequently */
  realT premerge_cos;     /*   'C-n'   cos_max when pre merging */
  realT postmerge_cos;    /*   'Cn'    cos_max when post merging */
  boolT DELAUNAY;         /* true 'd' if computing DELAUNAY triangulation */
  boolT DOintersections;  /* print hyperplaneintersections in 3-d and 4-d */
  int   DROPdim;          /* drops dim 'GDn' for 4-d -> 3-d output */
  boolT ERREXITcalled;    /* true during errexit (prevents duplicate calls */
  boolT FORCEoutput;      /* true 'Po' if forcing output despite degeneracies */
  int   GOODpoint;        /* 1+n, good facet if visible/not(-) from point n*/
  pointT *GOODpointp;     /*   the actual point */
  boolT GOODthreshold;    /* true if qh lower_threshold/upper_threshold defined 
  			     false if qh SPLITthreshold */
  int   GOODvertex;       /* 1+n, good facet if vertex for point n */
  pointT *GOODvertexp;     /*   the actual point */
  int   IStracing;        /* trace execution, 0=none, 1=least, 4=most, -1=events */
  boolT KEEPcoplanar;     /* true if keeping nearest facet for coplanar points */
  boolT KEEPinside;       /* true if keeping nearest facet for inside/coplanar points */
  boolT MERGING;          /* true if merging, with angle and centrum tests */
  realT   premerge_centrum;  /*   'R-n' centrum_radius when pre merging */
  realT   postmerge_centrum; /*   'Rn' centrum_radius when post merging */
  realT MINvisible;       /* min. distance for a facet to be visible */
  boolT ONLYgood; 	  /* only process points with good visible or horizon facets */
  boolT ONLYmax; 	  /* only process points that increase max_outside */
  boolT POINTSmalloc;     /* true if qh first_point/num_points allocated */
  boolT POSTmerge;        /* true if merging after buildhull (C-n or R-n) */
  boolT PREmerge;         /* true if merging during buildhull (Cn or Rn) */
  boolT PRINTcentrums;	  /* true 'Gc' if printing centrums */
  boolT PRINTcoplanar;    /* true 'Gp' if printing coplanar points */
  int	PRINTdim;      	  /* print dimension for Geomview output */
  boolT PRINTdots;        /* true 'Ga' if printing all points as dots */
  boolT PRINTgood;        /* true 'Pg' if printing good facets */
  boolT PRINTinner;	  /* true 'Gi' if printing inner planes */
  boolT PRINTneighbors;	  /* true 'PG' if printing neighbors of good facets */
  boolT PRINTnoplanes;	  /* true 'Gn' if printing no planes */
  boolT PRINTouter;	  /* true 'Go' if printing outer planes */
  boolT PRINTprecision;   /* false 'Pp' if not reporting precision problems */
  int   PRINTout[qh_PRINTEND]; /* list of output formats to print */
  boolT PRINTridges;      /* true 'Gr' if print ridges */
  boolT PRINTspheres;     /* true 'Gv' if print vertices as spheres */
  boolT PRINTstatistics;  /* true 'Ts' if printing statistics to stderr */
  boolT PRINTsummary;     /* true 's' if printing summary to stderr */
  boolT PROJECTdelaunay;  /* true if DELAUNAY, no readpoints() and
			     need projectinput() for Delaunay */
  int   PROJECTinput;     /* number of projected dimensions 'bn:0Bn:0' */
  boolT QUICKhelp;	  /* true if quick help message for degen input */
  boolT RANDOMdist;       /* true if randomly change distplane and setfacetplane */
  realT RANDOMfactor;     /*    maximum perturbation */
  realT RANDOMa;         /*  qh_randomfactor is randr * RANDOMa + RANDOMb */
  realT RANDOMb;
  boolT RANDOMoutside;    /* true if select a random outside point */
  int	REPORTfreq;       /* buildtracing reports every n facets */
  int	ROTATErandom;	  /* 'QRn' seed, 0 time, >= rotate input */
  boolT SCALEinput;       /* true if scaling input, 'Qb' */
  boolT SETroundoff;      /* true 'E' if qh DISTround is predefined */
  boolT SPLITthresholds;  /* true if upper_/lower_threshold defines a region
                               used only for printing (not for qh ONLYgood) */
  int	STOPcone;         /* 1+n for stopping after building cone for point n*/
  int	STOPpoint;        /* 1+n for stopping after/before(-) adding point n*/
  int   TRACElevel;       /* conditional IStracing level */
  int   TRACEpoint;       /* start tracing when point n becomes a vertex */
  realT TRACEdist;        /* start tracing when merge distance too big */
  int   TRACEmerge;       /* start tracing before this merge */
  boolT VERIFYoutput;     /* true 'Tv' if verify output at end of qhull */
  boolT VERTEXneighbors;  /* true if maintaining vertex neighbors */
  boolT VIRTUALmemory;    /* true if depth-first processing in buildhull */
  boolT VORONOI;	  /* true 'v' if computing Voronoi diagram */

  /* -input constants */
  int 	input_dim;	  /* dimension of input, set by initbuffers */
  int 	num_points;       /* number of input points */
  pointT *first_point;    /* first input point */
  int 	hull_dim;         /* dimension of hull, set by initbuffers */
  char 	qhull_command[256];/* command line that invoked this program */
  char 	rbox_command[256]; /* command line that produced the input points */
  realT *upper_threshold; /* don't print if facet->normal[k]>=upper_threshold[k]
                             must set either GOODthreshold or SPLITthreshold
  			     if Delaunay, default is 0.0 for upper envelope */
  realT *lower_threshold; /* don't print if facet->normal[k] <=lower_threshold[k] */
  realT *upper_bound;     /* scale point[k] to new upper bound */
  realT *lower_bound;     /* scale point[k] to new lower bound 
  			     project if both upper_ and lower_bound == 0 */

  /* -precision constants, computed in qh_maxmin */
  
  realT centrum_radius;   /* max centrum radius for convexity (roundoff added) */
  realT cos_max;	  /* max cosine for convexity (roundoff added) */
  realT maxmaxcoord;      /* max coordinate in any dimension */
  realT DISTround;        /* max round off error for distances, 'E' overrides */
  realT ONEmerge;         /* max distance for merging simplicial facets */
  realT MINdenom_1;       /* min. abs. value for 1/x */
  realT MINdenom;         /*    use divzero if denominator < MINdenom */
  realT MINdenom_1_2;     /* min. abs. val for 1/x that allows normalization */
  realT MINdenom_2;       /*    use divzero if denominator < MINdenom_2 */
  realT *NEARzero;        /* hull_dim array for near zero in gausselim */
  
  /* -internal constants */

  char qhull[sizeof("qhull")]; /* for checking ownership */
  void *old_stat;         /* pointer to saved qh_qhstat, qh_save_qhull */
  jmp_buf errexit;        /* exit label for qh_errexit */
  FILE *fin;              /* pointer to input file, init by qh_meminit */
  FILE *fout;             /* pointer to output file */
  FILE *ferr;             /* pointer to error file */
  pointT *interior_point; /* center point of the initial simplex*/
  int   normal_size;      /* size in bytes for facet normals and point coords*/
  int   center_size;      /* size in bytes for Voronoi centers */
  int   TEMPsize;         /* size for small, temporary sets (in quick mem) */

  /* -list of all facets, from facet_list to facet_tail, see qh_appendfacet */
 
  facetT *facet_list;     /* first facet */
  facetT *facet_next;     /* next facet for buildhull()
    			     all previous facets do not have outside sets*/
  facetT *visible_list;   /* list of visible facets, with visible set */
  int       num_visible;  /* current number of visible facets */
  facetT *newfacet_list;  /* list of new facets to end of facet_list */
  facetT *facet_tail;     /* end of facet_list (dummy facet) */
  unsigned tracefacet_id;  /* set at init, then can print whenever */
  facetT *tracefacet;     /*   set in newfacet/mergefacet, undone in delfacet*/
  unsigned tracevertex_id;  /* set at buildtracing, can print whenever */
  vertexT *tracevertex;     /*   set in newvertex, undone in delvertex*/
  vertexT *vertex_list;   /* list of all vertices, to vertex_tail */
  vertexT *vertex_tail;   
  vertexT *newvertex_list; /* list of vertices in newfacet_list 
                             all vertices have 'new' set */
  int 	num_facets;	  /* number of facets in facet_list
			     includes visble faces (num_visible) */
  int 	num_vertices;     /* number of vertices in facet_list */
  int   num_outside;      /* number of points in outsidesets */
  int 	num_coplanar; 	  /* number of coplanar points */
  int   num_good;         /* number of good facets (after findgood_all) */
  unsigned newfacet_id;   /* new facet if facet->id >= newfacet_id */
  int 	facet_id;         /* id of next, new facet from newfacet() */
  int 	ridge_id;         /* id of next, new ridge from newridge() */
  unsigned vertex_id;        /* id of next, new vertex from newvertex() */

  /* -variables */
  
  clock_t hulltime;       /* ignore time to set up input and randomize */
  int	CENTERtype;       /* current type of facet->center, qh_CENTER */
  int 	furthest_id;      /* pointid of furthest point, for tracing */
  facetT *GOODclosest;    /* closest facet to GOODthreshold in qh_findgood */
  realT max_outside;      /* maximum distance from a point to a facet,
			       before roundoff, not simplicial vertices */
  realT max_vertex;       /* maximum distance (>0) from vertex to a facet,
			       before roundoff, not simplicial vertices */
  realT min_vertex;       /* minimum distance (<0) from vertex to a facet,
			       before roundoff, not simplicial vertices 
			       defines coplanar points */
  boolT NEWfacets;        /* true while visible facets invalid
			      from makecone/attachnewfacets to deletevisible */
  boolT NEWmerges;        /* true while merging,
			     facet->flipped is defined as facet->newmerge */
  boolT NOerrexit;        /* true if qh_errexit is not available */
  realT PRINTcradius;     /* radius for printing centrums */
  realT PRINTradius;      /* radius for printing vertex spheres and points */
  boolT POSTmerging;      /* true when post merging */
  int 	printoutvar;	  /* temporary variable for qh_printbegin, etc. */
  int 	printoutnum;	  /* number of facets printed */
  boolT QHULLfinished;    /* True after qhull() is finished */
  int 	visit_id;         /* unique id for searching neighborhoods, */
  int 	vertex_visit;     /* unique id for searching vertices */
  
  /* -sets */
  setT *facet_mergeset;   /* temporary set of merges to be done */
  setT *initial_points;   /* initial simplex for buildhull() */
  setT *hash_table;	  /* hash table for matching ridges in qh_matchfacets 
                             size is setsize() */
  int   num_hashentries;  /* current number of hashentries */
  setT *other_points;     /* additional points (first is qh interior_point) */
  setT *del_vertices;     /* vertices to partition and delete with visible 
                             facets.  Have deleted set for checkfacet */

  /* -buffers */
  coordT *gm_matrix;      /* (dim+1)Xdim matrix for geom.c */
  coordT **gm_row;        /* array of gm_matrix rows */
  char* line;             /* malloc'd input line of maxline+1 chars */
  int maxline;
  
  /* -statics */
  boolT firstcentrum; 	  /* for qh_printcentrum */
  int  lastreport;        /* for qh_buildtracing */
  int  mergereport;       /* for qh_tracemerging */
  boolT old_randomdist;   /* save in io.c for RANDOMdist */
  int   ridgeoutnum;      /* number of ridges in 4OFF output */
  void *old_qhstat;       /* for saving qh_qhstat in save_qhull() */
  setT *old_tempstack;     /* for saving qhmem.tempstack in save_qhull */
};

/* =========== -macros- ========================= 
-otherfacet_(ridge, facet)   return neighboring facet for a ridge in facet
-getid_(p)		     return id or -1 if NULL
*/

#define otherfacet_(ridge, facet) \
                        (((ridge)->top == (facet)) ? (ridge)->bottom : (ridge)->top)
#define getid_(p)       ((p) ? (p)->id : (unsigned int) -1)

/* ---------------------------------------------
-FORALL and FOREACH macros

   These all iterate using a variable of the same name, e.g. FORALLfacets
   and FOREACHfacet_ uses 'facet' declared by 'facetT *facet'.  The macros
   may use auxiliary variables as indicated.

-FORALLfacets                iterate over all facets in facetlist 
-FORALLpoint_(points, num)   iterate over num points (uses 'pointT *pointtemp')
-FORALLvertices              iterate over all vertices in vertex_list

-FOREACHfacet_(facets)	     iterate over facet set (uses 'facetT **facetp')
-FOREACHneighbor_(facet)     iterate over facet->neighbors (uses 'facetT **neighborp')
-FOREACHpoint_(points)       iterate over point set (uses 'pointT **pointp')
-FOREACHridge_(ridges)	     iterate over ridge set (uses 'ridgeT **ridgep')
-FOREACHvertex_(vertice)     iterate over vertex set (uses 'vertexT **vertexp')
-FOREACHadjacent_(vertex)    iterate over adjacent vertices to vertex 
-FOREACHneighbor_(vertex)    iterate over neighboring facets to vertex 

-FOREACHfacet_i_(facets)    iterate over facets by facet_i and facet_n
-FOREACHneighbor_i_(facet)  iterate over facet->neighbors by neighbor_i, neighbor_n
-FOREACHvertex_i_(vertices) iterate over vertices by vertex_i, vertex_n
-FOREACHpoint_i_(points)    iterate over points by point_i, point_n
-FOREACHridge_i_(ridges)    iterate over ridges by ridge_i, ridge_n
-FOREACHadjacent_i_(vertex) iterate over adjacents by adjacent_i, adjacent_n
-FOREACHneighbor_i_(vertex) iterate over vertex->neighbors by neighbor_i, neighbor_n

 WARNING: nested loops can't use the same variable (define another FOREACH)
 WARNING: strange behavior if don't fully brace when nested (including
        intervening blocks, e.g. FOREACH...{ if () FOREACH...} )
 poly.h defines other FOREACH/FORALL macros
 set.h  defines FOREACHsetelement and contains additional notes
*/
#define FORALLfacets for (facet=qh facet_list;facet && facet->next;facet=facet->next)
#define FORALLpoints FORALLpoint_(qh first_point, qh num_points)
#define FORALLvertices for (vertex=qh vertex_list;vertex && vertex->next;vertex= vertex->next)

#define FORALLpoint_(points, num) for(point= (points), \
      pointtemp= (points)+qh hull_dim*(num); point < pointtemp; point += qh hull_dim)
#define FOREACHfacet_(facets)    FOREACHsetelement_(facetT, facets, facet)
#define FOREACHneighbor_(facet)  FOREACHsetelement_(facetT, facet->neighbors, neighbor)
#define FOREACHpoint_(points)    FOREACHsetelement_(pointT, points, point)
#define FOREACHridge_(ridges)    FOREACHsetelement_(ridgeT, ridges, ridge)
#define FOREACHvertex_(vertices) FOREACHsetelement_(vertexT, vertices,vertex)
#define FOREACHadjacent_(vertex) FOREACHsetelement_(vertexT, vertex->adjacencies, adjacent)

#define FOREACHfacet_i_(facets)    FOREACHsetelement_i_(facetT, facets, facet)
#define FOREACHneighbor_i_(facet)  FOREACHsetelement_i_(facetT, facet->neighbors, neighbor)
#define FOREACHpoint_i_(points)    FOREACHsetelement_i_(pointT, points, point)
#define FOREACHridge_i_(ridges)    FOREACHsetelement_i_(ridgeT, ridges, ridge)
#define FOREACHvertex_i_(vertices) FOREACHsetelement_i_(vertexT, vertices,vertex)

/* ======= -functions =========== 

  	see corresponding .c file for definitions

	Qhull functions (see qhull.c and qhull_a.h)
-qhull		construct the convex hull of a set of points
-addpoint       add point to hull (must be above facet)
-delpoint       delete vertex for point from hull

	User redefinable functions (see user.c)
-errexit	 	return exitcode to system after an error
-errprint		print erroneous facets, ridge, and vertex
-printfacetlist		print all fields for a list of facets
-user_memsizes          define up to 10 additional quick allocation sizes
  	
	Geometric functions (see geom.c and geom.h, other useful functions)
-gram_schmidt   implements Gram-Schmidt orthogonalization by rows
-projectinput  project input along one or more dimensions + Delaunay projection
-randommatrix   generate a random dimXdim matrix in range (-1,1)
-rotatepoints   rotate numpoints points by a row matrix
-scaleinput    scale input to new lowbound and highbound
-findbest      find visible facet for a point starting at a facet

	Global init/free functions (see global.c and qhull_a.h)
-freeqhull	     free memory used by qhull
-init_qhull_command  build qhull_command from argc/argv
-initflags	     set flags and initialized constants from command line
-initqhull_buffers   initialize global memory buffers
-initqhull_globals   initialize globals
-initqhull_mem	initialize mem.c for qhull
-initqhull_start     start initialization of qhull
-initthresholds	     set thresholds for printing and scaling from command line
-restore_qhull       restores a saved qhull
-save_qhull          saves qhull for later restoring

	Input/output functions (see io.c and io.h)
-readpoints     read points from input

	Polyhedron functions (see poly.c)
-check_output	check output data structure according to user flags
-check_points	verify that all points are inside the hull

-point          return point for a point id, or NULL if unknown
-pointid        return id for a point, or -1 if not known

-facetvertices  returns temporary set of vertices in a set of facets
-pointfacet	return temporary set of facets indexed by point id
-pointvertex	return temporary set of vertices indexed by point id
*/

/********* -qhull.c prototypes (duplicated from qhull_a.h) ***************/

void    qh_qhull ();
boolT   qh_addpoint (pointT *furthest, facetT *facet, boolT checkdist);
boolT   qh_delpoint (pointT *point, facetT *facet);

/***** -geom.c prototypes (duplicated from geom.h) ****************/

facetT *qh_findbest (pointT *point, facetT *facet, boolT bestoutside,
		     unsigned firstid, realT *dist, boolT *isoutside, int *numpart);
boolT   qh_gram_schmidt(int dim, realT **rows);
void    qh_projectinput ();
void    qh_randommatrix (realT *buffer, int dim, realT **row);
void    qh_rotateinput (realT **rows);
void    qh_scaleinput ();

/***** -global.c prototypes (alphabetical) ***********************/

void    qhull_fatal(int); 
void 	qh_freebuffers ();
void    qh_freeqhull (boolT allmem);
void 	qh_init_qhull_command (int argc, char *argv[]);
void    qh_initbuffers (coordT *points, int numpoints, int dim, boolT ismalloc);
void 	qh_initflags (char *command);
void 	qh_initqhull_buffers ();
void 	qh_initqhull_globals (coordT *points, int numpoints, int dim, boolT ismalloc);
void    qh_initqhull_mem ();
void 	qh_initqhull_start (FILE *infile, FILE *outfile, FILE *errfile);
void 	qh_initthresholds (char *command);
#if qh_QHpointer
void 	qh_restore_qhull (qhT **oldqh);
qhT    *qh_save_qhull (void);
#endif

/**************************************************************************/
/****************** some definitions from qhull_a.h ***********************/
/**************************************************************************/

/********* -poly.c prototypes (duplicated from poly.h) **********************/

void    qh_check_output ();
void    qh_check_points ();
setT   *qh_facetvertices (facetT *facetlist, setT *facets, boolT allfacets);
pointT *qh_point (int id);
setT   *qh_pointfacet ( /*qh.facet_list*/);
int     qh_pointid (pointT *point);
setT   *qh_pointvertex ( /*qh.facet_list*/);

/********* -stat.c prototypes (duplicated from stat.h) **********************/

void    qh_collectstatistics ();
void 	qh_freestatistics ();
void    qh_initstatistics ();
void    qh_printallstatistics (FILE *fp, const char *string);
void    qh_printstatistics (FILE *fp, const char *string);


/* ======= -constants- ====================== 

        System dependent constants
-SECticks       ticks per second from clock()
-RANDOMmax      maximum random number and random generator, default is random()

       	Memory constants for calling qh_meminitbuffers in global.c
-MEMalign	memory alignment (see mem.h). If using gcc, best alignment is
              #define qh_MEMalign fmax_(__alignof__(realT),__alignof__(void *))
-MEMbufsize 	memory buffer size
-MEMinitbuf 	initial memory buffer size.  It should hold enough
   		   facets to keep outsidesets in short memory.

-HASHfactor     total hash slots / used hash slots
-VERIFYdirect   verify all points against all facets if op count smaller
-MAXrandom      maximum random number
-ORIENTclock    true if clockwise orientation on output 

	Conditional compilation
-KEEPstatistics 1 removes most of statistic gathering and reporting
                    it reduces code by about 8%, time is the same.
-QUICKhelp      use abbreviated help messages for degenerate inputs
-UNIX	        true if compiling for UNIX

*/

#define qh_SECticks 1E6             /* CLOCKS_PER_SECOND for clock() */

#if 0     /* use 1 if your compiler supports random(), otherwise use 0 */
/* #define qh_RANDOMmax ((realT)0x7fffffffUL) */   /* 31 bits, random()/MAX */
#define qh_RANDOMint random()
#define qh_RANDOMseed_(seed) srandom(seed);
#else
/* #define qh_RANDOMmax ((realT)32767) */  /* 15 bits (System 5) */

#define qh_RANDOMmax 2147483647  /* Kludge added, ignorantly, by AMM */

                                /* WARNING: Sun produces 31 bits from rand() */
#define qh_RANDOMint  rand()
#define qh_RANDOMseed_(seed) srand((unsigned)seed);
#endif

#define qh_MEMalign fmax_(sizeof(realT), sizeof(void *))
#define qh_MEMbufsize 0x8000        /* allocate 32K memory buffers */
#define qh_MEMinitbuf 0x10000       /* initially allocate 64K buffer */
#define qh_HASHfactor 2             /* (int) at worst 50% occupancy for qh hash_table
                                       and normally 25% occupancy */
#define qh_VERIFYdirect 100000      /* if more tests, use qh_findbest instead */
#define qh_ORIENTclock 0            /* counter clockwise for Geomview inward */

#define qh_KEEPstatistics 1   /* 0 to take out statistics */
#define qh_QUICKhelp    0    /* 1 for short help messages */
#define qh_UNIX         1    /* 1 if compiling for UNIX */


/**************************************************************************/
/************** definitions and signatures from mem.h *********************/
/**************************************************************************/

/* to avoid bus errors, memory allocation must consider alignment requirements.
   malloc() automatically takes care of alignment.   Since mem.c manages
   its own memory, we need to explicitly specify alignment in
   qh_meminitbuffers().

   A safe choice is sizeof(double).  sizeof(float) may be used if doubles 
   do not occur in data structures and pointers are the same size.  Be careful
   of machines (e.g., DEC Alpha) with large pointers.  If gcc is available, 
   use __alignof__(double) or fmax_(__alignof__(float), __alignof__(void *)).

   see qhull_a.h for qhull's alignment
*/

#define qhmem_ERRmem 4    /* matches qh_ERRmem in qhull.h */
#define qhmem_ERRqhull 5  /* matches qh_ERRqhull in qhull.h */

/*---------------------------------------
-qhmemT - global memory structure for mem.c

   users should ignore qhmem except for writing extensions
   
   qhmem could be swapable like qh and qhstat, but then
   multiple qh's and qhmem's would need to keep in synch.  
   A swapable qhmem would also waste memory buffers.  As long
   as memory operations are atomic, there is no problem with
   multiple qh structures being active at the same time.
   If you need separate address spaces, you can swap the
   contents of qhmem.
*/

typedef struct qhmemT qhmemT;
extern qhmemT qhmem;  /* allocated in mem.c */

struct qhmemT {               /* global memory management variables */
  int      BUFsize;	      /* size of memory allocation buffer */
  int      BUFinit;	      /* initial size of memory allocation buffer */
  int      TABLEsize;         /* actual number of sizes in free list table */
  int      NUMsizes;          /* maximum number of sizes in free list table */
  int      LASTsize;          /* last size in free list table */
  int      ALIGNmask;         /* worst-case alignment, must be 2^n-1 */
  void	 **freelists;          /* free list table, linked by offset 0 */
  int     *sizetable;         /* size of each freelist */
  int     *indextable;        /* size->index table */
  void    *curbuffer;         /* current buffer, linked by offset 0 */
  void    *freemem;           /*   free memory in curbuffer */
  int 	   freesize;          /*   size of free memory in bytes */
  void 	  *tempstack;         /* stack of temporary memory, managed by users */
  FILE    *ferr;              /* file for reporting errors */
  int      IStracing;         /* =5 if tracing memory allocations */
  int cntquick;          /* count of quick allocations */
                         /* remove statistics doesn't effect speed */
  int cntshort;          /* count of short allocations */
  int cntlong;           /* count of long allocations */
  int curlong;           /* current count of inuse, long allocations */
  int freeshort;	      /* count of short memfrees */
  int freelong;	      /* count of long memfrees */
  int totshort;          /* total size of short allocations */
  int totlong;           /* total size of long allocations */
  int maxlong;           /* maximum totlong */
  int cntlarger;         /* count of setlarger's */
  int totlarger;         /* total copied by setlarger */
};


/* ======= -macros =========== 

qh_memalloc_(size, freelistp, object)  returns object of size bytes 
	assumes size<=qhmem.LASTsize and void **freelistp is a temp

qh_memfree_(object, size, freelistp) free up quick object
	object may be NULL
	assumes size<=qhmem.LASTsize and void **freelistp is a temp
*/

#define qh_memalloc_(size, freelistp, object) {\
  freelistp= (void **)(qhmem.freelists + qhmem.indextable[size]);\
  if ((object= (setT *)*freelistp)) {\
    qhmem.cntquick++;  \
    *freelistp= *((void **)*freelistp);\
  }else object= (setT *)qh_memalloc (size);}

#define float_qh_memalloc_(size, freelistp, object) {\
  freelistp= (void **)(qhmem.freelists + qhmem.indextable[size]);\
  if ((object= (float *)*freelistp)) {\
    qhmem.cntquick++;  \
    *freelistp= *((void **)*freelistp);\
  }else object= (float *)qh_memalloc (size);}

#define facetT_qh_memalloc_(size, freelistp, object) {\
  freelistp= (void **)(qhmem.freelists + qhmem.indextable[size]);\
  if ((object= (facetT *)*freelistp)) {\
    qhmem.cntquick++;  \
    *freelistp= *((void **)*freelistp);\
  }else object= (facetT *)qh_memalloc (size);}

#define ridgeT_qh_memalloc_(size, freelistp, object) {\
  freelistp= (void **)(qhmem.freelists + qhmem.indextable[size]);\
  if ((object= (ridgeT *)*freelistp)) {\
    qhmem.cntquick++;  \
    *freelistp= *((void **)*freelistp);\
  }else object= (ridgeT *)qh_memalloc (size);}

#define mergeT_qh_memalloc_(size, freelistp, object) {\
  freelistp= (void **)(qhmem.freelists + qhmem.indextable[size]);\
  if ((object= (mergeT *)*freelistp)) {\
    qhmem.cntquick++;  \
    *freelistp= *((void **)*freelistp);\
  }else object= (mergeT *)qh_memalloc (size);}

#define qh_memfree_(object, size, freelistp) {\
  if (object) { \
    qhmem .freeshort++;\
    freelistp= qhmem.freelists + qhmem.indextable[size];\
    *((void **)object)= *freelistp;\
    *freelistp= object;}}

/* ======= -functions =========== 

	see mem.c for definitions

	User level functions
-memalloc	allocate memory
-memfree	free memory
-memstatistics  print memory statistics

	Initialization and termination functions
-meminit	initialize memory
-meminitbuffers	initialize memory buffers
-memsize	define a free list for a size
-memsetup	set up memory (activates memalloc/free)
-memfreeshort	free up all memory buffers
*/

/*---------- -prototypes in alphabetical order -----------*/

void *qh_memalloc(int insize);
void qh_memfree (void *object, int size);
void qh_memfreeshort (int *curlong, int *totlong);
void qh_meminit (FILE *ferr);
void qh_meminitbuffers (int tracelevel, int alignment, int numsizes,
			int bufsize, int bufinit);
void qh_memsetup ();
void qh_memsize(int size);
void qh_memstatistics (FILE *fp);

/**** end mem.h ****/

/**************************************************************************/
/************** definitions and signatures from set.h *********************/
/**************************************************************************/

/* ----------------------------------------------
-constants and flags
*/
#define SETelemsize sizeof(void *) /* specifies size of set element in bytes */

/* ================= -structures- ===============
*/
#ifndef DEFsetT
#define DEFsetT 1
typedef struct setT setT;   /* a set is a sorted or unsorted array of pointers */
#endif

/* ----------------------------------------------
-setT- a set of anything
*/

struct setT {
  unsigned int maxsize; /* maximum number of elements (except NULL) */
  void *e[1];           /* array of pointers, tail is NULL */
                        /* last slot (unless NULL) is actual size+1 
                           e[maxsize]==NULL or e[e[maxsize]-1]==NULL */
};


/* =========== -macros- ========================= */

/*-----------------------------------------------
-FOREACHsetelement_(type, set, variable)- define FOREACH iterator
    variable is NULL at end of loop
    assumes *variable and **variablep are declared
    variablep is one beyond variable.  
    to repeat an element,
          variablep--; / *repeat* /
    use FOREACHsetelement_i_() if need index or include NULLs
    WARNING: strange behavior if don't use braces when nested

-FOREACHsetelement_i_(type, set, variable)- define FOREACH iterator
    assumes *variable, variable_n, and variable_i are declared
    variable_i is index, variable_n is qh_setsize()
    variable may be NULL inside looop
    variable is NULL at end of loop
    variable_i--; variable_n-- repeats for deleted element

-FOREACHsetelementreverse_
    same as FOREACHsetelement but returns elements in reverse order
    uses 'int variabletemp'

-FOREACHsetelementreverse12_
    same as FOREACHsetelement but returns e[1], e[0], e[2], e[3],

-FOREACHelem_(set)- for each element in a set of elements
-FOREACHset_(sets)- for each set in a set of sets
-SETindex_(set,elem)- returns index for iterated elem in set
*/
#define FOREACHsetelement_(type, set, variable) \
        if (set || (variable= NULL)) for(\
          variable##p= (type **)&((set)->e[0]); \
	  (variable= *variable##p++);)
#define FOREACHsetelement_i_(type, set, variable) \
        if (set || (variable= NULL)) for (\
          variable##_i= 0, variable= (type *)((set)->e[0]), \
                   variable##_n= qh_setsize(set);\
          variable##_i < variable##_n;\
          variable= (type *)((set)->e[++variable##_i]) )
#define FOREACHsetelementreverse_(type, set, variable) \
        if (set || (variable= NULL)) for(\
	   variable##temp= qh_setsize(set)-1, variable= qh_setlast(set);\
	   variable; variable= \
	   ((--variable##temp >= 0) ? SETelem_(set, variable##temp) : NULL))
#define FOREACHsetelementreverse12_(type, set, variable) \
        if (set || (variable= NULL)) for(\
          variable##p= (type **)&((set)->e[1]); \
	  (variable= *variable##p); \
          variable##p == ((type **)&((set)->e[0]))?variable##p += 2: \
	      (variable##p == ((type **)&((set)->e[1]))?variable##p--:variable##p++))
#define FOREACHelem_(set) FOREACHsetelement_(void, set, elem)
#define FOREACHset_(sets) FOREACHsetelement_(setT, sets, set)
#define SETindex_(set, elem) ((void **)elem##p - (void **)&(set)->e[1])

/*-----------------------------------------------
-SETelem_(set, n)- return the n'th element of set
      assumes that n is valid [0..size] and that set is defined
      may need a type cast
      
-SETelemaddr_(set, n, type)-return address of the n'th element of a set
      assumes that n is valid [0..size] and set is defined

-SETfirst_(set)- return first element of set
-SETsecond_(set)- return second element of set
-SETaddr_(set, type)-   return address of set's elements
*/
#define SETelem_(set, n)           ((set)->e[n])
#define SETelemaddr_(set, n, type) ((type **)(&((set)->e[n])))
#define SETfirst_(set)             ((set)->e[0])
#define SETsecond_(set)            ((set)->e[1])
#define SETaddr_(set,type)	   ((type **)(&((set)->e[0])))

/*-----------------------------------------------
-SETreturnsize_(set, size) - return size of a set
      set must be defined
      use qh_setsize(set) unless speed is critical

-SETempty_(set) - return true (1) if set is empty
      set may be NULL
*/
#define SETreturnsize_(set, size) (((size)= (long)((set)->e[(set)->maxsize]))?(--(size)):((size)= (set)->maxsize))
#define SETempty_(set) 	          (!set || (SETfirst_(set) ? 0:1))

/* ======= -functions =========== 

   see set.c for function definitions

	Add functions
-setaddsorted	    adds an element to a sorted set
-setaddnth	    adds newelem as n'th element of sorted or unsorted set
-setappend	    appends an element to a set
-setappend_set      appends a set to a set
-setappend2ndlast   makes newelem the next to the last element in set
-setlarger	    returns a larger set that contains elements of *setp
-setreplace	    replaces oldelem in set with newelem
-setunique	    add an element if not already in set

	Access and predicate functions	
-setin		    returns 1 if setelem is in a set, 0 otherwise
-setindex	    returns the index of elem in set.   If none, returns -1
-setlast	    return last element of set or NULL
-setequal	    returns 1 if two sorted sets are equal, otherwise returns 0
-setequal_except    returns 1 if two sorted sets are equal except at element
-setequal_skip	    returns 1 if two sorted sets are equal except for skips

	Delete functions
-setdel		    deletes oldelem from unsorted set.
-setdelsorted	    deletes oldelem from sorted set
-setdelnth	    delete and return nth element from unsorted set
-setdelnthsorted    delete and return nth element from sorted set
-setdellast	    delete and return last element from set or NULL
-setnew_delnthsorted create a sorted set not containing nth element

	Allocation and deallocation functions
-setnew		    create a new set
-setfree	    free the space occupied by a sorted or unsorted set
-setfreelong	    frees a set only if it's in long memory

	Temporary set functions
-settemp	    return a stacked, temporary set
-settempfree	    free temporary set at top of qhmem.tempstack
-settemppop	    pop qhmem.tempstack (makes temporary set permanent)
-settemppush	    push temporary set unto qhmem.tempstack (makes it temporary)
-settempfree_all    free all temporary sets in qhmem.tempstack

	Other functions
-setsize	    returns the size of a set
-setcopy	    copies a sorted or unsorted set into another
-setcheck	    check set for validity
-setprint	    print set elements to fp
-settruncate        truncate set to size elements
-setzero            zero remainder of set and set to maximum size
*/

/*---------- -prototypes in alphabetical order -----------*/

void  qh_setaddsorted(setT **setp, void *elem);
void  qh_setaddnth(setT **setp, int nth, void *newelem);
void  qh_setappend(setT **setp, void *elem);
void  qh_setappend_set(setT **setp, setT *setA);
void  qh_setappend2ndlast(setT **setp, void *elem);
void  qh_setcheck(setT *set, const char */* typename: GIVES ERROR */, int id);
setT *qh_setcopy(setT *set, int extra);
void *qh_setdel(setT *set, void *elem);
void *qh_setdellast(setT *set);
void *qh_setdelnth(setT *set, int nth);
void *qh_setdelnthsorted(setT *set, int nth);
void *qh_setdelsorted(setT *set, void *newelem);
int   qh_setequal(setT *setA, setT *setB);
int   qh_setequal_except (setT *setA, void *skipelemA, setT *setB, void *skipelemB);
int   qh_setequal_skip (setT *setA, int skipA, setT *setB, int skipB);
void  qh_setfree(setT **set);
void  qh_setfreelong(setT **set);
int   qh_setin(setT *set, void *setelem);
int   qh_setindex(setT *set, void *setelem);
void  qh_setlarger(setT **setp);
void *qh_setlast(setT *set);
setT *qh_setnew(int size);
setT *qh_setnew_delnthsorted(setT *set, int size, int nth, int prepend);
void  qh_setprint(FILE *fp, const char* string, setT *set);
void  qh_setreplace(setT *set, void *oldelem, void *newelem);
int   qh_setsize(setT *set);
setT *qh_settemp(int setsize);
void qh_settempfree(setT **set);
void qh_settempfree_all();
setT *qh_settemppop();
void qh_settemppush(setT *set);
void qh_settruncate (setT *set, int size);
int qh_setunique (setT **set, void *elem);
void qh_setzero (setT *set, int index, int size);

/**** end set.h ****/

/**************************************************************************/
/************** definitions and signatures from geom.h ********************/
/**************************************************************************/

/* ============ -macros- ======================== */

/*----------------------------------------------
-fabs_(a)          returns the absolute value of a
-fmax_(a,b)        returns the maximum value of a and b
-fmin_(a,b)        returns the minimum value of a and b
-maximize_(maxval, val)  sets maxval to val if greater
-minimize_(minval, val)  sets minval to val if less
*/
#define fabs_(a) (((a) < 0) ? -(a):(a))
#define fmax_(a,b)  ( (a) < (b) ? (b) : (a) )
#define fmin_(a,b)  ( (a) > (b) ? (b) : (a) )
#define maximize_(maxval, val) {if ((maxval) < (val)) (maxval)= (val);}
#define minimize_(minval, val) {if ((minval) > (val)) (minval)= (val);}

/*-----------------------------------------------
-det2_(a1, a2, 		2-d determinate
       b1, b2)
-det3_(a1, a2, a3,      3-d determinate
       b1, b2, b3,
       c1, c2, c3)
*/
#define det2_(a1,a2,b1,b2) ((a1)*(b2) - (a2)*(b1))
#define det3_(a1,a2,a3,b1,b2,b3,c1,c2,c3) ( (a1)*det2_(b2,b3,c2,c3) \
		- (b1)*det2_(a2,a3,c2,c3) + (c1)*det2_(a2,a3,b2,b3) )  

/*-----------------------------------------------
-dX, dY, dZ- coordinate differences given row pointers rows[]
*/
#define dX(p1,p2)  (*(rows[p1]) - *(rows[p2]))
#define dY(p1,p2)  (*(rows[p1]+1) - *(rows[p2]+1))
#define dZ(p1,p2)  (*(rows[p1]+2) - *(rows[p2]+2))
#define dW(p1,p2)  (*(rows[p1]+3) - *(rows[p2]+3))

/* ======= -functions =========== 

   see geom.c for definitions

      	Geometric functions
-crossproduct   compute the cross product of 2 3-d vectors
-determinant    compute the determinant of a square matrix
-detsimplex     return determinate of a simplex of points
-divzero        divide by a number that's nearly zero
-gausselim      Gaussian elimination with partial pivoting
-getangle       return cosine of angle (dot product of two qh hull_dim vectors)
-gram_schmidt   implements Gram-Schmidt orthogonalization by rows
-inthresholds   return True if normal within qh lower_/upper_threshold
-maxabsval      return max absolute value of a vector
-maxsimplex	determines maximum simplex for a set of points 
-minabsval     return min absolute value of a dim vector
-normalize      normalize a vector
-pointdist      return distance between two points
-printmatrix    print matrix given by row vectors
-printpoints    print pointids for a set of points starting at index 
-projectpoints  project points along one or more dimensions
-randomfactor	return a random factor within qh RANDOMdistmax of 1.0
-randommatrix   generate a random dimXdim matrix in range (-1,1)
-rotatepoints   rotate numpoints points by a row matrix
-scalepoints    scale points to new lowbound and highbound
-sethyperplane_det return hyperplane for oriented simplex, uses determinates
-sethyperplane_gauss return hyperplane for oriented simplex, uses Gaussian elimination
-voronoi_center return Voronoi center for a set of points

      	Qhull's geometric functions
-backnormal     solve for normal x using back substitution over rows U
-distplane      return distance from point to facet (>0 if point is above facet)
-facetcenter    return Voronoi center for a facet's vertices
-findbest	find visible facet for a point starting at a facet
-findgooddist   find best good facet visible for point from facet
-getcenter      return arithmetic center of a set of vertices
-getcentrum     return centrum for a facet
-maxmin         return max/min points for each dim., sets max roundoff errors
-orientoutside  make facet outside oriented via qh interior_point
-projectinput   project input using qh DELAUNAY and qh low_bound/high_bound
-projectpoint   project point onto a facet by distance
-rotateinput    rotate input using row matrix
-scaleinput     scale input using qh low_bound/high_bound
-setfacetplane  sets the hyperplane for a facet
*/

/*---------- -prototypes in alphabetical order -----------*/


void    qh_backnormal (realT **rows, int numrow, int numcol, boolT sign, coordT *normal, boolT *nearzero);
void    qh_crossproduct (int dim, realT vecA[3], realT vecB[3], realT vecC[3]);
realT 	qh_determinant (realT **rows, int dim, boolT *nearzero);
realT   qh_detsimplex (pointT *apex, setT *points, int dimension, boolT *nearzero);
void	qh_distplane (pointT *point, facetT *facet, realT *dist);
realT   qh_divzero(realT numer, realT denom, realT mindenom1, boolT *zerodiv);
pointT *qh_facetcenter (setT *vertices);
facetT *qh_findbest (pointT *point, facetT *facet, boolT bestoutside,
		unsigned firstid, realT *dist, boolT *isoutside, int *numpart);
facetT *qh_findgooddist (pointT *point, facetT *facetA, realT *distp);
void 	qh_gausselim(realT **rows, int numrow, int numcol, boolT *sign, boolT *nearzero);
realT   qh_getangle(pointT *vect1, pointT *vect2);
pointT *qh_getcenter(setT *vertices);
pointT *qh_getcentrum(facetT *facet);
boolT   qh_gram_schmidt(int dim, realT **rows);
boolT   qh_inthresholds (coordT *normal, realT *angle);
realT  *qh_maxabsval (realT *normal, int dim);
setT   *qh_maxmin(pointT *points, int numpoints, int dimension);
void    qh_maxsimplex (int dim, setT *maxpoints, pointT *points, int numpoints, setT **simplex);
realT   qh_minabsval (realT *normal, int dim);
void    qh_normalize (coordT *normal, int dim, boolT toporient);
boolT   qh_orientoutside (facetT *facet);
coordT  qh_pointdist(pointT *point1, pointT *point2, int dim);
void    qh_printmatrix (FILE *fp, const char *string, realT **rows, int numrow, int numcol);
void    qh_printpoints (FILE *fp, const char *string, setT *points);
void    qh_projectinput ();
pointT *qh_projectpoint(pointT *point, facetT *facet, realT dist);
void 	qh_projectpoints (signed char *project, int n, realT *points, 
             int numpoints, int dim, realT *newpoints, int newdim);
realT   qh_randomfactor ();
void    qh_randommatrix (realT *buffer, int dim, realT **row);
void    qh_rotateinput (realT **rows);
void    qh_rotatepoints (realT *points, int numpoints, int dim, realT **rows);
void    qh_scaleinput ();
void 	qh_scalepoints (pointT *points, int numpoints, int dim,
  		realT *newlows, realT *newhighs);
void    qh_setfacetplane(facetT *newfacets);
void 	qh_sethyperplane_det (int dim, coordT **rows, coordT *point0, 
         	boolT toporient, coordT *normal, realT *offset);
void 	qh_sethyperplane_gauss (int dim, coordT **rows, pointT *point0, 
	     boolT toporient, coordT *normal, coordT *offset, boolT *nearzero);
pointT *qh_voronoi_center (int dim, setT *points);

/**** end geom.h ****/


/**************************************************************************/
/************** definitions and signatures from merge.h *******************/
/**************************************************************************/

/* ============ -constants- ====================
-BESTcentrum     if > dim+n vertices, findbestneighbor tests centrums (faster)
                 else, findbestneighbor test all vertices (much better merges)

-BESTnonconvex   if > dim*n neighbors, findbestneighbor tests nonconvex ridges
                   needed because findbestneighbor is slow for large facets
		   
-MAXnewmerges    if >n newmerges, merge_nonconvex calls reducevertices_centrums
                   needed because postmerge can merge many facets at once

-MAXnewcentrum   if <= dim+n vertices (n approximates the number of merges),
                    reset the centrum in reducevertices_centrum 
                  needed to reduce cost and because centrums may move
		        too much if many vertices in high-d

*/
#define qh_BESTcentrum 20   /* findbestneighbor tests centrum or vertices */
#define qh_BESTnonconvex 5    /*findbestneighbor only tests nonconvex */
#define qh_MAXnewmerges 2   /*merge_nonconvex calls reducevertices_centrums*/
#define qh_MAXnewcentrum 5    /*reducevertices_centrums resets centrum */

#define qh_ANGLEredundant 6.0 /* angle for redundant merge in mergeT */
#define qh_ANGLEdegen     5.0 /* angle for degenerate facet in mergeT */
#define qh_ANGLEconcave  1.5  /* [2,4] for angle of concave facets in mergeT,
                                 may be <2 or >4 due to roundoff */

/* ============ -structures- ====================
*/

/* ----------------------------------------------
-mergeT- structure used to merge facets
*/

typedef struct mergeT mergeT;

struct mergeT {
  realT   angle;          /* angle between normals of facet1 and facet2 */
  facetT *facet1; 
  facetT *facet2;
  flagT   mergeridge:1;   /* set if merge due to qh_MERGEridge */
  flagT   newmerge:1;     /* set if new merge, for forcedmerges() */
  flagT   anglecoplanar:1; /* set if merge due to qh cos_max */
};


/* =========== -macros- =========================
-FOREACHmerge-  if qh_mergefacet() then must restart since facet_mergeset
                may change.
*/
#define FOREACHmerge_(merges) FOREACHsetelement_(mergeT, merges, merge)

/* ======= -functions and procedures- =========== 

	top-level merge functions
-merge_nonconvex   merges all nonconvex facets
-flippedmerges	   merge flipped facets into best neighbor
-forcedmerges	   merge across duplicated ridges and mutually flipped facets
-tracemerging      print trace message during post merging

	mergeset functions for identifying merges
-getmergeset_initial  initial mergeset for facets
-getmergeset	   returns facet_mergeset of facet-neighbor pairs to be merged
-degen_redundant_neighbors  append degen. and redundant neighbors to facet_mergeset
-test_appendmerge   facet/neighbor and appends to mergeset if nonconvex
-appendmergeset	   appends an entry to facet_mergeset, angle is optional
-facetdegen	   true if facet already in mergeset as a degenerate

	functions for determining the best merge
-findbest_test	   test neighbor for findbestneighbor()
-findbestneighbor  finds best neighbor (least dist) of a facet for merging
-getdistance	   returns the max and min distance of any vertex from neighbor

	functions for merging facets
-merge_degenredundant  merge degenerate and redundant facets
-mergefacet	   merges facet1 into facet2
-makeridges	   creates explicit ridges between simplicial facets
-mergeneighbors	   merges the neighbors of facet1 into facet2
-mergeridges	   merges the ridge set of facet1 into facet2
-mergevertex_neighbors merge the vertex neighbors of facet1 to facet2
-mergevertices	   merges the vertex set of facet1 into facet2
-mergevertices2d   merges vertices1 into vertices2 in 2-d case

	functions for renaming a vertex
-reducevertices_centrums reduce vertex sets and reset centrums
-rename_sharedvertex  detect and rename if shared vertex in facet
-redundant_vertex   returns true if detect and rename redundant vertex
-renamevertex	   renames oldvertex as newvertex in ridges 
-renameridgevertex renames oldvertex as newvertex in ridge
-maydropneighbor   drop neighbor relationship if no ridge between facet and neighbor
-remove_extravertices remove extra vertices in non-simplicial facets
-copynonconvex- copy non-convex flag to all ridges between same neighbors

	functions for identifying vertices for renaming
-find_newvertex    locate new vertex for renaming old vertex
-neighbor_intersections	 return intersection for vertex->neighbors
-vertexridges	   return temporary set of ridges adjacent to a vertex
-vertexridges_facet add adjacent ridges for vertex in facet
-hashridge	   add ridge to hashtable without oldvertex
-hashridge_find	   returns matching ridge in hashtable without oldvertex

	check functions
-checkridge_boundary  checks that ridges of a facet are boundaryless
*/

/*---------- -prototypes in alphabetical order -----------*/

mergeT *qh_appendmergeset(facetT *facet, facetT *neighbor, realT *angle);
void 	qh_checkridge_boundary(facetT *facet);
void    qh_copynonconvex (ridgeT *atridge);
void 	qh_degen_redundant_neighbors (facetT *facet);
boolT   qh_facetdegen (facetT *facet);
vertexT *qh_find_newvertex (vertexT *oldvertex, setT *vertices, setT *ridges);
void    qh_findbest_test (boolT testcentrum, facetT *facet, facetT *neighbor,
           facetT **bestfacet, realT *distp, realT *mindistp, realT *maxdistp);
facetT *qh_findbestneighbor(facetT *facet, realT *distp, realT *mindistp, realT *maxdistp);
void 	qh_flippedmerges(facetT *facetlist);
void 	qh_forcedmerges(facetT *facetlist);
realT   qh_getdistance(facetT *facet, facetT *neighbor, realT *mindist, realT *maxdist);
void	qh_getmergeset(facetT *facetlist);
void 	qh_getmergeset_initial (facetT *facetlist);
void    qh_hashridge (setT *hashtable, int hashsize, ridgeT *ridge, vertexT *oldvertex);
ridgeT *qh_hashridge_find (setT *hashtable, int hashsize, ridgeT *ridge, 
              vertexT *vertex, vertexT *oldvertex, int *hashslot);
void 	qh_makeridges(facetT *facet);
void    qh_maydropneighbor (facetT *facet);
boolT   qh_merge_degenredundant (facetT *facet1, facetT *facet2, realT *angle);
void 	qh_merge_nonconvex( /*newfacet_list*/);
void 	qh_mergefacet(facetT *facet1, facetT *facet2, realT *mindist, realT *maxdist, realT *angle);
void 	qh_mergeneighbors(facetT *facet1, facetT *facet2);
void 	qh_mergeridges(facetT *facet1, facetT *facet2);
void    qh_mergevertex_neighbors(facetT *facet1, facetT *facet2);
void	qh_mergevertices(setT *vertices1, setT **vertices);
void 	qh_mergevertices2d(setT *vertices1, setT *vertices2);
setT   *qh_neighbor_intersections (vertexT *vertex);
boolT   qh_reducevertices_centrums ();
vertexT *qh_redundant_vertex (vertexT *vertex);
boolT   qh_remove_extravertices (facetT *facet);
vertexT *qh_rename_sharedvertex (vertexT *vertex, facetT *facet);
void	qh_renameridgevertex(ridgeT *ridge, vertexT *oldvertex, vertexT *newvertex);
void    qh_renamevertex(vertexT *oldvertex, vertexT *newvertex, setT *ridges,
			facetT *oldfacet, facetT *neighborA);
boolT 	qh_test_appendmerge (facetT *facet, facetT *neighbor);
void    qh_tracemerging (const char *string);
setT   *qh_vertexridges (vertexT *vertex);
void    qh_vertexridges_facet (vertexT *vertex, facetT *facet, setT **ridges);

/**** end merge.h ****/


/**************************************************************************/
/************** definitions and signatures from poly.h ********************/
/**************************************************************************/

/*-----------------------------------------------
-constants-

	for calling checkconvex()
-ALGORITHMfault   flag for checkconvex for error during buildhull
-DATAfault        flag for checkconvex for error during initialhull

	set by matchneighbor, used by matchmatch and forcedmerges
-DUPLICATEridge   flag in facet->neighbors to indicate duplicated ridge
-MERGEridge       flag in facet->neighbors to indicate merged ridge
*/

#define qh_ALGORITHMfault 0
#define qh_DATAfault 1

#define qh_DUPLICATEridge (facetT *) 1
#define qh_MERGEridge (facetT *) 2


/* ============ -structures- ====================
*/

/* ----------------------------------------------
-hashentryT- hash table entry for matching sub-ridges in makecone()
*/

typedef struct hashentryT hashentryT;

struct hashentryT {
  facetT     *facet;        /* facet */
  hashentryT *next;         /* next hash table entry for this bucket */
  unsigned    skipindex;    /* skipped vertex in facet, for orientation */
};

/* =========== -macros- ========================= 
*/

/* ----------------------------------------------
-FOREACH... and FORALL... -- standard for loops
  see qhull.h for notes
*/
#define FORALLfacet_(facetlist) if (facetlist) for(facet=(facetlist);facet && facet->next;facet=facet->next)
#define FORALLnew_facets for(newfacet=qh newfacet_list;newfacet && newfacet->next;newfacet=newfacet->next)
#define FORALLvertex_(vertexlist) for (vertex=(vertexlist);vertex && vertex->next;vertex= vertex->next)
#define FORALLvisible_facets for (visible=qh visible_list; visible && visible->visible; visible= visible->next)

#define FOREACHentry_(entries) FOREACHsetelement_(hashentryT, entries, entry)
#define FOREACHvisible_(facets) FOREACHsetelement_(facetT, facets, visible)
#define FOREACHnewfacet_(facets) FOREACHsetelement_(facetT, facets, newfacet)
#define FOREACHvertexA_(vertices) FOREACHsetelement_(vertexT, vertices, vertexA)
#define FOREACHvertexreverse12_(vertices) FOREACHsetelementreverse12_(vertexT, vertices, vertex)

/* ======= -functions =========== 

see poly.c for definitions

	Facetlist functions
-appendfacet	    appends facet to end of qh facet_list,
-prependfacet	    prepends facet to start of facetlist
-removefacet	    unlinks facet from qh facet_list,
-clearvisible       clear facets from visible list

	Facet functions
-createsimplex	    creates a simplex of facets from a set of vertices
-makenewfacet	    creates a toporient? facet from vertices and apex
-makenewfacets	    make new facets from point, horizon facets, and visible facets
-makenew_nonsimplicial make new facets for ridges of visible facets
-makenew_simplicial make new facets for horizon neighbors
-attachnewfacets    attach new facets in qh newfacet_list to the horizon
-makeadjacencies    make adjacencies for non-simplicial facets

	Vertex, ridge, and point functions
-appendvertex	    appends vertex to end of qh vertex_list,
-removevertex	    unlinks vertex from qh vertex_list,
-clearnewvertices   clear vertices from qh newvertex_list
-point              return point for a point id, or NULL if unknown
-pointid            return id for a point, or -1 if not known
-vertexintersect    intersects two vertex sets
-vertexintersect_new intersects two vertex sets
-facetintersect	    intersect simplicial facets
-isvertex	    true if point is in the vertex set
-vertexsubset	    returns True if vertexsetA is a subset of vertexsetB
-nextridge3d	    iterate each ridge and vertex for a 3d facet
-facet3vertex	    return oriented vertex set for 3-d facet
-vertexneighhbors   for each vertex in hull, determine facet neighbors
-pointfacet	    return temporary set of facets indexed by point id
-pointvertex	    return temporary set of vertices indexed by point id

	Hashtable functions
-newhashtable	    allocates a new qh hash_table
-gethash	    return hashvalue for a set with firstindex
-matchnewfacets	    match newfacets in to their newfacet neighbors
-matchneighbor      try to match subridge of newfacet with a neighbor
-matchduplicate     try to match an unmatched duplicate ridge
-matchmatch         try to match duplicate matching pair and newfacet
-matchvertices	    tests whether a facet and hashentry match at a ridge
-printhashtable		print hash table

	Allocation and deallocation functions
-newfacet	    creates and allocates space for a facet
-newridge	    creates and allocates space for a ridge
-newvertex	    creates and allocates space for a vertex
-deletevisible	    delete visible facets and related structures
-delfacet	    frees up the memory occupied by a facet
-delridge	    deletes ridge from data structures it belongs to and frees up the
-delvertex	    deletes vertex and its memory
-clearcenters       clear old data from facet->center
	
	Check functions
-check_bestdist	    check that points are not outside their best facet
-check_maxout       updates max_outside, checks all points against bestfacet
-check_output	    performs the checks at the end of qhull algorithm
-check_point        check that point is not outside facet
-check_points	    checks that all points are inside all facets
-checkconvex	    check that each ridge in facetlist is convex
-checkfacet	    checks for consistency errors in facet
-checkflipped	    checks facet orientation to the interior point
-checkflipped_all   checks facet orientation for a facet list
-checkpolygon	    checks the correctness of the structure
-checkvertex        check vertex for consistency
-printlists         print out facet list for debugging
*/

/*---------- -prototypes in alphabetical order -----------*/

void    qh_appendfacet(facetT *facet);
void    qh_appendvertex(vertexT *vertex);
void 	qh_attachnewfacets ();
void 	qh_check_bestdist ();
void    qh_check_maxout ();
void    qh_check_output ();
void    qh_check_point (pointT *point, facetT *facet, realT *maxoutside, facetT **errfacet1, facetT **errfacet2);
void   	qh_check_points();
void 	qh_checkconvex(facetT *facetlist, int fault);
void    qh_checkfacet(facetT *facet, boolT newmerge, boolT *waserrorp);
boolT   qh_checkflipped (facetT *facet, realT *dist, boolT allerror);
void 	qh_checkflipped_all (facetT *facetlist);
void 	qh_checkpolygon(facetT *facetlist);
void    qh_checkvertex (vertexT *vertex);
void 	qh_clearcenters (int type);
void    qh_clearvisible ( /*qh visible_list*/);
void    qh_clearnewvertices ( /*qh newvertex_list*/);
void 	qh_createsimplex(setT *vertices);
int 	qh_degeneratematch(facetT *facet, setT *vertices, int skipindex);
void 	qh_deletevisible(/*qh visible_list, qh horizon_list*/);
void	qh_delfacet(facetT *facet);
void 	qh_delridge(ridgeT *ridge);
void    qh_delvertex (vertexT *vertex);
setT   *qh_facet3vertex (facetT *facet);
setT   *qh_facetintersect (facetT *facetA, facetT *facetB, int *skipAp,int *skipBp, int extra);
unsigned qh_gethash (int hashsize, setT *set, int size, int firstindex, void *skipelem);
vertexT  *qh_isvertex (pointT *point, setT *vertices);
facetT *qh_makenewfacet(setT *vertices, boolT toporient, facetT *facet);
vertexT *qh_makenewfacets (pointT *point /*horizon_list, visible_list*/);
facetT *qh_makenew_nonsimplicial (facetT *visible, vertexT *apex, int *numnew);
facetT *qh_makenew_simplicial (facetT *visible, vertexT *apex, int *numnew);
boolT   qh_matchduplicate (facetT *facet, int skip,
			   facetT *newfacet, int newskip, boolT keepfound);
boolT   qh_matchmatch (facetT *facet, int skip, facetT *matchfacet,
	               facetT *newfacet, int newskip, boolT ismatch,
		       boolT *keepfound, boolT *newmatched);
void    qh_matchneighbor (facetT *newfacet, int newskip, int hashsize,
			  int *hashcount, boolT matchall);
void	qh_matchnewfacets ();
boolT   qh_matchvertices (int firstindex, setT *verticesA, int skipA, 
			  setT *verticesB, int *skipB, boolT *same);
int 	qh_newhashtable(int newsize);
facetT *qh_newfacet();
ridgeT *qh_newridge();
vertexT *qh_newvertex(pointT *point);
ridgeT *qh_nextridge3d (ridgeT *atridge, facetT *facet, vertexT **vertexp);
pointT *qh_point (int id);
void 	qh_point_add (setT *set, pointT *point, void *elem);
setT   *qh_pointfacet ( /*qh facet_list*/);
int     qh_pointid (pointT *point);
setT   *qh_pointvertex ( /*qh facet_list*/);
void 	qh_prependfacet(facetT *facet, facetT **facetlist);
void	qh_printhashtable(FILE *fp);
void    qh_printlists ();
void 	qh_removefacet(facetT *facet);
void 	qh_removevertex(vertexT *vertex);
void    qh_vertexintersect(setT **vertexsetA,setT *vertexsetB);
setT   *qh_vertexintersect_new(setT *vertexsetA,setT *vertexsetB);
void    qh_vertexneighbors ( /*qh facet_list*/);
boolT 	qh_vertexsubset(setT *vertexsetA, setT *vertexsetB);


/**************************************************************************/
/************** definitions and signatures from stat.h ********************/
/**************************************************************************/
#ifndef qh_KEEPstatistics
#define qh_KEEPstatistics 1
#endif

/*----------------------------------------------------
-statistics, Zxxx for integers, Wxxx for reals

can pick up all statistics by:
    grep '[zw].*_[(][ZW]' *.c >z.x
    remove trailers with query-replace-regexp [,)].*
    remove leaders with  query-replace-regexp [ ^I]+  (
*/

#if qh_KEEPstatistics
enum statistics {     /* alphabetical after Z/W */
    Zacoplanar,
    Wacoplanarmax,
    Wacoplanartot,
    Zangle,
    Wangle,
    Wanglemax,
    Wanglemin,
    Zangletests,
    Zavoidold,
    Wavoidoldmax,
    Wavoidoldtot,
    Zback0,
    Zbestcentrum,
    Zbestdist,
    Zcentrumtests,
    Zcomputefurthest,
    Zconcave,
    Wconcavemax,
    Wconcavetot,
    Zconcaveridges,
    Zconcaveridge,
    Zcoplanar,
    Wcoplanarmax,
    Wcoplanartot,
    Zcoplanarangle,
    Zcoplanarcentrum,
    Zcoplanarhorizon,
    Zcoplanarinside,
    Zcoplanarpart,
    Zcoplanarridges,
    Wcpu,
    Zdegen,
    Wdegenmax,
    Wdegentot,
    Zdegenvertex,
    Zdelfacetdup, 
    Zdelridge,
    Zdetsimplex,
    Zdistcheck,
    Zdistconvex,
    Zdistgood,
    Zdistio,
    Zdistplane,
    Zdiststat,
    Zdoc1,
    Zdoc2,
    Zdoc3,
    Zdoc4,
    Zdoc5,
    Zdoc6,
    Zdoc7,
    Zdoc8,
    Zdoc9,
    Zdoc10,
    Zdropdegen,
    Zdropneighbor,
    Zduplicate,
    Wduplicatemax,
    Wduplicatetot,
    Zdupridge,
    Zfindvertex,
    Zfindfail,
    Zflipped, 
    Wflippedmax, 
    Wflippedtot, 
    Zflippedfacets,
    Zgauss0,
    Zgoodfacet,
    Zhashlookup,
    Zhashridge,
    Zhashridgetest,
    Zhashtests,
    Zinsidevisible,
    Zintersect,
    Zintersectfail,
    Zintersectmax,
    Zintersectnum,
    Zintersecttot,
    Zmaxneighbors,
    Wmaxoutside,
    Zmaxridges,
    Zmaxvertices,
    Zmaxvneighbors,
    Zmemfacets,
    Zmempoints,
    Zmemridges,
    Zmemvertices,
    Zmergeflip,
    Zmergeflipdup,
    Zmergehorizon,
    Zmergeintohorizon,
    Zmergenew,
    Zmergevertex,
    Wmindenom,
    Wminvertex,
    Zmultiflip,
    Zmultimatch,
    Znearlysingular,
    Zneighbor,
    Wnewbalance,
    Wnewbalance2,
    Znewvertex,
    Wnewvertex,
    Wnewvertexmax,
    Znotgood,
    Znotgoodnew,
    Znotmax,
    Znumfacets,
    Znumneighbors,
    Znumridges,
    Znumvertices,
    Znumvisibility,
    Znumvneighbors,
    Zpartcoplanar,
    Zpartneighbor,
    Zpartinside,
    Zpartition, 
    Zpartitionall,
    Zpbalance,
    Wpbalance,
    Wpbalance2, 
    Zpostfacets, 
    Zprocessed,
    Zremvertex,
    Zremvertexdel,
    Zrenameall,
    Zrenamepinch,
    Zrenameshare,
    Zsamevertices,
    Zsearchpoints,
    Zsetplane,
    Ztothorizon,
    Ztotmerge,
    Ztotpartcoplanar,
    Ztotpartition,
    Ztotridges,
    Ztotverify,
    Ztotvertices,
    Ztotvisible,
    Zverifypart,
    Wvertexmax,
    Wvertexmin,
    Zvertexridge,
    Zvertexridgetot,
    Zvertexridgemax,
    Zvertices,
    ZEND};
#else
enum statistics {     /* for zzdef etc. macros */
  Zback0,
  Zbestdist,
  Zcentrumtests,
  Zconcaveridges,
  Zcoplanarhorizon,
  Zcoplanarpart,
  Zcoplanarridges,
  Zdistcheck,
  Zdistconvex,
  Zdoc1,
  Zdoc3,
  Zflippedfacets,
  Zgauss0,
  Zmultiflip,
  Zmultimatch,
  Znearlysingular,
  Znumvisibility,
  Zpartcoplanar,
  Zpartition,
  Zpartitionall,
  Zprocessed,
  Zsetplane,
  Ztotmerge,
    ZEND};
#endif

/* ------------ -ztypes- ---------------------
the type of a statistic sets its initial value.  The type should
be the same as the macro for collecting the statistic
*/
enum ztypes {zdoc,zinc,zadd,zmax,zmin,ZTYPEreal,wadd,wmax,wmin,ZTYPEend};

/*------------ -macros -------------
macros:
  zdef_(type, name, doc, -1)	define a statistic (assumes 'qhstat next= 0;')
  zdef_(type, name, doc, count)	   printed as name/count
  zinc_(name)                   integer statistic is count
  zadd/wadd_(name, value)       integer or real statistic is total value
  zmax/wmax_(name, value)	integer or real statistic is max value
  zmin/wmin_(name, value)	integer or real statistic is min value
  zval/wval_(name)		set or return value of statistic
*/

#define MAYdebugx  /* maydebug() is called frequently to trap an error */
#define zzinc_(id) {MAYdebugx; qhstat stats[id].i++;}
#define zzadd_(id, val) {MAYdebugx; qhstat stats[id].i += (val);}
#define zzval_(id) ((qhstat stats[id]).i)
#define wwval_(id) ((qhstat stats[id]).r)
#define zzdef_(stype,name,string,cnt) qhstat id[qhstat next++]=name; \
  qhstat doc[name]= const_cast<char *>(string); qhstat count[name]= cnt; qhstat type[name]= stype

#if qh_KEEPstatistics
#define zinc_(id) {MAYdebugx; qhstat stats[id].i++;}
#define zadd_(id, val) {MAYdebugx; qhstat stats[id].i += (val);}
#define wadd_(id, val) {MAYdebugx; qhstat stats[id].r += (val);}
#define zmax_(id, val) {MAYdebugx; maximize_(qhstat stats[id].i,(val));}
#define wmax_(id, val) {MAYdebugx; maximize_(qhstat stats[id].r,(val));}
#define zmin_(id, val) {MAYdebugx; minimize_(qhstat stats[id].i,(val));}
#define wmin_(id, val) {MAYdebugx; minimize_(qhstat stats[id].r,(val));}
#define zval_(id) ((qhstat stats[id]).i)
#define wval_(id) ((qhstat stats[id]).r)

#define zdef_(stype,name,string,cnt) qhstat id[qhstat next++]=name; \
  qhstat doc[name]= const_cast<char *>(string); qhstat count[name]= cnt; qhstat type[name]= stype


#else  /* !qh_KEEPstatistics */
#define zinc_(id)
#define zadd_(id, val)
#define wadd_(id, val)
#define zmax_(id, val)
#define wmax_(id, val)
#define zmin_(id, val)
#define wmin_(id, val)
#define zval_(id) qhstat tempi
#define wval_(id) qhstat tempr
#define zdef_(type,name,doc,count)
#define ZMAXlevel 1
#endif
  
/* -typedef and extern-  types are defined below */

typedef struct qhstatT qhstatT;     /* global data structure for statistics */
typedef union intrealT intrealT;    /* union of int and realT */

#if qh_QHpointer
#define qhstat qh_qhstat->
extern qhstatT *qh_qhstat;  /* allocated in stat.c */
#else
#define qhstat qh_qhstat.
extern qhstatT qh_qhstat;  /* allocated in stat.c */
#endif

/*-------------------------------------------
-intrealT-  union of integer and real, used for statistics
*/
union intrealT {
    int i;
    realT r;
};

/*--------------------------------------------
-qhstatT- global data structure for statistics
*/
struct qhstatT {  
  intrealT stats[ZEND];  /* integer and real statistics */
  unsigned char id[ZEND];  /* id's in print order */
  char *doc[ZEND];     /* array of documentation strings */
  short int count[ZEND];   /* -1 if none, else index of count to use */
  char type[ZEND];      /* type, see ztypes above */
  char printed[ZEND];   /* true, if statistic has been printed */
  intrealT init[ZTYPEend];  /* initial values by types, set initstatistics */

  int next;           /* next index for zdef_ */
  int precision;      /* index for precision problems */
  int tempi;
  realT tempr;
};

/* ========== -functions- ===========
   see also qhull.h

-freestatistics free memory used for statistics
-newstats	returns True if statistics for zdoc
-nostatistic    true if no statistic to print
-stddev		compute the standard deviation and average from statistics
*/
  
void    qh_collectstatistics ();
void	qh_freestatistics ();
void    qh_initstatistics ();
boolT 	qh_newstats (int index, int *nextindex);
boolT 	qh_nostatistic (int i);
void    qh_printallstatistics (FILE *fp, const char *string);
void    qh_printstatistics (FILE *fp, const char *string);
void  	qh_printstatlevel (FILE *fp, int id, int start);
void  	qh_printstats (FILE *fp, int index, int *nextindex);
realT   qh_stddev (int num, realT tot, realT tot2, realT *ave);

/**** end stat.h ****/

/* ======= -macros- =========== */

/*-----------------------------------------------
-traceN((fp.ferr, "format\n", vars));  calls fprintf if IStracing >= N
  removing tracing reduces code size but doesn't change execution speed
*/
#define trace0(args) {if (qh IStracing) fprintf args;}
#define trace1(args) {if (qh IStracing >= 1) fprintf args;}
#define trace2(args) {if (qh IStracing >= 2) fprintf args;}
#define trace3(args) {if (qh IStracing >= 3) fprintf args;}
#define trace4(args) {if (qh IStracing >= 4) fprintf args;}
#define trace5(args) {if (qh IStracing >= 5) fprintf args;}


/* ======= -functions =========== 

see corresponding .c file for definitions

	Qhull functions (qhull.c)
-qhull		construct the convex hull of a set of points
-addpoint       add point to hull above a facet
-buildhull	constructs a hull by adding points one at a time
-buildtracing   for tracing execution of buildhull
-errexit2	return exitcode to system after an error for two facets
-findgood       identify good facets for qh ONLYgood
-findgood_all   identify good facets for qh PRINTgood
-findhorizon	find the horizon and visible facets for a point
-initialhull	construct the initial hull as a simplex of vertices
-initialvertices return non-singular set of initial vertices
-nextfurthest   returns next furthest point for processing
-partitionall	partitions all points into the outsidesets of facets
-partitioncoplanar partition coplanar point to a facet
-partitionpoint partitions a point as inside, coplanar or outside a facet
-partitionvisible partitions points in visible_list to newfacet_list

	Global.c internal functions (others in qhull.h)
-freebuffers	free up global memory buffers 
-initbuffers	initialize global memory buffers
-strtod/tol     duplicates strtod/tol
*/

/***** -qhull.c prototypes (alphabetical after qhull) ********************/

void 	qh_qhull ();
boolT   qh_addpoint (pointT *furthest, facetT *facet, boolT checkdist);
void 	qh_buildhull();
void    qh_buildtracing (pointT *furthest, facetT *facet);
void 	qh_errexit2(int exitcode, facetT *facet, facetT *otherfacet);
int 	qh_findgood (facetT *facetlist, int goodhorizon);
void 	qh_findgood_all (facetT *facetlist);
void    qh_findhorizon(pointT *point, facetT *facet, int *goodvisible,int *goodhorizon);
void 	qh_initialhull(setT *vertices);
setT   *qh_initialvertices(int dim, setT *maxpoints, pointT *points, int numpoints);
pointT *qh_nextfurthest (facetT **visible);
void 	qh_partitionall(setT *vertices, pointT *points,int npoints);
void    qh_partitioncoplanar (pointT *point, facetT *facet, realT *dist);
void    qh_partitionpoint (pointT *point, facetT *facet);
void 	qh_partitionvisible(boolT allpoints, int *numpoints);

/***** -global.c internal prototypes (alphabetical) ***********************/

void 	qh_freebuffers ();
void    qh_initbuffers (coordT *points, int numpoints, int dim, boolT ismalloc);
int     qh_strtol (const char *s, char **endp);
double  qh_strtod (const char *s, char **endp);


#endif /* qhDEFqhull */



