/*
**    copyright (c) 1995  Birk Huber
*/

#include "pelconv.h"

/* Coorcions from Gen to procedure Data structures: these do not
make a serious attempt to check validity, it is assumed that a
predicate from Pred.c will have been used first to check the
suitability of g for conversion */

/* Scalors first: */
polynomial1  Gen_To_Ply(Gen_node g) {
  polynomial1 p=nullptr;
 if (g==nullptr)  bad_error("NULL arg to Gen_To_Ply");             
  switch (g->type){
     case Int_T: p=ItoP(g->Genval.ival,Def_Ring);
                 break;
     case Dbl_T: p=DtoP(g->Genval.dval,Def_Ring);
                 break;
     case Cpx_T: p=CtoP(g->Genval.cval,Def_Ring);
                 break;
     case Ply_T: p=copyP(g->Genval.pval);
                 break;
        default: bad_error("not coorceable to ply in Gen_To_Ply");
                 break;
  }
  return p;
}            

fcomplex  Gen_To_Cpx(Gen_node g){
  fcomplex c;
  if (g==nullptr)  bad_error("NULL arg to Gen_To_Cpx\n");
  switch (g->type){
     case Int_T: c=ItoC(g->Genval.ival);
                 break;
     case Dbl_T: c=DtoC(g->Genval.dval);
                 break;
     case Cpx_T: c=g->Genval.cval;
                 break;
        default: bad_error("Don't know how to coerce in Gen_To_Cpx\n");
                 break;
  }
  return c;
}

double Gen_To_Dbl(Gen_node g)  {
  double d = 0.0;    
 if (g==nullptr) bad_error("NULL arg to Gen_To_Dbl");             
  switch (g->type){
     case Int_T: d=g->Genval.ival;       
                 break;
     case Dbl_T: d=g->Genval.dval;  
                 break;
        default: bad_error("Don't Know how to coerce in Gen_To_Dbl");
                 break;
  }
  return d;
}

int  Gen_To_Int(Gen_node g){
  int d = 0;
 if (g==nullptr) bad_error("NULL arg to Gen_To_Int");             
  switch (g->type){
     case Int_T: d=g->Genval.ival;
                 break;
        default: bad_error("Don't Know how to coerce in Gen_To_Int");
                 break;
  }
  return d;
}

Dmatrix Gen_to_Dmatrix(Gen_node g){
 Dmatrix Res;
 Gmatrix M;
 int i,j;
 M=Gen_Mtx(g);
 Res=Dmatrix_new(GMrows(M),GMcols(M));
 for(i=1;i<=GMrows(M);i++)
  for(j=1;j<=GMcols(M);j++)
    DMref(Res,i,j)=Gen_To_Dbl(*GMref(M,i,j));
 return Res;
}

Gen_node Dmatrix_to_Gen(Dmatrix M){
  Gmatrix Res;
  int i,j;
  Res=Gmatrix_new(DMrows(M),DMcols(M));
  for(i=1;i<=DMrows(M);i++)
    for(j=1;j<=DMcols(M);j++)
      *GMref(Res,i,j)=Dbl_To_Gen(DMref(M,i,j));
  return GMND(Res);
}



Imatrix Gen_to_Imatrix(Gen_node g){
 Imatrix Res;
 Gmatrix M;
 int i,j;
 M=Gen_Mtx(g);
 Res=Imatrix_new(GMrows(M),GMcols(M));
 for(i=1;i<=GMrows(M);i++)
  for(j=1;j<=GMcols(M);j++)
    *IMref(Res,i,j)=Gen_To_Int(*GMref(M,i,j));
 return Res;
}

Gen_node Imatrix_to_Gen(Imatrix M){
  Gmatrix Res;
  int i,j;
  Res=Gmatrix_new(IMrows(M),IMcols(M));
  for(i=1;i<=IMrows(M);i++)
    for(j=1;j<=IMcols(M);j++)
      *GMref(Res,i,j)=Int_To_Gen(*IMref(M,i,j));
  return GMND(Res);
}
  
 
 
 
/*-----------------------------------------------------------
 Conversions to Gen
------------------------------------------------------------*/

Gen_node Int_To_Gen(int i)
{ Gen_node g;
  g=gen_node();
  g->type=Int_T;
  g->Genval.ival=i;
  return g;
}

Gen_node Dbl_To_Gen(double d)
{ Gen_node g;
  g=gen_node();
  g->type=Dbl_T;
  g->Genval.dval=d;
  return g;
}

Gen_node Cpx_To_Gen(fcomplex c)
{ Gen_node g;
  g=gen_node();
  g->type=Cpx_T;
  g->Genval.cval=c;
  return g;
}

Gen_node Ply_To_Gen(polynomial1 p)  
{ Gen_node g;
  g=gen_node();
  g->type=Ply_T;
  g->Genval.pval=copyP(p);
  return g;
}


node  Gen_to_Dvector_list(Gen_node ptr){
    node  res=nullptr;
    LOCS(1);
    PUSH_LOC(res);
    while(ptr!=nullptr){
      if (Can_Be_Vector(ptr,Dbl_T)!=2*N+3){
        warning("bad list in Gen_to_Dvector");
        POP_LOCS();
        return nullptr;
      }
      res=Cons(atom_new((char *)Gen_to_Dmatrix(ptr),DMTX),res);
      ptr=Gen_next(ptr);
    }
    POP_LOCS();
    return res;
}

Gen_node Dvector_list_to_Gen(node DL){
  Gen_node ptc,res;
   ptc=(res=gen_node());
   while(DL!=nullptr){
      if (node_get_type(Car(DL),LEFT)!=DMTX){
        warning("bad list in Dvector_list_to_Gen\n");
        return nullptr;
      }
     Gen_set_next(ptc,Dmatrix_to_Gen((Dmatrix)Car(Car(DL))));
     ptc=Gen_next(ptc);
     DL=Cdr(DL);
   }
   ptc=Gen_node_to_List(Gen_next(res));
   free_Gen_node(res);
   return ptc;
}

Gen_node Xpl_to_Gen(node DL){
  Gen_node ptc,res;
   ptc=(res=gen_node());
   while(DL!=nullptr){
      if (node_get_type(Car(DL),LEFT)!=DMTX){
        warning("bad list in Dvector_list_to_Gen\n");
        return nullptr;
      }
     Gen_set_next(ptc,Dmatrix_to_Gen((Dmatrix)Car(Car(DL))));
     ptc=Gen_next(ptc);
     DL=Cdr(DL);
   }
   ptc=XPLND(Gen_next(res));
   free_Gen_node(res);
   return ptc;
}




Gen_node Ivector_list_to_Gen(node DL){
  Gen_node ptc,res;
   ptc=(res=gen_node());
   while(DL!=nullptr){
      if (node_get_type(Car(DL),LEFT)!=IMTX){
        warning("bad list in Ivector_list_to_Gen");
        return nullptr;
      }
     Gen_set_next(ptc,Imatrix_to_Gen((Imatrix)Car(Car(DL))));
     ptc=Gen_next(ptc);
     DL=Cdr(DL);
   }
   ptc=Gen_node_to_List(Gen_next(res));
   free_Gen_node(res);
   return ptc;
}

node  Gen_to_Ivector_list(Gen_node ptr){
    node  res=nullptr;
    LOCS(1);
    PUSH_LOC(res);
    while(ptr!=nullptr){
      if (Can_Be_Vector(ptr,Int_T)<=0){
        warning("bad list in Gen_to_Ivector");
        POP_LOCS();
        return nullptr;
      }
      res=Cons(atom_new((char *)Gen_to_Imatrix(ptr),IMTX),res);
      ptr=Gen_next(ptr);
    }
    POP_LOCS();
    return res;
}


Gen_node List_To_Aset(Gen_node g){
 Gen_node ptr;
 Gmatrix M,P;
 point pnt=nullptr;
 aset A=nullptr;
 int R,D,r=0,j,i;
 char  *s,c='a'-1;
 LOCS(2);
 PUSH_LOC(pnt);
 PUSH_LOC(A);
 if (Gen_Can_Be_Aset(g,&R,&D)!=TRUE){
        POP_LOCS(); return Rerror("",g);
 }
 A=aset_new(R,D);
 for(ptr=g;ptr!=nullptr;ptr=Gen_next(ptr)){
     M=Gen_Mtx(ptr);
     r++;
     c++;
     for(i=1;i<=GMcols(M);i++){
        s=(char *)mem_malloc(6*sizeof(char));
        s[0]=c;
        sprintf(s+1,"%d",i);
        pnt=aset_new_pt(D,s);
        P=Gen_Mtx(*GMref(M,1,i));
        for(j=1;j<=GMcols(P);j++){
           aset_pnt_set(pnt,j,Gen_int(*GMref(P,1,j)));
        }
        aset_add(A,r,pnt);
     }
 }
 POP_LOCS();
 free_Gen_list(g);
 return ASTND(A);
}

#define PV(i) Gen_poly(*GMref(PV,1,i))
psys Gen_to_psys(Gen_node g){
  Gmatrix PV;
  int n,m,r,i,j,deg,td;
  psys sys;
  polynomial1 ptr;
 
  PV=Gen_Mtx(g);
  /* pass1 collect stats on polynomial1  */
  n=poly_dim(PV(1));
  m=0; r=1;
  for(i=1;i<=GMcols(PV);i++){
    if (i>1&&orderPP(PV(i),PV(i-1))!=0) r++;
    ptr=PV(i);
    m++;
    while((ptr=poly_next(ptr))!=nullptr) m++;
  }
  sys=psys_new(n,m,r);
  *psys_block_start(sys,1)=1;
  r=1;
  /* pass2: fill psys */
  for(i=1;i<=GMcols(PV);i++){
    if (i>1&&orderPP(PV(i),PV(i-1))!=0) {
        *psys_block_start(sys,++r)=i;
    }
    ptr=PV(i);
    deg=poly_deg(ptr);
    while(ptr!=nullptr){
      while(ptr!=nullptr){
        psys_init_mon(sys);
        *psys_coef_real(sys)=(*poly_coef(ptr)).r;
        *psys_coef_imag(sys)=(*poly_coef(ptr)).i;
        *psys_def(sys)=*poly_def(ptr);
        td=0;
        for(j=1;j<=n;j++){
            *psys_exp(sys,j)=*poly_exp(ptr,j);
            td+=*psys_exp(sys,j);
        }
        *psys_homog(sys)=deg-td;
        psys_save_mon(sys,i);
        ptr=poly_next(ptr);
      }
    }
  }
  return sys;
}


Gen_node Gen_from_psys(psys sys){
 polynomial1 tmpm,tmpp;
 int j;
 Gmatrix PV;
 PV=Gmatrix_new(1,psys_d(sys));
 FORALL_POLY(sys,
   tmpp=nullptr;
   FORALL_MONO(sys,
     tmpm=makeP(Def_Ring);
     *poly_coef(tmpm)=Complex(*psys_coef_real(sys),
                              *psys_coef_imag(sys));
     *poly_def(tmpm)=*psys_def(sys);
     *poly_homog(tmpm)=*psys_homog(sys);
     for(j=1;j<=psys_d(sys);j++) *poly_exp(tmpm,j)=*psys_exp(sys,j);
     tmpp=addPPP(tmpp,tmpm,tmpp);
     freeP(tmpm);
   )
   *GMref(PV,1,psys_eqno(sys))=PLYND(tmpp);
 )
 return SYSND(PV);
}
