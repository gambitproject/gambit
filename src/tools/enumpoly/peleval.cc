/*
**    copyright (c) 1995  Birk Huber
*/

#include "peleval.h"

int EvLev=0;

Gen_node Eval(Gen_node g)
{ Gen_node (*proc)(Gen_node),arg,ptr,ans;

  if (g==nullptr) bad_error("Eval passed empty list"); 
  if (g->type == Lst_T) {
    ptr=g->Genval.lval;
    if (ptr!=nullptr) {
        arg=Eval(ptr);
        if (arg->type==Err_T){free_Gen_list(g);
			       return arg;}
        g->Genval.lval=arg;
        while(arg->next!=nullptr){
          ans=Eval(arg->next);
          if (arg->type==Err_T){ free_Gen_list(g);
                                 return ans;}
          arg->next=ans;
          arg=arg->next;
        }
        if (g->Genval.lval->type == Prc_T){
       	  proc=g->Genval.lval->Genval.proc;
          arg=g->Genval.lval->next;
          g->Genval.lval->next=nullptr;
          ans=proc(arg); 
	  ans->next=g->next;
	  g->next=nullptr;
         free_Gen_list(g);  
	  return ans;
        }
   }
 }
return g;
}           
 
     
    


