/*
**    copyright (c) 1995  Birk Huber
*/

#include "pelgmatr.h"

#define min(i,j) ((i) < (j) ? (i): (j))
/*--------------------------------------------------------------- 
  vector/matrix type  a linear array of int, whith auxilary info.
       *) the number of elements that can be stored is in elt[0]
       *) the current number of rows is in elt[1]
       *) the current number of collumbs is in elt[2]
The actual data are then stored in row major order from elt[3] on
---------------------------------------------------------------*/



/*-------------------------------------------------------------
 vector access macroes (which ignore any rows except for first)
-------------------------------------------------------------*/
#define Vstore(V)  ((V->store))	
#define Vlength(V) ((V->topr))
#define Vref1(V,i) (&(((V->coords)[i-1])))
#define Vref0(V,i)  (&(((V->coords)[i])))
#define Vref(V,i)  Vref1(V,i)

/*------------------------------------------------------------
 matrix access macroes
-------------------------------------------------------------*/
#define Mstore(V)  ((V->store))
#define MMrows(V)  ((V->store/V->ncols))	
#define Mrows(V) ((V->topr))
#define Mcols(V) ((V->topc))
#define MNcols(V) ((V->ncols))

#define Mref1(V,i,j)(&(((V->coords)[(i-1)*(V->ncols)+j-1])))
#define Mref0(V,i,j)(&(((V->coords)[i*(V->ncols)+j])))
#define Mref(V,i,j)  Mref1((V),i,j)



int GMstore(Gmatrix M)
{
    return Mstore(M);
}
int GMMrows(Gmatrix M)
{
    return MMrows(M);
}
int GMrows(Gmatrix M)
{
    return Mrows(M);
}
int GMcols(Gmatrix M)
{
    return Mcols(M);
}
Gen_node *GMref1(Gmatrix M, int i, int j)
{
    return Mref1(M, i, j);
}


/*
   **   Constructor/Destructors for Gmatrixes
   ** 
   ** Gmatrix Gmatrix_free(int r, int c); 
   **       New Gmatrix cabable of holding r rows, and c collumbs.
   ** Gmatrix Gmatrix_new(Gmatrix V);
 */

Gmatrix Gmatrix_new(int r, int c)
{
    Gmatrix V;
    int i, j;
    V = (Gmatrix) mem_malloc(sizeof(struct Gmatrix_t));
    if (!V)
	bad_error("allocation failure in Gmatrix_new()");
    V->coords = (Gen_node *) mem_malloc(r * c * sizeof(Gen_node));
    if (!V)
	bad_error("allocation failure 2 in Gmatrix_new()");
    Mstore(V) = r * c;
    Mrows(V) = r;
    Mcols(V) = c;
    MNcols(V) = c;
    for (i = 1; i <= r; i++) {
	for (j = 1; j <= c; j++)
	    *Mref(V, i, j)=nullptr;
    }
    return V;
}

void Gmatrix_free(Gmatrix V)
{
    int i, j;

    if (V != nullptr && V->coords != nullptr) {
	for (i = 1; i <= Mrows(V); i++)
	    for (j = 1; j <= Mcols(V); j++)
		free_Gen_list(*Mref(V, i, j));
	mem_free((char *) (V->coords));
    }
    if (V != nullptr)
	mem_free((char *) (V));
}


/*
   ** Gmatrix_resize(R,r,c)
   **   Reset R to hold an r,by  c matrix.
   **   if R has enough storage to hold an rxc matrix resets
   **   row and columb entrees of r to r and c. otherwise
   **   frees R and reallocates an rxc matrix
   ** DOES NOT PRESERVE INDECIES OF EXISTING DATA
 */
Gmatrix Gmatrix_resize(Gmatrix R, int r, int c)
{

    if (R == nullptr || Mstore(R) < (r * c)) {
        if (R != nullptr) Gmatrix_free(R);
        R = Gmatrix_new(r, c);
    } else {
        Mrows(R) = r;
        Mcols(R) = c;
        MNcols(R) = c;
    }
    return R;
}

Gmatrix Gmatrix_submat(Gmatrix R, int r, int c)
{

    if (R == nullptr || c > Mcols(R) || r > Mrows(R) * MNcols(R)) {
        bad_error("bad subscripts or zero matrix in Gmatrix_submat()");
    } else {
        Mrows(R) = r;
        Mcols(R) = c;
    }
    return R;
}





/*
   **  Gmatrix_print(M):  print a Gmatrix
   **    if M is null print <<>> and return fail.
   **    otherwise print matrix and return true.
 */
Gmatrix Gmatrix_print(Gmatrix M)
{
    int i, j;

    if (M == nullptr) {
	fprintf(stdout /* was Pel_Out */,"<>");
	return nullptr;
    }
    fprintf(stdout /* was Pel_Out */,"<");
   
 for (i = 1; i <= Mrows(M); i++) {
	for (j = 1; j <= Mcols(M); j++) {
	    print_Gen_list(*Mref(M, i, j)); 
	    /*
	    if (j < Mcols(M)) 
	      printf(",\n ");
	    */
	}  
	if (i < Mrows(M))
	    fprintf(stdout,";\n");
    }
    fprintf(stdout /* was Pel_Out */,">\n");
    return M;
}

/*
** Gmatrix_Dop(M1,M2, ) -- Add two Gmatrixes:
**  if M1, and M2 are incompatable (or null) complain and return false.
**  if *M3 has too little storage (or is null) free *M3 if nescesary
**                                            and create new storage.
*/
Gmatrix Gmatrix_Dop(Gmatrix M1, Gmatrix M2, Gen_node (*op)(Gen_node))
{
    int i, j;
    Gmatrix R;

    if (M1 == nullptr||M2 == nullptr||Mrows(M1)!=Mrows(M2)||
                          Mcols(M1)!= Mcols(M2)) {
	bad_error("matrix_add: dimensions don't match\n");
    }
    R=Gmatrix_new(Mrows(M1), Mcols(M1));
    for (i = 1; i <= Mrows(M1); i++)
	for (j = 1; j <= Mcols(M1); j++)
	   *Mref(R, i, j)=op(Link(
                       copy_Gen_list(*Mref(M1, i, j)),
                       copy_Gen_list(*Mref(M2, i, j))
                        ));
    return R;
}

Gmatrix Gmatrix_Sop(Gen_node g, Gmatrix M, Gen_node (*op)(Gen_node))
{
    int i, j;
    Gmatrix R;

    if (M == nullptr||g==nullptr) bad_error("matrix_Sop: null arg\n");
    R=Gmatrix_new(Mrows(M), Mcols(M));
    for (i = 1; i <= Mrows(M); i++)
        for (j = 1; j <= Mcols(M); j++)
           *Mref(R, i, j)=op(Link(
                       copy_Gen_list(g),
                       copy_Gen_list(*Mref(M, i, j))
                        ));
    return R;
}

   Gmatrix Gmatrix_Mop(Gmatrix M1, Gmatrix M2,
                   Gen_node Aidentity,
                   Gen_node (*opA)(Gen_node),
                   Gen_node (*opM)(Gen_node)){
   int i,j,k;
   Gmatrix R;

   if (M1==nullptr|| M2==nullptr || Mcols(M1)!=Mrows(M2)) {
   warning("Gmatrix_mull: incompatible matrices");
   return nullptr;
   }
   R=Gmatrix_new(Mrows(M1),Mcols(M2));
   for(i=1; i<=Mrows(M1); i++)
   for(j=1;j<=Mcols(M2); j++){
   *Mref(R,i,j)=copy_Gen_list(Aidentity);
   for(k=1; k<=Mcols(M1); k++)
     *Mref(R,i,j)=
        opA(
            Link(*Mref(R,i,j),
            opM(Link(
                  copy_Gen_list(*Mref(M1,i,k)),
                  copy_Gen_list(*Mref(M2,k,j))
             ))
             ));
   }
   return R;
   }




Gmatrix Gen_Mtx(Gen_node g){
  return (Gmatrix) g->Genval.gval;
}

Gen_node GMND(Gmatrix M){
     Gen_node a;
     a=gen_node();
     a->type=Mtx_T;
     a->Genval.gval=(char *)M;
     a->next=nullptr;
     return a;
}

Gmatrix Gmatrix_copy(Gmatrix M){
  Gmatrix N;
  int i,j;
  N=Gmatrix_new(Mrows(M),Mcols(M));
  for(i=1;i<=Mrows(M);i++)
    for(j=1;j<=Mcols(M);j++)
      *Mref(N,i,j)=copy_Gen_list(*Mref(M,i,j));
  return N;
}

Gmatrix Gmatrix_Transpose(Gmatrix M){
 Gmatrix N;
  int i,j;
  N=Gmatrix_new(Mcols(M),Mrows(M));
  for(i=1;i<=Mrows(M);i++)
    for(j=1;j<=Mcols(M);j++)
      *Mref(N,j,i)=copy_Gen_list(*Mref(M,i,j));
  return N;
}


int Gen_Mtx_Specs(Gen_node g,int *r, int *c, int *t){
  int i, j;
  Gmatrix M;
  if (Gen_type(g)!=Mtx_T&&Gen_type(g)!=Sys_T)return FALSE;
  M=Gen_Mtx(g);
  *r=GMrows(M);
  *c=GMcols(M);
  *t=Int_T;
  for(i=1;i<=*r;i++)
      for(j=1;j<=*c;j++)
          *t=Common_Type(*t,Gen_type(*GMref(M,i,j)));
  return TRUE;
}

Gen_node Link(Gen_node g1, Gen_node g2)
{ 

#ifdef LOG_PRINT
   if (g1->next!=nullptr)
    fprintf(stdout /* was Pel_Out */,
	    "Warning in Link, g1 already has successor\n");
#endif

   g1->next=g2;
   return g1;
 }

Gen_node SYSND(Gmatrix M){
     Gen_node a;
     a=gen_node();
     a->type=Sys_T;
     a->Genval.gval=(char *)M;
     a->next=nullptr;
     return a;
}

Gen_node XPLND(Gen_node g)
 {Gen_node a;
  a=gen_node();
  a->type=Xpl_T;
  a->Genval.lval=g;
  return a;
 }

