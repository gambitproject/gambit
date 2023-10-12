/*
**    copyright (c) 1995  Birk Huber
*/

#include "pelproc.h"

Gen_node PROC_ADD(Gen_node g){
Gen_node res,g1,g2;
int rt;
Gmatrix M=nullptr;
polynomial1 tp1,tp2;
if (Gen_length(g)!=2)
    return Rerror("wrong number of arguments to PROC_ADD",g);  
g1=Gen_elt(g,1);
g2=Gen_elt(g,2);
rt=Common_Type(Gen_type(g1),Gen_type(g2));

switch(rt){
 case Int_T: res=INTND(Gen_To_Int(g1)+Gen_To_Int(g2));
             break;
 case Dbl_T: res=DBLND(Gen_To_Dbl(g1)+Gen_To_Dbl(g2));
             break;
 case Cpx_T: res=CPXND(Cadd(Gen_To_Cpx(g1),Gen_To_Cpx(g2)));
             break;
 case Ply_T: tp1=Gen_To_Ply(g1);
             tp2=Gen_To_Ply(g2);
             res=PLYND(addPPP(tp1,tp2,nullptr));
             freeP(tp1);
             freeP(tp2);
             break;
 case Sys_T:
 case Mtx_T: if ((Gen_type(g1)==Sys_T || Gen_type(g1)==Mtx_T)&&
                 (Gen_type(g2)==Sys_T || Gen_type(g2)==Mtx_T))
                  M=Gmatrix_Dop(Gen_Mtx(g1),Gen_Mtx(g2),PROC_ADD);
             if (M==nullptr) return Rerror("Matrices not compatible",g);
             if (rt==Sys_T) res=SYSND(M);
             else res=GMND(M);
             break;
 default:    res=Rerror("PROC_ADD not defined on its arguments",nullptr);
             break;
 }
free_Gen_list(g);
return(res);
}


Gen_node PROC_SUB(Gen_node g){
Gen_node res,g1,g2;
int rt;
polynomial1 tp1,tp2;
Gmatrix M=nullptr;
if (Gen_length(g)!=2)
    return Rerror("wrong number of arguments to PROC_SUB",g);
g1=Gen_elt(g,1);
g2=Gen_elt(g,2);
rt=Common_Type(Gen_type(g1),Gen_type(g2));
switch(rt){
 case Int_T: res=INTND(Gen_To_Int(g1)-Gen_To_Int(g2));
             break;
 case Dbl_T: res=DBLND(Gen_To_Dbl(g1)-Gen_To_Dbl(g2));
             break;
 case Cpx_T: res=CPXND(Csub(Gen_To_Cpx(g1),Gen_To_Cpx(g2)));
             break;
 case Ply_T: tp1=Gen_To_Ply(g1);
             tp2=Gen_To_Ply(g2);
             res=PLYND(subPPP(tp1,tp2,nullptr));
             freeP(tp1);
             freeP(tp2);
             break;
case Sys_T:
case Mtx_T: if ((Gen_type(g1)==Sys_T || Gen_type(g1)==Mtx_T)&&
                 (Gen_type(g2)==Sys_T || Gen_type(g2)==Mtx_T))
                  M=Gmatrix_Dop(Gen_Mtx(g1),Gen_Mtx(g2),PROC_SUB);
             if (M==nullptr) return Rerror("Matrices not compatible",g);
             if (rt==Sys_T) res=SYSND(M);
             else res=GMND(M);
             break;
 default:    res=Rerror("PROC_SUB not defined on its arguments",nullptr);
             break;
}
free_Gen_list(g);
return(res);
}

Gen_node PROC_SUBM(Gen_node g){
Gen_node res,g1;
Gmatrix M=nullptr;
polynomial1 tp1;
if (Gen_length(g)!=1)
    return Rerror("wrong number of arguments to PROC_SUBM",g);
    g1=Gen_elt(g,1);
    switch (Gen_type(g1)){
      case Int_T: res=INTND(-1*Gen_To_Int(g));
                  break;
      case Dbl_T: res=DBLND(-1.0*Gen_To_Dbl(g));
                  break;
      case Cpx_T: res=CPXND(RCmul(-1.0,Gen_To_Cpx(g)));
                  break;
      case Ply_T: tp1=Gen_To_Ply(g);
                  res=PLYND(mulCPP(Complex(-1.0,0.0),tp1,tp1));
                  break;
      case Sys_T:
      case Mtx_T: g1=INTND(-1);
                  M=Gmatrix_Sop(g1,Gen_Mtx(g),PROC_MUL);
                  if (M==nullptr) return Rerror("error in unary minus",g);
                  if (Gen_type(g1)==Sys_T) res=SYSND(M);
                  else res=GMND(M);
                  free_Gen_node(g1);
                  break;
         default: res=Rerror("PROC_SUB not defined on its arguments",nullptr);
                  break;
  }
 free_Gen_list(g);
 return res;
}




Gen_node PROC_MUL(Gen_node g){
Gen_node res,g1,g2,scal,mtx;
int rt,t1,t2;
polynomial1 tp1,tp2;
Gmatrix M=nullptr;
if (Gen_length(g)!=2)
    return Rerror("wrong number of arguments to PROC_MUL",g);
g1=Gen_elt(g,1);
g2=Gen_elt(g,2);
rt=Common_Type(t1=Gen_type(g1),t2=Gen_type(g2));

switch(rt){
 case Int_T: res=INTND(Gen_To_Int(g1)*Gen_To_Int(g2));
             break;
 case Dbl_T: res=DBLND(Gen_To_Dbl(g1)*Gen_To_Dbl(g2));
             break;
 case Cpx_T: res=CPXND(Cmul(Gen_To_Cpx(g1),Gen_To_Cpx(g2)));
             break;
 case Ply_T: tp1=Gen_To_Ply(g1);
             tp2=Gen_To_Ply(g2);
             res=PLYND(mulPPP(tp1,tp2,nullptr));
             freeP(tp1);
             freeP(tp2);
             break;
 case Sys_T:
 case Mtx_T:if (t1==t2){
              M=Gmatrix_Mop(Gen_Mtx(g1),Gen_Mtx(g2),
                           (res=INTND(0)),PROC_ADD,PROC_MUL);
              free_Gen_node(res);
              if (M==nullptr) res= Rerror("Incompatible matrices in MUll",nullptr);
               if (rt==Sys_T) res=SYSND(M);
               else res=GMND(M); 
            }
            else {
              if (t1!=Mtx_T){
                    scal=copy_Gen_node(g1);
                    mtx=g2;
              }
              else {
                scal=copy_Gen_node(g2);
                mtx=g1;
              }
              M=Gmatrix_Sop(scal,Gen_Mtx(mtx),PROC_MUL);
              free_Gen_node(scal);
              if (M==nullptr) res =Rerror("Incompatible matrices in Mull",nullptr);
              if (rt==Sys_T) res=SYSND(M);
              else res=GMND(M);
            }
            break;
 default:    res=Rerror("PROC_MUL not defined on its arguments",nullptr);
             break;
 }
free_Gen_list(g);
return(res);
}

Gen_node PROC_DIV(Gen_node g){
Gen_node res,g1,g2;
int rt;
polynomial1 tp1,tp2;
Gmatrix M=nullptr;
if (Gen_length(g)!=2)
    return Rerror("wrong number of arguments to PROC_DIV",g);
g1=Gen_elt(g,1);
g2=Gen_elt(g,2);
rt=Common_Type(Gen_type(g1),Gen_type(g2));

/* should test for zero */
switch(rt){
 case Int_T: 
 case Dbl_T: res=DBLND(Gen_To_Dbl(g1)/Gen_To_Dbl(g2));
             break;
 case Cpx_T: res=CPXND(Cdiv(Gen_To_Cpx(g1),Gen_To_Cpx(g2)));
             break;
 case Ply_T: tp1=Gen_To_Ply(g1);
             tp2=Gen_To_Ply(g2); /* should make sure tp2 is a monomial*/
             res=PLYND(divMPP(tp2,tp1,nullptr)); 
             freeP(tp1);
             freeP(tp2);
             break;
 case Sys_T:
 case Mtx_T: if(Gen_type(g2)!=Mtx_T){
                res=PROC_DIV(Link(INTND(1),copy_Gen_node(g2)));
              M=Gmatrix_Sop(res,Gen_Mtx(g1),PROC_MUL);
              free_Gen_node(res);
              if (rt==Sys_T) res=SYSND(M);
              res=GMND(M);
            }
            else res=Rerror("PROC_DIV cannot divide matrices",nullptr);
            break;

 default:    res=Rerror("PROC_DIV not defined on its arguments",nullptr);
             break;
 }
free_Gen_list(g);
return(res);
}




Gen_node PROC_EXP(Gen_node g)
{
Gen_node res,g1,g2;
int i,ex,ri,ti;
double rd,td;
polynomial1 tp1,tp2;
if ( g==nullptr || g->next==nullptr || g->next->next !=nullptr)
     return Rerror("wrong number of arguments to PROC_EXP",g);
if (Gen_length(g)!=2)
    return Rerror("wrong number of arguments to PROC_DIV",g);
g1=Gen_elt(g,1);
g2=Gen_elt(g,2);

 if (Can_Be_Int(g2)==TRUE){
    ex=Gen_To_Int(g2);
    if (Can_Be_Int(g1)==TRUE){
       if (ex>=0){
           ri=(ti=Gen_To_Int(g1));
           for(i=2;i<=ex;i++) ri*=ti;
           res=Int_To_Gen(ri);
       }
       else res=Dbl_To_Gen(pow(Gen_To_Dbl(g1),Gen_To_Dbl(g2)));
    }
    else if (Can_Be_Dbl(g1)==TRUE){
           rd=(td=Gen_To_Dbl(g1));
           for(i=2;i<=ex;i++) rd*=td;                                 
           res=Dbl_To_Gen(rd);
    }
    else if (Can_Be_Cpx(g1)==TRUE){
           res=Cpx_To_Gen(Cpow(Gen_To_Cpx(g1),ex));
    }
    else if (Can_Be_Poly(g1)==TRUE){
           if (ex<0) res=Rerror("can not divide polynomial1s",nullptr);
           else {
            tp1=Gen_To_Ply(g1);
            tp2=expIPP(ex,tp1,nullptr);
            res=Ply_To_Gen(tp2);
            freeP(tp2);
            freeP(tp1);
           }
    }
    else res=Rerror("Exp not defined on its arguments",nullptr);
 }
 else if (Can_Be_Dbl(g2)==TRUE && Can_Be_Dbl(g1)==TRUE){
    res=Dbl_To_Gen(pow(Gen_To_Dbl(g1),Gen_To_Dbl(g2)));
 }
 else res=Rerror("Exp not defined on its arguments",nullptr);
 
 free_Gen_list(g);   
 return(res);
}

Gen_node PROC_SET(Gen_node g)
{
Gen_node res,g1,g2;
Sym_ent ent;
if (Gen_length(g)!=2)
  return Rerror("wrong number of argument to PROC_SET",g);

 g1=Gen_elt(g,1);
 g2=Gen_elt(g,2);
if ( Gen_type(g2) == Err_T) {
     free_Gen_node(g1);
     g1=IDND("ANS");
  }

switch(Gen_type(g1)){
 case Idf_T:
         ent=Slookup(Gen_idval(g1));
          if (ent!=nullptr){
                 if (locked(ent)!=0) 
                      return Rerror("can not reset reserved word",g);
                 free_Gen_list(ent->def);
                 ent->def=g2;
                  }
          else ent=install(Gen_idval(g1),g2);
          free_Gen_node(g1);
          res=ent->def;
          break; 
 default:
         res=Rerror("first arg to PROC_SET must be itentifyer",g);
 break;
}
return res;
}

Gen_node PROC_EXIT(Gen_node g)
{
empty_symbol_table();
if(Def_Ring!=nullptr) free_Pring(Def_Ring);
node_free_store();
exit(0);
return g;
}

Gen_node Set_Ring(Gen_node g)
{
  Pring R;
  polynomial1 tp;
  int n=0;
  Gen_node pt,pt1;
  
  pt=g;
  
  while(pt!=nullptr) { 
    n++;
    pt=Gen_next(pt);
  } 
  
  R=makePR(n-1);
  n=0;
  pt=g;
  while(n<ring_dim(R)) {
    tp=makeP(R);
    *poly_coef(tp)=Complex(1.0,0.0);
    *poly_exp(tp,n+1)=1;
    pt1=PLYND(tp);
    install(pt->Genval.idval,pt1);
    ring_set_var(R,n,Gen_idval(pt));
    n++;
    
    pt=Gen_next(pt);
  }
  
  tp=makeP(R); 
  
  *poly_coef(tp)=Complex(1.0,0.0);
 
  *poly_def(tp)=1;
  pt1=PLYND(tp);
  
  install(pt->Genval.idval,pt1);
 
  ring_set_def(R,Gen_idval(pt));


  free_Gen_list(g);

  Def_Ring=R;
  N=ring_dim(R);

  return IDND("You have tried to print the Gen_node containing  the Default Ring"); 
}

Gen_node PROC_LUP(Gen_node g)
{ Sym_ent nd;
  Gen_node res,g1;
 if (Gen_length(g)!=1||Gen_type(g1=Gen_elt(g,1))!=Idf_T)
      return Rerror("null or non identifier passed to PROC_LUP",g);
 nd=Slookup(Gen_idval(g));
 if ( nd  == nullptr ) return g;
 free_Gen_node(g);
 res=copy_Gen_list(nd->def);
 return res;
}


Gen_node PROC_LAC(Gen_node g)
{
Gen_node res,g1;
int targ;
 if (Gen_length(g)!=2 ||
     Can_Be_List(Gen_elt(g,1))!=TRUE ||
     Gen_type(Gen_elt(g,2))!=Int_T)
  return Rerror("PROC_LAC wrong number of arguments",g);
 g1=Gen_lval(Gen_elt(g,1));
 targ=Gen_To_Int(Gen_elt(g,2));
 if (targ<1||targ>Gen_length(g1) )
         return Rerror("too few elements in list",g);
 res=copy_Gen_node(Gen_elt(g1,targ));
 free_Gen_list(g);  
 return res;
}

Gen_node PROC_MAC(Gen_node g)
{
 Gmatrix M;
 Gen_node g1,g2,g3,res;
 int r,c;
 if ((Gen_length(g)!=3)||
     ((Gen_type(g1=Gen_elt(g,1))!=Mtx_T)&&(Gen_type(g1)!=Sys_T))||
     (Gen_type(g2=Gen_elt(g,2))!=Int_T)||
     (Gen_type(g3=Gen_elt(g,3))!=Int_T)) 
             return Rerror("bad args PROC_MAC",g);
 M=Gen_Mtx(g1);
 r=Gen_To_Int(g2);
 c=Gen_To_Int(g3);
 if (1>r || GMrows(M)<r || 1>c || GMcols(M)<c)
             return Rerror("bad indices PROC_MAC",g);
 res=copy_Gen_list(*GMref(M,r,c));
 free_Gen_list(g);
 return res;
}
    

 
Gen_node PROC_MAT(Gen_node g){
 int i=0,j=0,r,c;
 Gmatrix M;
 Gen_node ptr=g,ptc;

 if ((r=Gen_length(ptr))==0||Can_Be_List(ptr)==FALSE) 
       return Rerror("bad argument to PROC_MAT",g);
 c=Gen_length(Gen_lval(ptr));
 while((ptr=Gen_next(ptr))!=nullptr){
  if (Gen_length(Gen_lval(ptr))!=c) return Rerror("Bad Arg to Proc_mat",g);
 }
 M=Gmatrix_new(r,c);
 ptr=g;
 while(ptr!=nullptr){
  i++; j=0;
   ptc=Gen_lval(ptr);
     while(ptc!=nullptr){
       ++j;  
        *GMref(M,i,j)=copy_Gen_node(ptc);
        ptc=Gen_next(ptc);
     }
   ptr=Gen_next(ptr);
 }
free_Gen_list(g);
return GMND(M);
}
