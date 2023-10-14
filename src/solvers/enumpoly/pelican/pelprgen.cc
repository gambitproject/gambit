/*
**  Proc_Gen.c 
**     Commands defining Pelican functions 
** 
**    copyright (c) 1995  Birk Huber
*/

/* This version is obtained from editing by Andy McLennan - 1999 */

#include "pelprgen.h"

static int time0 = 0; /* initialized to 0 to get rid of warning - AMM */

/* --------------------------------------------------------------
 Install_Command(Gen_node (*G)(),char *s)
   takes a pointer to a Shell Procedure and a string s, installs
   the pair on the symbol table-- and returns the value of the 
   resulting symbol table entry. 

 Error Conditions: 
    if either G or S are NULL nothing is done and Null is returned
    if Gen_node() fails NULL is returned.
    install will return NULL on some error conditions and this
    will be passed through lock and returned.
                   
--------------------------------------------------------------*/
/*
Sym_ent Install_Command(Gen_node (*G)(),char *s){
    return lock(install(s,PND(G))); 
}
*/

/*----------------------------------------------------------------- 
int Install_Gen_Commands()    
        Uses Install_Command to install all Commands
        On the symbol table. Always returns 1;
  
Error Conditions: None;
-----------------------------------------------------------------*/


/********************************************************************
 Begin Command Definitions
********************************************************************/

/*
** Help Command:
*/
Gen_node G_Help(Gen_node g){
 char *lable;
 if (Gen_length(g)!=1 || (lable=Gen_idval(Gen_elt(g,1)))==nullptr)
   return Rerror("usage: help(topic) where topic is a string",g);
 /*  print_doc(lable); REMOVED */
 free_Gen_list(g);
 g=IDND("");
 return g;
}

/* 
** Aset Command          (tested on poly's)
*/
/* aset psys_to_aset(psys); IN psys.h */
Gen_node G_Aset(Gen_node g){
  psys P;
  aset A=nullptr;
  /*
  if (Gen_length(g)==1&&Can_Be_Vector(g,Ply_T)>0){
  
 
  */
if (Gen_length(g)==1&&Can_Be_Vector(g,Ply_T)>0){
  
P=Gen_to_psys(g);
/*
    free_Gen_list(g);
    */
    A=psys_to_aset(P);

    psys_free(P);

    return ASTND(A);
  }

 return List_To_Aset(g);
}

/* 
** Cayley Triangulation Command      (tested: looses 2 )
**                                (problem probably in cly_triangulate)
*/
Gen_node G_Ctrig(Gen_node g){
  aset A=nullptr,CP=nullptr;
  Ivector T=nullptr;
  int r = 0;
  Gen_node ptr;
  int nargs;
  LOCS(2);
  PUSH_LOC(A);
  PUSH_LOC(CP);
  nargs=Gen_length(g);
  if (  (nargs ==0) ||
        (nargs==1  && Can_Be_Aset(Gen_elt(g,1))!=TRUE)||
        (nargs==2  && (r=Can_Be_Vector(Gen_elt(g,2),Int_T))<0)){
        POP_LOCS();
        return Rerror("Usage: Ctrig(Aset,<Int>)",g);
  }
  A=Gen_aset(Gen_elt(g,1));
  if (nargs==2 ){
     if (r==aset_r(A)) T=Gen_to_Imatrix(Gen_elt(g,2));
     else {
       POP_LOCS();
       return Rerror("Usage: Ctrig(Aset,<Int>)",g);
     }
  }
  
  time0=set_mark();
  CP=cly_triangulate(A,T,TRUE,TRUE); 
#ifdef LOG_PRINT 
 fprintf(stdout /* was Pel_Out */,"\ntime used %d,\n",read_mark(time0))
#endif
;
  free_Gen_list(g);

  if (CP!=nullptr){
     ptr=(g=Imatrix_to_Gen((Imatrix)Car(Car(CP))));
     while((CP=Cdr(CP))!=nullptr) ptr=Gen_set_next(ptr,
                              Imatrix_to_Gen((Imatrix)Car(Car(CP))));
     g=Gen_node_to_List(g);
  }
  else g=IDND("");

  if (T!=nullptr) Imatrix_free(T);
  POP_LOCS();
  return g;
}

/*
** Cayley Triangulation Command      (tested: looses 2 )
**                                (problem probably in cly_triangulate
)
*/
Gen_node G_GenSolve(Gen_node g, int tweak){
  aset A=nullptr;
  int seed;
  psys Sys;
  /* Cayley_continue(aset,Ivector,node *,int,int); IN cly_all.h */
  node Sols=nullptr;
  Ivector T=nullptr;
  int r;
  int nargs;
  LOCS(2);
  PUSH_LOC(A);
  PUSH_LOC(Sols);
  nargs=Gen_length(g);
  if (  (nargs <2) ||
      (Can_Be_Aset(Gen_elt(g,1))!=TRUE)||
      ((r=Can_Be_Vector(Gen_elt(g,2),Int_T))<0)||
      (nargs>=3  && Can_Be_Int(Gen_elt(g,3))!=TRUE)||
      (nargs>3)){
    POP_LOCS();
    return Rerror("Usage: GenSolve(Aset,<Int>,Int)",g);
  }
  A=Gen_aset(Gen_elt(g,1));
  if (nargs>=2 ){
    if (r==aset_r(A)) T=Gen_to_Imatrix(Gen_elt(g,2));
    else {
      POP_LOCS();
      return Rerror("Usage: GenSolve(Aset,<Int>,Int)",g);
    }
  }
  if (nargs==3) seed=Gen_To_Int(Gen_elt(g,3));
  else seed=20;

  time0=set_mark();
  Sys=Cayley_continue(A,T,&Sols,seed,tweak);
#ifdef LOG_PRINT  
fprintf(stdout /* was Pel_Out */,"\ntime used %d,\n",read_mark(time0))
#endif
;
#ifdef LOG_PRINT
  xpl_fprint(stdout /* was Pel_Out */,Sols)
#endif
;
  free_Gen_list(g);
  g=Gen_node_to_List(Link(Gen_from_psys(Sys),Xpl_to_Gen(Sols))); 
  if (T!=nullptr) Imatrix_free(T);
  psys_free(Sys);
  POP_LOCS();
  return g;
}



/*
** MSD Command           (not working, not tested)
*/
Gen_node G_MSD(Gen_node g){
  aset A=nullptr,CP=nullptr;
  Ivector T=nullptr;
  int r;
  Gen_node ptr;
  int nargs;
  LOCS(2);
  PUSH_LOC(A);
  PUSH_LOC(CP);
  nargs=Gen_length(g);
  if((nargs!=2) ||
     (Can_Be_Aset(Gen_elt(g,1))!=TRUE)||
     ((r=Can_Be_Vector(Gen_elt(g,2),Int_T))<0)){
       POP_LOCS();
       return Rerror("Usage: Ctrig(Aset,<Int>)",g);
  }
  A=Gen_aset(Gen_elt(g,1));
  if (r==aset_r(A)) T=Gen_to_Imatrix(Gen_elt(g,2));
  else {
    POP_LOCS();
    return Rerror("Usage: Ctrig(Aset,<Int>)",g);
  }

  time0=set_mark();
  CP=MSD(A,T); 
#ifdef LOG_PRINT 
 fprintf(stdout /* was Pel_Out */,"\ntime used %d,\n",read_mark(time0))
#endif
;
  free_Gen_list(g);
  if (CP!=nullptr){
     ptr=(g=Imatrix_to_Gen((Imatrix)Car(Car(CP))));
     while((CP=Cdr(CP))!=nullptr) ptr=Gen_set_next(ptr,
                              Imatrix_to_Gen((Imatrix)Car(Car(CP))));
     g=Gen_node_to_List(g);
  }
  else g=IDND("");
  if (T!=nullptr) Imatrix_free(T);
  POP_LOCS();
  return g;
}

/*
** Qtrig Command               (tested)
*/
Gen_node G_Qtrig(Gen_node g){
  aset A=nullptr,CP=nullptr;
  Ivector T=nullptr;
  int r = 0;
  Gen_node ptr;
  int nargs;
  LOCS(2);
  PUSH_LOC(A);
  PUSH_LOC(CP);
  nargs=Gen_length(g);
  if ((nargs ==0) ||
      (nargs==1  && Can_Be_Aset(Gen_elt(g,1))!=TRUE )||
      (nargs==2  && (r=Can_Be_Vector(Gen_elt(g,2),Int_T))<0)){
    POP_LOCS();
    return Rerror("Usage: Qtrig(Aset,<Int>)",g);
  }
  A=Gen_aset(Gen_elt(g,1));
  if (nargs==2 ){
    if (r==aset_r(A)) T=Gen_to_Imatrix(Gen_elt(g,2));
    else {
      POP_LOCS();
      return Rerror("Usage: Qtrig(Aset,<Int>)",g);
    }
  }
  time0=set_mark();
  CP=aset_print_subdiv(A,aset_lower_facets(A),T);   

  /*
#ifdef LOG_PRINT 
 fprintf(stdout // was Pel_Out
 ,"\ntime used %d,\n",read_mark(time0))
#endif
;
*/
  /*  -- SUSPICIOUS
 free_Gen_list(g);
 */  
if (CP!=nullptr){
    ptr=(g=Imatrix_to_Gen((Imatrix)Car(Car(CP))));
    while((CP=Cdr(CP))!=nullptr){
      ptr=Gen_set_next(ptr,Imatrix_to_Gen((Imatrix)Car(Car(CP))));
    }
    g=Gen_node_to_List(g);
  }
  else g=IDND("");
  if (T!=nullptr) Imatrix_free(T);
  POP_LOCS();
  return g;
}


/* 
** Extremal Command                 (tested)
*/
Gen_node G_Extremal(Gen_node g){
  aset A;
  if (Gen_length(g)!=1 || Can_Be_Aset(Gen_elt(g,1))!=TRUE)
        return Rerror("Extremal: Usage Extremal(Ast)",g);
  A=Gen_aset(g);
  time0=set_mark();
  aset_extremal(A);
  /*
#ifdef LOG_PRINT
  fprintf(stdout // was Pel_Out
,"\ntime used %d,\n",read_mark(time0))
#endif
;
*/
  return g;
}

/*
** UnLift Command         (tested)
*/
Gen_node G_UnLift(Gen_node g){
  Gen_node ptr;
  psys M;




  if (Gen_length(g)==1){
    switch (Gen_type(g)){
       case Ast_T: aset_unlift(Gen_aset(g));
                   return g;
                   break;
       case Ply_T: unliftP(Gen_poly(g));
                   return g;
                   break;
       case Sys_T:
       case Mtx_T: if (Can_Be_Vector(g,Ply_T)>0){
                    M=Gen_to_psys(g);
		    /*                    
free_Gen_list(g);
*/                    
psys_lift(M,0);
                    g=Gen_from_psys(M);
                    psys_free(M);
                    return g;
                   }
                   break;
       default: 
          if ( Can_Be_List(g)==TRUE ){
              ptr=Gen_lval(g);
              while(ptr!=nullptr){
                if (Can_Be_Vector(ptr,Dbl_T)<1){
                   return Rerror("Usage: Unlift({<Dbl>})",g);
                 }
                free_Gen_node(*GMref(Gen_Mtx(ptr),1,
                                     GMcols(Gen_Mtx(ptr))));
                *GMref(Gen_Mtx(ptr),1,GMcols(Gen_Mtx(ptr)))=DBLND(0.0);
                 ptr=Gen_next(ptr);
              }
              return g;
          }
          else return Rerror("Usage: Unlift(Aset|<Ply>|{<Dbl>})",g);

    }
  }
return nullptr; /* not reachable */
}

/*
** Randlift Command          (tested)
*/
Gen_node G_RandLift(Gen_node g){
  aset A;
  /* 
  int seed, low, high;

 if (Gen_length(g)!=4 || 
      Can_Be_Aset(Gen_elt(g,1))!=TRUE||
      Can_Be_Int(Gen_elt(g,2))!=TRUE||
      Can_Be_Int(Gen_elt(g,3))!=TRUE||
      Can_Be_Int(Gen_elt(g,4))!=TRUE)
        return Rerror("Usage: RandLift(Ast,Int,Int,Int)",g);
  A=Gen_aset(Gen_elt(g,1));
  seed=Gen_To_Int(Gen_elt(g,2));
  low=Gen_To_Int(Gen_elt(g,3));
  high=Gen_To_Int(Gen_elt(g,4));
  aset_randlift(A,seed,low,high);
  free_Gen_list(Gen_elt(g,2));
   
 return g;
 */
  
  A=Gen_aset(g);
  aset_randlift(A,10,0,200);
  return g;
}

/*
** Lift Command
*/

/*
** Face Command
*/
Gen_node G_Face(Gen_node g){
  aset A;
  int n;
  Imatrix Norm;
  if (Gen_length(g)==2 &&  Can_Be_Aset(Gen_elt(g,1))==TRUE ) {
     A=Gen_aset(Gen_elt(g,1));
     n=Can_Be_Vector(Gen_elt(g,2),Int_T);
     if (n==aset_dim(A)) {
        Norm=Gen_to_Imatrix(Gen_elt(g,2));
        free_Gen_list(g);
        g=ASTND(aset_face(A,Norm));
        Imatrix_free(Norm);
        return g;
     }
  }
 return Rerror("Usage: Face(Aset,Norm)",g);
}

Gen_node G_Save(Gen_node g){
  FILE *tmp;
  if (Gen_length(g)!=1) return Rerror("Print: too many arguments",g);
  tmp=stdout /* was Pel_Out */;
/*  stdout // was Pel_Out 
=Pel_Log; */
#ifdef LOG_PRINT
  fprintf(stdout /* was Pel_Out */,"\n")
#endif
;                                             
  switch (Gen_type(g)){ 
      case Ast_T:  aset_print(Gen_aset(g));
                    break;                 
      default: print_Gen_node(g);          
  }                              

#ifdef LOG_PRINT
 fprintf(stdout /* was Pel_Out */,"\n")
#endif
;          
 /*  Pel_Log=tmp;  */
 free_Gen_list(g);
 return IDND("");      
 }                     


/* 
** System Command                     (tested)
*/
Gen_node G_System(Gen_node g) { 
  int i,order=0;
  Gmatrix M;
  /*   Gen_node SYSND(); IN gennode.h */
  Gen_node T;

if (Can_Be_Vector(Gen_elt(g,1),Ply_T)<0){    


return Rerror("System: Usage System(<Ply>)",g);
  }
 


 M=Gmatrix_copy(Gen_Mtx(g));
 
  free_Gen_list(g);
  while (order==0){
     order=1;
     for(i=1;i<GMcols(M);i++){


     if (orderPP(Gen_poly(*GMref(M,1,i)),
                 Gen_poly(*GMref(M,1,i+1)))<0){
           order=0;
           T=*GMref(M,1,i+1);
           *GMref(M,1,i+1)=*GMref(M,1,i);
           *GMref(M,1,i)=T;
	     
  }
  
     }
 }
return SYSND(M);
}

/* 
** GenPoly Command         (tested)
*/
psys aset_to_psys(aset,Ivector,int);
Gen_node G_Gen_Poly(Gen_node g){
  aset A;
  Ivector I;
  int seed,l;
  psys P;
  l=Gen_length(g);
  if (
      (l<2) ||
      (l>=2 && Can_Be_Aset(Gen_elt(g,1))!=TRUE
            && Can_Be_Vector(Gen_elt(g,2),Int_T)<1) ||
      (l==3 && Can_Be_Int(Gen_elt(g,3))!=TRUE) ||
      (l>3))
      {
  return Rerror("Usage: GenPoly(Aset,<Int>,[Int]);",g);
  }

   A=Gen_aset(Gen_elt(g,1));
   I=Gen_to_Imatrix(Gen_elt(g,2));
   if (l==3) seed=Gen_To_Int(Gen_elt(g,3));
   else seed=3;
   P=aset_to_psys(A,I,seed);
   free_Gen_list(g);
   g=Gen_from_psys(P);
   psys_free(P);
   Imatrix_free(I);
   return g;
}

/*
** Atype command                        (tested)
*/
Gen_node G_AType(Gen_node g){
  Ivector I;
  psys P;
  if (Gen_length(g)!=1 || Can_Be_Vector(Gen_elt(g,1),Ply_T)<1)
      {
  return Rerror("Usage: AType(<Ply>);",g);
  }
  P=Gen_to_psys(g);
  /* 
 free_Gen_list(g);
 */  
I=psys_type(P);
  g=Imatrix_to_Gen(I);
  Imatrix_free(I);
  psys_free(P);
  return g;
}

/* 
** Continuation Command    (tested)
*/
Gen_node G_Cont(Gen_node g, int tweak){
   psys P;
   node Sl;
   if ( Gen_length(g)!=2||
        Can_Be_Vector(Gen_elt(g,1),Ply_T)<=0||
        Can_Be_List(Gen_elt(g,2))!=TRUE)
        return Rerror("Usage: Cont(PSys,<Dbl>)",g);

   P=Gen_to_psys(Gen_elt(g,1));
   Sl=Gen_to_Dvector_list(Gen_lval(Gen_elt(g,2)));
   time0=set_mark();
   Sl=psys_hom(P,Sl,tweak);
   /*
#ifdef LOG_PRINT 
  fprintf(stdout // was Pel_Out 
,"time used %d,\n",read_mark(time0))
#endif
;
*/  
 free_Gen_list(g);
   psys_free(P);
   return Xpl_to_Gen(Sl);
 }

/*
** Solve Command             (tested)
*/
Gen_node G_Solve(Gen_node g, int tweak){
   psys P;
   node Sl=nullptr,Nl=nullptr;
   /*   char *tmp; UNUSED */
   LOCS(2);
   PUSH_LOC(Sl);
   PUSH_LOC(Nl);
   if ( Gen_length(g)!=2||
        Can_Be_Vector(Gen_elt(g,1),Ply_T)<=0||
        Can_Be_List(Gen_elt(g,2))!=TRUE)
        return Rerror("Usage: Solve(PSys,{<Int>})",g);
   
   P=Gen_to_psys(Gen_elt(g,1));
   Nl=Gen_to_Ivector_list(Gen_lval(Gen_elt(g,2)));
   time0=set_mark();
   while (Nl!=nullptr){
     Sl=list_cat(psys_solve(P,(Imatrix)Car(Car(Nl)),tweak),Sl);
     Nl=Cdr(Nl);
   }
   /*   Pel_New_Log(strcat(FilePrefix,".start")); */
#ifdef G_SOLVE_PRINT 
  fprintf(stdout /* was Pel_Log */,"%% Pelican Output File: Generic System and Solutions\n");

   fprintf(stdout /* was Pel_Log */,"G=");
   psys_fprint(stdout /* was Pel_Log */,P);
   fprintf(stdout /* was Pel_Log */,";\n");
   fprintf(stdout /* was Pel_Log */,"S=\n");
   xpl_fprint(stdout /* was Pel_Log */,Sl);
   /*   Pel_Open_Log(Pel_LogName);  */
   
  
#endif
;
   /*  SUSPICIOUS
 free_Gen_list(g);
 */
  psys_free(P);
   POP_LOCS();
   return Xpl_to_Gen(Sl);
 }

/*
** ParamSet Command                      (tested)
*/

Gen_node G_ParamSet(Gen_node g){
 Gen_node ptr;
 if (g==nullptr) return Rerror("NULL Arg in SetParam",g);
 ptr=g;
  while (ptr!=nullptr){
   if (ptr->type!=Idf_T)
      return Rerror("Expecting Identifier in SetParam",g);
   SET_D_PARAM(Hom_tol)
 /*  SET_I_PARAM(Hom_use_proj) */
   SET_D_PARAM(PN_dt0)
   SET_D_PARAM(PN_maxdt)
   SET_D_PARAM(PN_mindt)
   SET_D_PARAM(PN_scaledt)
   SET_D_PARAM(PN_cfac)
   SET_D_PARAM(PN_NYtol)
   SET_D_PARAM(PN_NDtol)
   SET_D_PARAM(PN_Nratio)
   SET_D_PARAM(PN_tfinal)
   SET_D_PARAM(PN_FYtol)
   SET_D_PARAM(PN_FDtol)
   SET_D_PARAM(PN_Fratio)
   SET_D_PARAM(PN_Nratio)
   SET_I_PARAM(PN_maxsteps)
   SET_B_PARAM(Show_Sys)
   SET_B_PARAM(Show_Xpl)
   SET_S_PARAM(FilePrefix)
   else if (strcmp(ptr->Genval.idval,"Use_Hompack")==0){
    Cont_Alg=USE_HOMPACK;
    ptr=ptr->next;
   }
   else if (strcmp(ptr->Genval.idval,"Use_PNewton")==0){
    Cont_Alg=USE_PNEWTON;
    ptr=ptr->next;
   }
   else return Rerror("Unrecognized Parameter",g);
  }
  free_Gen_list(g);
  return IDND("");
}

#undef SET_D_PARAM
#undef SET_I_PARAM
#undef SET_B_PARAM

/*
** ParamShow Command
*/
#define SHOW_PARAM(Svar,T)\
    else if (strcmp(ptr->Genval.idval,#Svar)==0)\
         fprintf(stdout /* was Pel_Out */,#Svar T,Svar);
#define SHOW_I_PARAM(Svar) SHOW_PARAM(Svar,"=%d")
#define SHOW_D_PARAM(Svar) SHOW_PARAM(Svar,"=%g")
#define SHOW_S_PARAM(Svar) SHOW_PARAM(Svar,"=%s")
#define SHOW_B_PARAM(Svar)\
    else if (strcmp(ptr->Genval.idval,#Svar)==0){\
        if (Svar==TRUE) fprintf(stdout /* was Pel_Out */,#Svar "=TRUE");\
        else fprintf(stdout /* was Pel_Out */,#Svar "=FALSE");\
    }
#define DIS_PARAM(Svar,T) fprintf(stdout /* was Pel_Out */,#Svar T,Svar);


Gen_node G_ParamShow(Gen_node g){
 Gen_node ptr;
 if (g==nullptr) return Rerror("NULL Arg in ShowParam",g);
 ptr=g;
 while (ptr!=nullptr){
   if (ptr->type!=Idf_T)
         return Rerror("Expecting Identifier ShowParam",g);
   SHOW_D_PARAM(Hom_tol)
   SHOW_D_PARAM(PN_dt0)
   SHOW_D_PARAM(PN_maxdt)
   SHOW_D_PARAM(PN_mindt)
   SHOW_D_PARAM(PN_scaledt)
   SHOW_D_PARAM(PN_cfac)
   SHOW_D_PARAM(PN_tfinal)
   SHOW_D_PARAM(PN_NYtol)
   SHOW_D_PARAM(PN_NDtol)
   SHOW_D_PARAM(PN_Nratio)
   SHOW_D_PARAM(PN_FYtol)
   SHOW_D_PARAM(PN_FDtol)
   SHOW_D_PARAM(PN_Fratio)
   SHOW_I_PARAM(PN_maxsteps)
   SHOW_B_PARAM(Show_Sys)
   SHOW_B_PARAM(Show_Xpl)
   SHOW_S_PARAM(FilePrefix)
/*   SHOW_I_PARAM(Hom_use_proj) */
   else if (strcmp(ptr->Genval.idval,"Cont_Alg")==0){
      if (Cont_Alg==USE_HOMPACK)
#ifdef LOG_PRINT
fprintf(stdout /* was Pel_Out */,"Using Hompack")
#endif
;
      else if (Cont_Alg==USE_PNEWTON)
#ifdef LOG_PRINT
fprintf(stdout /* was Pel_Out */,"Using PNewton")
#endif

;
   }
   else if (strcmp(ptr->Genval.idval,"Hom_Params")==0)
      print_Hom_params(stdout /* was Pel_Out */);
   else if (strcmp(ptr->Genval.idval,"PN_Params")==0){
     DIS_PARAM(PN_dt0,"= %g,   ")
     DIS_PARAM(PN_scaledt,"=%g, \n")
     DIS_PARAM(PN_maxdt,"= %g,  ")
     DIS_PARAM(PN_mindt,"= %g\n")
     DIS_PARAM(PN_cfac, "= %g,  ")
     DIS_PARAM(PN_tfinal," = %g,  ")
     DIS_PARAM(PN_maxsteps,"=%d\n")
     DIS_PARAM(PN_NYtol,"= %g,  ")
     DIS_PARAM(PN_NDtol,"= %g,  ")
     DIS_PARAM(PN_Nratio,"= %g\n")
     DIS_PARAM(PN_FYtol,"= %g,  ")
     DIS_PARAM(PN_FDtol,"= %g,  ")
     DIS_PARAM(PN_Fratio,"= %g\n")
   }
   else return Rerror("Unrecognized Parameter",g);
   ptr=ptr->next;
 }
 free_Gen_list(g);
 return IDND("");
}
#undef SHOW_PARAM
#undef SHOW_I_PARAM
#undef SHOW_D_PARAM
#undef SHOW_B_PARAM
#undef DIS_PARAM


/*
** NormSub Command (tested)
*/
Gen_node G_NormSub(Gen_node g){
  psys P;
  Imatrix Nrm;
  if (Gen_length(g)!=2 ||
     (Can_Be_Vector(Gen_elt(g,1),Ply_T)<=0)||
     (Can_Be_Vector(Gen_elt(g,2),Int_T)<=0))
      return Rerror("Usage: NormSub(<Ply>,<Int>)",g);
  P=Gen_to_psys(Gen_elt(g,1));
  Nrm=Gen_to_Imatrix(Gen_elt(g,2));
  P=psys_norm_sub(P,Nrm);
  free_Gen_list(g);
  g=Gen_from_psys(P);
  psys_free(P);
  Imatrix_free(Nrm);
  return g;
}

/*
** Lead Command (tested)
*/
Gen_node G_Lead(Gen_node g){                                 
  psys PS1,PS2;
  if (Gen_length(g)!=1 ||
     (Can_Be_Vector(Gen_elt(g,1),Ply_T)<=0))
      return Rerror("Usage: <Ply> Lead(<Ply>)",g);
  PS1=Gen_to_psys(g);             
  free_Gen_list(g);                 
  PS2=psys_lead(PS1);
  g=Gen_from_psys(PS2);
  psys_free(PS2);
  psys_free(PS1);
  return g;
}

/*
** Verify Command (tested)
*/
Gen_node G_Verify(Gen_node g){
  Gen_node ptr,res,ptc;
  psys PS;
  Dmatrix D;

  if (Gen_length(g)!=2|| 
      Can_Be_Vector(Gen_elt(g,1),Ply_T)<0||
      Can_Be_List(Gen_elt(g,2))!=TRUE)
  return Rerror("Usage: Verify(<Ply>,{<Dbl>}",g);
  PS=Gen_to_psys(Gen_elt(g,1));
  ptr=Gen_lval(Gen_elt(g,2));
  ptc=(res=gen_node());
  while(ptr!=nullptr){
    if (Can_Be_Vector(ptr,Dbl_T)==2*N+3){
       D=Gen_to_Dmatrix(ptr);
       Gen_set_next(ptc,Dbl_To_Gen(psys_abs(PS,D)));
       ptc=Gen_next(ptc);
       Dmatrix_free(D);
    }
    ptr=Gen_next(ptr);
  }
  psys_free(PS);
  /*
  free_Gen_list(g);
  */ 
 g=Gen_node_to_List(Gen_next(res));
  free_Gen_node(res);
  return g;
}

/*
** Eval Command
*/
Gen_node G_Eval(Gen_node g){
  Gen_node ptr,res,ptc;
  psys PS;
  Dmatrix X=nullptr,Y=nullptr;
  Gmatrix V=nullptr;
  int i,xp;

  if (Gen_length(g)!=2||
      Can_Be_Vector(Gen_elt(g,1),Ply_T)<0||
         ( ((xp=Can_Be_Xpnt(Gen_elt(g,2)))==FALSE) &&
           (Can_Be_List_Of(Gen_elt(g,2),*Can_Be_Xpnt)==FALSE)))
  return Rerror("Usage: Eval(<Ply>,{<Dbl>}",g);
  PS=Gen_to_psys(Gen_elt(g,1));
  if (xp==TRUE) ptr=Gen_elt(g,2);
  else ptr=Gen_lval(Gen_elt(g,2));
  ptc=(res=gen_node());
  while(ptr!=nullptr){
    if (Can_Be_Vector(ptr,Dbl_T)==2*N+3){
       X=Gen_to_Dmatrix(ptr);
       V=Gmatrix_new(1,N);
       Y= psys_eval(PS,X,Y);
       for(i=1;i<=N;i++){
         *GMref(V,1,i)=Cpx_To_Gen(Complex(DVref(Y,2*i-1),DVref(Y,2*i)));
       }
       Gen_set_next(ptc,GMND(V));
       ptc=Gen_next(ptc);
       Dmatrix_free(X);
    }
    ptr=Gen_next(ptr);
  }
  psys_free(PS);
  free_Gen_list(g);
  Dmatrix_free(Y);
  if (xp==TRUE) g=Gen_next(res);
  else  g=Gen_node_to_List(Gen_next(res));
  free_Gen_node(res);
  return g;
}



/*
** BinSolve Command:  (tested)
*/
Gen_node G_BinSolve(Gen_node g){
   psys P;
   node Sl;
   if ( Gen_length(g)!=1|| Can_Be_Vector(Gen_elt(g,1),Ply_T)<=0)
      return Rerror("Usage: BinSolve(<Ply>)",g);
   P=Gen_to_psys(Gen_elt(g,1));
   time0=set_mark();
   Sl=psys_binsolve(P);
   free_Gen_list(g);
   psys_free(P);
   return Xpl_to_Gen(Sl);
 }

/*
** Scale Command       (tested)
*/
Gen_node G_Scale(Gen_node g){
 psys PS;
 Dmatrix S;
 int l;
 if ( 
    ((l=Gen_length(g))<1)||
    ((l>=1)&&(Can_Be_Vector(Gen_elt(g,1),Ply_T)<=0))||
    ((l>=2)&&(Gen_type(Gen_elt(g,2))!=Idf_T))||
    (l>2))
     return Rerror("Usage: Scale(<Ply>,[Idf])",g);
 PS=Gen_to_psys(g);
 S=psys_scale(PS);
 if (l==2) install(Gen_idval(Gen_elt(g,2)),Dmatrix_to_Gen(S));
 /*
 free_Gen_list(g);
 */ 
g=Gen_from_psys(PS);
 Dmatrix_free(S);
 psys_free(PS);
return g;
}

/* 
** UnScale Command         (tested)
*/
Gen_node G_UnScale(Gen_node g){
 Gen_node ptr, res,rptr;
 Dmatrix scl;
 Dmatrix root = nullptr;  /* Initialized to get rid of C++ warning */

 if (g==nullptr || 
     Gen_length(g)!=2 || 
     Can_Be_Vector(Gen_elt(g,2),Dbl_T)!=N ||
     Can_Be_List(g)!=TRUE)
    
  return Rerror("Usage: UnScale({<Dbl>},<Dbl>)version 1",g);
 
 rptr=(res=gen_node());
 ptr=Gen_lval(g);
 scl=Gen_to_Dmatrix(Gen_elt(g,2));
 while(ptr!=nullptr){
   if (Can_Be_Vector(ptr,Dbl_T)!=2*N+3){
      Dmatrix_free(scl);
      free_Gen_list(Gen_next(res));
      free_Gen_node(res);
       return
        Rerror("Usage: UnScale({<Dbl>},<Dbl>) version 2",g);
     }

  root=Gen_to_Dmatrix(ptr);
  xpnt_unscale(root,scl);
  Gen_set_next(rptr,Dmatrix_to_Gen(root));
  rptr=Gen_next(rptr);
  ptr=Gen_next(ptr);
 }
 /*
 free_Gen_list(g);
 */ 
 g=XPLND(Gen_next(res));
 free_Gen_node(res);
 Dmatrix_free(scl);
 Dmatrix_free(root);
 return g;             
}

/*
** ScaleXPL Command         (tested)
*/
Gen_node G_ScaleXPL(Gen_node g){
 Gen_node ptr, res,rptr;
 Dmatrix scl;
 Dmatrix root = nullptr; /* Initialized to get rid of C++ warning. */
 int i;
   
 if (g==nullptr ||
     Gen_length(g)!=2 ||
     Can_Be_Vector(Gen_elt(g,2),Dbl_T)!=N ||
     Can_Be_List(g)!=TRUE)
     return Rerror("Usage: XScale({<Dbl>},<Dbl>)",g);
    
 rptr=(res=gen_node());
 ptr=Gen_lval(g);
 scl=Gen_to_Dmatrix(Gen_elt(g,2));
 while(ptr!=nullptr){
   if (Can_Be_Vector(ptr,Dbl_T)!=2*N+3){
      Dmatrix_free(scl);
      free_Gen_list(Gen_next(res));
      free_Gen_node(res);
       return
        Rerror("Usage: UnScale({<Dbl>},<Dbl>)",g);
     }
  for(i=1;i<=N;i++){ if (DVref(scl,i)!=0.0) DVref(scl,i)=1.0/DVref(scl,i);}
  root=Gen_to_Dmatrix(ptr);
  xpnt_unscale(root,scl);
  Gen_set_next(rptr,Dmatrix_to_Gen(root));
  rptr=Gen_next(rptr);
  ptr=Gen_next(ptr);
 } 
 /* 
free_Gen_list(g);
*/ 
g=XPLND(Gen_next(res));
 free_Gen_node(res);
 Dmatrix_free(scl);
 Dmatrix_free(root);
 return g;
}



/*
** Affine Command
*/ 
Gen_node G_Affine(Gen_node g){
 Gen_node ptr, res,rptr;
 Dmatrix root;

 
 if (g==nullptr ||
     Gen_length(g)!=1 ||
     Can_Be_List(g)!=TRUE)
     return Rerror("Usage: Affine({<Dbl>})",g);

 rptr=(res=gen_node());
 ptr=Gen_lval(g);
 while(ptr!=nullptr){
   if (Can_Be_Vector(ptr,Dbl_T)!=2*N+3){
      free_Gen_list(Gen_next(res));
      free_Gen_node(res);
       return
        Rerror("Usage: Affine({<Dbl>})",g);
   }
  root=Gen_to_Dmatrix(ptr);
  xpnt_affine(root);
  Gen_set_next(rptr,Dmatrix_to_Gen(root));
  rptr=Gen_next(rptr);
  ptr=Gen_next(ptr);
 }
 /*
 free_Gen_list(g);
 */ 
g=XPLND(Gen_next(res));
 free_Gen_node(res);
 return g;
}

/*
** Normalize Command
*/
Gen_node G_Normalize(Gen_node g){
 Gen_node ptr, res,rptr;
 Dmatrix root;

 if (g==nullptr ||
     Gen_length(g)!=1 ||
     Can_Be_List(g)!=TRUE)
     return Rerror("Usage: Normalize({<Dbl>})",g);

 rptr=(res=gen_node());
 ptr=Gen_lval(g);
 while(ptr!=nullptr){
   if (Can_Be_Vector(ptr,Dbl_T)!=2*N+3){
      free_Gen_list(Gen_next(res));
      free_Gen_node(res);
       return
        Rerror("Usage: Normalize({<Dbl>})",g);
   }
  root=Gen_to_Dmatrix(ptr);
  xpnt_normalize(root);
  Gen_set_next(rptr,Dmatrix_to_Gen(root));
  rptr=Gen_next(rptr);
  ptr=Gen_next(ptr);
 }
 free_Gen_list(g);
 g=Gen_node_to_List(Gen_next(res));
 free_Gen_node(res);
 return g;
}



/*
** Set_T Command                (tested)
*/
Gen_node G_Set_T(Gen_node g){
  Gen_node ptr;
  double t0;
   if ( Gen_length(g)!=2||
        Can_Be_List(Gen_elt(g,1))!=TRUE ||
        Can_Be_Number(Gen_elt(g,2),Dbl_T)==FALSE)
        return Rerror("Usage: SetT({<Dbl>},Dbl)",g);

   ptr=Gen_lval(Gen_elt(g,1));
   t0=Gen_To_Dbl(Gen_elt(g,2));
   free_Gen_list(Gen_next(g));
   while(ptr!=nullptr){
     if (Can_Be_Vector(ptr,Dbl_T)<1){
       return Rerror("Usage: SetT({<Dbl>},Dbl)",g);
     }
     free_Gen_node(*GMref(Gen_Mtx(ptr),1,GMcols(Gen_Mtx(ptr))));
     *GMref(Gen_Mtx(ptr),1,GMcols(Gen_Mtx(ptr)))=DBLND(t0);
     ptr=Gen_next(ptr);
   }
   return g;
}

Gen_node G_Re(Gen_node g){
   fcomplex c;
   if ( Gen_length(g)!=1|| Can_Be_Cpx(Gen_elt(g,1))!=TRUE)
        return Rerror("Usage: Re(Cpx)",g);
   c=Gen_To_Cpx(g);
   free_Gen_list(g);
   return DBLND(Real(c));
}
 

Gen_node G_Im(Gen_node g){ 
   fcomplex c; 
   if ( Gen_length(g)!=1|| Can_Be_Cpx(Gen_elt(g,1))!=TRUE)
        return Rerror("Usage: Re(Cpx)",g); 
   c=Gen_To_Cpx(g); 
   free_Gen_list(g); 
   return DBLND(Imag(c)); 
} 


