/* cly_all.c */

#include "pelclyal.h"


/************************************************************************/
/****************** implementation code from cly_cells.c ****************/
/************************************************************************/

/*
******************************************************************
** Global  Variables
******************************************************************
*/
int cly_Npts=0;      /* the number of points in the config */
int cly_N=0;        /* the dimension of the cayley point config */
int cly_R=0;        /* the number of point configs */
int cly_Dim=0;        /* the dimension of the Aset */
int next_id=1; /*for debuging cells are labled by a unique id*/

/*
** The matrices here are used for now to avoid problems,
** they will eventually be stored with the individual cells.
*/

HMatrix cly_U=0;   /* factor matrix */ 
HMatrix cly_M=0;   /* an n+1xn+1 matrix   */
HMatrix cly_L=0;   /* an n+1 vector   */
Imatrix cly_T=0;   /* an R vector */

/*
** two temporary variables used by the Hint macroes
*/
Hint cly_temp;
int cly_det;
/*
** controll parameters
**
*/
int cly_order=TRUE;
int cly_lift=TRUE;


/* 
** Display functions: mainly for debugging
**  Ipnt_print  print complete list of data fields.
**  Ipnt_list_print  print list print lables of all points accesable
**                   from a given points through nex pointers
*/
void Ipnt_fprint(FILE *fout,Ipnt p){
  int i;
#ifdef LOG_PRINT
  fprintf(fout,"%% %s,",Ipnt_lable(p))
#endif
;
  for (i=1;i<=cly_N;i++) 
#ifdef LOG_PRINT
fprintf(fout,"%d, ",Ipnt_coord(p,i))
#endif
;
#ifdef LOG_PRINT
  fprintf(fout,"lift = %d, idx =%d \n",Ipnt_lift(p),Ipnt_idx(p))
#endif
;
 }

void Ipnt_list_fprint(FILE *fout,Ipnt p){
#ifdef LOG_PRINT
 fprintf(fout,"%%{Pl\n")
#endif
;
 for(;p!=0;p=Ipnt_next(p)) Ipnt_fprint(fout,p);
#ifdef LOG_PRINT
 fprintf(fout,"%%Pl}\n")
#endif
;
}

/*
** Creator/Destructor
*/
 Ipnt Ipnt_new(node pnt,int r){
    Ipnt p;
    int j;
    p=(Ipnt)mem_malloc(sizeof(struct Ipnt_t));
    if (p==0) bad_error("malloc 1 failed in new_Ipnt");
    p->point=pnt;
    p->idx=r;
    p->lable=(char *)mem_strdup(pnt_lable(pnt));
    p->coords=(int *)mem_malloc(cly_N*sizeof(int));
    if (p==0) bad_error("malloc 2 failed in new_Ipnt");
    for(j=1;j<=cly_Dim;j++) Ipnt_coord(p,j)=*IVref(pnt_coords(pnt),j);
    for(j=cly_Dim+1;j<=cly_N;j++) Ipnt_coord(p,j)=0;
    if (r>1) Ipnt_coord(p,cly_Dim+r-1)=1;
    p->lift=0;
    p->next=0;
    return p;
}


void Ipnt_free(Ipnt p){
  if (p!=0){
     mem_free(p->lable);
     mem_free((char*)(p->coords));
     mem_free((char *)p);
  }
 }

void points_free(Ipnt p){
  Ipnt jk;
  while(p!=0){
    jk=p;
    p=Ipnt_next(p);
    Ipnt_free(jk);
  }
 }

void lift_original_points(Ipnt p){
 for( ;p!=0; p=Ipnt_next(p))
     *IVref(pnt_coords(Ipnt_pnt(p)),cly_Dim+1)=Ipnt_lift(p);
}

/**************************************************************
** functions for basic acces and manipulation of cells.
***************************************************************/

/*
** cell_new(int n,int r)
**   reserve space for a new cell, for holding an n-simplex
**   the points and pointers fields are initialized to null
*/
 cell cell_new(int n,int r){
  cell c;
  int i;
  if ((c=(cell)mem_malloc(sizeof(struct cell_t)))==0)
                bad_error("malloc failed in cell_new()");
  if ((c->points=(Ipnt *)mem_malloc((n+1)*sizeof(Ipnt)))==0)
                bad_error("malloc failed in cell_new()");
  if ((c->ptrs=(cell *)mem_malloc((n+1)*sizeof(cell)))==0)
                bad_error("malloc failed in cell_new()");
  c->H=HMnew(n,n+1);
  c->U=HMnew(n,n);
  c->T=Ivector_new(r);
  c->norm=HVnew(n+1);
  c->n=n;
  c->r=r;
  c->id=next_id++;
  c->freeptrs=n+1;
  c->volume=0;
  c->Tindx=0;
  c->next=0;
  for(i=0;i<=n;i++) {c->ptrs[i]=0; c->points[i]=0;}
  return c;
}

/*
** cell_free(cell c)
**           free space allocated for a cell
**           (does not effect any of the objects pointed to by the
**            points or pointers fields).
*/
 void cell_free(cell c){
  if (c!=0){
     mem_free((char *)c->points);
     mem_free((char *)c->ptrs);
     HMfree(c->norm);
     Imatrix_free(c->T);
     HMfree(c->U);
     HMfree(c->H);
     mem_free((char *)c);
  }
}
 void subdiv_free(cell c){
 cell jk;
 while(c!=0){ jk=c; c=cell_next(c); cell_free(jk);}
}

/*
**   cell_print(cell c)-   print the contents of a single cell.
*/
 void cell_fprint(FILE *fout, cell c){
   int i,k,tog;

#ifdef LOG_PRINT
   fprintf(fout,"%% {")
#endif
;
   for(k=1;k<=cly_R;k++){
#ifdef LOG_PRINT
     fprintf(fout,"{")
#endif
;
     tog=0;
     for (i=0;i<=c->n;i++){
       if (Ipnt_idx(cell_pnt(c,i))==k){
         if (tog==0) tog=1;
         else printf(",");
     #ifdef LOG_PRINT
    fprintf(fout," %s",Ipnt_lable(cell_pnt(c,i)))
#endif
;
       }
     }
#ifdef LOG_PRINT
     fprintf(fout,"}")
#endif
;
     if (k<cly_R) fprintf(fout,",");
   }
#ifdef LOG_PRINT
   fprintf(fout,"}   < %d",cell_type(c,1))
#endif
;
   for(i=2;i<=cly_R;i++) 
#ifdef LOG_PRINT
fprintf(fout,", %d",cell_type(c,i))
#endif
;
#ifdef LOG_PRINT
   fprintf(fout," >        vol=%d",cell_volume(c));
   fprintf(fout,"\n")
#endif
;
}

/*
** subdiv_print(cell c)-   print contents of all cells in a
**    subdivision, a list of cells linked through their next fields
*/
 void subdiv_fprint(FILE *fout, cell c){
#ifdef LOG_PRINT 
  fprintf(fout,"\n")
#endif
;
  while(c!=0){
      cell_fprint(fout,c);
      c=cell_next(c);
  }

}

/*
** point_is_in(Ipnt pt, cell c)
**   Input:  a point pt,
**           a cell c.
**   Output: TRUE if cell contains pt
**           FALSE otherwise.
** (a point is determined by its memory address, points with same
**  coordinates and lables but stored in diferent places are still
**  distinct).
*/
 int point_is_in(Ipnt pt, cell c){
  int i;
  for(i=0;i<=cell_n(c);i++)
          if (pt==cell_pnt(c,i)) return TRUE;
  return FALSE;
 }
 int cell_type_cmp(cell c1,cell c2){
  int i,t;
  if (c1==0 && c2==0) return 0;
  else if (c1==0) return 1;
  else if (c2==0) return -1;
  for(i=1;i<=cly_R;i++){
     t=cell_type(c1,i)-cell_type(c2,i);
     if (t!=0) return t;
  }
  return 0;
}

 void order_subdiv(cell *Subdiv){
  cell Res=0,S=*Subdiv, ptr=0,pts=0;
  while(S!=0){
    pts=S;
    S=cell_next(S);
    if (Res==0 || cell_type_cmp(pts,Res)>=0){
      cell_next(pts)=Res;
      Res=pts;
    }
    else {
      ptr=Res;
      while(cell_next(ptr)!=0 && cell_type_cmp(pts,cell_next(ptr))<0){
         ptr=cell_next(ptr);
      }
      cell_next(pts)=cell_next(ptr);
      cell_next(ptr)=pts;
    }
  }
 *Subdiv=Res;
}
 int fprint_all_volumes(FILE *fout,cell S){
  int vol=0, i;
  while(S!=0){
     vol+=cell_volume(S);
     if (cell_next(S)==0 || cell_type_cmp(S,cell_next(S))!=0){
      #ifdef LOG_PRINT
  fprintf(fout,"%% Vol( %d", cell_type(S,1))
#endif
;
        for(i=2;i<=cly_R;i++) 
#ifdef LOG_PRINT
fprintf(fout,", %d",cell_type(S,i))
#endif
;
#ifdef LOG_PRINT
        fprintf(fout,")= %d\n",vol)
#endif
;
        vol=0;
     }
     S=cell_next(S);
  }
  return 1;
}

 int cell_set_volume(cell c){
 int i,j,r,row;

 cly_T=Imatrix_resize(cly_T,1,cly_R);
 cly_U=HMresize(cly_U,cly_Dim,cly_Dim);
 cly_M=HMresize(cly_M,cly_Dim,cly_Dim);

 for(i=1;i<=cly_R;i++)*IVref(cly_T,i)=-1;

 row=1;
 for(i=0;i<=cly_N;i++){
   r=Ipnt_idx(cell_pnt(c,i));
   if (*IVref(cly_T,r)==-1){
             *IVref(cly_T,r)=i;
   }
   else{
    for(j=1;j<=cly_Dim;j++){
      HLMset(cly_M,row,j,cell_point(c,i,j)-cell_point(c,*IVref(cly_T,r),j));
    }
    row++;
   }
 }
 HMdet(cly_M,cly_U,r);
 return (cell_volume(c)=r);
}

/*
** subdiv_union
**    Input:        subdivisions SD1 and SD2
**   Output:        SD2 appended to SD1.
**   Side Effects:  if SD1 is not null SD1 also points to union.
*/
 cell cly_subdiv_union(cell SD1, cell SD2){
  cell ptr=SD1;
  if (SD1==0) return SD2;
  while(cell_next(ptr)!=0) ptr=cell_next(ptr);
  cell_next(ptr)=SD2;
  return SD1;
}

/* end cly_cells.c */

/************************************************************************/
/***************** implementation code from cly_initial.c ***************/
/************************************************************************/

/*******************************************************************
** Initial Simplex Program
*******************************************************************/
/*
**  cell  Initial_simplex(node *S1, node *S2){
**
**  input:
**       A list of points S1.
**
**  output:
**       NULL if "PC" lies in a hyperplane.
**       A full dimensional simplex (S) from "PC" otherwise.
**
**  effects:
**         "B" points to a list containing all points of "PC"
**             wich are not in (S).
**
** method:
**   a) initialize (S) with  upper and lower hulls of PC with
**      respect to the first standard basis vector ei.
**   b) repeatedly  calculate a normal (N) to the span of (S),
**      and add one of the  extreme points of PC (with respect
**       to (N)) which does not already lie in the span of (S).
*/
static int check_normal(Ipnt,HMatrix,Ipnt *,Ipnt *);
#define CU cell_U(initial)
#define CM cell_H(initial)
#define CL cell_norm(initial)

cell  cly_initial_splx(Ipnt *S1, Ipnt *S2){
  cell initial;
  int i,r,j;
  Ipnt ptr=*S1,refpt=0,newpt,tmp;

  initial=cell_new(cly_N,cly_R);

  /*
  ** Initialize global matrices
  **   M will represent the linear space spanned by points
  **   allready chosen. (this will be an invarient of the loop).
  **   L will hold the next search direction, originally (0,...,0,1)
  */
  HMresize(CU,1,1);
  HMresize(CL,1,cly_N);
  HMresize(CM,1,cly_N);
  HLVset(CL,1,1);
  for(i=2;i<=cly_N;i++) HLVset(CL,i,0);
  /*
  ** find two points not normal to CL=(0,....,0,1); and update M
  */
  if (check_normal(ptr,CL,&refpt,&newpt)==FALSE) return FALSE;
  cell_pnt(initial,0)=refpt;
  cell_pnt(initial,1)=newpt;
  for(i=1;i<=cly_N;i++)
      HLMset(CM,1,i,cell_point(initial,1,i)-cell_point(initial,0,i));
  r=1;
  while (++r<=cly_N){
    /*
    ** Calculate new search direction L
    ** (normal to affine span of points allready chosen)
    */
    
    HMfactor(CM,CU);
    HMbacksolve(CM,CL);
    HMgcd_reduce(CL); 


    /*
    ** if possible pick a new point not in the same hyperplane,
    ** normal to L, as points allready chosen.
    */
    if (check_normal(ptr,CL,&refpt,&newpt)==FALSE){
 #ifdef LOG_PRINT     
      fprintf(stderr /* was cly_err */,"Failure in initial_simplex():");
      fprintf(stderr /* was cly_err */,"Point Config is not full dimensional\n")
#endif
;
       return 0;
    }
    cell_pnt(initial,r)=newpt;
    CM=HMsubmat(CM,r,cly_N);
    CU=HMresize(CU,r,r);
    for(i=1;i<=cly_N;i++)
       HLMset(CM,r,i,cell_point(initial,r,i)-cell_point(initial,0,i));
  }
  /* 
  ** reset sizes for matrices in cell 
  */
  HMresize(cell_norm(initial),1,cly_N+1);
  HMresize(cell_U(initial),cly_N,cly_N);
  HMresize(cell_H(initial),cly_N,cly_N+1);
  /*
  ** set normal to initial simplex
  **   (it is unlifted so norm = (0,...,0,1)
  */
  for(i=1;i<=cly_N;i++) HLVset(cell_norm(initial),i,0);
  HLVset(cell_norm(initial),cly_N+1,1);

  /* set up type vector */
  for(i=1;i<=cly_R;i++) cell_type(initial,i)=-1;
  for(i=0;i<=cly_N;i++) cell_type(initial,Ipnt_idx(cell_pnt(initial,i)))++;
  /*
  ** setup factorization matrix CU[v1-v0,....,vn-v0,0]=H
  */
  for(i=1;i<=cly_N;i++){
     for(j=1;j<=cly_N;j++){
        HLMset(cell_H(initial),j,i,
              cell_point(initial,i,j)-cell_point(initial,0,j));
     }
     HLMset(cell_H(initial),i,cly_N+1,0);
  }
  HMfactor(cell_H(initial),cell_U(initial));
  if (HLeq(HMget(cell_H(initial),cly_N,cly_N),0)){
            bad_error("initial simplex not full d");
  }
  cell_set_volume(initial);
  /*
  ** Set S2=points in initial simplex.
  **     S1=S1/S2.
  */
  ptr=*S1;
  *S1=0;
  *S2=0;
  while(ptr!=0){
     int In=FALSE;
     for(i=0;i<=cly_N;i++){
        if(ptr==cell_pnt(initial,i)) In=TRUE;
     }
     tmp=ptr;
     ptr=Ipnt_next(ptr);
     if (In==FALSE) {
         Ipnt_next(tmp)=*S1;
         *S1=tmp;
     }
     else{
         Ipnt_next(tmp)=*S2;
         *S2=tmp;
     }
  }
 return initial;
}


/*
** int check_normal(node PC,Imatrix norm, node *refpt, node *newpt){
**
** (auxilary function for initial_simplex)                        
**                                                                
**  input:                                                        
**    A point configuration "PC"                                  
**    A normal direction "N" and a reference point "refpt"        
**          (if "refpt" is null, "refpt" gets initalized to point 
**            to a point on the lower hull)                       
**          ("norm" and "refpt" together determin a hyperplane H) 
**                                                                
**  output:                                                       
**    FALSE if the point configuration lies hyperplane normal to N
**    TRUE otherwise.                                             
**                                                                
**  side effects:                                                 
**   "newpt" holds a point which has maximal distance from (H).   
**    if refpt was null it gets initialized (see "input").        
**                                                                
**  method:  just computes dot products and save a point acheiving
**           maximum and minimum values.                       
**           then compair these against value for refpt.
*/

static int check_normal(Ipnt PC,
                        HMatrix norm,
                        Ipnt *refpt, Ipnt *newpt){
  Hint dot,dmin,dmax;
  int i,first=TRUE;
  Ipnt pmin = NULL;
  Ipnt pmax = NULL;
  Hinit(dot,0);
  Hinit(dmin,0);
  Hinit(dmax,0);
  while(PC!=0){
    Hnorm_dot(dot,norm,PC);
    if (first==TRUE) {
       HHset(dmin,dot);
       HHset(dmax,dot);
       pmin=pmax=PC;
       first=FALSE;
    }
    else {
      if (HHlt(dot,dmin)){
          HHset(dmin,dot);
          pmin=PC;
      }
      else if (HHlt(dmax,dot)){
        HHset(dmax,dot);
        pmax=PC;
      }
    }
  PC=Ipnt_next(PC);
  }
  if (HHeq(dmin,dmax)) return FALSE;
  if (*refpt==0) {
     *refpt=pmin;
     *newpt=pmax;
  }
  else{
      Hnorm_dot(dot,norm,*refpt);
      if (HHeq(dot,dmin)) *newpt=pmax;
       else *newpt=pmin;
  }
  Hfree(dot);
  Hfree(dmin);
  Hfree(dmax);
  return TRUE;
}

/* end cly_initial.c */

/************************************************************************/
/***************** implementation code from cly_update.c ****************/
/************************************************************************/

static int is_flipped(cell c1, cell c2, int *i1, int *i2); 
static cell cell_pivot(cell C, Ipnt x, int idx);
static cell subdiv_add_cell(cell c, cell SDx);
int cell_find_lift(cell c, Ipnt pt);


/*
** is_flipped
**   Input:  cells   c1, and c2.
**   Output: TRUE if c1 and c2 are related by pivoting
**           FALSE otherwise.
**  Side Effects: i1 and i2 contain the indices of the pivoting
**                vertices for c1 and c2 respectivly.
*/
static int is_flipped(cell c1, cell c2, int *i1, int *i2){
  int i, found1=FALSE, found2=FALSE;
  for(i=0;i<=cly_N;i++){
    if (point_is_in(cell_pnt(c1,i),c2)==FALSE){
      if (found1==FALSE){
         *i1=i;
         found1=TRUE;
      }
      else return FALSE;
    }
    if (point_is_in(cell_pnt(c2,i),c1)==FALSE){
      if (found2==FALSE){
         *i2=i;
         found2=TRUE;
      }
      else return FALSE;
    }
  }
  if (found1==FALSE||found2==FALSE)
     bad_error("is_flipped: cell differences not detected");
  return TRUE;
}

/*
**   cell_pivot
**     Input.  A cell C.
**             A point x.
**             An index idx.
**    Output.  A copy of C, with point x substituted for vertex idx.
**             (the pointer field is set to indicate that cell C
**              and the new cell are related by a flip).
** 
*/
static cell cell_pivot(cell C, Ipnt x, int idx){
  int i,j;
  cell ncell;
  if (C==0) bad_error("null cell in cell_pivot\n");

  /* copy old cell */
  ncell=cell_new(cly_N,cly_R);
  for(i=1;i<=cly_R;i++) cell_type(ncell,i)=cell_type(C,i);
  for(i=0;i<=cly_N;i++) cell_pnt(ncell,i)=cell_pnt(C,i);

  cell_type(ncell,Ipnt_idx(cell_pnt(C,idx)))--;
  cell_type(ncell,Ipnt_idx(x))++;
  cell_pnt(ncell,idx)=x; /* substitute point x for vertex idx*/
  cell_ptr(ncell,idx)=C;   /* pivoting ncell at idx gives c*/
  cell_ptr(C,idx)=ncell;   /* pivoting c at idx gives ncell*/
  cell_fptr(ncell)--;
  cell_fptr(C)--;

  /* set up to calculate normal */
  /* load coordinate matrix */
  for(i=1;i<=cly_N;i++){
    for(j=1;j<=cly_N;j++){
      HLMset(cell_H(ncell),i,j,
             cell_point(ncell,i,j)-cell_point(ncell,0,j));
      }
    HLMset(cell_H(ncell),i,cly_N+1,cell_lift(ncell,i)-cell_lift(ncell,0));
  }
  
  /* calculate normal and remove common factors from coordinates*/
  HMfactor(cell_H(ncell),cell_U(ncell));
  HMbacksolve(cell_H(ncell),cell_norm(ncell));
  HMgcd_reduce(cell_norm(ncell));

  /* switch direction if necessary to ensure inner normal */
  if (HLlt(HVget(cell_norm(ncell),cly_N+1),0)) {
     for(j=1;j<=cly_N+1;j++) Hneg(HVget(cell_norm(ncell),j));
  }
  else if (HLeq(HVget(cell_norm(ncell),cly_N+1),0)) 
      bad_error("Cell perpendicular in Cell_norm");

 /* load and factor point matrix */
  for(i=1;i<=cly_N;i++){
     for(j=1;j<=cly_N;j++){
        HLMset(cell_H(ncell),j,i,
               cell_point(ncell,i,j)-cell_point(ncell,0,j));
     }
     HLMset(cell_H(ncell),i,cly_N+1,0);
  }
  HMfactor(cell_H(ncell),cell_U(ncell));

  cell_set_volume(ncell);
  
  return ncell;
}

/*
**  subdiv_add_cell
**    Input:     A cell c
**               A list of cells SDx
**               (with the invariant condition that all cells
**                of SDx related by a flip point to each other
**                through the relevent pointer fields.)
**   Output:     c appended to SDx
**               (with the invariant condition maintained).
**
**
*/
static cell subdiv_add_cell(cell c, cell SDx){
  int i,j;
  cell ptr;
  if (c==0) bad_error("null cell in subdiv_add_cell\n");
  if (SDx==0) return c;
  ptr=SDx;
  while(ptr!=0){
      if (is_flipped(c,ptr,&i,&j)==TRUE){
          if (cell_ptr(c,i)!=0)
              bad_error("new cell already flipped in sudiv_add_cell");
          if (cell_ptr(ptr,j)!=0)
              bad_error("old cell already flipped in sudiv_add_cell");
          cell_ptr(c,i)=ptr;
          cell_fptr(c)--;
          cell_ptr(ptr,j)=c;
          cell_fptr(ptr)--;
      }
      if (cell_next(ptr)==0){
              cell_next(ptr)=c;
              cell_next(c)=0;
              ptr=0;
      }
      else ptr=cell_next(ptr);
  }
  return SDx;
}

/*
**    New_Facets
**      Input        a subdivision SD
**                   a point x
**      Output       The new cells resulting by "placeing"
**                   x (implicitly using a conservitive lifting)
**
**      Method:        (let n1=n+1).
**             Given a cell C and an affine relation
**                  x=g0*v_0+...gn1*v_n1,     g1+...+gn1=1;
**
**             Lemma 2.20 of Verschelde et. al shows that
**             should be added by pivoting x into C in position
**             i iff gi<0.
**
**          For each Cell C.
**            The matrix M=(v_1-v_0,...,v_{n-1}-v_0,x-v_0) is formed
**            and the system M*L=0 is solved,
**            (giving g_i=-L_i/L_n and g_0=(L_0+...+L_n1)/L_n1
**
**            if (g_i < 0 and C has not allready been pivoted at v_i)
**                         (i.e. facet oposite v_i is an outer facet)
**            then
**             C is pivoted at v_i and vi is added to teh list of new
**             cells
**
**
*/
 cell cly_new_cells(cell D,cell Dl, Ipnt x){
  cell SDx=0,B;
  int i,j,n,tog;
  Hint l0,tmp;

  n=cly_N;
  Hinit(l0,0);
  Hinit(tmp,0);
  HHset(tmp,l0);
  HMresize(cly_L,1,cly_N+1);
  if (D==0){
    tog=1;
    B=Dl;
  }
  else {
    tog=0;
    B=D;
  }
  while(B!=0){
    if (cell_is_outer(B)==TRUE){
      /* load matrix L=(x-c0) */
      for(j=1;j<=n;j++)
        HLMset(cly_L,1,j,Ipnt_coord(x,j)-cell_point(B,0,j));
      /* put U*L in last collumb of H */
      for(i=1;i<=n;i++){
        HLMset(cell_H(B),i,cly_N+1,0);
        for(j=1;j<=n;j++){ 
          HHmul(tmp,HMget(cell_U(B),i,j),HMget(cly_L,1,j));
          HHadd(HMget(cell_H(B),i,cly_N+1),HMget(cell_H(B),i,cly_N+1),tmp);
        }   
      }
      /* solve H*L=0 */
      HMbacksolve(cell_H(B),cly_L);
      if (HLeq(HVget(cly_L,n+1),0)){
         cell_print(B);
         bad_error("simplex not full dim in new_facets()");
      }
      /*
      ** we have L1*(c1-c0)+...Ln*(cn-c0)+Ln1*(x-c0)
      **   that is we have l1c0+...+lncn=x for
      **     l0=(L1+...+Ln+Ln1)/Ln1;
      **     li=-Li/Ln1
      **    we are supposed to flip if li<0 so for i in 1...n
      **        we test sign(Li)*sign(Ln1)>0
      **    and for i=0 we test sign(Ln1)*sign(L1+...+Ln)<0
      */

      HHset(l0,HVget(cly_L,n+1));
      for(i=1;i<=n;i++){
       HHmul(tmp,HVget(cly_L,cly_N+1),HVget(cly_L,i));
       if ((HLgt(tmp,0)) && (cell_ptr(B,i)==0)){
          SDx=subdiv_add_cell(cell_pivot(B,x,i),SDx);
        }
        HHadd(l0,l0,HVget(cly_L,i));
      }
      HHmul(tmp,HVget(cly_L,cly_N+1),l0);
      if ((HLlt(tmp,0)) && (cell_ptr(B,0)==0)){
        SDx=subdiv_add_cell(cell_pivot(B,x,0),SDx);
      }
    }
    B=cell_next(B);
    if (B==0 && tog ==0){
      B=Dl;
      tog=1;
    }
  }
  Hfree(tmp);
  Hfree(l0);
  return SDx;
}

/*
**  cell_find_lift
**   Input: a cell c
**          a point pt
**   Output: lowest lifting value wich can be given s while
**           keeping it in upper half space defined by lifted cell
**           c.
**
**   Note:  cell_find_lift requires normals to be stored with cell.
*/
int cell_find_lift(cell c, Ipnt pt){
  Hint dotp,dotc,res;
  int i;
  HMatrix norm;
  Hinit(dotp,0);
  Hinit(dotc,0);
  Hinit(res,0);
  norm=cell_norm(c);
  Hnorm_dot(dotp,norm,pt);
  Hnorm_dot(dotc,norm,cell_pnt(c,0));
  HLmul(res,HVget(norm,cly_N+1),cell_lift(c,0));
  HHadd(dotc,dotc,res);
  HHsub(res,dotc,dotp);
  HHdiv(dotp,res,HVget(norm,cly_N+1));
  HLadd(res,dotp,1);
  Hfree(dotp);
  Hfree(dotc);
  if (HLlt(res,1)) return 1;
  return HtoL(res);
}
/*
** subdiv_find_lift
**   Input:   a subdivision s
**            a point pt
**  Output:   maximum lifting values pt can be given while keeping
**            it simultaniously in upper (open) half spaces defined
**            by cells of s.
*/
 int cly_find_lift(cell s, Ipnt pt){
 int res, temp;
 if (s==0) return 1;
 res=cell_find_lift(s,pt);
 while((s=cell_next(s))!=0)
  if (res<(temp=cell_find_lift(s,pt))) res=temp;
 return res;
}

/* end cly_update.c */

/************************************************************************/
/************** implementation code from cly_triangulate.c **************/
/************************************************************************/

Ipnt Internalize_Aset(aset A);
void free_globals(void);
static cell new_cayley_triangulate(Ipnt *PC);
static node subdiv_get_norms(cell S, Imatrix T, int *mv);

node cly_triangulate(aset A, Imatrix T, int order, int lift){
   int mv;
   Ipnt Pts;
   cell D;
   node Res=0;
   LOCS(2);
   PUSH_LOC(A);
   PUSH_LOC(Res);
   Pts=Internalize_Aset(A);
   cly_order = order;
   cly_lift = lift; 

   D=new_cayley_triangulate(&Pts);

/*   Ipnt_list_print(Pts);*/
   if (order==TRUE) order_subdiv(&D);
   subdiv_print(D);
   if (order==TRUE) print_all_volumes(D);
   if (lift==TRUE){
     lift_original_points(Pts);
     if (T!=0) Res=subdiv_get_norms(D,T,&mv);
     else Res=0;
   }
   else {
    warning("finding facets not supported\n");
 /*   Res=subdiv_to_facets(D);*/
   }
   subdiv_free(D);
   points_free(Pts);
   free_globals();
   POP_LOCS();
   return Res;
}

Ipnt Internalize_Aset(aset A){
  Ipnt point=0,points=0;
  node ptr,ptc,ptp;
  int r;
  cly_Dim=aset_dim(A)-1;
  cly_R=aset_r(A);
  cly_N=cly_Dim+cly_R-1;
  cly_U=HMnew(cly_N,cly_N);
  cly_M=HMnew(cly_N+1,cly_N+1);
  cly_L=HVnew(cly_N+1);
  Hinit(cly_temp,0);

  /* internalize points of Aset */
  ptr = aset_start_cfg(A);
  r=0;
  cly_Npts=0;
  while ((ptc = aset_next_cfg(&ptr)) != 0) {
      r++;
      while ((ptp = aset_next_pnt(&ptc)) != 0) {
        point=Ipnt_new(ptp,r);
        Ipnt_next(point)=points;
        cly_Npts++;
        points=point;
      }
  }
  return points;
}



void free_globals(void){
/* should also reset all global params*/
 HMfree(cly_L);
 HMfree(cly_M);
 HMfree(cly_U);
 Hfree(cly_temp);
 next_id=1;
}

/*
**  Alg 2.10 of Vershelde
**
**  Input: A point configuration (with an extra coordinate to hold
**         lifting values (weights) should be initialized to zeros
**
**  Output: A triangulation of the subdivision (represented by
**          A list of point configurations)
*/
static cell new_cayley_triangulate(Ipnt *PC)
{
    Ipnt x=0,S1=0,S2=0;
    cell Dl=0;
 
    S1=*PC;
    if ((Dl=cly_initial_splx(&S1,&S2))==0)
         bad_error("failure in initial simplex");

    while(S1!=0){
      x=S1;
      S1=Ipnt_next(x);
      if (cly_lift==TRUE) Ipnt_lift(x)=cly_find_lift(Dl,x);          
      Dl=cly_subdiv_union(Dl,cly_new_cells(0,Dl,x));
      Ipnt_next(x)=S2;
      S2=x;
   }
   *PC=S2;
   return Dl;
}

/*
** subdiv_get_norms(cell S, Imatrix T, *int mv);
**
**     Input: a subdivision S (not sorted), and a type matrix T.
**    Output: a list of lifted inner normals to cells of S with
**            type T.
**    side effects: All space reserved for S is freed.
**                  mv holds the type T mixed volume.
*/
#define Norm(i) (*IVref(Norm,i))
static node subdiv_get_norms(cell S, Imatrix T, int *mv){
   node Nlist=0;
   Imatrix Norm;
   int i;
   LOCS(1);
   PUSH_LOC(Nlist);
   *mv=0;
   while (S!=0){
     if (Imatrix_order(T,cell_T(S))==0){
         Norm=Ivector_new(cly_Dim+1);
         for(i=1;i<=cly_Dim;i++) Norm(i)=HLVget(cell_norm(S),i);
         Norm(cly_Dim+1)=HLVget(cell_norm(S),cly_N+1);
         Nlist=Cons(atom_new((char *)Norm,IMTX),Nlist);
         *mv+=cell_volume(S);
     }
     S=cell_next(S);
   }
   POP_LOCS();
   return Nlist;
}

/* end cly_triangulate.c */

/************************************************************************/
/**************** implementation code from cly_continue.c ***************/
/************************************************************************/

static cell cayley_continue(Ipnt *PC, int threshold, int tweak);
static Ipnt Internalize_Aset_Cont(aset,Imatrix,int);
       Ipnt Internalize_Aset();
static void free_continuation_globals();
       void free_globals();
static void Update_Poly(Ipnt x);
static void Update_Solutions(cell Dl, int tweak);
static node relift(node *, int tweak);
static void norm_reset(cell ncell);
int cell_find_lift(cell,Ipnt);

static psys     Poly_Sys=0;
static node     Poly_Sols=0;
static Imatrix  Poly_Type=0;
static Imatrix  Poly_Norm=0;
static Imatrix  Poly_TNorm=0;
static Ipnt     *Poly_Pnts=0;
#define PPnts(i) (Poly_Pnts[(i)-1])
static HMatrix   Poly_H=0;
static HMatrix   Poly_U=0;
#define Poly_Type(i) (*IVref(Poly_Type,i))
#define Poly_Norm(i) (*IVref(Poly_Norm,i))
#define Poly_TNorm(i) (*IVref(Poly_TNorm,i))
/******************************************************************
**  Cayley Triangulation and Continuation
******************************************************************/

psys Cayley_continue(aset A,Imatrix T,node *Sols,int seed,int tweak){
   Ipnt S;
   cell D;
   psys res;
   S=Internalize_Aset_Cont(A,T,seed);
   D=cayley_continue(&S,10,tweak);
   subdiv_free(D);
   points_free(S);
   *Sols=Poly_Sols;
   res=Poly_Sys;
   free_continuation_globals();
   return res; 
}

#define T(i) (*IVref(T,i))
Ipnt Internalize_Aset_Cont(aset A, Imatrix T,int seed){
  Ipnt Pts, res;
  int i,tmp,max_monomials=0;
  /* 
  ** seed random number generator
  */
  rand_seed(seed);
  /* 
  ** initialize regular triangulation globals
  */
  res=(Pts=Internalize_Aset(A));
  Poly_Pnts=(Ipnt *)mem_malloc(cly_Npts*sizeof(Ipnt));
  /* 
  ** Store type vector in globally accessable location
  */
  Poly_Type=T;
  Poly_Norm=Ivector_new(cly_Dim+1);
  Poly_TNorm=Ivector_new(cly_Dim+1);
  Poly_H=HMnew(cly_N,cly_N+1);
  Poly_U=HMnew(cly_N,cly_N);
  /*
  ** Count total number of monomials needed
  */
  i=1;
  while(Pts!=0){
    PPnts(i++)=Pts;
    max_monomials+= T(Ipnt_idx(Pts));
    Pts=Ipnt_next(Pts);
  }
  /*
  ** initialize new psys- create it set up the block structure
  **                      implied by the type vector T.
  */
  Poly_Sys=psys_new(cly_Dim,max_monomials,cly_R);
  tmp=1;
  for(i=1;i<=IVlength(Poly_Type)+1;i++){
     *psys_block_start(Poly_Sys,i)=tmp;
     tmp+=Poly_Type(i);
  }
return res;
}
#undef T

void free_continuation_globals(){
  Poly_Sys=0;
  Poly_Type=0;
  Imatrix_free(Poly_Norm);
  Imatrix_free(Poly_TNorm);
  HMfree(Poly_U);
  HMfree(Poly_H);
  free_globals();
}


 

/*
**  Alg 2.10 of Vershelde
**
**  Input: A point configuration (with an extra coordinate to hold
**         lifting values (weights) should be initialized to zeros
**
**  Output: A triangulation of the subdivision (represented by
**          A list of point configurations)
*/
static cell cayley_continue(Ipnt *PC, int threshold, int tweak)
{
    Ipnt pt, x=0,S1=0,S2=0;
    cell D=0,Dl=0;
    int l;
    LOCS(1);
    PUSH_LOC(Poly_Sols);

    S1=*PC;
    D=cly_initial_splx(&S1,&S2);
    for(l=0;l<=cell_n(D);l++) Update_Poly(cell_pnt(D,l));
    
    while(S1!=0){
      x=S1;
      S1=Ipnt_next(x);
      Ipnt_lift(x)=(l=cly_find_lift(Dl,x));
      Update_Poly(x);
      if (l>threshold){
        Update_Solutions(Dl,tweak); 
 #ifdef LOG_PRINT
	fprintf(stdout /* was cly_out */,"flattening\n")
#endif
;
        for(pt=S2;pt!=0;pt=Ipnt_next(pt)) Ipnt_lift(pt)=0;
        psys_lift(Poly_Sys,0);
        D=cly_subdiv_union(D,Dl);
        Dl=0;
        Ipnt_lift(x)=1;
      }
      Dl=cly_subdiv_union(Dl,cly_new_cells(D,Dl,x));
      Ipnt_next(x)=S2;
      S2=x;
   }
   Update_Solutions(Dl,tweak);
   D=cly_subdiv_union(D,Dl);
   *PC=S2;
   POP_LOCS();
   return D;
}

  
/*
** Update_Poly(Ipnt x)  a monomail representing x is chosen with
**                      a random coefficient and added to the poly.
*/
static void Update_Poly(Ipnt x){
     int j,n;
     double t;
     
     n=psys_d(Poly_Sys);
     psys_Bstart_poly(Poly_Sys,Ipnt_idx(x));
     do{
       psys_init_mon(Poly_Sys);
       *psys_aux(Poly_Sys)=x;
       t=rand_double((int)0.0,(int)(2*PI));
       *psys_coef_real(Poly_Sys)=cos(t);
       *psys_coef_imag(Poly_Sys)=sin(t);
       *psys_def(Poly_Sys)=Ipnt_lift(x);
       for(j=1;j<=n;j++){
           *psys_exp(Poly_Sys,j)=Ipnt_coord(x,j);
       }
       psys_save_mon(Poly_Sys,psys_eqno(Poly_Sys));
     } 
     while(psys_Bnext_poly(Poly_Sys,Ipnt_idx(x))==TRUE);
}

int comp(node g1,node g2){
  return Imatrix_order(cell_norm((cell)Car(g1)),
                      cell_norm((cell)Car(g2)));
}

void update_list_insert(node g, node * L)
{
  node ptr = *L;
  LOCS(2);
  PUSH_LOC(*L);
  PUSH_LOC(g);
  if (g==0) bad_error("null node passed to update list");
  if (*L == 0 || comp(g, Car(*L)) > 0) *L = Cons(g, *L);
  else {
    while ((Cdr(ptr) != 0) && comp(g, Car(Cdr(ptr))) <= 0) 
       ptr = (node) Cdr(ptr);
    node_set_ptr(ptr,Cons(g, Cdr(ptr)), NODE, RIGHT);
  }
  POP_LOCS();
}

/*
** Udate_Solutions(cell D)
**                      use continuation on all solutions already 
**                      found.
**                      then for each cell in D do a lifting homotopy
**                      to add more solutions.
**                at end flatten all monomials, and solutions.
*/
static void Update_Solutions(cell Dl, int tweak){
   node tmp=0,lhead=0;
   LOCS(1);
   PUSH_LOC(lhead);
   tmp=Poly_Sols;
   /* update old solutions */
   while(tmp!=0){
       xpnt_t_set((Dmatrix)Car(Car(tmp)),0.0);
       tmp=Cdr(tmp);
   }

   if (Poly_Sols!=0){
  #ifdef LOG_PRINT     
     fprintf(stdout /* was cly_out */,"updating old cells\n")
#endif
;
       Poly_Sols=psys_hom(Poly_Sys,Poly_Sols,tweak); 
   }     

   /* find new solutions */

   /* order list of good cells by normal */
   /*      (with low numbered cells first) */
   while(Dl!=0){
     if (Imatrix_equal(Poly_Type,cell_T(Dl))==TRUE){
       update_list_insert(atom_new((char *)Dl,CELL),&lhead); 
     }
     Dl=cell_next(Dl);
   }
   /* cut out blocks, of cells */
   while(lhead!=0){
      Poly_Sols=list_cat(relift(&lhead,tweak),Poly_Sols);
   }   
   psys_lift(Poly_Sys,0);
   POP_LOCS();
}
   
/*
** 
*/
#define Tmp_Norm(i) (*IVref(Tmp_Norm,i))
static node relift(node *lhead, int tweak){
    Imatrix Tmp_Norm;
    int i,ht,tmp_ht;
    cell Tmp_Cell;
    psys Lead_Sys=0, Norm_Sys=0;
    node Lead_Sols=0,list2=0,lptr=0;
    Ipnt Mpt;
    LOCS(3);
    PUSH_LOC(*lhead);
    PUSH_LOC(Lead_Sols);
    PUSH_LOC(list2);
    /*
    ** check if cell list is empty
    */
    if (*lhead==0) bad_error("null list in relift");
    /*
    ** make copy of truncated original norm
    */
    Tmp_Norm=cell_norm((cell)Car(Car(*lhead)));
    for(i=1;i<=cly_Dim;i++){
       Poly_Norm(i)=Tmp_Norm(i);
    }
    Poly_Norm(cly_Dim+1)=Tmp_Norm(cly_N+1);
    /* 
    ** set up transformed and leading equations 
    */
    Norm_Sys=psys_norm_sub(psys_copy(Poly_Sys),Poly_Norm);
    Lead_Sys=psys_lead(Norm_Sys);
    /*
    ** initialize all points to unseen.
    */ 
    for(i=1;i<=cly_Npts;i++){
        Ipnt_seen(PPnts(i))=FALSE;
    }
    /*
    ** relift points, and cells
    */
    Tmp_Cell=(cell)Car(Car(*lhead));
    for(i=0;i<=cly_N;i++){
     Ipnt_seen(cell_pnt(Tmp_Cell,i))=TRUE;
     Ipnt_lift(cell_pnt(Tmp_Cell,i))=0;
    }
    while (Cdr(*lhead)!=0 && comp(Car(*lhead),Car(Cdr(*lhead)))==0){
      norm_reset(Tmp_Cell);
      list_push(list_pop(lhead),&list2);
      Tmp_Cell=(cell)Car(Car(*lhead));
      for(i=0;i<=cly_N;i++){
        if (Ipnt_seen(cell_pnt(Tmp_Cell,i))!=TRUE){
          lptr=list2;
          ht=cell_find_lift((cell)Car(Car(lptr))
                              ,cell_pnt(Tmp_Cell,i));
          while ((lptr=Cdr(lptr))!=0){
              tmp_ht=cell_find_lift((cell)Car(Car(lptr))
                                    ,cell_pnt(Tmp_Cell,i));
              if (ht<tmp_ht) ht=tmp_ht;
          }
          Ipnt_seen(cell_pnt(Tmp_Cell,i))=TRUE;
          Ipnt_lift(cell_pnt(Tmp_Cell,i))=ht;
        }
      }
    }
    norm_reset(Tmp_Cell);
    list_push(list_pop(lhead),&list2);

   /* lift monomials */
    FORALL_POLY(Lead_Sys,
      FORALL_MONO(Lead_Sys,
        Mpt=(Ipnt)(*psys_aux(Lead_Sys));
      if (Ipnt_seen(Mpt)==TRUE){
       *psys_def(Lead_Sys)=Ipnt_lift(Mpt);
      }
      else{
        lptr=list2;
        ht=cell_find_lift((cell)Car(Car(lptr)),Mpt);
        while ((lptr=Cdr(lptr))!=0){
           tmp_ht=cell_find_lift((cell)Car(Car(lptr)),Mpt);
           if (ht<tmp_ht) ht=tmp_ht;
        }
        Ipnt_seen(Mpt)=TRUE;
        Ipnt_lift(Mpt)=ht;
        *psys_def(Lead_Sys)=Ipnt_lift(Mpt);
      }
      );
    );
    /* 
    ** take solutions to Lead_Sys, as starting points for
    ** Norm_Sys and use homotopy.
    */
    lptr=list2;
    while (lptr!=0){
     Tmp_Norm=cell_norm((cell)Car(Car(lptr)));
     for(i=1;i<=cly_Dim;i++){
       Poly_TNorm(i)=Tmp_Norm(i);
     }
     Poly_TNorm(cly_Dim+1)=Tmp_Norm(cly_N+1);
#ifdef LOG_PRINT
     fprintf(stdout /* was cly_out */,"deforming to face:\n")
#endif
;
     Lead_Sols=list_cat(psys_solve(Lead_Sys,Poly_TNorm,tweak),Lead_Sols);
    lptr=Cdr(lptr);
    }
   /* 
   ** reset t value for these and use continuation on Sys_Norm,
   ** and return 
   */
   lptr=Lead_Sols;
   while(lptr!=0){
       xpnt_t_set((Dmatrix)Car(Car(lptr)),0.0);
       lptr=Cdr(lptr);
   }
 #ifdef LOG_PRINT
   fprintf(stdout /* was cly_out */,"deforming from face:\n")
#endif
;
   if (Lead_Sols!=0) Lead_Sols=psys_hom(Norm_Sys,Lead_Sols,tweak);
   psys_free(Lead_Sys);
   psys_free(Norm_Sys);
   POP_LOCS();
   return Lead_Sols;}
#undef Tmp_Norm

static void  norm_reset(cell ncell){
   int i,j;
  /* load coordinate matrix */
  for(i=1;i<=cly_N;i++){
    for(j=1;j<=cly_N;j++){
      HLMset(Poly_H,i,j,
             cell_point(ncell,i,j)-cell_point(ncell,0,j));
    }
 HLMset(Poly_H,i,cly_N+1,cell_lift(ncell,i)-cell_lift(ncell,0));
  }

  /* calculate normal and remove common factors from coordinates*/
  HMfactor(Poly_H,Poly_U);
  HMbacksolve(Poly_H,cell_norm(ncell));
  HMgcd_reduce(cell_norm(ncell));

  /* switch direction if necessary to ensure inner normal */
  if (HLlt(HVget(cell_norm(ncell),cly_N+1),0)) {
     for(j=1;j<=cly_N+1;j++) Hneg(HVget(cell_norm(ncell),j));
  }
  else if (HLeq(HVget(cell_norm(ncell),cly_N+1),0))
      bad_error("Cell perpendicular in  reset norm");
}
#undef Poly_Type
#undef Poly_Norm
#undef Poly_TNorm

/* end cly_continue.c */
