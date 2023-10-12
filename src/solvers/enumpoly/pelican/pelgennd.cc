/*
**    copyright (c) 1995  Birk Huber
*/

#include <iostream>
#include "pelgennd.h"

extern node SaveList;
node Dlist_add(node,node);
node Dlist_del(node,node);
node Dlist_data(node);


Pring Def_Ring;
int N;


Gen_node gen_node()
{
 Gen_node a;
 a=(Gen_node)mem_malloc(sizeof(struct Gen_node_tag)); 
 if (a == nullptr) bad_error(" malloc failure in gen_node()");
 a->next=nullptr;
 a->type=0;
 return a;}

Gen_node free_Gen_node(Gen_node a)
{
 if (a==nullptr) return nullptr;
 switch (a->type){
   case Xpl_T:  
   case Npl_T:
   case Lst_T: if(a->Genval.lval!=nullptr) free_Gen_list(a->Genval.lval);
               break;
   case Ast_T: Dlist_del(SaveList,(node)(a->Genval.gval));
               break;
   case Sys_T: 
   case Mtx_T: if(a->Genval.lval!=nullptr) 
                     Gmatrix_free((Gmatrix)a->Genval.gval);
               break;
   case Err_T: break; 
   case Idf_T: if(a->Genval.idval!=nullptr) mem_free(a->Genval.idval); 
               break;
   case Str_T: if(a->Genval.gval!=nullptr) mem_free(a->Genval.gval); 
               break;
   case Ply_T: if(a->Genval.pval!=nullptr) freeP(a->Genval.pval); 
               break;
   default : break;
  }
 mem_free((char *)a); 
 return nullptr;
} 

Gen_node free_Gen_list(Gen_node a)
{ 
  Gen_node b;
  while(a!=nullptr){
    b=a->next;
    a=free_Gen_node(a);
    a=b;}
  return nullptr;
}

Gen_node G_Print(Gen_node);

void print_Gen_node(Gen_node g)
{

/* DEBUG */
  /*
  if (g->type > 10)
    fprintf(stdout, "The type number is %d.\n", g->type);  
  */
 
  switch (g->type) {

  case Int_T: 
#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */," %d ",g->Genval.ival)
#endif
      ;
    break;

  case Dbl_T:  
#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */," %f ",g->Genval.dval)
#endif
      ;
    break;

  case Idf_T:  
#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */," %s ",g->Genval.idval)
#endif
      ;
    break;

  case Str_T: 
#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */," %s ",g->Genval.gval);
    fprintf(stdout /* was Pel_Out */," %s "," case Str_T")
#endif
      ;
    break;

  case Err_T: 
#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */," %s \n", "new gen_node:\n\n")
#endif
      ;
    break;                   
    
  case Xpl_T: 
    free_Gen_node(G_Print(copy_Gen_node(g)));
#ifdef LOG_PRINT
    /*     fprintf(stdout," %s "," case Xpl_T") */
#endif
      ; 
    break;

  case Npl_T:
    ;
    break;

  case Lst_T: 
#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */,"{") ;
    print_Gen_list(g->Genval.lval);
    fprintf(stdout /* was Pel_Out */,"}")
#endif
;
							       ;
    break;
  
  case Ast_T: 
	/* DEBUG */
	/* printf("The type is Ast_T which is %d.\n",Ast_T); */
    node_print(Dlist_data((node)g->Genval.gval));
 #ifdef LOG_PRINT
/*   fprintf(stdout */ /* was Pel_Out */ /*, " %s "," case Ast_T") */
#endif
    ;
    break;

  case Sys_T: 
	/* DEBUG */
	  /* printf("The type is Sys_T which is %d.\n",Sys_T); */
    free_Gen_node(G_Print(copy_Gen_node(g)));
#ifdef LOG_PRINT
/* fprintf(stdout */ /* was Pel_Out */ /*, " %s "," case Sys_T") */
#endif
    ; 
    break;

  case Mtx_T: 
    fprintf(stdout /* was Pel_Out */, "%s","The vector of numbers of polynomials of each support type is:\n");
    Gmatrix_print((Gmatrix)g->Genval.gval);
    break;

  case Prc_T: 

	/* DEBUG */
    printf("The type is Prc_T which is %d.\n",Prc_T);
    print_Proc(g->Genval.proc);
#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */, " %s "," case Prc_T:")
#endif
      ; 
    break;
    
  case Cpx_T:  printC(g->Genval.cval);

	/* DEBUG */
    printf("The type is Cpx_T which is %d.\n",Cpx_T);

#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */, " %s "," case  Cpx_T")
#endif
      ;
    break;

  case Ply_T: 

	/* DEBUG */
	  /*  printf("  The type is Ply_T which is %d.  ",Ply_T);  */

    printP(g->Genval.pval);
    break;

  case Rng_T:  
#ifdef LOG_PRINT
    fprintf(stdout /* was Pel_Out */,"cannot display ring yet\n");
    fprintf(stdout /* was Pel_Out */, " %s "," case Rng_T")
#endif
      ; 
    break;
    
  default: bad_error("unknown type in print_Gen_node");
  }
}

void print_Gen_list(Gen_node g)
 {
   int tog=0;
   while (g!=nullptr){ 
     if (tog==1)
       fprintf(stdout /* was Pel_Out */,",");
     print_Gen_node(g);
     tog=1;
     g=g->next;
   }
}

void silent_print_Gen_list(Gen_node g)
{
  while (g!=nullptr)
    g=g->next;
}

Gen_node Gen_node_to_List(Gen_node g)
 {Gen_node a;
  a=gen_node();
  a->type=Lst_T;
  a->Genval.lval=g;
  return a;
 }

Gen_node Cat(Gen_node g1,Gen_node g2){
 Gen_node a;
 if((a=g1)==nullptr) return g2;
 while(a->next!=nullptr) a=a->next;
 a->next=g2;
 return g1;
}



Gen_node copy_Gen_list(Gen_node a)
 {
Gen_node n,pt;
 if (a==nullptr) return nullptr;
 n=copy_Gen_node(a);
 pt=n;
 a=a->next;
 while(a!=nullptr){ pt->next=copy_Gen_node(a);
              pt=pt->next;
              a=a->next;
            }
return n;
}
     
Gen_node copy_Gen_node(Gen_node a)
 { Gen_node b;
   b=gen_node();
   b->type=a->type;
   switch(a->type){
         case Int_T: b->Genval.ival=a->Genval.ival;
                     break;
         case Dbl_T: b->Genval.dval=a->Genval.dval;
                     break;
         case Prc_T: b->Genval.proc=a->Genval.proc; 
                     break;
        case Xpl_T:
        case Npl_T:
         case Lst_T: b->Genval.lval=copy_Gen_list(a->Genval.lval); 
                     break;
         case Ast_T: b->Genval.gval=
                    (char *)Dlist_add(SaveList,
                                    Dlist_data((node)a->Genval.gval));
                     break;
         case Sys_T: 
         case Mtx_T: 
	   b->Genval.gval=(char *)Gmatrix_copy((Gmatrix)a->Genval.lval);
                     break;
         case Idf_T: b->Genval.idval=Copy_String(a->Genval.idval); 
                     break;
         case Str_T: b->Genval.gval=Copy_String(a->Genval.gval);
                     break;
         case Err_T: break; 
         case Cpx_T:
                     b->Genval.cval=a->Genval.cval;
                     break;
         case Ply_T: 
                    b->Genval.pval=copyP(a->Genval.pval);
                    break;
         case Rng_T:  warning("can not copy ring");
         default: warning("incomplete copy in copy_Gen_node()"); 
         break;
       }

return b;
}


char *Gen_idval(Gen_node g){
  if (g==nullptr||(g->type!=Idf_T&&g->type!=Str_T)) return nullptr;
  else return g->Genval.idval;
}

Gen_node Gen_set_next(Gen_node g,Gen_node h){
  if (g==nullptr) return nullptr;
  else return g->next=h;
}
Gen_node Gen_lval(Gen_node g){
  if (g==nullptr) return nullptr;
  else return g->Genval.lval;
}
Gen_node Gen_set_lval(Gen_node g,Gen_node g1){
  if (g==nullptr) return nullptr;
  else return (g->Genval.lval=g1);
}
Gen_node Gen_next(Gen_node g){
  if (g==nullptr) return nullptr;
  else return g->next;
}
int Gen_type(Gen_node g){
  if (g==nullptr) bad_error("requesting type of null node");
  return g->type;
}
int Gen_set_int(Gen_node g,int i){ 
  if (g==nullptr) bad_error("setting int field of null node");
  return g->Genval.ival=i;
}

int Gen_int(Gen_node g){
  if (g==nullptr) bad_error("getting int field of null node");
  return g->Genval.ival;
}
int Gen_length(Gen_node g){
  int ct=1;
  if (g==nullptr) return 0;
  while((g=Gen_next(g))!=nullptr)ct++;
  return ct;
}

Gen_node Gen_elt(Gen_node g, int idx){
  while(--idx>0) g=Gen_next(g);
  return g;
}

polynomial1 Gen_poly(Gen_node g){
 if (g==nullptr) bad_error("getting polynomial1 from null node in Gentype (Pat)");
 return (polynomial1)g->Genval.gval;
}

node Gen_aset(Gen_node g){
 if (g==nullptr) bad_error("getting aset from null node");
 return Dlist_data((node)(g->Genval.gval));
}
  
void print_Proc(Gen_node (*p)(Gen_node))
{
#ifdef LOG_PRINT
  std::cout << p;
#endif
}
				     

/*used for reader.lex-- 
takes a string in quotes and removes outside quotes*/
char *Copy_String_NQ(char *s)
{ int l,i,j=0;
  char *res;
  l=strlen(s);
  res=(char *)mem_malloc((l-1)*sizeof(char));
  for(i=0;i<=l;i++) if (s[i]!='"') res[j++]=s[i];
  return res;
}

char *Copy_String(const char *s)
{ /* char *strdup(); CANT DECLARE BUILTINS UNDER C++ */
  return mem_strdup(const_cast<char *>(s))
;}


Gen_node IDND(const char *s){
     Gen_node a;


     a=gen_node();


     a->type=Idf_T;


     a->Genval.idval=Copy_String(s);


     a->next=nullptr;


     return a;
}


Gen_node ASTND(node n){ 
     Gen_node a;
  
    a=gen_node();

    a->type=Ast_T;

     a->Genval.gval=(char *)Dlist_add(SaveList,n);


     a->next=nullptr;

    return a;
}

Gen_node INTND(int n){ 
     Gen_node a;
     a=gen_node();
     a->type=Int_T;
     a->Genval.ival=n;
     a->next=nullptr;
     return a;
}

Gen_node DBLND(double d){ 
     Gen_node a;
     a=gen_node();
     a->type=Dbl_T;
     a->Genval.dval=d;
     a->next=nullptr;
     return a;
}

Gen_node CPXND(fcomplex c){ 
     Gen_node a;
     a=gen_node();
     a->type=Cpx_T;
     a->Genval.cval=c;
     a->next=nullptr;
     return a;
}

Gen_node PLYND(polynomial1 p) {
     Gen_node a;
     a=gen_node();
     a->type=Ply_T;
     a->Genval.pval=p;
     a->next=nullptr;
     return a;
}

Gen_node PND(Gen_node p(Gen_node)) {
     Gen_node a;
     a=gen_node();
     a->type=Prc_T;
     a->Genval.proc=p;
     a->next=nullptr;
     return a;
}

Gen_node Rerror(const char *s,Gen_node g)
{ 
  bad_error("We had an Rerror");

  Gen_node ans;
  if (g!=nullptr) free_Gen_list(g);
#ifdef LOG_PRINT
  fprintf(stderr /* was Pel_Err */,"%s\n",s)
#endif
;
  ans=gen_node();
  ans->type=Err_T;
  return ans;
}


/*
** Print Command
*/
Gen_node G_Print(Gen_node g){
  psys sys;
  node xl;
  if (Gen_length(g)!=1) 
    return Rerror("Print: too many arguments",g);
;
  switch (Gen_type(g)){
   case Ast_T:  aset_print(Gen_aset(g));
                break;
  case Sys_T:  sys=Gen_to_psys(g); 
                
     psys_print(sys);
     psys_free(sys);
                break;

   case Xpl_T:  xl=Gen_to_Dvector_list(Gen_lval(g));
#ifdef LOG_PRINT           
     xpl_fprint(stdout /* was Pel_Out */,xl)
#endif
;
                break;
      default: print_Gen_node(g);
  }
#ifdef LOG_PRINT
 fprintf(stdout /* was Pel_Out */,"\n")
#endif
;
 free_Gen_list(g);
 return IDND(""); 
 }
