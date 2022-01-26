/*
**    copyright (c) 1995  Birk Huber
*/

#include "pelsymbl.h"

#define HASHSIZE 100
static Sym_ent hashtab[HASHSIZE];

/*-----------------------------------------------------------------
empty_symbol_table()

free all data structures on the symbol table;
------------------------------------------------------------------*/
void init_symbol_table()
{ int i;
 for(i=0;i<HASHSIZE;i++) hashtab[i]=nullptr;
}


void empty_symbol_table()
{
int i;
Sym_ent np,np1;
for(i=0;i<HASHSIZE;i++) {
   np=hashtab[i];
   while(np!=nullptr) { np1=np;
                  np=np->next;
                  free_Gen_list(np1->def);
                  mem_free(np1->name); 
                  mem_free(np1); 
                 }
    }
}

/*-------------------------------------------------------------------
hashing functiion
---------------------------------------------------------------------*/
int hash(char *s)
{
 int hashval=0;
 while( *s !='\0') hashval+=*s++;
 return hashval % HASHSIZE;
}

/* -----------------------------------------------------------------
  lookup takes a string and returns the value on
   the hashtable corresponding to the string, or returns NULL if  
    there is no such entry
----------------------------------------------------------------- */

Sym_ent Slookup(char *s)
 { Sym_ent np;
   for(np=hashtab[hash(s)]; np!=nullptr;np=np->next)
       if (strcmp(s,np->name)== 0) return(np);
  return nullptr;
  }

/* --------------------------------------------------------------------
Sym_ent install(char *s, Gen_node t)
 if s is already on the symbol table and is unlocked its value
     is freed and replaced by t;
 if s is already on the symbol table and is locked a warning is 
     printed to stderr, and NULL is returned.
 otherwise a new_node is created and the values for s and t are 
     placed on the symbol table

Error Conditions:
 if either s or t is NULL then nothing is done and NULL is returned
 A malloc failure in the attempt to create a newnode is treated
  as an unrecoverable error.
--------------------------------------------------------------------*/ 

Sym_ent install(char *s, Gen_node t)
{
  Sym_ent np; // Sym_ent lookup();
 int hashval;

 if (s==nullptr||t==nullptr) return nullptr;
 if((np=Slookup(s))==nullptr) {
    np=(Sym_ent)mem_malloc(sizeof(struct Sym_ent_tag)); 
    if (np==nullptr) bad_error("malloc bad malloc in install");
    np->name=Copy_String(s); 
    hashval=hash(np->name);
    np->next=hashtab[hashval];
    hashtab[hashval]=np;
    np->lock=0;
    np->def=t;
 }
 else
    if (np->lock==0){
         free_Gen_list(np->def);
         np->def=t;
         np->lock=0;
    }
    else {
      fprintf(stderr /* was Pel_Err */,"warning trying to reinstall a reserved symbol (%s)\n",s);
     return nullptr;
    }
return np;
}

/* ---------------------------------------------------------------

Sym_ent lock(Sym_ent s) 
   Sets lock switch on Symbol table entry so that it is marked as 
        belonging to a reserved word (If s is NULL does nothing).
   return s;

Int locked(Sym_ent s) 
    Returns value of lock switch on s if s is nonNULL. If s is NULL
    returns 0 indicate symbol is unlocked (this case could legitimatly
    happen if one does try to lock the result of a lookup on a string 
    wich has no value on the symbol table. Such a symbol is undefined
    and clearly should not be considered reserved).
-----------------------------------------------------------------*/
  
Sym_ent lock(Sym_ent s)
{ if (s!=nullptr) s->lock=1;
  return s;}

int locked(Sym_ent s)
{ if (s==nullptr) return 0;
  return s->lock;}
