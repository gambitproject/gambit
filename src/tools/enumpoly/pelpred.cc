/*
**    copyright (c) 1995  Birk Huber
*/

#include "pelpred.h"

int Is_Numeric(int t){
    switch(t){
       case Int_T: case Dbl_T: case Cpx_T: case Ply_T: case Mtx_T:
                   case Sys_T: return TRUE; break;
          default: return FALSE;
    }

    return FALSE;
  }


/*
** Common_Type      takes two ptype and returns the simplest
**                  ptype to which both can be coerced.
** 
**                  Vector, and List handling must still be implemented.
**                  The code does make assumptions about the values 
**                  associated with the types.
*/
int Common_Type(int t1, int t2){
  int temp;
  if (t1==t2)return t2;
  if (t1>t2){ temp=t2; t2=t1; t1=temp;}
  if (Is_Numeric(t1)==TRUE&&Is_Numeric(t2)==TRUE) return t2;
return INVALID_T;  
}

int Can_Be_Poly(Gen_node g)
 {
  if (g==nullptr) return 0;
  switch (g->type){ case Int_T: case Dbl_T: case Cpx_T: case Ply_T:
                 return 1;
                 break;
        default:
                 return 0;
  }
  return 0;
}

int Can_Be_Cpx(Gen_node g)
 {
  if (g==nullptr) return 0;
  switch (g->type){ case Int_T: case Dbl_T: case Cpx_T: 
                 return 1;
                 break;
        default:
                 return 0;
  }
  return 0;
}
         
int Can_Be_Dbl(Gen_node g)
 {
  if (g==nullptr) return 0;
  switch (g->type){ case Int_T: case Dbl_T:
                 return 1;
                 break;
        default:
                 return 0;
  }
  return 0;
}

int Can_Be_Int(Gen_node g)
 {
  if (g==nullptr) return 0;
  switch (g->type){ case Int_T: 
                 return 1;
                 break;
        default:
                 return 0;
  }
  return 0;
}          
 
int Can_Be_Number(Gen_node g, int t){
  if (Common_Type(Gen_type(g),t)==t) return TRUE;
  else return FALSE;
}

int Can_Be_Vector(Gen_node g, int Tp){
  int r,c,t;
  if (Gen_Mtx_Specs(g,&r,&c,&t)==FALSE) return -1;

 

  if (r!=1 || c<1 || Common_Type(t,Tp)!=Tp) return -1;
 
 return c;                 

}            
   
int Can_Be_Matrix(Gen_node g, int Tp){
  int r,c,t;
  if (Gen_Mtx_Specs(g,&r,&c,&t)==FALSE) return -1;
  if (r<1 || c<1 || Common_Type(t,Tp)!=Tp) return -1;
  return c;
}

int Can_Be_Aset(Gen_node g){
 if (g==nullptr) return FALSE;
  if (Gen_type(g)==Ast_T) return TRUE;
  else return FALSE;
}


int Can_Be_List(Gen_node g){
 if (g==nullptr) return FALSE;
  switch (Gen_type(g)){
      case Xpl_T: case Npl_T: 
      case Lst_T: /*case SLL_T:*/
      return TRUE; break;
      default: break;
  } 
  return FALSE;
}
                
int Can_Be_Xpnt(Gen_node ptr){
  if (Can_Be_Vector(ptr,Dbl_T)==2*N+3) return TRUE; 
  else return FALSE;
}
 
int Can_Be_List_Of(Gen_node ptr, int (*pred)(Gen_node)){

    if (Can_Be_List(ptr)==FALSE) return FALSE; 
    ptr=Gen_lval(ptr);
    while(ptr!=nullptr){
      if (pred(ptr)!=TRUE) return FALSE;
      ptr=Gen_next(ptr);
    }
    return TRUE;
}


/*
** Gen_Can_Be_Aset
**   Input:  A Gen_node g
**   Output: TRUE if g consists of a list of point configuration
**                (each point configuration represented by a
**                 vectors of points)
**                (each point represented by a vector of integers)
**                 (all points must have same number of coords).
**           FALSE otherwise;
**   Side Effects   r is set to number of.
**                  d is set to number of coords for points.
*/
int Gen_Can_Be_Aset(Gen_node g,int *r, int *d){
  Gen_node ptr;
  Gmatrix M;
  int j,tn=0, td=0, tt=0,n;
  *r=0; *d=0;
  for(ptr=g;ptr!=nullptr;ptr=Gen_next(ptr)){
    Gen_Mtx_Specs(ptr,&tn,&td,&tt);
    if (tn!=1 || td==0) return FALSE;
    M=Gen_Mtx(ptr);
    n=tn;
    for(j=1;j<=n;j++){
       Gen_Mtx_Specs(*GMref(M,1,j),&tn,&td,&tt);
       if (tt!=Int_T || tn!=1 || td==0) return FALSE;
       if (*d!=0 && *d!=td ) return FALSE;
       if (*d==0) *d=td;
    }
    (*r)++;
  }
 return TRUE;
}

