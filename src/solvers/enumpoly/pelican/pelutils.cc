/*
  This file contains the implementation information that had been in globals.h,
and also the various implementation files that formerly resided in the Utils
subdirectory of the Pelican distribution.  
*/

#include "pelutils.h"


/**************************************************************************/
/*********************** implementations from Mem.c ***********************/
/**************************************************************************/

/*
   **  Mem.c               Created 1-27-95                Birk Huber
   **
   **  Memory Management for Pelican Shell. 
   **  
   **  Maintains storage for the basic unit of storage from which
   **  lists are built.
   ** 
   **  A node consists of left,and right (values,type) combinations, and
   **  a boolean for garbage collection. The values are often pointers to
   **  other structures wich must be freed when nodes are freed.
   **
   ** The Free_Stack is used to store a list of (known) free nodes, 
   ** The Local_Stack points to a list of pointers to nodes
   **                           which are known to be in use.
   **               
   **  Any time a new_node is requested and the Free_Stack is 
   **  empty garbage collection is initiated. Garbage collection 
   **  proceeds by  first marking all nodes accessable from the 
   **  Local_Stack and then putting the rest on the free list.
   **
   **  Any time garbage collection fails node_more_store() is called
   **  to add another block of nodes to the pool of available nodes.
   **
 */

#define BLOCKSIZE 10000
#define YES 0
#define NO 1

#define Node_LT(n) ((n->LT))
#define Node_L(n) ((n->L))
#define Node_RT(n) ((n->RT))
#define Node_R(n) ((n->R))
#define Node_Marked(n) ((n)->Mark)
#define Node_Unset_Mark(n)((n)->Mark=NO)
#define Node_Set_Mark(n)((n)->Mark=YES)

static int node_gc();
/* static int mark(); */
static int node_more_store();
static node node_free(node N);




struct node_t {
    int LT, RT, Mark;
    union {
	void *ptr;
	int ival;
	double dval;
    } L, R;
};

/* 
   ** 
 */
typedef struct node_block_t *node_block;
struct node_block_t {
    struct node_t store[BLOCKSIZE];
    struct node_block_t *next;
};



static int N_MALLOC = 0;	/*number of mallocs called from module */
static int N_FREE = 0;		/*number of frees called from this module */
static node_block Node_Store = nullptr;	/*beginning of node storage */
static node Free_List = nullptr;	/*top of free node list */
static local_v Locals_Stack = nullptr;	/*stack of  declared pointers 
					   into node storage */



node node_set_ptr(node N, void *v, int tp, int side)
{
    if (N == nullptr)
	bad_error("null node to node_set_ptr");
    if (side == LEFT) {
	N->L.ptr = v;
	N->LT = tp;
    } else if (side == RIGHT) {
	N->R.ptr = v;
	N->RT = tp;
    } else
	bad_error("bad side given in node_set_ptr");
    return N;
}

node node_set_int(node N, int v, int tp, int side)
{
    if (N == nullptr)
	bad_error("null node to node_set_int");
    if (side == LEFT) {
	N->L.ival = v;
	N->LT = tp;
    } else if (side == RIGHT) {
	N->R.ival = v;
	N->RT = tp;
    } else
	bad_error("bad side given in node_set_int");
    return N;
}


node node_set_double(node N, double v, int tp, int side)
{
    if (N == nullptr)
	bad_error("null node to node_set_double");
    if (side == LEFT) {
	N->L.dval = v;
	N->LT = tp;
    } else if (side == RIGHT) {
	N->R.dval = v;
	N->RT = tp;
    } else
	bad_error("bad side given in node_set_double");
    return N;
}



int node_get_int(node N, int side)
{
    if (N == nullptr)
	bad_error("null node to node_get_int");
    if (side == LEFT)
	return N->L.ival;
    else if (side == RIGHT)
	return N->R.ival;
    bad_error("bad side given in node_get_int");
    return 0;
}
double node_get_double(node N, int side)
{
    if (N == nullptr)
	bad_error("null node to node_get_ptr");
    if (side == LEFT)
	return N->L.dval;
    else if (side == RIGHT)
	return N->R.dval;
    bad_error("bad side given in node_get_double");
    return 0;
}

void *node_get_ptr(node N, int side)
{
    if (N == nullptr)
	bad_error("null node to node_get_ptr");
    if (side == LEFT)
	return N->L.ptr;
    else if (side == RIGHT)
	return N->R.ptr;
    bad_error("bad side given in node_get_ptr");
    return nullptr;
}

int node_set_type(node N, int Tp, int side)
{
    if (N == nullptr)
	bad_error("null node to node_get_type");
    if (side == LEFT)
	return N->LT;
    else if (side == RIGHT)
	return N->RT=Tp;
    bad_error("bad side given in node_get_type");
    return 0;
}
int node_get_type(node N, int side)
{
    if (N == nullptr)
	bad_error("null node to node_get_type");
    if (side == LEFT)
	return N->LT;
    else if (side == RIGHT)
	return N->RT;
    bad_error("bad side given in node_get_type");
    return 0;
}


node Cons(node N1, node N2)
{
    node Res;
    LOCS(2);
    PUSH_LOC(N1);
    PUSH_LOC(N2);
    Res = node_new();
    node_set_ptr(Res, (void *) N1, NODE, LEFT);
    node_set_ptr(Res, (void *) N2, NODE, RIGHT);
    POP_LOCS();
    return Res;
}

node Car(node N1)
{
    if (N1 == nullptr)
        return nullptr;
    return (node) node_get_ptr(N1, LEFT);
}

node Cdr(node N1)
{
    if (N1 == nullptr)
        return nullptr;
    return (node) node_get_ptr(N1, RIGHT);
}

int node_atomp(node N1)
{
    
    if ((N1!=nullptr) && (node_get_type(N1, LEFT)) == NODE)
        return FALSE;
    else
        return TRUE;
}

int node_nullp(node N1)
{
    if (N1 == nullptr)
        return TRUE;
    else
        return FALSE;
}






char *mem_strdup(char *str){
  /*   char *strdup(char *); CAN'T DECLARE BUILTINS UNDER C++ */
  N_MALLOC++;
  return strdup(str);
}
void *mem_malloc(int sz)
{
    N_MALLOC++;
    return (void *) malloc(sz);
}
void mem_free(void *ptr)
{
    N_FREE++;
    free(ptr);
}

static node node_free(node N)
{
    Node_LT(N) = NODE;
    Node_L(N).ptr = nullptr;
    Node_RT(N) = NODE;
    Node_R(N).ptr = Free_List;
    Free_List = N;
    return N;
}

/* 
   **  storage is stored in blocks of size BLOCKSIZEg. whenever
   **  node_new is called and garbage collection fails to produce 
   **  new nodes (because there is no more free storage) 
   **  node_more_store() is called. 
   **  It is responcible for 
   **     a) claiming another block of nodes with malloc
   **     b) adding this block to the storage list
   **     c) putting all the new blocks on the free list.
   **  finally if this is the first block to be created the locals stack
   **  must be initialized.
 */
static int node_more_store()
{
    int i;
    node_block next_node = nullptr;
  
    next_node = (node_block) malloc(sizeof(struct node_block_t));
    if (next_node == nullptr) {
#ifdef LOG_PRINT
	fprintf(stderr, "malloc failing in node_more_store\n")
#endif
;
	return FALSE;
    }
    next_node->next = Node_Store;
    Node_Store = next_node;
    for (i = 0; i < BLOCKSIZE; i++)
	Node_Unset_Mark(node_free(Node_Store->store + i));

    return TRUE;
}


int node_init_store()
{
    if (node_more_store() == FALSE)
	bad_error("node_init_store failing");
    return TRUE;
}



void node_free_store()
{
    int i;
    node_block junk;
    printf("in free_store()\n");

    while (Node_Store != nullptr) {
	/* free any space held by elements in the block */
	for (i = 0; i < BLOCKSIZE; i++)
	    atom_free(Node_Store->store + i);
	Node_Store = (junk = Node_Store)->next;
	free((char *) junk);
    }
    printf("N_malloc = %d,  N_free=%d;=\n \n", N_MALLOC, N_FREE);
}


/* 
   ** Allocate nodes from free list-- If no nodes available initiate
   ** a garbage collection.  Note: routines which call new-node directly
   ** must protect their arguments, and localvariables themselve rather
   ** than make any assumptions about wheather they have been saved.
 */

node node_new()
{
    node res;
    if (Free_List != nullptr) {
	res = Free_List;
	Free_List = (node) node_get_ptr(Free_List, RIGHT);
	node_set_ptr(res, (void *) nullptr, NODE, LEFT);
	node_set_ptr(res, (void *) nullptr, NODE, RIGHT);
	return res;
    }
    if (node_gc() != 0)
	return node_new();
    if (node_more_store() != FALSE)
	return node_new();
#ifdef LOPG_PRINT  
  fprintf(stderr, "out of nodes: system not giving more memory\n")
#endif
;
    abort();
    return node_new();
}

/*
   ** void node_push_local(node *local)
   **   puts the address of a local variable onto the Locals_Stack stack 
   **   ( of starting points for garbage collection). 
   **
   ** void node_pop_local()
   **     removes one ptr off the Locals_Stack stack.
 */

void print_locals_stack()
{
    local_v ptr;
    ptr = Locals_Stack;
    printf("printing local stack \n");
    while (ptr != nullptr) {
	node_print(*(ptr->val));
	ptr = ptr->next;
    }
    printf(" done \n");
}

void node_push_local(local_v loc, node * val)
{
#ifdef MEM_DEBUG
  if (Locals_Stack == loc)
    bad_error("trying to make circular stack\n");
#endif

    loc->next = Locals_Stack;
    loc->val = val;
    Locals_Stack = loc;
}

void node_pop_local()
{
    if (Locals_Stack == nullptr)
	bad_error("poping empty local stack");
    Locals_Stack = Locals_Stack->next;
}




/*
   **  Mark all nodes accessable from a node n.
   **  if n is marked Yes then both its subtrees are already 
   **  marked and should be skipped. 
   **  otherwise cal mark on any pointers to nodes stored in the 
   **  node (is_node is used to check this)
 */
static int mark(node n)
{
    if ((n != nullptr) && (Node_Marked(n) == NO)) {
	Node_Set_Mark(n);
	if (Node_LT(n) == NODE && Node_L(n).ptr != nullptr)
	    mark((node) Node_L(n).ptr);
	else if (Node_LT(n) == NPTR && Node_L(n).ptr != nullptr)
	    mark(*((node *) Node_L(n).ptr));
	if (Node_RT(n) == NODE && Node_R(n).ptr != nullptr)
	    mark((node) Node_R(n).ptr);
	else if (Node_RT(n) == NPTR && Node_R(n).ptr != nullptr)
	    mark(*((node *) Node_R(n).ptr));
	return 0;
    }
    return 1;
}

/*
   ** Collect garbage
 */
static int node_gc()
{
    int i, free_cnt = 0;
    node_block next_block = Node_Store;
    local_v ptr;
#ifdef LOG_PRINT   
  fprintf(stdout, "Colecting Garbage .. ")
#endif
;
    fflush(stdout);

    /*call mark on variables in local list */
    ptr = Locals_Stack;
    while (ptr != nullptr) {
	mark(*(ptr->val));
	ptr = ptr->next;
    }

    /* now put anything that is not already marked on the free_list */
    while (next_block != nullptr) {
	for (i = 0; i < BLOCKSIZE; i++) {
	    if (Node_Marked(next_block->store + i) == YES)
		Node_Unset_Mark(next_block->store + i);
	    else {
		atom_free(next_block->store + i);
		node_free(next_block->store + i);
		free_cnt++;
	    }
	}
	next_block = next_block->next;
    }

#ifdef LOG_PRINT   
 fprintf(stdout, "Done  %d nodes freed\n", free_cnt)
#endif
;
    fflush(stdout);
    return free_cnt;
}


/**************************************************************************/
/********************** implementations from error.c **********************/
/**************************************************************************/

#ifdef GAMBIT_EXCEPTIONS
ErrorInPelican::~ErrorInPelican() { }

std::string ErrorInPelican::GetDescription(void) const
{
  return "Error somewhere in Pelican";
}
#endif

void bad_error(const char *m)     /* generates an error message and aborts*/
{
#ifdef GAMBIT_EXCEPTIONS
  throw ErrorInPelican();
#endif

 #ifdef LOG_PRINT
       fprintf(stderr /* was Pel_Err */,"%s\n",m)
#endif
;
        exit(1);
}

void warning(const char *m){
#ifdef LOG_PRINT
fprintf(stderr /* was Pel_Err */,"Warning:%s\n",m)
#endif
;}


/**************************************************************************/
/*********************** implementations from Rand.c **********************/
/**************************************************************************/

/*
** rand_seed  -- seed the random number generator with seedval.
*/
void rand_seed(long int seedval)
{
#if defined(HAVE_SRAND48)
srand48(seedval);
#else
srand(seedval);
#endif  /* defined(HAVE_SRAND48) */
}

/*
**rand_int  -- return a random integer r with low<=r<=high
**             it produces double between low and high and rounds
**              by adding .5-epsilon and truncating, (if we just add
**              .5 then there is a slight chance we could end up with
**              high+1.)
*/
int rand_int(int low, int high)
{
#if defined(HAVE_DRAND48)
  return (int)(low+drand48()*(high-low)+.499999999999); 
#else
  return (int)(low+rand()*(high-low)+.499999999999); 
#endif  /* defined(HAVE_DRAND48) */
}

/*
**rand_double  -- return a random integer r with low<=r<=high
*/
double rand_double(int low, int high)
{
#if defined(HAVE_DRAND48)
  return (drand48()*(high-low)+low);
#else
  return (rand()*(high-low)+low);
#endif  /* defined(HAVE_DRAND48) */
}


/**************************************************************************/
/********************** implementations from Dlist.c **********************/
/**************************************************************************/

void Dlist_empty(node L){ Dlist_first(L)=nullptr;}

/*
** invariants: Dnode_next(Dnode_prev(pos)) ==pos 
**             for all pointers to Dlist node entrees.
**             (NOTE: Dnode_next(L) := Dlist_first(L), 
**                    where L is list header) 
**
**             Dnode_prev(Dnode_next(pos))=pos
**             for all non-zero pointers to Dlist node entrees 
**             and also for list header
**             
*/

node Dlist_add(node L, node data){
  node tmp=nullptr;
  LOCS(2);
  PUSH_LOC(data);
  PUSH_LOC(tmp);
  tmp=node_new();
  Dnode_link(tmp)=node_new();
  Dnode_data(tmp)=data;
  Dnode_prev(tmp)=L;
  Dnode_next(tmp)=Dlist_first(L);
  if (Dnode_next(tmp)!=nullptr) Dnode_prev(Dnode_next(tmp))=tmp;
  Dlist_first(L)=tmp; 
  POP_LOCS();
  return tmp;
}


node Dlist_del(node L, node pos){
  if (Dnode_next(pos)!=nullptr) Dnode_prev(Dnode_next(pos))=Dnode_prev(pos);
  if (Dnode_prev(pos)!=L) Dnode_next(Dnode_prev(pos))=Dnode_next(pos);
  else Dlist_first(L)=Dnode_next(pos);
  return (node)Dnode_data(pos);
}

node Dlist_data(node pos){ return (node)Dnode_data(pos);}

/* end Dlist.c */


/**************************************************************************/
/******************** implementations from Dmatrix.c **********************/
/**************************************************************************/

/*--------------------------------------------------------------- 
  vector/matrix type  a linear array of int, whith auxilary info.
       *) the number of elements that can be stored is in elt[0]
       *) the current number of rows is in elt[1]
       *) the current number of collumbs is in elt[2]
The actual data are then stored in row major order from elt[3] on
---------------------------------------------------------------*/
#ifndef DMATRIX_FAST
struct Dmatrix_t {
    int store;
    int nrows;
    int ncols;
    double *coords;
};
#endif

/*-------------------------------------------------------------
 vector access macroes (which ignore any rows except for first)
-------------------------------------------------------------*/
#define Vstore(V)  ((V->store))	/* maximum #elts available */
#define Vlength(V) ((V->nrows))	/* actual #elts stored      */
#define Vref1(V,i) (((V->coords)[i-1]))	   /*acces ith elt (starting at 1) */
#define Vref0(V,i)  (((V->coords)[i]))	/*acces ith elt (starting at 0) */
#define Vref(V,i)  Vref1(V,i)

/*------------------------------------------------------------
 matrix access macroes
-------------------------------------------------------------*/
#define Mstore(V)  ((V->store))	/* maximum #elts available */
#define MMrows(V)  ((V->store/V->ncols))	/* maximum #rows          */
#define Mrows(V) ((V->nrows))	/* number rows stored */
#define Mcols(V) ((V->ncols))	/* number cols stored */
#define Mref1(V,i,j)(((V->coords)[(i-1)*(V->ncols)+j-1]))
#define Mref0(V,i,j)(((V->coords)[i*(V->ncols)+j]))
#define Mref(V,i,j)  Mref1((V),i,j)

#ifndef DMATRIX_FAST
double DMstore(Dmatrix M)
{
    return Mstore(M);
}
double DMMrows(Dmatrix M)
{
    return MMrows(M);
}
double DMrows(Dmatrix M)
{
    return Mrows(M);
}
double DMcols(Dmatrix M)
{
    return Mcols(M);
}
double *DMelts(Dmatrix M)
{
    return M->coords;
}
double *DMref_P(Dmatrix M, int i, int j)
{
    return &(Mref0(M, i, j));
}

#endif 

/*
   **   Constructor/Destructors for Dmatrixes
   ** 
   ** Dmatrix Dmatrix_free(int r, int c); 
   **       New Dmatrix cabable of holding r rows, and c collumbs.
   ** Dmatrix Dmatrix_new(Dmatrix V);
 */

Dmatrix Dmatrix_new(int r, int c)
{
    Dmatrix V;
    /*    void *mem_malloc(int); */
    V = (Dmatrix) mem_malloc(sizeof(struct Dmatrix_t));
    if (!V)
	bad_error("allocation failure in Dmatrix_new()");
    V->coords = (double *) mem_malloc(r * c * sizeof(double));
    if (!V)
	bad_error("allocation failure 2 in Dmatrix_new()");
    Mstore(V) = r * c;
    Mrows(V) = r;
    Mcols(V) = c;
    return V;
}

void Dmatrix_free(Dmatrix V)
{   /* void mem_free(); */
    if (V != nullptr && V->coords != nullptr)
	mem_free((char *) (V->coords));
    if (V != nullptr)
	mem_free((char *) (V));
}
#undef mem_malloc
#undef mem_free
/*
   ** Dmatrix_resize(R,r,c)
   **   if R has enough storage to hold an rxc matrix resets
   **   row and columb entrees of r to r and c. otherwise
   **   frees R and reallocates an rxc matrix
 */
Dmatrix Dmatrix_resize(Dmatrix R, int r, int c)
{

    if (R == nullptr || Mstore(R) < (r * c)) {
	if (R != nullptr)
	    Dmatrix_free(R);
	R = Dmatrix_new(r, c);
    } else {
	Mrows(R) = r;
	Mcols(R) = c;
    }
    return R;
}


/*
   **  Dmatrix_print(M):  print an Dmatrix
   **    if M is null print <<>> and return fail.
   **    otherwise print matrix and return true.
 */
Dmatrix Dmatrix_fprint(FILE *fout, Dmatrix M)
{
    int i, j;

    if (M == nullptr) {
#ifdef LOG_PRINT
	fprintf(fout,"<<>>")
#endif
;
	return nullptr;
    }
#ifdef LOG_PRINT
    fprintf(fout,"<")
#endif
;
    for (i = 1; i <= Mrows(M); i++) {
#ifdef LOG_PRINT
	fprintf(fout,"<")
#endif
;
	for (j = 1; j <= Mcols(M); j++) {
#ifdef LOG_PRINT
	    fprintf(fout,"%8.4f", Mref(M, i, j))
#endif
;
	    if (j < Mcols(M))
#ifdef LOG_PRINT	
	fprintf(fout,", ")
#endif
;
	}
#ifdef LOG_PRINT
	fprintf(fout,">")
#endif
;
	if (i < Mrows(M))
#ifdef LOG_PRINT
	    fprintf(fout,"\n")
#endif
;
    }
#ifdef LOG_PRINT
    fprintf(fout,">")
#endif
;
    return M;
}

/*
   ** matrix_add(M1,M2,&M3) -- Add two Dmatrixes:
   **  if M1, and M2 are incompatable (or null) complain and return false.
   **  if *M3 has too little storage (or is null) free *M3 if nescesary
   **                                            and create new storage.
 */
Dmatrix Dmatrix_add(Dmatrix M1, Dmatrix M2, Dmatrix * M3)
{
    int i, j;
    Dmatrix R = *M3;

    if (M1 == nullptr || M2 == nullptr || Mrows(M1) != Mrows(M2) || Mcols(M1) != Mcols(M2)) {
#ifdef LOG_PRINT
	fprintf(stderr, "matrix_add: dimensions don't match\n")
#endif
;
	return nullptr;
    }
    Dmatrix_resize(R, Mrows(M1), Mcols(M1));
    for (i = 1; i <= Mrows(M1); i++)
	for (j = 1; j <= Mcols(M1); j++)
	    Mref(R, i, j) = Mref(M1, i, j) + Mref(M2, i, j);
    M3 = &R;
    return R;
}


/*
   **Dmatrix_mull(M1,M2,&M3) -- multiply two Dmatrixes:
   ** if M1, and M2 are incompatable (or null) complain and return false.
   ** if *M3 has too little storage (or is null) free *M3 if nescesary
   **                                           and create new storage.
   ** NOT USED YET
 */

Dmatrix Dmatrix_mull(Dmatrix M1, Dmatrix M2, Dmatrix * M3)
{
    int i, j, k;
    Dmatrix R = *M3;

    if (M1 == nullptr || M2 == nullptr || Mcols(M1) != Mrows(M2)) {
#ifdef LOG_PRINT
	fprintf(stderr, "Dmatrix_mull: incompatible matrices\n")
#endif
;
	return nullptr;
    }
    Dmatrix_resize(R, Mrows(M1), Mcols(M2));
    for (i = 1; i <= Mrows(M1); i++)
	for (j = 1; j <= Mcols(M2); j++) {
	    Mref(R, i, j) = 0;
	    for (k = 1; k <= Mcols(M1); k++)
		Mref(R, i, j) += Mref(M1, i, k) * Mref(M2, k, j);
	}
    M3 = &R;
    return R;
}

/*
   ** Dmatrix_dot(M1,M2,&M3) -- calculate M1*Transpose(M2):                
   ** if M1, and M2 are incompatable (or null) complain and return false.
   ** if *M3 has too little storage (or is null) free *M3 if nescesary 
   **                                           and create new storage.   
 */
Dmatrix Dmatrix_dot(Dmatrix M1, Dmatrix M2, Dmatrix M3)
{
    int i, j, k;

    if (M1 == nullptr || M2 == nullptr || Mcols(M1) != Mcols(M2)) {
#ifdef LOG_PRINT
	fprintf(stderr, "Dmatrix_dot: incompatible matrices\n")
#endif
;
	return nullptr;
    }
    M3 = Dmatrix_resize(M3, Mrows(M1), Mrows(M2));

    for (i = 1; i <= Mrows(M1); i++)
	for (j = 1; j <= Mrows(M2); j++) {
	    Mref(M3, i, j) = 0;
	    for (k = 1; k <= Mcols(M1); k++)
		Mref(M3, i, j) += Mref(M1, i, k) * Mref(M2, j, k);
	}
    return M3;
}


/*
** Dmatrix_equal(M1,M2)  
**     return true if matrices represented by M1 and M2 are equal.
**     false otherwise.                     NOT TESTED YET
*/
int Dmatrix_equal(Dmatrix M1, Dmatrix M2)
{
    int i, j;
    if (M1 == nullptr && M2 == nullptr)
	return 1;
    if (M1==nullptr||M2==nullptr||(Mrows(M1)!=Mrows(M2))||(Mcols(M1)!=Mcols(M2)))
        return 0;
    for (i = 1; i <= Mrows(M1); i++)
	for (j = 1; j <= Mcols(M1); j++)
	    if (Mref(M1, i, j) != Mref(M2, i, j))
		return 0;

    return 1;
}

/*
** Dmatrix_GQR
**    Use givens QR on a sqaure matrix:
**        after completion Q should be orthogonal, R triangular
**        and Q*A = original matrix A
*/
void Dmatrix_GQR(Dmatrix Q,Dmatrix A)
        /* A->ch<=A->rh=Q->rh=Q->ch */
   { int i,j,k;
     int r,c;
     double s,s1,s2;
     double t1,t2;
     r=Mrows(A);
     c=Mcols(A);
       for(i=1;i<=r;i++){
              for(k=1;k<=r;k++)Mref(Q,i,k)=0.0;
              Mref(Q,i,i)=1.0;}

       for (i=1;i<=c;i++)
         for (k=i+1;k<=r;k++)
              /* performing givens rotations to zero A[k][i] */
             if (Mref(A,k,i)!=0){
               s=sqrt(Mref(A,i,i)*Mref(A,i,i)+
                      Mref(A,k,i)*Mref(A,k,i));
               s1=Mref(A,i,i)/s;
               s2=Mref(A,k,i)/s;
               for(j=1;j<=c;j++) {
                          t1=Mref(A,i,j);
                          t2=Mref(A,k,j);
                          Mref(A,i,j)=s1*t1+s2*t2;
                          Mref(A,k,j)=-s2*t1+s1*t2;}
            /* actually doing givens row rotations on transpose Q */
               for(j=1;j<=r;j++){
                    t1=Mref(Q,j,i);
                    t2=Mref(Q,j,k);
                    Mref(Q,j,i)=s1*t1+s2*t2;
                    Mref(Q,j,k)=-s2*t1+s1*t2;}
             }
         }

/* 
** Dmatrix_Solve, Solve a square matrix in place
*/
#define LHS(i,j) Mref(LHS,i,j)
#define RHS(i) Vref(RHS,i)
void Dmatrix_Solve(Dmatrix LHS, Dmatrix RHS,int n){
 int i,j,k,m=n;
 double s,s1,s2,t1,t2;

 /*
 ** Use Givens rotations to triangularize LHS,i.e.LHS becomes Q*LHS.
 ** (triangular)
 ** and also apply same givens rotations to RHS i.e. RHS becomes Q*RHS.
 */
 for (i=1;i<=n;i++){
   for (k=i+1;k<=m;k++){
     if (LHS(k,i)!=0.0){
         /* compute rotation */
         s=sqrt(LHS(i,i)*LHS(i,i)+LHS(k,i)*LHS(k,i));
         s1=LHS(i,i)/s;
         s2=LHS(k,i)/s;
         /* apply rotation to LHS */
         for(j=1;j<=n;j++) {
             t1=LHS(i,j);
             t2=LHS(k,j);
             LHS(i,j)=s1*t1+s2*t2;
             LHS(k,j)=-s2*t1+s1*t2;
         }
         /* apply rotation to RHS */
         t1=RHS(i);
         t2=RHS(k);
         RHS(i)=s1*t1+s2*t2;
         RHS(k)=-s2*t1+s1*t2;
     }
   }
 }
 /*
 ** Back solve   R*X=Y
 **   R=top square portion of LHS. (2nx2nupper triangular)
 **   Y=top 2n entrees of RHS.
 **   results in X which solves original least squares problem.
 */
 for(j=n;j>=1;j--){
   t1=RHS(j);
   for(i=j+1;i<=n;i++) t1-=LHS(j,i)*RHS(i);
   t1=t1/LHS(j,j);
   RHS(j)=t1;
 }
}
#undef LHS
#undef RHS

/* end Dmatrix.c */

/**************************************************************************/
/********************* implementations from Imatrix.c *********************/
/**************************************************************************/

#ifndef min
#define min(i,j) ((i) < (j) ? (i): (j))
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


#ifndef IMATRIX_FAST 
/*--------------------------------------------------------------- 
  vector/matrix type  a linear array of int, whith auxilary info.
       *) the number of elements that can be stored is in elt[0]
       *) the current number of rows is in elt[1]
       *) the current number of collumbs is in elt[2]
The actual data are then stored in row major order from elt[3] on
---------------------------------------------------------------*/
struct Imatrix_t {
    int store;	/* maximum number of helts reserved */
    int topc;	/* effective number of columbs */
    int topr;	/* effective number of rows */
    int ncols;	/* number of elts between first elts of rows */
    int *elts;	/* ptr to first elt of block of space */
};

/*
**  a couple of functions which are defined in files that
**  I don't want to include right now, when things stableize
**      bad_error -- used mainly to stop things when a function
**                   is passed bad input, as things stablizes
**                   less catastrophic reactions may be introduced.
**      mem_alloc,
**      mem_free -- call malloc and free with some extra book-keeping.
*/
#endif

/*
void bad_error(char *);
char *mem_malloc(size_t);
void mem_free(char *);
*/

/*-------------------------------------------------------------
 vector access macroes (which ignore any rows except for first)
-------------------------------------------------------------*/

#define ImatrixVstore(V)  (((V)->store))	/* maximum #elts available */
#define ImatrixVlength(V) (((V)->topc))	/* actual #elts stored      */
#define ImatrixVref1(V,i)  (((V)->elts[i-1]))/* acces ith elt (starting at 1)*/
#define ImatrixVref0(V,i)  (((V)->elts[i]))/* acces ith elt (starting at 0) */
#define ImatrixVref(V,i)  ImatrixVref1(V,i)

/*------------------------------------------------------------
 matrix access macroes
-------------------------------------------------------------*/
#define ImatrixMstore(V)  (((V)->store))	/* maximum #elts available */
#define ImatrixMMrows(V)  (((V)->store/(V)->ncols))   /* maximum #rows    */
#define ImatrixMrows(V) (((V)->topr))	/* number rows stored */
#define ImatrixMcols(V) (((V)->topc))	/* number cols stored */
#define ImatrixMNcols(V) (((V)->ncols))	/* number cols stored */
#define ImatrixMref1(V,i,j) ((((V))->elts[(i-1)*((V)->ncols)+j-1]))
#define ImatrixMref0(V,i,j) (((V)->elts[(i*(V)->ncols)+j]))
#define ImatrixMref(V,i,j)  ImatrixMref1(V,i,j)


#ifndef IMATRIX_FAST
int IMstore(Imatrix M)
{
    return ImatrixMstore(M);
}
int IMMrows(Imatrix M)
{
    return ImatrixMMrows(M);
}
int IMrows(Imatrix M)
{
    return ImatrixMrows(M);
}
int IMcols(Imatrix M)
{
    return ImatrixMcols(M);
}
int *IMref1(Imatrix M, int i, int j)
{
    return &(ImatrixMref1(M, i, j));
}
#endif



/*
   **   Constructor/Destructors for Imatrixes
   ** 
   ** Imatrix Imatrix_free(int r, int c); 
   **       New Imatrix cabable of holding r rows, and c collumbs.
   ** Imatrix Imatrix_new(Imatrix V);
 */

Imatrix Imatrix_new(int r, int c)
{
    Imatrix V;
    V = (Imatrix) mem_malloc(sizeof(struct Imatrix_t));
    if (!V)
	bad_error("allocation failure 1 in Imatrix_new()");
    V->elts = (int *) mem_malloc(r * c * sizeof(int));
    if (!V)
	bad_error("allocation failure 2 in Imatrix_new()");
    V->store = r * c;
    V->topc = V->ncols = c;
    V->topr = r;
    return V;
}

void Imatrix_free(Imatrix V)
{
    if (V != nullptr) {
	if (V->elts != nullptr)
	    mem_free((char *) (V->elts));
	mem_free((char *) (V));
    }
}

/*
   ** Imatrix_resize(R,r,c)
   **   Reset R to hold an r,by  c matrix.
   **   if R has enough storage to hold an rxc matrix resets
   **   row and columb entrees of r to r and c. otherwise
   **   frees R and reallocates an rxc matrix
   ** DOES NOT PRESERVE INDECIES OF EXISTING DATA
 */
Imatrix Imatrix_resize(Imatrix R, int r, int c)
{

    if (R == nullptr || ImatrixMstore(R) < (r * c)) {
	if (R != nullptr)
	    Imatrix_free(R);
	R = Imatrix_new(r, c);
    } else {
	ImatrixMrows(R) = r;
	ImatrixMcols(R) = c;
	ImatrixMNcols(R) = c;
    }
    return R;
}


Imatrix Imatrix_submat(Imatrix R, int r, int c)
{

    if (R == nullptr || c > ImatrixMNcols(R) || r > ImatrixMrows(R) * ImatrixMNcols(R)) {
	bad_error("bad subscripts or zero matrix in Imatrix_submat()");
    } else {
	ImatrixMrows(R) = r;
	ImatrixMcols(R) = c;
    }
    return R;
}


/*
   **  Imatrix_print(M):  print an Imatrix
   **    if M is null print <<>> and return fail.
   **    otherwise print matrix and return true.
 */
Imatrix Imatrix_fprint(FILE *fout,Imatrix M)
{
    int i, j;

    if (M == nullptr) {
 #ifdef LOG_PRINT
       fprintf(fout,"<>\n")
#endif
;
        return nullptr;
    }
 #ifdef LOG_PRINT
   fprintf(fout,"<")
#endif
;
    for (i = 1; i <= ImatrixMrows(M); i++) {
        for (j = 1; j <= ImatrixMcols(M); j++) {
   #ifdef LOG_PRINT
         fprintf(fout,"%d", ImatrixMref(M, i, j))
#endif
;
            if (j < ImatrixMcols(M))  
#ifdef LOG_PRINT
                fprintf(fout,", ")
#endif
;
        }
        if (i < ImatrixMrows(M))
#ifdef LOG_PRINT
            fprintf(fout,";\n")
#endif
;
    }
#ifdef LOG_PRINT
    fprintf(fout,">\n")
#endif
;
    return M;
}

/*
** matrix_add(M1,M2,&M3) -- Add two Imatrixes:
** if M1, and M2 are incompatable (or null) complain and return false.
**  if *M3 has too little storage (or is null) free *M3 if nescesary
**                                            and create new storage.
*/
Imatrix Imatrix_add(int i1,Imatrix M1,int i2, Imatrix M2, Imatrix M3)
{
    int i, j;
    Imatrix R = M3;

    if (M1 == nullptr || M2 == nullptr || ImatrixMrows(M1) != ImatrixMrows(M2) || ImatrixMcols(M1) != ImatrixMcols(M2)) {
#ifdef LOG_PRINT
	fprintf(stderr, "matrix_add: dimensions don't match\n")
#endif
;
	return nullptr;
    }
    R=Imatrix_resize(R, ImatrixMrows(M1), ImatrixMcols(M1));
    for (i = 1; i <= ImatrixMrows(M1); i++)
	for (j = 1; j <= ImatrixMcols(M1); j++)
	    ImatrixMref(R, i, j) =i1* ImatrixMref(M1, i, j) + i2*ImatrixMref(M2, i, j);
    return R;
}


/*
   **Imatrix_mull(M1,M2,&M3) -- multiply two Imatrixes:
   ** if M1, and M2 are incompatable (or null) complain and return false.
   ** if *M3 has too little storage (or is null) free *M3 if nescesary
   **                                           and create new storage.
   ** NOT USED YET
 */

Imatrix Imatrix_mul(Imatrix M1, Imatrix M2, Imatrix M3)
{
    int i, j, k;
    Imatrix R;

    if (M1 == nullptr || M2 == nullptr || ImatrixMcols(M1) != ImatrixMrows(M2)) {
#ifdef LOG_PRINT	
fprintf(stderr, "Imatrix_mull: incompatible matrices\n")
#endif
;
	return nullptr;
    }
    if (M3!=M1&&M3!=M2) R=Imatrix_resize(M3, ImatrixMrows(M1), ImatrixMcols(M2));
      else R=Imatrix_new(ImatrixMrows(M1),ImatrixMcols(M2));
    for (i = 1; i <= ImatrixMrows(M1); i++)
	for (j = 1; j <= ImatrixMcols(M2); j++) {
	    ImatrixMref(R, i, j) = 0;
	    for (k = 1; k <= ImatrixMcols(M1); k++)
		ImatrixMref(R, i, j) += ImatrixMref(M1, i, k) * ImatrixMref(M2, k, j);
	}
    if (M3==M1&&M3==M2) Imatrix_free(M3);
    return R;
}

/*
   ** Imatrix_dot(M1,M2,&M3) -- calculate M1*Transpose(M2):                
   ** if M1, and M2 are incompatable (or null) complain and return false.
   ** if *M3 has too little storage (or is null) free *M3 if nescesary 
   **                                           and create new storage.   
 */
Imatrix Imatrix_dot(Imatrix M1, Imatrix M2, Imatrix M3)
{
    int i, j, k;

    if (M1 == nullptr || M2 == nullptr || ImatrixMcols(M1) != ImatrixMcols(M2)) {
#ifdef LOG_PRINT
	fprintf(stderr, "Imatrix_dot: incompatible matrices\n")
#endif
;
	return nullptr;
    }
    M3 = Imatrix_resize(M3, ImatrixMrows(M1), ImatrixMrows(M2));

    for (i = 1; i <= ImatrixMrows(M1); i++)
	for (j = 1; j <= ImatrixMrows(M2); j++) {
	    ImatrixMref(M3, i, j) = 0;
	    for (k = 1; k <= ImatrixMcols(M1); k++)
		ImatrixMref(M3, i, j) += ImatrixMref(M1, i, k) * ImatrixMref(M2, j, k);
	}
    return M3;
}

/*
   ** dot_Ivector(M1,M2) -- vector dot product (integer valued)
   **  calculate dot product of first rows of M1, and M2.
   ** if M1, and M2 have are incompatable(or null) complain and return 0.
   ** NOT USED YET
 */
int dot_Ivector(Imatrix M1, Imatrix M2)
{
    int k, d = 0;
    if (M1 == nullptr || M2 == nullptr || ImatrixMcols(M1) != ImatrixMcols(M2)) {
#ifdef LOG_PRINT
	fprintf(stderr, "dot_Ivector: incompatible or null vectors\n")
#endif
;
	return 0;
    }
    for (k = 1; k <= ImatrixMcols(M1); k++)
	d += ImatrixVref(M1, k) * ImatrixVref(M2, k);
    return d;
}

/*
   ** Imatrix_equal(M1,M2)  
   **     return true if matrices represented by M1 and M2 are equal.
   **     false otherwise.  
 */
int Imatrix_equal(Imatrix M1, Imatrix M2)
{
    int i, j;
    if (M1 == nullptr && M2 == nullptr)
	return TRUE;
    if (M1 == nullptr || M2 == nullptr || (ImatrixMrows(M1) != ImatrixMrows(M2)) || (ImatrixMcols(M1) != ImatrixMcols(M2))) {
	printf("in Imatrix_equal comparison failing\n");
	return FALSE;
    }
    for (i = 1; i <= ImatrixMrows(M1); i++)
	for (j = 1; j <= ImatrixMcols(M1); j++)
	    if (ImatrixMref(M1, i, j) != ImatrixMref(M2, i, j))
		return FALSE;

    return TRUE;
}


int Imatrix_order(Imatrix M1, Imatrix M2)
{
    int i, j, c = 0;
    if (M1 == nullptr && M2 == nullptr)
	return TRUE;
    if (M1 == nullptr || M2 == nullptr || (ImatrixMrows(M1) != ImatrixMrows(M2)) || (ImatrixMcols(M1) != ImatrixMcols(M2))) {
	printf("in Imatrix_equal comparison failing\n");
	return FALSE;
    }
    for (i = 1; i <= ImatrixMrows(M1); i++)
	for (j = 1; j <= ImatrixMcols(M1); j++)
	    if ((c = ImatrixMref(M1, i, j) - ImatrixMref(M2, i, j)) != 0)
		return c;
    return 0;
}



int Imatrix_gcd_reduce(Imatrix M)
{
    int i, j, g = 0, gcd(int, int);

    if (M == nullptr)
	return 1;
    for (i = 1; i <= ImatrixMrows(M); i++)
	for (j = 1; j <= ImatrixMcols(M); j++) {
	    if (g == 0 && ImatrixMref(M, i, j) != 0)
		g = abs(ImatrixMref(M, i, j));
	    else if (ImatrixMref(M, i, j) != 0)
		g = abs(gcd(g, ImatrixMref(M, i, j)));
	}

    for (i = 1; i <= ImatrixMrows(M); i++)
	for (j = 1; j <= ImatrixMcols(M); j++)
	    ImatrixMref(M, i, j) = ImatrixMref(M, i, j) / g;

    return g;
}


int gcd(int a, int b)
{
    int c;
    if (b == 0)
	return a;
    c = a % b;
    if (c == 0)
	return b;
    else
	return gcd(b, c);
}

/*
** int Imatrix_rref(Imatrix N, int *det)
**   replaces N by its reduced row echelon form (upper triangular).
**   returns the rank of the  matrix, and puts the determinant
**   of the first full rank maximal minor in det.
**   ALGORITHM 9.1 of Algorithms for Computer Algebra,
**      Geddes,Czapor,Labahn
**  NOTE: I have actually changed this to reduce by the gcd when posible
**        to protect against over flow. (for the C-integer version).
*/
#define M(i,j) (ImatrixMref(N,i,j))

int Imatrix_rref(Imatrix N,int *det){
  int divisor=1,sign=1,r=1,p,k,j,i,g;

  for(k=1;k<=ImatrixMcols(N)&&r<=ImatrixMrows(N);k++){
    for(p=r; p<=ImatrixMrows(N)&&M(p,k)==0;p++); 
    if (p<=ImatrixMrows(N)) {
        for(j=k;j<=ImatrixMcols(N);j++){i=M(p,j); M(p,j)=M(r,j); M(r,j)=i;}
        if (r!=p) sign*=-1;
        for(i=r+1;i<=ImatrixMrows(N);i++){
          g=gcd(M(i,k),M(r,k)); 
          for(j=k+1;j<=ImatrixMcols(N);j++){
            M(i,j)=(M(r,k)*M(i,j)-M(r,j)*M(i,k))/divisor; 
          }
          M(i,k)=0;
        }
        divisor=M(r,k);
        r++;
    }
  }
  *det=sign*divisor;
  return r-1;
}
/*
** int Imatrix_backsolve(Imatrix N, Imatrix S)
**  finds a solution to a triangular system of equations with more
**  equtions then unknowns
*/
int Imatrix_backsolve(Imatrix N, Imatrix S){
 int p,j,g,k;


 if (N==nullptr || S==nullptr || IMcols(N)!=IMcols(S)||IMcols(N)<=IMrows(N)) 
     bad_error("bad dimensions in backsolve");
 
 /* 
 ** find first missing pivot (in row p;  p=IMrows(N)+1 if no missing pivots
 */
 for(p=1;p<=ImatrixMrows(N) && ImatrixMref(N,p,p)!=0;p++) /* null body*/;
 /* 
 ** now set up for a solution to the AX=0 problem with A a p-1xp matrix whoose
 ** first pxp minor is nonsingular (use simple minded back substitution)
 */
 for(j=1;j<=ImatrixMcols(N);j++) ImatrixVref(S,j)=0;
 ImatrixVref(S,p)=1;
 for (j=p-1;j>=1;j--){
   for(k=j+1;k<=p;k++) ImatrixVref(S,j)-=ImatrixMref(N,j,k)*ImatrixVref(S,k);
   g=gcd(ImatrixVref(S,j),ImatrixMref(N,j,j));
   ImatrixVref(S,j)=ImatrixVref(S,j)/g;
   g=(ImatrixMref(N,j,j)/g);
   if (ImatrixVref(S,j)!=0) for(k=j+1;k<=p;k++) ImatrixVref(S,k)*=g;
 }
return 1;
}
#undef M



/*
** Imatrix_hermite:
**    Input: S an n by m Imatrix.
**   Output: True 
**
**   Sidefects: U points to a unitary nxn matrix
**              S gets replaced by its hermite normal form (U*S)
**            
**  Method:
**    U = Inxn
**    c = 1
**    while (c<=n) do
**      find mc>=c such that abs(S(mc,c)) has minimum non-zero value
**      if (mc!=c) then  switch rows mc and c (in S, and U)
**      if (S(c,c)<0) then multiply row c by -1
**      if (S(c,c)!=0) then 
**        for i in c+1:n  add S(i,c)/S(c,c) times row c to row i;
**      end(if)
**      if S(i,c)==0 for all i in c+1 ... n set c=c+1
**    end(while)
**
*/

int Imatrix_hermite(Imatrix S, Imatrix U){
  int c=1,mc,i,j,m,n,done,sign;
  int t=0,mv=0;
  m=ImatrixMcols(S);
  n=ImatrixMrows(S);

  if (S==nullptr || U==nullptr || ImatrixMrows(U)!=n || ImatrixMrows(U)!=n)
     bad_error("Incompatible matrices in Imatrix_hermite");

  /* Initialize U to nxn identity */
  U=Imatrix_resize(U,n,n);
  for(i=1;i<=n;i++){
    for(j=1;j<=n;j++){
      if (i==j) ImatrixMref(U,i,j)=1;
      else ImatrixMref(U,i,j)=0;
    }     
  }

  while(c<=n){
    /* find minimum entry in col c */
    mv=abs(ImatrixMref(S,c,c));
    mc=c;
    for(i=c+1;i<=n;i++){
      t=abs(ImatrixMref(S,i,c)); 
      if(mv==0 || (mv>t && t!=0)){
           mv=t;
           mc=i;
      }
    }

    /* if nescesary pivot to put min in row c and multiply by+-1
       to ensure diagonal entry is positive */
    if (mc!=c||ImatrixMref(S,mc,c)<0){
      if (ImatrixMref(S,mc,c)<0) sign=-1;
      else sign=+1;
      for(j=c;j<=m;j++){
        t=ImatrixMref(S,mc,j);
        ImatrixMref(S,mc,j)=ImatrixMref(S,c,j);
        ImatrixMref(S,c,j)=sign*t;
      }
      for(j=1;j<=n;j++){
        t=ImatrixMref(U,mc,j);
        ImatrixMref(U,mc,j)=ImatrixMref(U,c,j);
        ImatrixMref(U,c,j)=sign*t;
      }
    }

    /* if collumb is not zero do a reduction step */
    done=TRUE;
    if (ImatrixMref(S,c,c)!=0){
      for(i=c+1;i<=n;i++){
        t=ImatrixMref(S,i,c)/ImatrixMref(S,c,c);
        for(j=c;j<=m;j++){
           ImatrixMref(S,i,j)-=t*ImatrixMref(S,c,j);
        }
        for(j=1;j<=n;j++){
           ImatrixMref(U,i,j)-=t*ImatrixMref(U,c,j);
        }
        if (ImatrixMref(S,i,c)!=0) done=FALSE;
      }
    }
    /* if all entrees of col c bellow row c are zero go tonext col */
    if (done==TRUE) c++;
  }
  return TRUE;
 }

Imatrix Imatrix_dup(Imatrix M,Imatrix N){
  Imatrix R;
  int i,j;
  if (M==nullptr) return nullptr;
  if (M==N) bad_error("Imatrix_dup: source and destination equal");
  R=Imatrix_resize(N,ImatrixMrows(M),ImatrixMcols(M));
  for(i=1;i<=ImatrixMrows(M);i++)
     for(j=1;j<=ImatrixMcols(M);j++) ImatrixMref(R,i,j)=ImatrixMref(M,i,j);
  return R;
}


int Imatrix_is_zero(Imatrix M){
  int i,j;
  if (M==nullptr) return TRUE;
  for(i=1;i<=IMrows(M);i++){
       for(j=1;j<=IMcols(M);j++){
         if (*IMref(M,i,j)!=0) return FALSE;
       }
  }
  return TRUE;
}

/* end Imatrix.c */

/**************************************************************************/
/********************** implementations from Lists.c **********************/
/**************************************************************************/

void bad_error(const char *);
void mem_free(void *);

node list_push(node item,node *stack){
   *stack=Cons(item,*stack);
   return item;
} 
  
int list_length(node L){
  int ct=0;
  while(L!=nullptr){
    ct++;
    L=Cdr(L);
  }
  return ct;
}

node list_pop(node *stack){
   node temp;
   temp=Car(*stack);
   *stack=Cdr(*stack);
   return temp;
}
  
node list_first(node list){
  return Car(list);
}

node list_rest(node list){
  return Cdr(list);
}

int list_empty(node list){
  if (list==nullptr) return TRUE;
  else return FALSE;
}

/* 
** Inserts a node g onto a list L in order (according to a comparison
** function comp) -- IF There is allready and equivalent object on
** the list and uniq is TRUE it is not inserted, and a value of zero 
** is returned, otherwise it is inserted and a value of 1 is returned.
*/

int list_insert(node g, node * L, int (*comp) (node, node), int uniq)
{
  int flg = 1;
  node ptr = *L;
  LOCS(2);
  PUSH_LOC(*L);
  PUSH_LOC(g);

  if ((*L == nullptr) || ((flg = comp(g, Car(*L))) >= 0)) {
    if (uniq==FALSE || flg != 0) *L = Cons(g, *L);
  } 
  else {
    while ((Cdr(ptr) != nullptr) &&
      ((flg = comp(g, Car(Cdr(ptr))))< 0)) {
       ptr = (node) Cdr(ptr);
    }
    if (uniq==FALSE || flg != 0)
      node_set_ptr(ptr,Cons(g, Cdr(ptr)), NODE, RIGHT);
  }
  POP_LOCS();
  return flg;
}

/* 
** list remove -- remove a specific elt from a list 
*/

int list_Imatrix_comp(node g1, node g2)
{
    if (g1 == nullptr || g2 == nullptr ||
    node_get_type(g1, LEFT) != IMTX || node_get_type(g2, LEFT) != IMTX) {
	bad_error("Non-IMTX nodes in sexpr_IMTX_comp()");
    }
    return Imatrix_order((struct Imatrix_t *) Car(g1),
			 (struct Imatrix_t *) Car(g2));
}


node list_cat(node l1,node l2){
  node ptr=l1;
  if (ptr==nullptr) return l2;
  while(Cdr(ptr)!=nullptr) ptr=Cdr(ptr);
  node_set_ptr(ptr,(void *)l2,NODE,RIGHT);
  return l1;
}



void xpl_fprint(FILE *fout,node L){
 node ptr;
 Dmatrix P;
 int i;
#ifdef LOG_PRINT
 fprintf(fout, "%% Solution list :\n");
 fprintf(fout, 
 "%%      Re(H) ,       Im(H) ,      Re(Xi) ,      Im(Xi) ,          T  \n");
 fprintf(fout, "%%\n");
 fprintf(fout,"{\n")
 #endif
;
ptr=L;
 while(ptr!=nullptr){
   P=(Dmatrix)(Car(Car(ptr)));
 #ifdef LOG_PRINT
  fprintf(fout,"<");
   fprintf(fout,"%12g, %12g,",DVref(P,1),DVref(P,2))
#endif
;
   for(i=3;i<DVlength(P);i++){
#ifdef LOG_PRINT
         fprintf(fout," %12g,",DVref(P,i))
#endif
; 
         if (i%2==0 && i<DVlength(P)-1)
      #ifdef LOG_PRINT
           fprintf(fout,"\n                            ")

#endif
;
   }
 #ifdef LOG_PRINT
  fprintf(fout," %12g >",DVref(P,i))

#endif

;
   ptr=Cdr(ptr);
   if (ptr!=nullptr) 
#ifdef LOG_PRINT
fprintf(fout,",")

#endif
;
 #ifdef LOG_PRINT
  fprintf(fout,"\n")
#endif
;
 }
#ifdef LOG_PRINT
 fprintf(fout,"};")
#endif
;
}

/* end Lists.c */

/**************************************************************************/
/********************** implementations from Pconfig.c ********************/
/**************************************************************************/

/*
** Pconfig.c                                Birk Huber 2-1-1994
**
**  Module for points and point configurations in affine space.
**
**  A point are represented by a node with a label(cstring) on the
**     left and a vector of coordinates(Imatrix) on the right.
**    
**       -----------------------
**       | PNT      | IMTX     |
**       -----------------------
**       |(char *)  | Imatrix  | ------> structure holding coords
**       -----------------------
**           |------------------------> string
** 
** A point configuration is represented by a node with a list of 
**      points on the right and the number of points in the list 
**      stored on the 
**      left.
**
**       -----------------------
**       | PCFG     |  NODE    |    
**       -----------------------        ------------------
**       | int      | Imatrix  | ------>| NODE   | NODE  | 
**       -----------------------        ------------------
**                                      | node   | node  | ----->
**                                      ------------------
**                                         |
**                                        point 1
 */

node pcfg_start(node P){ return Cdr(P);}
node pcfg_next_pnt(node *ptc){ 
    node pt;
    pt=Car(*ptc);
    *ptc=Cdr(*ptc);
    return pt;}

/*
** node pnt_new(char *s,Imatrix m)
**       input:  a string s 
**               an integer matrix m
**      output: a point with lable s, and coordinates m
*/
node pnt_new(char *s, Imatrix m)
{
    node R;
    R = node_new();
    node_set_ptr(R, s, PNT, LEFT);
    node_set_ptr(R, m, IMTX, RIGHT);
    return R;
}
/*
** pnt_free
**    input: a node
**   output: none
** effects: frees space allocated for lable and coordinates.
**          resets fields of node to null nodeptrs.
** error :  checks that n is a non-null pnt.
*/
void pnt_free(node n)
{
    if (n==nullptr||node_get_type(n, LEFT) == PNT) {
        mem_free((char *) node_get_ptr(n, LEFT));
        Imatrix_free((Imatrix) node_get_ptr(n, RIGHT));
        node_set_ptr(n, (void *) nullptr, NODE, RIGHT);
        node_set_ptr(n, (void *) nullptr, NODE, LEFT);
    } else
        bad_error("error: pnt_free() called on non-point\n");
}
/*
** default display for points.
*/
int pnt_print(node n)
{
    if (n==nullptr||node_get_type(n, LEFT) == PNT) {
	printf("(");
	printf("%s", (char *) node_get_ptr(n, LEFT));
	printf(",");
	Imatrix_print((Imatrix) node_get_ptr(n, RIGHT));
	printf(")\n");
	return 0;
    }
    bad_error("error: pnt_print() called on non-point\n");
    return 1;			/*bad error causes abort */
}

/*
** pnt_is_point  
**       input: a node n
**       output:  TRUE if n is a point (and non-null)
**               FALSE  otherwise
*/
int pnt_is_point(node n){ 
      if (n!=nullptr&&node_get_type(n,LEFT)==PNT) return TRUE;
      else return FALSE;
}
/*
** pnt_comp_lbl
**       input: two points;
**      output: value of strcmp on the lables of pt1 and pt2.
**      error conditions: checks that g1, and g2 are (non-null) points
*/
int pnt_comp_lbl(node g1, node g2){
  if ((pnt_is_point(g1)!=TRUE)||(pnt_is_point(g2)!=TRUE)){
    bad_error("Non-PNT nodes in pnt_comp_lbl()");
  }
  return strcmp(pnt_lable(g2),pnt_lable(g1));
}

/* 
** pnt_comp_rand
**    input:  two points
**   output:  1 or -1 randomly chosen.
**   error conditions: checks that g1 and g2 are (non-null)points
**
** NOTE: this is used to randomize point lists
*/
int pnt_comp_rand(node g1, node g2){
    if ((pnt_is_point(g1)!=TRUE)||(pnt_is_point(g2)!=TRUE)){
    bad_error("Non-PNT nodes in pnt_comp_rand()");
  }
  if (rand_int(0,1)==0) return -1;
  else return 1;
}

int pnt_comp_const(node g1, node g2){
  if ((pnt_is_point(g1)!=TRUE)||(pnt_is_point(g2)!=TRUE)){
    bad_error("Non-PNT nodes in pnt_comp_rand()");
  }
  return 1;
}

/*
** pnt_lable(node n);
**         input: a point n
**        outpur: the lable associated with n
**         error: checks that n is a (non-null) point
*/
char *pnt_lable(node n)
{
    if (n == nullptr || node_get_type(n, LEFT) != PNT) {
	bad_error("error: pnt_label() called on non-point\n");
	/* will cause an exit */
    }
    return (char *) node_get_ptr(n, LEFT);
}

/*
** POINT CONFIGURATIONS
*/
/* 
** pcfg_new
**      input: none
**     output: an empty point configuration.
*/
node pcfg_new()
{
    node g;
    g = node_new();
    node_set_int(g, 0, PCFG, LEFT);
    node_set_ptr(g, (void *) nullptr, NODE, RIGHT);
    return g;
}

node pcfg_print(node n)
{
    if (n != nullptr && node_get_type(n, LEFT) == PCFG) {
	printf("<");
	node_print((node)node_get_ptr(n, RIGHT));
	printf(">");
    } else
	bad_error("error: pcfg_print() called on non-PCFG\n");
    return n;
}

node pcfg_print_short(node n)
{
    if (n == nullptr || node_get_type(n, LEFT) != PCFG)
	bad_error("error: pcfg_print_short() called on non-PCFG\n");
    n = (node) node_get_ptr(n, RIGHT);
    printf("<");
    while (n != nullptr) {
	printf("%s ", (char *) node_get_ptr(Car(n), LEFT));
	n = (node) node_get_ptr(n, RIGHT);
    }
    printf(">");
    return n;
}

/*
** pcfg_add
**   input:  a point "point"
**           a point config "config"
**   output: TRUE on successfull completion
**           (right now failure causes the program to abort)
**  side effects: point is added into config 
**                (which is kept in order according to point lables)
*/
int pcfg_add(node point, node config)
{   node ptr;
    LOCS(1);
    PUSH_LOC(config);
    if (config == nullptr || 
         point == nullptr || 
         node_get_type(config, LEFT) != PCFG ||
	 node_get_type(point, LEFT) != PNT){
	    bad_error("error: pcfg_add() called on non-PCFG\n");
    }
    ptr=Cdr(config);
    list_insert(point,&ptr,&(pnt_comp_lbl),FALSE);
    node_set_ptr(config,ptr,NODE,RIGHT);
    node_set_int(config, node_get_int(config, LEFT) + 1, PCFG, LEFT);
    POP_LOCS();
    return TRUE;
}
/*
** pcfg_remove
**   input:  a point "point"
**           a point configuration "config"
**           a pointer ptr locating "point" in "config" i.e.
**   output: TRUE if point is in config  
**           FALSE if point is not in config
**  side effect: if point is in config it is removed from config.
**  error conditions: checks that config is non-null pcfg
**                                pnt is non-null pnt
**  Note: ptr should point to the cons node for the point before
**        "point" in config's point list, if it doesn't then 
**        pcfg_remove will find the previous point itself
**           
*/
int pcfg_remove(node point, node config, node ptr)
{
  if (config == nullptr ||point == nullptr||
        node_get_type(config, LEFT) != PCFG ||
        node_get_type(point, LEFT) != PNT)
     bad_error("error: pcfg_add() called on non-PCFG\n");
 /* check if location given for point is really right*/
  if (Car(Cdr(ptr))!=point) ptr=nullptr;  
 /* find location of point if it is not allready known*/
  if (ptr==nullptr)
     for(ptr=Cdr(config); Car(ptr)!=point && ptr!=nullptr; ptr=Cdr(ptr)){
        ; /* null body*/
     }
  if (ptr==nullptr) return FALSE; 
  node_set_ptr(ptr,Cdr(Cdr(ptr)),NODE,RIGHT);
  node_set_int(config,node_get_int(config,LEFT)-1,PCFG,LEFT);
  return TRUE;
}

/*
**pcfg_in(node point, node config);
**  test if point is in config. point must be given by a pointer
**  to a point in config. 
**
** Warning: Point must be given by a pointer to a point in config.
**          Point_config_in DOES NOT TELL wheather their is a point 
**          in Config with same coords/lable.
*/
int pcfg_in(node point, node config)
{
    if (config == nullptr || point == nullptr || node_get_type(config, LEFT) != PCFG ||
	node_get_type(point, LEFT) != PNT)
	bad_error("error: pcfg_in() called on non-PCFG\n");
    while ((config = Cdr(config)) != nullptr)
	if (point == Car(config))
	    return TRUE;
    return FALSE;
}

/* 
** Imatrix pcfg_coords(node n, Imatrix R)
**    returns a matrix whoose rows are the coordinates of
**    points in the configuration. Space for the matrix is
**    allocated from R if possible, otherwise R is freed and
**    space realocated.
*/

Imatrix pcfg_coords(node n, Imatrix R)
{
    int i, j;
    node ptr = n;
    if (n == nullptr || node_get_type(n, LEFT) != PCFG)
	bad_error("error: pcfg_coords() called on non-PCFG\n");
    R = Imatrix_resize(R, pcfg_npts(n), pcfg_dim(n));
    for (i = 1; i <= pcfg_npts(n); i++) {
	ptr = Cdr(ptr);
	for (j = 1; j <= pcfg_dim(n); j++)
	    *(IMref(R, i, j)) = *(IVref(pnt_coords(Car(ptr)), j));
    }
    return R;
}

/* 
** Imatrix pcfg_M(node n, Imatrix R)
**    returns a matrix whoose rows are the coordinates of
**    points in the configuration, after translation to put the 
**    first point at the origen.    Space for the matrix is
**    allocated from R if possible, otherwise R is freed and
**    space realocated.
*/
Imatrix pcfg_M(node n, Imatrix R)
{
    int i, j;
    Imatrix P0;
    node ptr = n;



    if (n == nullptr || node_get_type(n, LEFT) != PCFG)
	bad_error("error: pcfg_M called on non-PCFG\n");
    R = Imatrix_resize(R, pcfg_npts(n) - 1, pcfg_dim(n));
    ptr = Cdr(ptr);
    P0 = pnt_coords(Car(ptr));
    for (i = 1; i <= pcfg_npts(n) - 1; i++) {
	ptr = Cdr(ptr);
	for (j = 1; j <= pcfg_dim(n); j++)
	    *(IMref(R, i, j)) = *(IVref(pnt_coords(Car(ptr)), j))
		- *(IVref(P0, j));
    }
    return R;
}
/* 
** node pcfg_face(node PC, Imatrix norm);
**   returns the point configuration consisting of points 
**   of PC which lie on the face suported by N.
*/
node pcfg_face(node PC, Imatrix norm)
{
    node Face = nullptr, ptr = nullptr;
    Imatrix M = nullptr, D = nullptr;
    int s, i;
    LOCS(2);

    /* save locals */
    PUSH_LOC(PC);
    PUSH_LOC(Face);
    M = pcfg_coords(PC, M);
    D = Imatrix_dot(norm, M, D);
    s = *IMref(D, 1, 1);
    for (i = 2; i <= IMcols(D); i++)
	if (*IMref(D, 1, i) < s)
	    s = (*IMref(D, 1, i));
    Face = pcfg_new();
    ptr = PC;
    for (i = 1; i <= IMcols(D); i++) {
	ptr = Cdr(ptr);
	if (*IMref(D, 1, i) == s)
	    pcfg_add(Car(ptr), Face);
    }
    Imatrix_free(D);
    Imatrix_free(M);
    POP_LOCS();
    return Face;
}
/*
**  is_normal_good(Imatrix normal, Imatrix N)
**  determine if the (inner normal) associated to a facet
**  satasfies the sign conditions specified in N.
**
**  each coordinate of the (inner)normal satasfy the condition
**  specified by the corresponding coordinate of N:
**           1:   normal[i]  must be non-negative
**           2:   normal[i]  must be positive
**          -1:   normal[i]  must be non-positive
**          -2:   normal[i]  must be negative
**           0:   no condition required of normal[i]
*/

int is_normal_good(Imatrix normal, Imatrix N)
{
    int i;
    if (N == nullptr)
	return TRUE;
    for (i = 1; i <= IVlength(N); i++) {
	if (*(IVref(N, i)) == 0)	/* Skip */
	    ;
	else if (*(IVref(N, i)) * (*IVref(normal, i)) < 0)
	    return FALSE;
	else if ((*(IVref(normal, i)) == 0) && (abs(*IVref(N, i)) == 2))
	    return FALSE;
    }
    return TRUE;
}

/* end Pconfig.c */

/**************************************************************************/
/******************** implementations from Pcomplex.c *********************/
/**************************************************************************/

#ifndef PI
#define PI (double)3.14159265358979323846264338328
#endif


fcomplex Cadd(fcomplex a,fcomplex b)
{	fcomplex c;
	c.r=a.r+b.r;
	c.i=a.i+b.i;
	return c;
}

fcomplex Csub(fcomplex a,fcomplex b)
{	fcomplex c;
	c.r=a.r-b.r;
	c.i=a.i-b.i;
	return c;
}

fcomplex Cmul(fcomplex a,fcomplex b)
{	fcomplex c;
	c.r=a.r*b.r-a.i*b.i;
	c.i=a.i*b.r+a.r*b.i;
	return c;
}

              
fcomplex ItoC(int i)
{ fcomplex c;
  c.r=i;
  c.i=0.0;
  return c;
}

fcomplex DtoC(double i)
{ fcomplex c;
  c.r=i;
  c.i=0.0;
  return c;
}

fcomplex Complex(double re, double im)
{	fcomplex c;
	c.r=re;
	c.i=im;
	return c;
}

fcomplex Conjg(fcomplex z)
{	fcomplex c;
	c.r=z.r;
	c.i = -z.i;
	return c;
}

fcomplex Cdiv(fcomplex a,fcomplex b)
{	fcomplex c;
	double r,den;
	if (fabs(b.r) >= fabs(b.i)) {
		r=b.i/b.r;
		den=b.r+r*b.i;
		c.r=(a.r+r*a.i)/den;
		c.i=(a.i-r*a.r)/den;
	} else {
		r=b.r/b.i;
		den=b.i+r*b.r;
		c.r=(a.r*r+a.i)/den;
		c.i=(a.i*r-a.r)/den;
	}
	return c;
}

fcomplex Cpow(fcomplex a, int d)
  { int i;
    fcomplex c;
    c.r=1.0;
    c.i=0.0;
    if (d<0) { a=Cdiv(c,a);
               d*=-1;}
    for (i=1;i<=d;i++) c=Cmul(c,a);
    return c;
    }
double Cabs(fcomplex z)
{	double x,y,ans,temp;
	x=fabs(z.r);
	y=fabs(z.i);
	if (x == 0.0)
		ans=y;
	else if (y == 0.0)
		ans=x;
	else if (x > y) {
		temp=y/x;
		ans=x*sqrt(1.0+temp*temp);
	} else {
		temp=x/y;
		ans=y*sqrt(1.0+temp*temp);
	}
	return ans;
}

fcomplex Csqrt(fcomplex z)
{	fcomplex c;
	double x,y,w,r;
	if ((z.r == 0.0) && (z.i == 0.0)) {
		c.r=0.0;
		c.i=0.0;
		return c;
	} else {
		x=fabs(z.r);
		y=fabs(z.i);
		if (x >= y) {
			r=y/x;
			w=sqrt(x)*sqrt(0.5*(1.0+sqrt(1.0+r*r)));
		} else {
			r=x/y;
			w=sqrt(y)*sqrt(0.5*(r+sqrt(1.0+r*r)));
		}
		if (z.r >= 0.0) {
			c.r=w;
			c.i=z.i/(2.0*w);
		} else {
			c.i=(z.i >= 0) ? w : -w;
			c.r=z.i/(2.0*c.i);
		}
		return c;
	}
}

fcomplex RCmul(double x, fcomplex a)
{	fcomplex c;
	c.r=x*a.r;
	c.i=x*a.i;
	return c;
}

void printC(fcomplex c)
{	if (c.r!=0 && c.i !=0) {
		if (c.i>0) printf("(%g+%g*I)",c.r,c.i);
		else  printf("(%g-%g*I)",c.r,fabs(c.i));
	}
	else if (c.r!=0) printf("%g",c.r);
	else if (c.i!=0) printf("(%g*I)",c.i);
        else printf("0.0");
}

/*-------------------------------------------------------------------------
function  (fcomplex) Croot
  arguments:   n an integer;
               z a complex number;
  returns:     a primitive nth root of z;
-----------------------------------------------------------------------*/


fcomplex RootOfOne(int j, int n)
{ fcomplex C;
  C=Complex(cos((2.0*PI*j)/n),sin((2.0*PI*j)/n));
  return C;}



/*-------------------------------------------------------------------------
function  (fcomplex) Croot
  arguments:   n an integer;
               z a complex number;
  returns:     a primitive nth root of z;
-----------------------------------------------------------------------*/
fcomplex Croot(fcomplex z, int n)
  {   fcomplex w;
      double mod,arg,ni=1.0/n;

      if ((z.i==0)&&(z.r==0)) w=Complex(0.0,0.0);
         else{ mod=Cabs(z);
               if (fabs(z.i)>=fabs(z.r)){
                      arg=acos(z.r/mod);
                      if (z.i<0) arg=-1*arg;}
                else { arg=asin(z.i/mod);
                      if (z.r<0) arg=PI-arg;}
               arg=arg/n;
               mod=fabs(pow(mod,ni));
               w=Complex(cos(arg),sin(arg));
               w=RCmul(mod,w); }

     return w;}

/* end Pcomplex.c */

/**************************************************************************/
/*********************** implementations from Poly.c **********************/
/**************************************************************************/

void bad_error(const char *);

int *poly_exp(monomial m, int i){
  if (m==nullptr || i<1 || m->R->n < i ) {
      printf("i=%d, n=%d\n",i,m->R->n);
      bad_error("index out of bounds in monomial");
 }
  return &(m->exps[i-1]);
}   

int poly_deg(polynomial1 p){
    int i,n,deg=0,tdeg;
    n=p->R->n;
    for(i=0;i<n;i++) deg+=p->exps[i];
    while((p=p->next)!=nullptr){
      tdeg=0;
      for(i=0;i<n;i++) tdeg+=p->exps[i];
      if (tdeg>deg) deg=tdeg;
    }
   return deg;
}

int *poly_homog(polynomial1 p){
 return &(p->homog);
}

void ring_set_var(Pring R, int n, char *lable){
  strncpy(const_cast<char *>(R->vars[n]),lable,RING_VAR_L);
}
char *ring_var(Pring R,int i){return const_cast<char *>(R->vars[i]);}
void ring_set_def(Pring R,  char *lable){
   strncpy(R->def,lable,RING_VAR_L);
}
char *ring_def(Pring R){return R->def;}
int poly_dim(monomial m){  
  if (m==nullptr) bad_error("null monomial in poly_def");
  return m->R->n;
}
int ring_dim(Pring R){  
  if (R==nullptr) bad_error("null Ring in ring_dim");
  return R->n;
}
int *poly_def(monomial m){  
  if (m==nullptr) bad_error("null monomial in poly_def");
  return &(m->def);
}
fcomplex *poly_coef(monomial m){
  if (m==nullptr) bad_error("null monomial in poly_coef");
  return &(m->coef);
}
 monomial poly_next(monomial m){
  if (m==nullptr) bad_error("null monomial in poly_next");
  return m->next;
}
monomial poly_set_next(monomial m,monomial m2){
  if (m==nullptr) bad_error("null monomial in poly_next");
  return (m->next=m2);
}

Pring poly_ring(monomial m){
  if (m==nullptr) bad_error("null monomial in poly_next");
  return m->R;
}


Pring makePR(int n)

{ 
	int i;
	Pring a;
	a=(Pring)mem_malloc(sizeof(struct Pring_tag)); 
	if (a==nullptr) bad_error("malloc failed 1 in make_PR");
	a->n=n;
	a->vars=(char **)mem_malloc(n*sizeof(char *));  
	if (a->vars==nullptr) bad_error("malloc failed 2 in make_PR");
	for(i=0;i<n;i++) { 
		a->vars[i]=(char *)mem_malloc(RING_VAR_L*sizeof(char)); 
		if (a->vars==nullptr) bad_error("malloc failed 3 in make_PR");
	}
        a->def=(char *)mem_malloc(RING_VAR_L*sizeof(char)); 
        if (a->vars==nullptr) bad_error("malloc failed 4 in make_PR");
    return a;
}

Pring free_Pring(Pring R)
 {
 int i;
 mem_free(R->def);  
 for (i=0;i<R->n;i++){ mem_free(R->vars[i]);}
 mem_free( (char *) R->vars);
 mem_free( R); 
 return nullptr;
 }
       
polynomial1 makeP(Pring R)

{ 
  polynomial1 m;
  int i,n;

  n=R->n;
  m=(polynomial1)mem_malloc(sizeof(struct mono_tag));
  if (m==nullptr){ printf("malloc 1 failed: makeP\n"); exit(2); }
  m->R=R;
  m->exps=(int *)mem_malloc(n*sizeof(int));
  if (m->exps==nullptr){printf(" malloc 2 failed: makeP\n"); exit(2);}
  for(i=0;i<n;i++) m->exps[i]=0;
  m->coef=Complex(0.0,0.0);
  m->def=0;
  m->next=nullptr;
  return m;
}
     
polynomial1 freeP(polynomial1 p)
{
  polynomial1 m=p;
  while (m!=nullptr){
      p=p->next;
      mem_free((char *)m->exps); 
      mem_free((char *)m); 
      m=p;
  }
  return nullptr;
}

polynomial1 copyM(polynomial1 p)
{
  polynomial1 p1;
  int i;
  if (p==nullptr) return nullptr;
  p1=makeP(p->R);
  p1->coef=p->coef;
  p1->def=p->def;
  for (i=0;i<p->R->n;i++) p1->exps[i]=p->exps[i];
 return p1;
}

 
polynomial1 copyP(polynomial1 p)
{   
  polynomial1 p1,p2;
  int i;
	
  if (p==nullptr) return nullptr;
  p1=makeP(p->R);
  p2=p1;
  while (p!=nullptr){
    if (p->next!=nullptr) p2->next=makeP(p->R);
    p2->coef=p->coef;
    p2->def=p->def;
    for (i=0;i<p->R->n;i++) p2->exps[i]=p->exps[i];
    p=p->next;
    p2=p2->next;
  }
 return p1;
}
    
void printP(polynomial1 P)
{ 
  int i,zt=0;

  /* DEBUG */
  /* printf(" In "); */
 
  while (P!=nullptr){
     if (P->coef.i!=0 || P->coef.r!=0){
        printC(P->coef);
        zt=1;

	/*	printf("("); */

        for (i=0;i<P->R->n;i++) { 
      
	  /*       printf("\n(P->exps([%d])= %d\n", i,P->exps[i]);    */
	  /*
	  printf("%d", P->exps[i]);
	  if (i == P->R->n - 1)
	    printf(")");
	  else
	    printf(",");
	  */

       if (P->exps[i]!=0) {
           zt=1;
	   
	   printf("*%s",P->R->vars[i]);
    	   if (P->exps[i]!=1) {
	     /*              printf("^{without P->def-loop}");   */
              if (P->exps[i]>0) 
		printf("^%d",P->exps[i]);
              else 
		printf("(%d)",P->exps[i]);
           }
        }
     }
     if (P->def!=0){
 

      printf("*%s\n",P->R->def);
        if (P->def !=1) {
             printf("^{with  P->def-loop}");
             if (P->def >0) printf("%d ",P->def);
              else printf("(%d){second} ",P->def);
             }
     } 
     /*     if (P->next != 0) printf("+{new mono}\n");   */
     if (P->next != nullptr) printf(" + ");
     }
     P=P->next;
  }
  if (zt==0) printf("0.0nana");

  /* DEBUG */
  /* printf("  Out  "); */
} 

int orderMM(polynomial1 P1,polynomial1 P2)
{
  int i,d;
  if (P1==nullptr && P2==nullptr) return 0;
   else if (P1==nullptr) return -1;
    else if (P2==nullptr) return 1;
  if (P1->R != P2->R) 
    bad_error("error in order: monomials must belong to same ring");
	
  d=P1->def-P2->def;
  if (d>0) return 1;
  else if (d<0) return -1;

  for (i=0;i<P1->R->n;i++) { 
    d=P1->exps[i]-P2->exps[i];
    if (d>0) return 1;
     else if (d<0) return -1;
  }
  return 0;
}

int orderPP(polynomial1 P1,polynomial1 P2)
{
	int i,d;
	while (P1!=nullptr && P2!=nullptr){
                 d=P1->def-P2->def;
                 if (d>0) return 1;
                 else if (d<0) return -1;

		for (i=0;i<P1->R->n;i++) {
                	d=P1->exps[i]-P2->exps[i];
               		 if (d>0) return 1;
                	else if (d<0) return -1;
        	}
                P1=P1->next;
                P2=P2->next;
	}
        if (P1!=nullptr) return 1;
         else if (P2 !=nullptr) return -1;
          else return 0;
}
   
polynomial1 ItoP(int c,Pring R)
{ 
  polynomial1 p;
  p=makeP(R);
  p->coef.r=c;
  return p;
}

polynomial1 DtoP(double c, Pring R)
{ 
  polynomial1 p;
  p=makeP(R);
  p->coef.r=c;
  return p;
}

polynomial1 CtoP(fcomplex c, Pring R)
{ 
  polynomial1 p;
  p=makeP(R);
  p->coef=c;
  return p;
}
 
polynomial1 addPPP(polynomial1 P1, polynomial1 P2, polynomial1 P3)
{
  polynomial1 pt;
  struct mono_tag A;
  int d,i;
	
  pt=&(A);
if (P1==nullptr){ if (P3==nullptr) freeP(P3);
            return copyP(P2);
          }
if (P2==nullptr){ if (P3==nullptr) freeP(P3);
            return copyP(P1);
          }
/*
if (P1->R != P2->R) bad_error(" polynomial1s in addPPP must have equal rings");
*/ 
 A.R=P1->R;
  A.next=nullptr;
  while(P1!=nullptr&&P2!=nullptr){
    d=orderMM(P1,P2);
    if (d==0) {
          A.coef=Cadd(P1->coef,P2->coef);
          if (Cabs(A.coef)!=0){
            pt->next=makeP(A.R);
            pt=pt->next;
            pt->coef=A.coef;
            pt->def=P1->def;
            for(i=0;i<A.R->n;i++)pt->exps[i]=P1->exps[i];
          }
          P1=P1->next;
          P2=P2->next;
    }
    else if (d==-1){
          pt->next=makeP(A.R);
          pt=pt->next;
          pt->coef=P2->coef;
          for(i=0;i<A.R->n;i++)pt->exps[i]=P2->exps[i];
          pt->def=P2->def;
          P2=P2->next;
          }
    else {
          pt->next=makeP(A.R);
	  pt=pt->next;
          pt->def = P1->def;
          pt->coef=P1->coef;
          for(i=0;i<A.R->n;i++)pt->exps[i]=P1->exps[i];
          P1=P1->next;
	}
  }

if (P1!=nullptr) pt->next=copyP(P1);
 else if (P2!=nullptr) pt->next=copyP(P2);

if (P3!=nullptr) freeP(P3);

if (A.next==nullptr) A.next=makeP(A.R);
return A.next;
}		

polynomial1 subPPP(polynomial1 P1, polynomial1 P2, polynomial1 P3)
{
  polynomial1 pt;
  struct mono_tag A;
  int d,i;

  pt=&(A);
if (P1!=nullptr || P2!=nullptr){
  if (P1->R != P2->R) bad_error(" polynomial1s in addPPP must have equal rings");
  A.R=P1->R;
  A.next=nullptr;
  while(P1!=nullptr&&P2!=nullptr){
    d=orderMM(P1,P2);
    if (d==0) {
          A.coef=Csub(P1->coef,P2->coef);
          if (Cabs(A.coef)!=0){
            pt->next=makeP(A.R);
            pt=pt->next;
            pt->coef=A.coef;
            pt->def=P1->def;
            for(i=0;i<A.R->n;i++)pt->exps[i]=P1->exps[i];
          }
          P1=P1->next;
          P2=P2->next;
    }
    else if (d==-1){
          pt->next=makeP(A.R);
          pt=pt->next;
          pt->coef=RCmul(-1.0,P2->coef);
          pt->def=P2->def;
          for(i=0;i<A.R->n;i++)pt->exps[i]=P2->exps[i];
          P2=P2->next;
          }
    else {
          pt->next=makeP(A.R);
          pt=pt->next;
          pt->coef=RCmul(1.0,P1->coef);
          pt->def=P1->def;
          for(i=0;i<A.R->n;i++)pt->exps[i]=P1->exps[i];
          P1=P1->next;
        }
  }
}
if (P1!=nullptr) pt->next=copyP(P1);
 else if (P2!=nullptr) pt->next=mulCPP(Complex(-1.0,0.0),P2,nullptr);

if (P3!=nullptr) freeP(P3);

if (A.next==nullptr) A.next=makeP(A.R);
return A.next;
}

polynomial1 mulCPP(fcomplex c, polynomial1 P1, polynomial1 P2)
{
  polynomial1 p;

  if (P1==nullptr) { if (P2!=nullptr) freeP(P2);
                          return nullptr;}
  if (P2!=P1 ) { if ( P2 !=nullptr)  freeP(P2); 
                 P2=copyP(P1);}
  p=P2;
  while (p!=nullptr){
    p->coef=Cmul(c,p->coef);
    p=p->next;
  }
 return P2;
}

polynomial1 divCPP(fcomplex c, polynomial1 P1, polynomial1 P2)
{
  polynomial1 p;

  if (P1==nullptr) { if (P2!=nullptr) freeP(P2);
                          return nullptr;}
  if (P2!=P1 ) { if ( P2 !=nullptr)  freeP(P2);
                 P2=copyP(P1);}
  p=P2;
  while (p!=nullptr){
    p->coef=Cdiv(p->coef,c);
    p=p->next;
  }
 return P2;
}


polynomial1 mulMPP(polynomial1 mi, polynomial1 P1, polynomial1 P2)
{
  polynomial1 p,m=mi;
  int i,free_m=0;

m->R=P1->R;

if (P1==nullptr||m==nullptr) { if (P2!=nullptr) freeP(P2);
                      return nullptr;}
  if (P1->R!=m->R){ printf("\nRings must be equal in mulMPP");
                   if (P2!=nullptr) freeP(P2);
                          return nullptr;}
  if (P2==m) free_m = 1;
  if (P2!=P1 ) { if ( P2 !=nullptr && P2 !=m)  freeP(P2); 
                 P2=copyP(P1);}
  p=P2;
  while (p!=nullptr){
    p->coef=Cmul(m->coef,p->coef);
    p->def+=m->def;
    for(i=0;i<m->R->n;i++) p->exps[i]+=m->exps[i];
    p=p->next;
  }
 if (free_m==1) freeP(m);
 return P2;
}

polynomial1 divMPP(polynomial1 mi, polynomial1 P1, polynomial1 P2)
{
  polynomial1 p,m=mi;
  int i,free_m=0;

  if (P1==nullptr||m==nullptr) { if (P2!=nullptr) freeP(P2);
                      return nullptr;}
  if (P1->R!=m->R){ printf("Rings must be equal in divMPP");
                   if (P2!=nullptr) freeP(P2);
                          return nullptr;}
  if (m->next!=nullptr){ printf("divisor must be a monomial in divMPP");
                   if (P2!=nullptr) freeP(P2);
                    return nullptr;}

  if (P2==m) free_m = 1;
  if (P2!=P1 ) { if ( P2 !=nullptr && P2 !=m)  freeP(P2); 
                 P2=copyP(P1);}
  p=P2;
  while (p!=nullptr){
    p->coef=Cdiv(p->coef,m->coef);
    p->def-=m->def;
    for(i=0;i<m->R->n;i++) p->exps[i]-=m->exps[i];
    p=p->next;
  }
 if (free_m==1) freeP(m);
 return P2;
}


polynomial1 mulPPP(polynomial1 P1, polynomial1 P2, polynomial1 P3)
{
  polynomial1 pt2,pt3=nullptr;
    
  if (P1==nullptr || P2==nullptr) { 
    if (P3!=nullptr) freeP(P3);
    return nullptr;
  }
  /*
  if (P1->R != P2->R) {
    if (P3!=0) freeP(P3);
    bad_error("error in addPP: unequal rings");
  }
  */
  while(P1!=nullptr) { 
    pt2=mulMPP(P1,P2,nullptr);
    pt3=addPPP(pt2,pt3,pt3);
    freeP(pt2);               /* this realy should be done more efficiently */
    P1=P1->next;
    }

 if (P3!=nullptr) free(P3);
 return pt3;
}


			
polynomial1 expIPP(int x, polynomial1 P, polynomial1 P3)
{
  polynomial1 m,tmp;
      int i;
      if (P==nullptr) bad_error("undefined poly in powPIP");
      if (x==0) {m=makeP(P->R);
                 m->coef.r=1.0;
                 if (P3!=nullptr) freeP(P3);
                 return m;}
      if (P->next==nullptr) { m=copyP(P);
                       m->coef=Cpow(m->coef,x);
                       m->def*=x;
                       for(i=0;i<m->R->n;i++) m->exps[i]*=x;
                       if(P3!=nullptr) freeP(P3);
                       return m;}
      if (x<0) bad_error("powPIP can not raise a polynomial1 to a negative power");
      m=copyP(P);
      for(i=2;i<=x;i++) {
                       tmp=mulPPP(m,P,nullptr);
                       freeP(m);
                       m=tmp;}
       if (P3!=nullptr) freeP(P3);
       return m;
       }
       
polynomial1 unliftP(polynomial1 p)
{
  polynomial1 p1,p2;
  int i;

  if (p==nullptr) return nullptr;
  p1=makeP(p->R);
  p2=p1;
  while (p!=nullptr){
    if (p->next!=nullptr) p2->next=makeP(p->R);
    p2->coef=p->coef;
    p2->def=0;
    for (i=0;i<p->R->n;i++) p2->exps[i]=p->exps[i];
    p=p->next;
    p2=p2->next;
  }
 return p1;
}

polynomial1 Homogenize(polynomial1 pi,Pring R)
{ 
  polynomial1 pt,p;
  int i = 0;
  int d = 0;
  int dp = 0;

  if (pi==nullptr) bad_error("Homogenize called on null polynomial1");
  p=copyP(pi);
  pt=p;
  while (pt!=nullptr){
    d=0;
    for (i=0;i<p->R->n-1;i++) d+=pt->exps[i];
    if (p==pt) dp=d;
      else if (dp<d) dp=d;
    pt->R=R;
    pt=pt->next;
  }
  pt=p;
  while (pt!=nullptr){
    d=0;
    for (i=0;i<p->R->n-1;i++) d+=pt->exps[i];        
    pt->exps[p->R->n-1]=dp-d;
    pt=pt->next;
  }
return p;}

polynomial1 Prog_Eq(Pring R){
 polynomial1 p,pt;
 int i;
 pt=(p=ItoP(-1,R));
 for(i=0;i<R->n;i++) {
   pt->next=ItoP(1,R);
   pt=pt->next;
   pt->exps[i]=2;
 }
 return p;
}

/* end Poly.c */

/**************************************************************************/
/********************** implementations from utime.c **********************/
/**************************************************************************/

int set_mark(){
  return clock();
}

int read_mark(int timeset){
 return (clock()-timeset) / CLOCKS_PER_SEC;
}

/* end utime.c */

/**************************************************************************/
/*********************** implementations from Aset.c **********************/
/**************************************************************************/

/* 
** An Aset is an r-tupple of point configurations. It represents
** the support set of a polynomial system, and is a presentation for
** the minkowski sum of the convex hulls of its constituent
** point configurations. 
** 
** Asets are constructed from nodes as follows:
** _____________     ____________     _____________       ------------
**| ASET | NODE |   | INT | NODE |   | NODE | NODE |     | NODE | NODE
** -------------     ------------     -------------       ------------
**|  R   | node-|-->|  D  | node |-->| node | node |->...| node | NULL
** -------------     ------------     --|----------       --|---------
**                                    point config 1.   point config R
**                                                       
**
*/

#define R(A) (node_get_int(A,LEFT))
#define D(A) ((int)node_get_int((node)node_get_ptr(A,RIGHT),LEFT))
#define St(A) ((node)(node_get_ptr(A,RIGHT)))
#define C(P) ((node)(node_get_ptr(P,LEFT)))

node aset_new(int r, int d)
{
    node A = nullptr, ptr = nullptr;
    LOCS(2);
    PUSH_LOC(A);
    PUSH_LOC(ptr);
    A = node_new();
    node_set_int(A, r, ASET, LEFT);
    node_set_ptr(A, node_new(), NODE, RIGHT);
    node_set_int((node) Cdr(A), d, INT, LEFT);
    while ((r--) > 0)
	ptr = Cons(pcfg_new(), ptr);
    node_set_ptr((node) Cdr(A), ptr, NODE, RIGHT);
    POP_LOCS();
    return A;
}

int aset_r(node A)
{
    return R(A);
}
int aset_dim(node A)
{
    return D(A);
}
int aset_npts(node A)
{
    int i = 0;
    A = St(A);
    while ((A = Cdr(A)) != nullptr)
	i += pcfg_npts(Car(A));
    return i;
}
node aset_start_cfg(node ptr)
{
    return Cdr(ptr);
}
node aset_next_cfg(node * ptr)
{
    *ptr = Cdr(*ptr);
    return Car(*ptr);
}

node aset_start_pnt(node ptr)
{
    return Cdr(ptr);
}

node aset_next_pnt(node * ptr)
{
    *ptr = Cdr(*ptr);
    return Car(*ptr);
}

int aset_add(node A, int R, node point)
{
    node ptr = A;
    LOCS(1);
    PUSH_LOC(A);
    if (A == nullptr)
	bad_error("Null Aset in aset_add()");
    if (point == nullptr)
	bad_error("Null point in aset_add()");
    if (R >= 1 && R(A) >= R) {
	ptr = St(A);
	while ((R--) > 0)
	    ptr = Cdr(ptr);
	pcfg_add(point, Car(ptr));
    } else
	bad_error("index out of bounds in aset_add()");
    POP_LOCS();
    return 1;
}

node aset_new_pt(int N, char *lable)
{
    Imatrix C;
    node point;
    C = Ivector_new(N);
    point = pnt_new(lable, C);
    return point;
}

int aset_pnt_set(node point, int i, int j)
{
    return (*IVref((Imatrix) Cdr(point), i) = j);
}
int aset_pnt_get(node point, int i)
{
    return *IVref((Imatrix) Cdr(point), i);
}

int aset_unlift(node A)
{
    int d;
    node ptp, ptc;

    d = aset_dim(A);
    A = St(A);
    while ((ptc = aset_next_cfg(&A)) != nullptr)
	while ((ptp = aset_next_pnt(&ptc)) != nullptr)
	    aset_pnt_set(ptp, d, 0);

    return 1;
}

int aset_randlift(node A, int seed, int L, int U)
{
    int d;
    node ptp, ptc;

    rand_seed(seed);
    d = aset_dim(A);
    A = St(A);
    while ((ptc = aset_next_cfg(&A)) != nullptr)
	while ((ptp = aset_next_pnt(&ptc)) != nullptr)
	    aset_pnt_set(ptp, d, rand_int(L, U));

    return 1;
}

node aset_print(node A)
{
 int R,D,N,i,first=1;
 Imatrix Coords;
 node ptr,ptc;

 if (A != nullptr && node_get_type(A, LEFT) == ASET) {
   R = R(A);
   D = D(A);
   ptr = St(A);
   while ((ptr = (node) Cdr(ptr)) != nullptr) {
     first=1;
     ptc=C(ptr);
     N=pcfg_npts(ptc);
     while((ptc=Cdr(ptc))!=nullptr){
        if (first==1){
             printf("   <");
             first=0;
        }
        else printf("    ");
        Coords=pnt_coords(Car(ptc));    
        printf("<");
        for(i=1;i<=D;i++) {
           printf(" %d",*IVref(Coords,i));
           if (i<D) printf(",");
        }
        if (--N>0) printf(" >, ");
        else {
          printf(" >>");
          if (--R > 0) printf(",");
          else printf(" ");
        }
        printf("       %% %s \n",(char *)pnt_lable(Car(ptc)));
     }
   }
 } else if (A != nullptr)
    bad_error("error: aset_print() called on non-ASET\n");
   return A;
}

node aset_print_short(node A)
{
 int R,N,first=1;  
 node ptr,ptc;
 if (A != nullptr && node_get_type(A, LEFT) == ASET) {
   R = R(A);
   ptr = St(A);
   printf("{ ");
   while ((ptr = (node) Cdr(ptr)) != nullptr) {
     first=1;
     ptc=C(ptr);
     N=pcfg_npts(ptc);
     while((ptc=Cdr(ptc))!=nullptr){
        if (first==1){
             printf("{");
             first=0;
        }
        printf(" %s",(char *)pnt_lable(Car(ptc)));   
        if (--N ==0) printf("}");
        else printf(",");
     }
     if (--R>0)printf(",");
     else printf(" }");
   }
 } else if (A != nullptr)
    bad_error("error: aset_print() called on non-ASET\n");
   return A;
}

node aset_face(node A, Imatrix N)
{
    node ptr = A, ptc = nullptr, res = nullptr, rptr = nullptr;
    LOCS(2);
    PUSH_LOC(A);
    PUSH_LOC(res);
    res = aset_new(aset_r(A), aset_dim(A));
    ptr = St(A);
    rptr = St(res);
    while ((ptc = aset_next_cfg(&ptr)) != nullptr) {
	rptr = Cdr(rptr);
	node_set_ptr(rptr, pcfg_face(ptc, N), NODE, LEFT);
    }
    POP_LOCS();
    return res;
}

Imatrix aset_type(node A, Imatrix T)
{
    int r = 1, i;
    Imatrix M = nullptr;
    node ptr, ptc;
    LOCS(1);
    PUSH_LOC(A);
    ptr = aset_start_cfg(A);
    T = Imatrix_resize(T, 1, aset_r(A));
    while ((ptc = aset_next_cfg(&ptr)) != nullptr) {
	M = pcfg_M(ptc, M);
	for (i = 1; i <= IMrows(M); i++)
	    *IMref(M, i, IMcols(M)) = 0;
	*IVref(T, r++) = Imatrix_rref(M, &i);
    }
    Imatrix_free(M);
    POP_LOCS();
    return T;
}

Imatrix aset_M(node A, Imatrix M)
{
    node ptc, ptr, ptp, pt0;
    int j, r = 1;

    M = Imatrix_resize(M, aset_npts(A) - aset_r(A), aset_dim(A) - 1);
    ptr = St(A);
    while ((ptc = aset_next_cfg(&ptr)) != nullptr) {
	pt0 = aset_next_pnt(&ptc);
	while ((ptp = aset_next_pnt(&ptc)) != nullptr) {
	    for (j = 1; j <= aset_dim(A) - 1; j++)
		*IMref(M, r, j) = aset_pnt_get(ptp, j) - aset_pnt_get(pt0, j);
	    r++;
	}
    }
    return M;
}

/* end Aset.c */

/**************************************************************************/
/********************** implementations from Types.c **********************/
/**************************************************************************/

static int level=0;
node node_print(node N)
{
  /*DEBUG */
  /*  printf("Entering node_print with node_type = %d.\n", node_get_type(N,LEFT)); */

    if (node_nullp(N) == TRUE)
	return N;
    switch (node_get_type(N, LEFT)) {
    case IDF:
    case STR:
    case ERR:
	printf("%s", (char *) node_get_ptr(N, LEFT));
	break;
    case IMTX:
	Imatrix_print((Imatrix)node_get_ptr(N, LEFT));
	break;
    case DMTX:
	Dmatrix_print((Dmatrix)node_get_ptr(N, LEFT));
	break;
    case PNT:
	pnt_print(N);
	break;
    case PCFG:
	pcfg_print_short(N);
	break;
    case ASET:
      /* aset_print_short(N); */
	aset_print(N);
	break;
    case DBL:
	printf("%f ", node_get_double(N, LEFT));
	break;
    case INT:
	printf("%d ", node_get_int(N, LEFT));
	break;
    case CELL:
        printf("<cly_cell>");
         break;
    case NODE:
        if (level++==0)printf("( ");
        if (node_atomp(Car(N))==FALSE) printf("(");
        node_print((node) Car(N));
        if (node_atomp(Cdr(N))==TRUE&&node_nullp(Cdr(N))==FALSE)
          printf(" . ");
        else printf("  ");
        node_print((node) Cdr(N));
        if ((node_atomp(Cdr(N))==TRUE)||(node_nullp(Cdr(N))==TRUE)){ 
           printf(" )");
        }
        level--;
	break;
    case PROC: printf("%ld",(long)node_get_ptr(N,LEFT));
        break;
    default:
	printf("Unknown type %d in Node_Print\n",
	       node_get_type(N, LEFT));
	break;
    }
    return N;
}


node atom_int(int val)
{
    node R;
    R = node_new();
    node_set_int(R, val, INT, LEFT);
    return R;
}

node atom_double(double val)
{
    node R;
    R = node_new();
    node_set_double(R, val,DBL, LEFT);
    return R;
}

node atom_id(char *val)
{
    node R;
    R = node_new();
    node_set_ptr(R, mem_strdup(val), IDF, LEFT);
    return R;
}
node atom_proc(node (*prc)(node))
{
    node R;
    R = node_new();
    node_set_ptr(R, reinterpret_cast<void *>(prc), PROC, LEFT);
    return R;
}

node ERRND(char *s)
{ node ans;                                
  ans=node_new();                          
  node_set_ptr(ans,mem_strdup(s),ERR,LEFT);
  return ans;
}


node atom_new(char *val, int tp)
{
    node R;
    R = node_new();
    node_set_ptr(R, val, tp, LEFT);
    return R;
}



void atom_free(node N)
{

    if (node_nullp(N) == TRUE)
	return;
    if (node_atomp(N) != TRUE)
	return;
    switch (node_get_type(N, LEFT)) {
    case ERR:
    case STR:
    case IDF:
	mem_free((char *) node_get_ptr(N, LEFT));
	break;
    case IMTX:
	Imatrix_free((Imatrix)node_get_ptr(N, LEFT));
	break;
    case DMTX:
	Dmatrix_free((Dmatrix)node_get_ptr(N, LEFT));
	break;
    case PNT:
	pnt_free(N);
	break;
    case CELL:
    case ASET:
    case PCFG:
    case INT:
    case NPTR:
	break;
    default:
	printf("Unknown type %d in atom_free\n",
	       node_get_type(N, LEFT));
	break;
    }
}

int numericP(int t){
   switch(t){
      case INT: case DBL: case CMPX: case POLY: return TRUE;
                break;
      default: return FALSE;
   }
   return FALSE;
}

/* end Types.c */


/**************************************************************************/
/********************* implementations from globals.c *********************/
/**************************************************************************/

#define IN_GLOBALS_C TRUE

/* - THIS DIDN'T WORK
#define Pel_Out stdout
#define Pel_Err stderr
#define Pel_In  stdin
#define Pel_Log stdout
*/

/*
FILE *Pel_Out;
FILE *Pel_Err;
FILE *Pel_In;
FILE *Pel_Log;
*/

/*
Pel_Out = stdout;
Pel_Err = stderr;
Pel_In  = stdin;
Pel_Log = stdout;
*/
/*
FILE *Pel_Out = stdout;
FILE *Pel_Err = stderr;
FILE *Pel_In  = stdin;
FILE *Pel_Log = stdout;
*/

char *Pel_LogName = nullptr;
char *FilePrefix =  nullptr;

int Cont_Alg=1;
int Show_Sys=TRUE;
int Show_Xpl=TRUE;

/* end globals.c */

/**************************************************************************/
/********************* implementations from Dtypes.c **********************/
/**************************************************************************/

#define S(i) (DVref(S,i))
void xpnt_unscale(xpnt X,Dvector S){
  int n,i;
  fcomplex ctmp;

  n=(int)xpnt_n(X);
  for(i=1;i<=n;i++){
    ctmp=RCmul(S(i),xpnt_xi(X,i));
    xpnt_xi_set(X,i,ctmp);
  }
}
#undef S
/*
** xpnt_affine
**     rescale an xpnt so that its homogenization coordinate has
**     value 1 (if possible) otherwise its first non-zero coordinate
**     is set to 1.
*/
void xpnt_affine(xpnt X){
 int n,i;
 fcomplex ctmp;
 n=(int)xpnt_n(X);
 if (Cabs(xpnt_h(X))!=0.0){
   for(i=1;i<=n;i++){
      ctmp=Cdiv(xpnt_xi(X,i),xpnt_h(X));
      xpnt_xi_set(X,i,ctmp);
   }
   xpnt_h_set(X,Complex(1.0,0.0));
 }
}

/*
** xpnt_normalize 
**     rescale an xpnt so that it has norm 1 (not including t coord).
*/
void xpnt_normalize(xpnt X){
  int n,i;
  double abs=0.0;
  fcomplex ctmp;
  n=(int)xpnt_n(X);
  
  for(i=1;i<=2*n+2;i++){
    abs+= DVref(X,i)*DVref(X,i);
         
  }
  abs=1/sqrt(abs);
  for(i=1;i<=n;i++){
    ctmp=RCmul(abs,xpnt_xi(X,i));
    xpnt_xi_set(X,i,ctmp);
  }
}

/***********************************************************************/
/***************** implementation code from Extremal.h *****************/
/***********************************************************************/

/*
** global storage for Lin prog solver
*/
int LP_M=0, LP_N=0;
Dmatrix LP_A=nullptr, LP_B=nullptr, LP_C=nullptr, LP_X=nullptr,LP_Q=nullptr, LP_R=nullptr, LP_T1=nullptr, LP_T2=nullptr;
Ivector LP_basis=nullptr, LP_nonbasis=nullptr;
extern double RS_zt;

#define X(i) (DVref(LP_X,i))
#define A(i,j) (DMref(LP_A,i,j))
#define B(i) (DVref(LP_B,i))
#define DtypesC(i) (DVref(LP_C,i))


/*
**  int  pcfg_vertex(node pnt,node PC)
** 
** 
**   return TRUE if pnt is a vertex of PC, 
**          FALSE otherwise (also if pnt not in PC)
**  
**   Notes) a)should check its arguments.
**          b)ratio,LP,and basis should be pointers to matrices
**            of the appropriate type, (they can be 0 to begin
**            with). After completion they will point to the space
**            allocated for the linear program, this allows vertex
**            to be called a number of times without having to 
**            constantly reallocate blocks of space.
**          c)ignores lifting values.
**           
*/

int pcfg_vertex(node pnt,
		node PC)
{
    int i, j, N, D;
    node ptr;

    N = pcfg_npts(PC)-1;
    D = pcfg_dim(PC)-1; /* ignore last coord ... treat it as a lifting value*/

  /* 
  ** initialize basis,nonbasis,X and DtypesC
  */ 
    for (i = 1; i <= N; i++){
      *IVref(LP_nonbasis,i)=i;
      X(i)=0.0;   
      DtypesC(i)=0.0;   
    }
   for (i=1; i<= D+1; i++){
      *IVref(LP_basis,i)=N+i;
      X(N+i)=1.0;   
      DtypesC(N+i)=1.0;   
    }

/* load matrix for linear program */
    ptr = PC;
    for (i = 1; i <= N; i++) {
	ptr = Cdr(ptr);
	if (Car(ptr)==pnt) ptr=Cdr(ptr);
	/* positive and negative coeficients for points */
        for (j = 1; j <= D; j++) {
           A( j, i) = *IVref(pnt_coords(Car(ptr)), j);
        }
        A( D+1,i)=1.0;
    }
    for (i=N+1;i<=D+N+1;i++){
      for (j=1;j<=D+1;j++) A(j,i)=0.0;
    }
    A(D+1,N+1)=1.0;
    B(D+1)=1.0;
    for(i=1;i<=D;i++){
      A(i,N+1+i)=1.0;
      B(i)=*IVref(pnt_coords(pnt),i);
      X(N+1+i)=B(i);
    }
 Rsimp(LP_A,LP_B,LP_C,LP_X,LP_basis,LP_nonbasis,LP_R,LP_Q,LP_T1,LP_T2); 
 for(i=N+1;i<=N+1+D;i++) if (X(i)>RS_zt) return TRUE;
 return FALSE;
}
#undef X
#undef A
#undef B
#undef DtypesC

/*
** pcfg_extremal(node PC)  
**         Input: A point configuration PC.
**         Outut: Same point configuration PC.
**  Side Effects: All non-extremal points are removed from PC.
** 
**      Method: Calls pcfg_vertex on each point to see if it
**              is a vertex and removes those that arent.
**             
**       note:
**       The main loop maintains the following invariant conditions
**                    Car(ptr_nxt) is point being checked
**                    Cdr(ptr_old) = ptr_nxt
*/
node pcfg_extremal(node PC)
{
  node ptr_old, ptr_nxt;
  int N,D;
  
/* allocate space for the linear program and initialize */
    N = pcfg_npts(PC)-1;
    D = pcfg_dim(PC)-1; /* ignore last coord ... treat it as a lifting value*/
    LP_M = D+1;
    LP_N = N+D+1;
    LP_A =  Dmatrix_resize(LP_A, LP_M, LP_N);
    LP_B =  Dmatrix_resize(LP_B, 1, LP_M);
    LP_C =  Dmatrix_resize(LP_C, 1, LP_N);
    LP_X =  Dmatrix_resize(LP_X, 1, LP_N);
    LP_Q =  Dmatrix_resize(LP_Q, LP_M, LP_M);
    LP_R =  Dmatrix_resize(LP_R, LP_M, LP_M);
    LP_T1 = Dmatrix_resize(LP_T1,1, LP_M);
    LP_T2 = Dmatrix_resize(LP_T2,1, LP_M);
    LP_basis    = Imatrix_resize(LP_basis,1,  LP_M);
    LP_nonbasis = Imatrix_resize(LP_nonbasis,1, LP_N-LP_M);


  ptr_nxt=Cdr(PC);
  ptr_old=PC;        
  while(ptr_nxt != nullptr) {
    if (pcfg_vertex(Car(ptr_nxt), PC) != TRUE){
      pcfg_remove(Car(ptr_nxt),PC,ptr_old); 
    }
    else ptr_old= ptr_nxt; 
    ptr_nxt = Cdr(ptr_nxt);
  }

    Dmatrix_free(LP_A); LP_A=nullptr;
    Dmatrix_free(LP_B); LP_B=nullptr;
    Dmatrix_free(LP_C); LP_C=nullptr;
    Dmatrix_free(LP_X); LP_X=nullptr;
    Dmatrix_free(LP_Q); LP_Q=nullptr;
    Dmatrix_free(LP_R); LP_R=nullptr;
    Dmatrix_free(LP_T1); LP_T1=nullptr;
    Dmatrix_free(LP_T2); LP_T2=nullptr;
    Imatrix_free(LP_basis);    LP_basis=nullptr;
    Imatrix_free(LP_nonbasis); LP_nonbasis=nullptr;
  return PC;
}

node aset_extremal(node A)
{
    node ptr, ptc;
    LOCS(1);
    PUSH_LOC(A);
    ptr = aset_start_cfg(A);
    while ((ptc = aset_next_cfg(&ptr)) != nullptr) {
        pcfg_extremal(ptc);
    }
    POP_LOCS();
    return A;
}

/***********************************************************************/
/****************** implementation code from RSimp.c *******************/
/***********************************************************************/
                  
/*
** Rsimp   revised simplex method (Using Bland's rule) 
**        and a qr factorization to solve the linear equations
**
**      Adapted from algorithms presented in 
**             Linear Approximations and Extensions                  
**             (theory and algorithms)
**             Fang & Puthenpura
**             Prentice Hall, Engelwood Cliffs NJ (1993)
**      and 
**            Linear Programming
**            Chvatal 
**            Freeman and Company, New York, 1983
** 
**      (developed first in Octave, many thanks to the author)
** 
**
**  Solve the problem 
**       minimize C'x, 
**       subject to A*x=b,  x>=0
**       for x,c,b n-vectors, and A an m,n matrix with full row rank 
** 
** Assumptions:
**    A mxn matrix with full row rank.
**    b an m matrix. 
**    c an n-vector.
**    x an n-vector holding a basic feasible solution, 
**    basis m-vector holding indices of the basic variables in x
**    nonbasis n-m vector holding the indices not appearing in x.
** 
**  Returns: 
**      FAIL if algorithm doesn't terminate.
**      UNBD if problem is unbounded
**      OPT  if optimum found
**  efects:
**    A,b,c unchanged.
**    x basis, nonbasis, hold info describing last basic feasible 
**                       solution.
**    Q,R hold qrdecomp of last basis matrix.
**    t1,t2 undefined.
**
**
*/ 

/* #include "pelutils.h" */

 #define verbose 0
 #define OPT 0
 #define UNBD 1
 #define FAIL -1
 #define zero_tol RS_zt
 #define nonbasis(j) (*IVref(nonbasis,j))
 #define basis(j) (*IVref(basis,j))
 #define AN(i,j) (DMref(A,i,nonbasis(j)))
 #define AB(i,j) (DMref(A,i,basis(j)))
 #define CB(i) (DVref(c,basis(i)))
 #define CN(i) (DVref(c,nonbasis(i)))
 #define XB(i) (DVref(x,basis(i)))
 #define XN(i) (DVref(x,nonbasis(i)))
 #define Y(i)  (DVref(t1,i))
 #define W(i)  (DVref(t2,i))
 #define DtypesD(i)  (DVref(t2,i))
 #define DtypesR(i,j) (DMref(DtypesR,i,j))
 #define Q(i,j) (DMref(Q,i,j))

double RS_zt=0.0000001;

int Rsimp(Dmatrix A, Dvector b, Dvector c,
      Dvector x,Ivector basis,Ivector nonbasis,
      Dmatrix DtypesR, Dmatrix Q, Dvector t1, Dvector t2){
 int m,n,i,j,k,l,q,qv;
 int max_steps=20;
 double r,a,at;

 m=(int)DMrows(A);
 n=(int)DMcols(A);
 max_steps=4*n;

 /*
 **  Dimension assumptions:
 **     A(m,n) x(n) c(n) b(m) Q(mxm) DtypesR(m,m) t1(m) t2(m) 
 **     basis(m) nonbasis(n-m)   
 */   

 for(k=0; k<=max_steps;k++){
   /*
   ** Step 0) load new basis matrix and factor it
   */
    for(i=1;i<=m;i++){
      for(j=1;j<=m;j++){ 
        DtypesR(i,j)=AB(i,j);
      }
    }
    Dmatrix_GQR(Q,DtypesR);

   /*
   ** Step 1) solving system  B'*w=c(basis)
   **      a) forward solve DtypesR'*y=c(basis)
   */
    for(i=1;i<=m;i++){
      Y(i)=0.0;
      for(j=1;j<=i-1;j++){
        Y(i)+=DtypesR(j,i)*Y(j);
      }
      Y(i)=(CB(i)-Y(i))/DtypesR(i,i);
    }
   /*  
   **      b) find w=Q*y
   **         note: B'*w=(Q*DtypesR)'*Q*y= DtypesR'*(Q'*Q)*y=DtypesR'*y=c(basis)
   */
    for(i=1;i<=m;i++){
      W(i)=0.0;
      for(j=1;j<=m;j++){
        W(i)+=Q(i,j)*Y(j);
      }
    }

   /*
   ** Step 2)find entering variable, 
   ** (use lexicographically first variable with negative reduced cost)
   */
    q=n+1;
    for(i=1;i<=n-m;i++){
      /* calculate reduced cost */
      r=CN(i);
      for(j=1;j<=m;j++){
        r-=W(j)*AN(j,i);      
      }
      if (r<-zero_tol){
        if (q==n+1 || nonbasis(i)<nonbasis(q)){
          q=i;                 
        }
      }
    }
   /*
   ** if ratios were all nonnegative current solution is optimal
   */
    if (q==n+1){           
      if (verbose>0) printf("optimal solution found in %d iterations\n",k);
      return OPT;
    }
   /* 
   ** Step 3)Calculate translation direction for q entering
   **        by solving system  B*d=-A(:,nonbasis(q));
   **   a) let y=-Q'*A(:,nonbasis(q));
   */
    for(i=1;i<=m;i++){
      Y(i)=0.0;
      for(j=1;j<=m;j++){
        Y(i)-=Q(j,i)*AN(j,q);
      }
    }
  
   /*
   **  b) back solve Rd=y  (d=R\y)
   **     note B*d= Q*DtypesR*d=Q*y=Q*-Q'*A(:nonbasis(q))=-A(:,nonbasis(q)) 
   */
    for(i=m;i>=1;i--){
      DtypesD(i)=0.0;
      for(j=m;j>=i+1;j--){
        DtypesD(i)+=DtypesR(i,j)*DtypesD(j);
      }
      DtypesD(i)=(Y(i)-DtypesD(i))/DtypesR(i,i);
    }
   /*
   ** Step 4 Choose leaving variable 
   **     (first variable to become negative, by moving in direction DtypesD)
   **     (if none become negative, then objective function unbounded)
   */
    a=0;
    l=0;
    for(i=1;i<=m;i++){
      if (DtypesD(i)<-zero_tol){
         at=-1*XB(i)/DtypesD(i);
         if (l==0 || at<a){
           a=at;
           l=i;
         }
      }
    }
    if (l==0){
      if (verbose>0){
         printf("Objective function Unbounded (%d iterations)\n",k);
      }
      return UNBD;
    }
   /*
   ** Step 5) Update solution and basis data
   */
    XN(q)=a;        
    for(j=1;j<=m;j++){
      XB(j)+=a*DtypesD(j);
    }
    XB(l)=0.0;       /* enforce strict zeroness of nonbasis variables */
    qv=nonbasis(q);
    nonbasis(q)=basis(l);
    basis(l)=qv;
  }
  if (verbose>=0){ 
      printf("Simplex Algorithm did not Terminate in %d iterations\n",k);
  }
  return FAIL;
}

/***********************************************************************/
/******************* implementation code from MSD.c ********************/
/***********************************************************************/

/*
**    copyright (c) 1995  Birk Huber
*/

/*
** MSD.c     
**        Prune and search based computation of cells of mixed subdivisions
*/

/* #include "pelutils.h" -Seems to be redundant */

/* node atom_new(); */
/* node set_up_FaceLists(); */

#define  msd_out  stdout /* was Pel_Out */

/*
*********************************************************************
** Aset_I: internal representation for Asets 
** logical layout:
**   (indices start at one for all vector typesin Aset_I)
**    A_r(A)      ---- number of point configs
**    A_n(A)      ---- dimension (without the lifting value).
**    A_m(A)      ---- total number of points in Aset
**    A_npts(A,i) ---- number of points in ith polytope 
**    A_pt(A,i,j) ---- return jth point of ith polytope
**    A_pt_coords(A,i,j) --- return coordinate vector of jth pnt of ith ptope
**    A_pt_coord(A,i,j,k) --- return kth coord of jth pnt of ith ptope
**
**  physical layout:
**  r,n,m are stored seperatly as integers.
**  all points are stored in one C-vector, with indexing information 
**  stored in one long integer vector brocken up logically as
**  |m_1,....,m_r|s1,....,sr|
*/
#define A_r(A) (A->r)
#define A_n(A) (A->n)
#define A_m(A) (A->m)
#define A_npts(A,r) (((A->store)[(r)-1]))
#define A_ptst(A,r) (((A->store)[A_r(A)+(r)-1]))
#define A_pt(A,r,i) (((A->pts)[A_ptst(A,r)+(i)-1]))
#define A_pt_flat(A,i) (((A->pts)[(i)-1]))
#define A_pt_coords(A,i,j) (pnt_coords(A_pt(A,i,j)))
#define A_pt_coord(A,i,j,k) (*IVref(A_pt_coords(A,i,j),k))

/* NOW IN pelutils.h
typedef struct Aset_Itag{
     int r;
     int n;
     int m;
     node *pts;   */ /*a vector of points */ /*
     int  *store;
     }*Aset_I; */

/*
** Internalize_Aset 
**    Input: an Aset S.
**   Output: an internal (Aset_I) representation of S.
*/
Aset_I internalize_aset(aset S){
  Aset_I A;
  int r=0,pt=0;
  node ptr,ptc,ptp;

  if((A=(Aset_I)malloc(sizeof(struct Aset_Itag)))==nullptr)
      bad_error("malloc failed in internalize_aset");
  A_r(A)=aset_r(S);
  A_n(A)=aset_dim(S)-1;
  A_m(A)=aset_npts(S);
  if((A->store=(int *)malloc((2*A_r(A))*sizeof(int)))==nullptr)
     bad_error("malloc failed in internalize_aset");
  if((A->pts=(node *)malloc(A_m(A)*sizeof(node)))==nullptr)
     bad_error("malloc failed in internalize_aset");

  ptr = aset_start_cfg(S);
  while ((ptc = aset_next_cfg(&ptr)) != nullptr) {
        A_npts(A,++r)=0;
        A_ptst(A,r)=pt;
        while ((ptp = aset_next_pnt(&ptc)) != nullptr) {
            A_npts(A,r)++;
            A_pt_flat(A,++pt)=ptp;
        }
    }
  return A;
}
/* 
** Aset_I_free
**     free all storage allocated to an Aset_I.
*/
void Aset_I_free(Aset_I A){
 if (A!=nullptr){
    free((char *)A->store);
    free((char *)A->pts);
    free((char *)A);
 }
}
/*
** Aset_I_print
**    Display internal aset (for debugging purposes)
*/
void Aset_I_print(Aset_I A){
 int i,r;
#ifdef LOG_PRINT   
fprintf(msd_out," R=%d, N=%d, M=%d \n",A_r(A),A_n(A),A_m(A));
   fprintf(msd_out," tops = <")
#endif
;
   for(i=1;i<=A_r(A);i++){
 #ifdef LOG_PRINT    
    fprintf(msd_out,"%d ",A_npts(A,i))
#endif
;
         if (i<A_r(A)) 
#ifdef LOG_PRINT
fprintf(msd_out,",")
#endif
;
   }
 #ifdef LOG_PRINT
  fprintf(msd_out,">\n");
   fprintf(msd_out," sts = <")
#endif
;
      for(i=1;i<=A_r(A);i++){       
#ifdef LOG_PRINT
         fprintf(msd_out,"%d ",A_ptst(A,i))
#endif
;
         if (i<A_r(A)) 
#ifdef LOG_PRINT
fprintf(msd_out,",")
#endif
;
   }
#ifdef LOG_PRINT
   fprintf(msd_out,">\n")
#endif
;
   for(r=1;r<=A_r(A);r++){
#ifdef LOG_PRINT
    fprintf(msd_out,"configuration %d :\n",r)
#endif
;
    for(i=1;i<=A_npts(A,r);i++) {
      pnt_print(A_pt(A,r,i));
    }
   }
} 
/*
*****************************************************************
** Internal representation for mixed cells
** logical layout:
**   C_r(C)          number of point configs
**   C_type(C,i)        dimension of ith peice, 
**                      i from 1 to r.
**   C_idx(C,i,j)     index of jth point of ith peice. 
**                      j from 0 to T[i].      
** 
** physical layout:
**     store[0] ... store[r-1]     hold T.
**     store[r] ... store[2r-1]    hold indices to first rows of Pt
**     store[2r] ... store[3r+n-1] hold Pt
**
*/

/* NOW IN pelutils.h
typedef struct Cell_Itag {
  int r;
  int *store;
}*Cell_I;
*/

#define C_r(C) (((C)->r))
#define C_type(C,i) (((C)->store)[(i)-1])
#define C_st(C,i) (((C)->store)[C_r(C)+(i)-1])
#define C_idx(C,r,i) (((C)->store)[C_st(C,r)+(i)])
#define C_pt(A,C,r,i) (A_pt(A,r,C_idx(C,r,i)))
#define C_pt_coords(A,C,r,i) (pnt_coords(C_pt(A,C,r,i)))
#define C_pt_coord(A,C,r,i,j) (*IVref(C_pt_coords(A,C,r,i),j))

/*
** initialize_cell
**   Input:  an Aset A, and a Type vector T.
**   Output: A blank cell to hold fine mixed type T cells of A
*/

Cell_I initialize_cell(Aset_I A, Ivector T){
 Cell_I C;
 int i,j;
 if ((C=(Cell_I)malloc(sizeof(struct Cell_Itag)))==nullptr) 
     bad_error("malloc failed in initialize_cell");
 C_r(C)=A_r(A);
 if((C->store = (int *)malloc((3*C_r(C)+A_n(A))*sizeof(int)))==nullptr) 
      bad_error("malloc failed in initialize_cell");
 j=2*C_r(C);             /* location of C_Pt(C,1,0) */
 for(i=1;i<=C_r(C);i++){
  C_type(C,i)=*IVref(T,i);  /* copy T into C_type feild */
  C_st(C,i)=j;           /* set location of C_Pt(C,i,0)*/
  j+=C_type(C,i)+1;
 }
/* initialize C_Pt to all zeros (asthetic value only)*/
 for(i=1;i<=C_r(C);i++) for(j=0;j<=C_type(C,i);j++) C_idx(C,i,j)=0;

 return C;
}

/*
** Cell_I_free
**     free all storage allocated to a Cell_I.
*/
void Cell_I_free(Cell_I C){
 if (C!=nullptr){
    free((char *)C->store);
    free((char *)C);
 }
}

/*
** Cell_I_print
**    display a cells points
*/
void Cell_I_print(Cell_I C){
 int i,j;
#ifdef LOG_PRINT
 fprintf(msd_out,"[")
#endif
;
 for(i=1;i<=C_r(C);i++){
   for(j=0;j<=C_type(C,i);j++){
#ifdef LOG_PRINT
      fprintf(msd_out,"%d",C_idx(C,i,j))
#endif
;
      if (j<C_type(C,i)) 
#ifdef LOG_PRINT
fprintf(msd_out,", ")
#endif
;
   }
   if (i<C_r(C)) 
#ifdef LOG_PRINT
fprintf(msd_out,": ")
#endif
;
 }
#ifdef LOG_PRINT
 fprintf(msd_out,"]")
#endif
;
}
/*
*******************************************************************
** Face lists  list of faces of an Aset
**
** Logically: A collection of r lists of Ivectors, the Ivectors in the
**            ith list contain the indices of points in the various 
**            faces of type i in the lower hull of A_i
**            List_Start(i) -- a pointer to the node holding a pointer to the
**                             start of the ith list.
**            List_Ptr(i)   -- a pointer used in iterating through the ith
**                             list.
** Physically: The Variable List_Store holds a pointer to the list of
**             of starting points of the lists (ugh):
**    _______      ______      ______
**   | 0 | | |<---|- | | |<---|- | | |<-----List_Store
**    -----+-      ----+-      ----+-
**         |           |           |
**     List_1      List_2       List_R
**   
**                    ________      ________
**   where List_i ==  | | |  -+---->| | |  -+---> ,,,,,,,
**                    --+-----      --+-----
**                     Face_1       Face_2
*/
 node List_Store=nullptr;
 int List_R=0;
 node *List_Start;
 node *List_Ptrs;
 #define LStart(i)  (List_Start[(i)-1])
 #define LPtr(i)    (List_Ptrs[(i)-1])

/*
** init_Face_List_storeage:
**    Input: A number R of lists to create.
**    Output: FALSE if error occurs, TRUE otherwise.
**    Sets List_R to DtypesR.
**    Creates a chain of DtypesR nodes starting at List_Store, (with nodes linked 
**       through their Left pointers, violating the usual convention for lists)
**    The List_Start, and List_Ptr, arrays are created and initialized so that
**       the ith entrees point to the ith nodes on the list List_Store.
**    The Right Pointer of each of these nodes should be interpereted as a 
**       pointer to the start of the corresponding list, initialized to 0.
*/
int init_Face_List_storeage(int r){
 node tmp;
 int i;
 LOCS(1);
 PUSH_LOC(List_Store);
 List_R=r;
 List_Start=(node *)mem_malloc(r*sizeof(node));
 List_Ptrs=(node *)mem_malloc(r*sizeof(node));
 for(i=1;i<=r;i++){
    tmp=node_new();
    node_set_ptr(tmp,(void *)List_Store,NODE,LEFT);
    node_set_ptr(tmp,(void *)nullptr         ,NODE,RIGHT);
    List_Store=tmp;
    LStart(i)=tmp;
    LPtr(i)=tmp;
 }
 POP_LOCS();
 return TRUE;
}
/*
** free_Face_list -- free data storage for the List_Start and List_Ptrs arrays
**                   reset List_Store and List_R to 0
*/
void free_Face_list(){
 mem_free((char *)List_Start);
 mem_free((char *)List_Ptrs);
 List_Store=nullptr;
 List_R=0;
}

/*
** display all faces on face list
*/
void print_Face_list(){
 int i;
 node ptr;
 for(i=1;i<=List_R;i++){
   ptr=LStart(i);
   while((ptr=(node)node_get_ptr(ptr,RIGHT))!=nullptr){
       Imatrix_print((Imatrix)node_get_ptr(ptr,LEFT));
#ifdef LOG_PRINT     
  fprintf(msd_out,"\n")
#endif
;
   }
 }
}

/*
** Push_Face:
*/
void Push_Face(Imatrix F,int r){
    node tmp,ptr;
    tmp=node_new();
    ptr=(node)node_get_ptr(LPtr(r),RIGHT);
    node_set_ptr(tmp,(void *)F,IMTX,LEFT);
    node_set_ptr(tmp,(void *)ptr,NODE,RIGHT);
    node_set_ptr(LPtr(r),(void *)tmp,NODE,RIGHT);
}

void zeroth_face(int j){
 LPtr(j)=LStart(j);
}

int next_face(Cell_I *C,Aset_I A, int r){
 int i;
 Imatrix F;
 /* advance pointer */
 if (LPtr(r)==nullptr) return 0;
 LPtr(r)=(node)node_get_ptr(LPtr(r),RIGHT);
 if (LPtr(r)==nullptr) return 0;
 /* copy face */
 F=(Imatrix)node_get_ptr(LPtr(r),LEFT);
 for(i=0;i<=C_type(*C,r);i++) C_idx(*C,r,i)=*IVref0(F,i);
 return 1;
}

/*
** Intermediate testing -- handles testing for all incomplete cells
*/

static int MSD_LP_M=0, MSD_LP_N=0;
static Dmatrix MSD_LP_A=nullptr, MSD_LP_B=nullptr, MSD_LP_C=nullptr, MSD_LP_X=nullptr;
static Dmatrix MSD_LP_Q=nullptr, MSD_LP_R=nullptr, MSD_LP_T1=nullptr, MSD_LP_T2=nullptr;
static Ivector MSD_LP_basis=nullptr, MSD_LP_nonbasis=nullptr;

/*
** set_up_LP
**   Input: An Aset_I A and a Cell_I C 
**   Output: LP,Ratio, and Basis have enough space reserved for them
**           to allow them to hold any of the linear programs 
**           required by MSD for A and C with only reseting bounds.
*/
void set_up_LP(Aset_I A, Cell_I C){
  MSD_LP_M=A_n(A)+A_r(A);
  MSD_LP_N=A_m(A);
  MSD_LP_A =  Dmatrix_resize(MSD_LP_A, MSD_LP_M, MSD_LP_N);
  MSD_LP_B =  Dmatrix_resize(MSD_LP_B, 1, MSD_LP_M);
  MSD_LP_C =  Dmatrix_resize(MSD_LP_C, 1, MSD_LP_N);
  MSD_LP_X =  Dmatrix_resize(MSD_LP_X, 1, MSD_LP_N);
  MSD_LP_Q =  Dmatrix_resize(MSD_LP_Q, MSD_LP_M, MSD_LP_M);
  MSD_LP_R =  Dmatrix_resize(MSD_LP_R, MSD_LP_M, MSD_LP_M);
  MSD_LP_T1 = Dmatrix_resize(MSD_LP_T1,1, MSD_LP_M);
  MSD_LP_T2 = Dmatrix_resize(MSD_LP_T2,1, MSD_LP_M);
  MSD_LP_basis    = Imatrix_resize(MSD_LP_basis,1,  MSD_LP_M);
  MSD_LP_nonbasis = Imatrix_resize(MSD_LP_nonbasis,1, MSD_LP_N-MSD_LP_M);
}

void free_LP(){
  MSD_LP_M=0; MSD_LP_N=0;
  Dmatrix_free(MSD_LP_A); MSD_LP_A=nullptr;
  Dmatrix_free(MSD_LP_B); MSD_LP_B=nullptr;
  Dmatrix_free(MSD_LP_C); MSD_LP_C=nullptr;
  Dmatrix_free(MSD_LP_X); MSD_LP_X=nullptr;
  Dmatrix_free(MSD_LP_Q); MSD_LP_Q=nullptr;
  Dmatrix_free(MSD_LP_R); MSD_LP_R=nullptr;
  Dmatrix_free(MSD_LP_T1); MSD_LP_T1=nullptr;
  Dmatrix_free(MSD_LP_T2); MSD_LP_T2=nullptr;
  Imatrix_free(MSD_LP_basis);    MSD_LP_basis=nullptr;
  Imatrix_free(MSD_LP_nonbasis); MSD_LP_nonbasis=nullptr;
}

/*
** Load_LP
**
*/

#define MSD_A(i,j)      (DMref(MSD_LP_A,i,j))
#define MSD_B(i)        (DVref(MSD_LP_B,i))
#define MSD_C(i)        (DVref(MSD_LP_C,i))
#define MSD_X(i)        (DVref(MSD_LP_X,i))
#define MSDbasis(i)    (*IVref(MSD_LP_basis,i))
#define MSDnonbasis(i) (*IVref(MSD_LP_nonbasis,i))

void Load_LP(int st,int tp,Cell_I C, Aset_I A){
   int cidx,bidx;
   int i,j,k;
   int nrows,ncols;
   int n;          /* dimenstion of points (without lifting) */
   int tm=0;       /* total number of points in configs ts..tp  */
   int tr=1+tp-st; /* number of distinct configs to consider*/
   int tc=0;
   double tmp;
   Imatrix coords;

   /* calculate new matrix dims */
   n=A_n(A);
   for(i=st;i<=tp;i++){
      tm+=A_npts(A,i);
   }
   nrows=n+tr;
   ncols=tm+n;
   MSD_LP_A=Dmatrix_resize(MSD_LP_A,nrows,ncols);
   MSD_LP_B=Dmatrix_resize(MSD_LP_B,1,nrows);
   MSD_LP_C=Dmatrix_resize(MSD_LP_C,1,ncols);
   MSD_LP_X=Dmatrix_resize(MSD_LP_X,1,ncols);
   MSD_LP_Q=Dmatrix_resize(MSD_LP_Q,nrows,nrows);
   MSD_LP_R=Dmatrix_resize(MSD_LP_R,nrows,ncols);
   MSD_LP_T1=Dmatrix_resize(MSD_LP_T1,1,nrows);
   MSD_LP_T2=Dmatrix_resize(MSD_LP_T2,1,nrows);
   MSD_LP_basis=Imatrix_resize(MSD_LP_basis,1,nrows);
   MSD_LP_nonbasis=Imatrix_resize(MSD_LP_nonbasis,1,ncols-nrows);

   for(j=1;j<=ncols;j++){
        MSD_X(j)=0.0;
        for(i=1;i<=nrows;i++) MSD_A(i,j)=0;
   }
   for(i=1;i<=nrows;i++) MSD_B(i)=0; 

   /* 
   ** put n independent collumns at start of lp
   */
   for(j=1;j<=n;j++){
     for(i=1;i<=nrows;i++) MSD_A(i,j)=rand_double(0,10);
     MSD_C(j)=1500.0;
   }

   /* 
   ** 
   */
   cidx=n;
   bidx=0;
   for(i=st;i<=tp;i++){
      for(j=1;j<=A_npts(A,i);j++){
         MSD_A(1+i-st,cidx+j)=1.0;
         coords=pnt_coords(A_pt(A,i,j));
         for(k=1;k<=n;k++){
            MSD_A(tr+k,cidx+j)=*IVref(coords,k);
         }
         MSD_C(cidx+j)=*IVref(coords,n+1); 
      }
      MSD_B(1+i-st)=1.0;
      tmp=1.0/(C_type(C,i)+1);
      for(j=0;j<=C_type(C,i);j++){
         MSDbasis(++bidx)=cidx+C_idx(C,i,j);
         MSD_X(cidx+C_idx(C,i,j))=tmp;
         for(k=1;k<=n;k++){
           MSD_B(tr+k)+=tmp*MSD_A(tr+k,cidx+C_idx(C,i,j));
         }
      }
   cidx+=A_npts(A,i);
   }
   for(j=1;j<=n;j++){
      if (j<=nrows-bidx) MSDbasis(bidx+j)=j;
      else MSDnonbasis(j-nrows+bidx)=j;
   }
   bidx=bidx-nrows+n;
   for (i=n-tc+1;i<=ncols;i++) if(MSD_X(i)==0.0)MSDnonbasis(++bidx)=i;
}
#undef MSD_A
#undef MSD_B
#undef MSD_C
#undef MSD_X
#undef MSDbasis
#undef MSDnonbasis

/*
** Final testing -- Verification of complete cells
*/
Imatrix M;
Imatrix U; 
Imatrix Norm;
int vol;
void set_up_Final(int n){
 M=Imatrix_new(n,n+1);
 U=Imatrix_new(n,n);
 Norm=Imatrix_new(1,n+1);
 vol=0;
}

void free_Final(){
 Imatrix_free(M);
 Imatrix_free(U);
 Imatrix_free(Norm);
 M=nullptr;Norm=nullptr;U=nullptr;vol=0;
}


int Final_Check(Aset_I A, Cell_I C){
 int i,j,k;
 int row=0,s0,s1;
 
 /* load matrix */
 for(i=1;i<=A_r(A);i++){
    for(j=1;j<=C_type(C,i);j++){
       row++;
       for(k=1;k<=A_n(A)+1;k++){
         *IMref(M,row,k)=C_pt_coord(A,C,i,j,k)-C_pt_coord(A,C,i,0,k);
       }
    }
 }
 /* factor matrix */
 Imatrix_hermite(M,U);

 /* calculate volume */
 vol=1;
 for(i=1;i<=A_n(A);i++) (vol)*=*IMref(M,i,i);

 if (vol==0) return FALSE;

 /* calculate normal and fix direction */
 Imatrix_backsolve(M,Norm);
 if (*IVref(Norm,A_n(A)+1)<0){
  for(i=1;i<=A_n(A)+1;i++) *IVref(Norm,i)*=-1;
 } 
 
 for(i=1;i<=A_r(A);i++){
    /* find offset of first point of cell */
    s0=0;
    for(k=1;k<=A_n(A)+1;k++){
      s0+=*IVref(Norm,k)*C_pt_coord(A,C,i,0,k);
    }
    /*check that remaining points have same norm */
    for(j=1;j<=C_type(C,i);j++){
       s1=0;  
       for(k=1;k<=A_n(A)+1;k++){
           s1+=*IVref(Norm,k)*C_pt_coord(A,C,i,j,k);
       }
       if (s1!=s0){warning("bad normall");
                    return FALSE;}
    }
    /* check that no points have lower offset */
    for(j=1;j<=A_npts(A,i);j++){
       s1=0;                
       for(k=1;k<=A_n(A)+1;k++){
         s1+=(*IVref(Norm,k))*A_pt_coord(A,i,j,k);
       }
       if (s1<s0) return FALSE;
    }
 }

return TRUE;
}


/*
*******************************************************
**  Algorithm: Finding All type t mixed cells
**
**  Input A_1,...,A_r  a lifted Aset
**        t_1,...,t_r  type of cell desired
**
**  Output S a list of all type t cells of our subdivision.
**
**  Begin
**     S={}
**     C_1 = 0
**     i=1;
**     while(i>0){
**       if ((C_i = next_face(C_i,A_i)!=0) {
**         if (i < r){
**            Add rows to simplex tableau to look for C_i
**            if (new system is feasible) {
**               i++;
**               C_i = 0;
**            }
**         }
**         else {
**           if (C is full dim and C lies on lower hull ){
**              S=S union {C_1,...C_n}
**           }
**         }
**       }
**       else i--
**     }
*/

node MSD(aset Ast, Ivector T){
  Aset_I A;                  /* internal representation of Ast*/
  Cell_I C;                  /* internal rep of target cell */
  int i=1;                   /* how many fields of C speceifed*/
  node Normals=nullptr;            /* output normals list */
  int mv=0;                  /* total mixed volume */
  LOCS(2);
  PUSH_LOC(Normals);
  PUSH_LOC(List_Store);

  /* input verification should check A and T for consistancy*/
  if (T==nullptr) bad_error("type vector must be specified in MSD");
  
  /*
  **  Initialization 
  */
  A=internalize_aset(Ast);
  C=initialize_cell(A,T);
  set_up_LP(A,C);
  set_up_Final(A_n(A));
  set_up_FaceLists(A,C);

  /* 
  ** Iteration
  */
  zeroth_face(1);
  while(i>0){
   if (next_face(&C,A,i)>0){
     if (i<A_r(A)){
       Load_LP(1,i,C,A);
       if (TRUE==IsLower()){
         i++;
         zeroth_face(i);
       }
     }
     else {
       if (Final_Check(A,C)==TRUE){
         Cell_I_print(C); 
#ifdef LOG_PRINT
fprintf(msd_out," vol %d\n",vol)
#endif
;
         mv+=vol;
         Normals=Cons(atom_new((char *)Imatrix_dup(Norm,nullptr),IMTX),
                      Normals);         
       }
     }
   }
   else i--;
  }

/* 
**Clean Up
*/
 Aset_I_free(A);
 Cell_I_free(C);
 free_LP();
 free_Final();
 free_Face_list();
 POP_LOCS();

 #ifdef LOG_PRINT  
 fprintf(msd_out,"Mixed Volume = %d",mv)
#endif
;
 return Normals;
 }

/* 
** Enumerate all k_i Faces 
*/

void zeroth_set(Cell_I *C,int j){
 int i;
 for(i=0;i<=C_type(*C,j);i++) C_idx(*C,j,i)=i+1;
 C_idx(*C,j,C_type(*C,j))--;
}


int next_set(Cell_I *C,Aset_I A, int j){
  int cid,top;
  cid=(top=C_type(*C,j));
  while(cid>=0 && cid<=top){
    if (C_idx(*C,j,cid)<(A_npts(A,j)-top+cid)){
        C_idx(*C,j,cid)++;
        if (cid<top) C_idx(*C,j,cid+1)=C_idx(*C,j,cid);
        cid++;
    }
    else cid--;
  }
  return cid;
}


node set_up_FaceLists(Aset_I A, Cell_I C){
  int i,j,k;     
  Imatrix ML,UL,Itmp;
  LOCS(1);
  PUSH_LOC(List_Store);
  ML=Imatrix_new(A_n(A)+1,A_n(A)+1);
  UL=Imatrix_new(A_n(A)+1,A_n(A)+1);

  init_Face_List_storeage(A_r(A));

  for(i=1;i<=A_r(A);i++){
    ML=Imatrix_submat(ML,A_n(A),C_type(C,i));
    UL=Imatrix_submat(UL,A_n(A),A_n(A));
    zeroth_set(&C,i);
    while(next_set(&C,A,i)>0){
      Load_LP(i,i,C,A); 
      if ( TRUE==IsLower()){
        for(j=1;j<=C_type(C,i);j++){
          for(k=1;k<=A_n(A);k++){
            *IMref(ML,k,j)=C_pt_coord(A,C,i,j,k)-
                           C_pt_coord(A,C,i,0,k);
          }
        }
        Imatrix_hermite(ML,UL);
        k=1; for(j=1;j<=C_type(C,i);j++) k*=*IMref(ML,j,j);
        if (k!=0) {
          Itmp=Imatrix_new(1,C_type(C,i)+1);
          for(j=0;j<C_type(C,i)+1;j++)*IVref0(Itmp,j)=C_idx(C,i,j);
          Push_Face(Itmp,i);
        }
      }  
    }   
  }
  Imatrix_free(ML);
  Imatrix_free(UL);
  POP_LOCS();
  return List_Store;
}



 #define verbose 0
 #define OPT 0
 #define UNBD 1
 #define FAIL -1
 #define MSDzero_tol 0.0000001
 #define MSDnonbasis(j) (*IVref(MSD_LP_nonbasis,j))
 #define MSDbasis(j) (*IVref(MSD_LP_basis,j))
 #define MSD_AN(i,j) (DMref(MSD_LP_A,i,MSDnonbasis(j)))
 #define MSD_AB(i,j) (DMref(MSD_LP_A,i,MSDbasis(j)))
 #define MSD_CB(i) (DVref(MSD_LP_C,MSDbasis(i)))
 #define MSD_CN(i) (DVref(MSD_LP_C,MSDnonbasis(i)))
 #define MSD_XB(i) (DVref(MSD_LP_X,MSDbasis(i)))
 #define MSD_XN(i) (DVref(MSD_LP_X,MSDnonbasis(i)))
 #define MSD_Y(i)  (DVref(MSD_LP_T1,i))
 #define MSD_W(i)  (DVref(MSD_LP_T2,i))
 #define MSD_D(i)  (DVref(MSD_LP_T2,i))
 #define MSD_R(i,j) (DMref(MSD_LP_R,i,j))
 #define MSD_Q(i,j) (DMref(MSD_LP_Q,i,j))

int IsLower(){
 int m,n,i,j,k,l,q,qv;
 int max_steps=20;
 double r,a,at;

 m=(int)DMrows(MSD_LP_A);
 n=(int)DMcols(MSD_LP_A);
 max_steps=4*n;

 /*
 **  Dimension assumptions:
 **     A(m,n) x(n) c(n) b(m) Q(mxm) R(m,m) t1(m) t2(m) 
 **     MSDbasis(m) MSDnonbasis(n-m)   
 */   
/*
   printf("\n A=\n");
   Dmatrix_print(MSD_LP_A);printf("\n B=");fflush(stdout);
   Dmatrix_print(MSD_LP_B);printf("\n C=");fflush(stdout);
   Dmatrix_print(MSD_LP_C);printf("\n X=");fflush(stdout);
   Dmatrix_print(MSD_LP_X);printf("\n Basis=");fflush(stdout);
   Imatrix_print(MSD_LP_basis);printf("\n NonBasis=");fflush(stdout);
   Imatrix_print(MSD_LP_nonbasis);printf("\n");
   printf("\n\n");
*/

 for(k=0; k<=max_steps;k++){
   /*
   ** Step 0) load new basis matrix and factor it
   */
    for(i=1;i<=m;i++){
      for(j=1;j<=m;j++){ 
        MSD_R(i,j)=MSD_AB(i,j);
      }
    }
    Dmatrix_GQR(MSD_LP_Q,MSD_LP_R);

   /*
   ** Step 1) solving system  B'*w=c(basis)
   **      a) forward solve R'*y=c(basis)
   */
    for(i=1;i<=m;i++){
      MSD_Y(i)=0.0;
      for(j=1;j<=i-1;j++){
        MSD_Y(i)+=MSD_R(j,i)*MSD_Y(j);
      }
      if (MSD_R(i,i)!=0) MSD_Y(i)=(MSD_CB(i)-MSD_Y(i))/MSD_R(i,i);
      else {
        printf("singular matrix found\n");
        return TRUE;
      }
    }
   /*  
   **      b) find w=Q*y
   **         note: B'*w=(Q*R)'*Q*y= R'*(Q'*Q)*y=R'*y=c(basis)
   */
    for(i=1;i<=m;i++){
      MSD_W(i)=0.0;
      for(j=1;j<=m;j++){
        MSD_W(i)+=MSD_Q(i,j)*MSD_Y(j);
      }
    }

   /*
   ** Step 2)find entering variable, 
   ** (use lexicographically first variable with negative reduced cost)
   */
    q=n+1;
    for(i=1;i<=n-m;i++){
      /* calculate reduced cost */
      r=MSD_CN(i);
      for(j=1;j<=m;j++){
        r-=MSD_W(j)*MSD_AN(j,i);      
      }
      if (r<-MSDzero_tol){
        if (q==n+1 || MSDnonbasis(i)<MSDnonbasis(q)){
          q=i;                 
        }
      }
    }
   /*
   ** if ratios were all nonnegative current solution is optimal
   */
    if (q==n+1){           
      if (verbose>0){
         printf("optimal solution found in %d iterations\n",k);
      }
      return TRUE;
    }
   /* 
   ** Step 3)Calculate translation direction for q entering
   **        by solving system  B*d=-A(:,MSDnonbasis(q));
   **   a) let y=-Q'*A(:,MSDnonbasis(q));
   */
    for(i=1;i<=m;i++){
      MSD_Y(i)=0.0;
      for(j=1;j<=m;j++){
        MSD_Y(i)-=MSD_Q(j,i)*MSD_AN(j,q);
      }
    }
  
   /*
   **  b) back solve Rd=y  (d=R\y)
   **     note B*d= Q*R*d=Q*y=Q*-Q'*A(:MSDnonbasis(q))=-A(:,MSDnonbasis(q)) 
   */
    for(i=m;i>=1;i--){
      MSD_D(i)=0.0;
      for(j=m;j>=i+1;j--){
        MSD_D(i)+=MSD_R(i,j)*MSD_D(j);
      }
      MSD_D(i)=(MSD_Y(i)-MSD_D(i))/MSD_R(i,i);
    }
   /*
   ** Step 4 Choose leaving variable 
   **     (first variable to become negative, by moving in direction D)
   */
    a=0;
    l=0;
    for(i=1;i<=m;i++){
      if (MSD_D(i)<-MSDzero_tol){
         at=-1*MSD_XB(i)/MSD_D(i);
         if (l==0 || at<a){
           a=at;
           l=i;
         }
      }
    }
    if (l==0){
      if (verbose>0){
         printf("Objective function Unbounded (%d iterations)\n",k);
      }
      return FAIL;
    }
   /*
   ** Step 5) If step is non-degenerate stop. otherwise update basis 
   */
   if (a>=MSDzero_tol){
     if (verbose>0){
       printf("non-degenerate step after %d iterations\n",k);
     }
     return FALSE;
    }
    qv=MSDnonbasis(q);
    MSDnonbasis(q)=MSDbasis(l);
    MSDbasis(l)=qv;
   }
   
  if (verbose>=0){ 
      printf("Simplex Algorithm did not Terminate in %d iterations\n",k);
  }
  return FAIL;
}



/* end Dtypes.c */



