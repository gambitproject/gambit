/* *****************************************************************

   This file, and all other pel*.h and pel*.cc files in the Gambit
source code, are derived from the source code for Pelican, an
implementation of the Huber-Sturmfels algorithm for solving sparse
systems of polynomials written by Birk Huber.  That code in turn
incorporated the source code for HOMPACK and Qhull, which are included
here after being modified in the construction of both Pelican and
Gambit.  We are grateful to the authors of Pelican, Qhull, and
HOMPACK, for code that has been enormously useful in Gambit.
    More information about Qhull can be obtained from:

http://www.geom.edu/software/qhull/

    For more information concerning HOMPACK see:

http://netlib2.cs.utk.edu/hompack/

***************************************************************** */

/*
  This is a header file derived from the file below due to Huber.  
It was created by transporting all normal header information from 
that file to here.
 */

/*
**  Proc_Gen.c 
**     Commands defining Pelican functions 
** 
**    copyright (c) 1995  Birk Huber
*/

#include "pelclhpk.h"
#include "pelclqhl.h"
#include "pelpscon.h"
#include "peleval.h"
#include "pelgennd.h"
#include "pelclyal.h"

/* node MSD(aset Ast, Ivector T); IN pelutils.h */

Gen_node G_Re(Gen_node g);
Gen_node G_Im(Gen_node g);
Gen_node G_Help(Gen_node g);
/* Gen_node G_Print(Gen_node g); IN gennode.h */
Gen_node G_Save(Gen_node g);
Gen_node G_Aset(Gen_node g);
Gen_node G_Ctrig(Gen_node g);
Gen_node G_Qtrig(Gen_node g);
Gen_node G_MSD(Gen_node g);
Gen_node G_Extremal(Gen_node g);
Gen_node G_RandLift(Gen_node g);
Gen_node G_UnLift(Gen_node g);
Gen_node G_Set_T(Gen_node g);
Gen_node G_Face(Gen_node g);
Gen_node G_System(Gen_node g);
Gen_node G_Gen_Poly(Gen_node g);
Gen_node G_AType(Gen_node g);
Gen_node G_Cont(Gen_node g, int tweak);
Gen_node G_BinSolve(Gen_node g);
Gen_node G_NormSub(Gen_node g);
Gen_node G_Lead(Gen_node g);
Gen_node G_Solve(Gen_node g, int tweak);     
Gen_node G_GenSolve(Gen_node g, int tweak);     
Gen_node G_Verify(Gen_node g);
Gen_node G_Eval(Gen_node g);
Gen_node G_ParamSet(Gen_node g);
Gen_node G_ParamShow(Gen_node g);
Gen_node G_Scale(Gen_node g);
Gen_node G_UnScale(Gen_node g);
Gen_node G_ScaleXPL(Gen_node g);
Gen_node G_Affine(Gen_node g);
Gen_node G_Normalize(Gen_node g);

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
Sym_ent Install_Command(Gen_node (*G)(),char *s);
*/

/*----------------------------------------------------------------- 
int Install_Gen_Commands()    
        Uses Install_Command to install all Commands
        On the symbol table. Always returns 1;
  
Error Conditions: None;
-----------------------------------------------------------------*/
/*
int Install_Gen_Commands();
*/

/********************************************************************
 Begin Command Definitions
********************************************************************/

/*
** Help Command:
*/
Gen_node G_Help(Gen_node g);

/* 
** Aset Command          (tested on poly's)
*/
/* aset psys_to_aset(psys); IN psys.h */
Gen_node G_Aset(Gen_node g);

/* 
** Cayley Triangulation Command      (tested: looses 2 )
**                                (problem probably in cly_triangulate)
*/
Gen_node G_Ctrig(Gen_node g);

/*
** Cayley Triangulation Command      (tested: looses 2 )
**                                (problem probably in cly_triangulate
)
*/
Gen_node G_GenSolve(Gen_node g, int tweak);

/*
** MSD Command           (not working, not tested)
*/
Gen_node G_MSD(Gen_node g);

/*
** Qtrig Command               (tested)
*/
Gen_node G_Qtrig(Gen_node g); 

/* 
** Extremal Command                 (tested)
*/
Gen_node G_Extremal(Gen_node g); 

/*
** UnLift Command         (tested)
*/
Gen_node G_UnLift(Gen_node g);

/*
** Randlift Command          (tested)
*/
Gen_node G_RandLift(Gen_node g);

/*
** Lift Command (??? - AMM)
*/

/*
** Face Command
*/
Gen_node G_Face(Gen_node g);

/*
** Print Command
*/
/* Gen_node G_Print(Gen_node g); IN gennode.h */

Gen_node G_Save(Gen_node g);


/* 
** System Command                     (tested)
*/
Gen_node G_System(Gen_node g);

/* 
** GenPoly Command         (tested)
*/
psys aset_to_psys(aset,Ivector,int);
Gen_node G_Gen_Poly(Gen_node g);

/*
** Atype command                        (tested)
*/
Gen_node G_AType(Gen_node g);

/* 
** Continuation Command    (tested)
*/
Gen_node G_Cont(Gen_node g, int tweak);

/*
** Solve Command             (tested)
*/
Gen_node G_Solve(Gen_node g, int tweak);

/*
** ParamSet Command                      (tested)
*/

#define SET_D_PARAM(Svar)\
   else if (strcmp(ptr->Genval.idval,#Svar)==0){\
    if (ptr->next==0||ptr->next->type!=Dbl_T)\
      return Rerror(#Svar "requires a floating point argument",g);\
    Svar=ptr->next->Genval.dval;\
   ptr=ptr->next->next;\
   }

#define SET_B_PARAM(Svar)\
   else if (strcmp(ptr->Genval.idval,#Svar)==0){\
    if (ptr->next==0 || ptr->next->type!=Int_T || \
       (ptr->next->Genval.ival!=TRUE && ptr->next->Genval.ival!=FALSE))\
      return Rerror(#Svar "requires a TRUE/FALSE argument",g);\
    Svar=ptr->next->Genval.ival;\
    ptr=ptr->next->next;\
   }

#define SET_I_PARAM(Svar)\
   else if (strcmp(ptr->Genval.idval,#Svar)==0){\
    if (ptr->next==0 || ptr->next->type!=Int_T)\
      return Rerror(#Svar "requires a floating point argument",g);\
    Svar=ptr->next->Genval.ival;\
    ptr=ptr->next->next;\
   }   
 
#define SET_S_PARAM(Svar)\
   else if (strcmp(ptr->Genval.idval,#Svar)==0){\
    if (ptr->next==0 ||\
       ( ptr->next->type!=Str_T &&  ptr->next->Genval.ival!=Idf_T ))\
      return Rerror(#Svar "requires a String argument",g);\
    strcpy(Svar,ptr->next->Genval.idval);\
    ptr=ptr->next->next;\
   }


  

Gen_node G_ParamSet(Gen_node g);

/*
#undef SET_D_PARAM
#undef SET_I_PARAM
#undef SET_B_PARAM
*/

/*
** ParamShow Command
*/
#define SHOW_PARAM(Svar,T)\
    else if (strcmp(ptr->Genval.idval,#Svar)==0)\
         fprintf(Pel_Out,#Svar T,Svar);
#define SHOW_I_PARAM(Svar) SHOW_PARAM(Svar,"=%d")
#define SHOW_D_PARAM(Svar) SHOW_PARAM(Svar,"=%g")
#define SHOW_S_PARAM(Svar) SHOW_PARAM(Svar,"=%s")
#define SHOW_B_PARAM(Svar)\
    else if (strcmp(ptr->Genval.idval,#Svar)==0){\
        if (Svar==TRUE) fprintf(Pel_Out,#Svar "=TRUE");\
        else fprintf(Pel_Out,#Svar "=FALSE");\
    }
#define DIS_PARAM(Svar,T) fprintf(Pel_Out,#Svar T,Svar);


Gen_node G_ParamShow(Gen_node g);
#undef SHOW_PARAM
#undef SHOW_I_PARAM
#undef SHOW_D_PARAM
#undef SHOW_B_PARAM
#undef DIS_PARAM


/*
** NormSub Command (tested)
*/
Gen_node G_NormSub(Gen_node g);

/*
** Lead Command (tested)
*/
Gen_node G_Lead(Gen_node g);

/*
** Verify Command (tested)
*/
Gen_node G_Verify(Gen_node g);

/*
** Eval Command
*/
Gen_node G_Eval(Gen_node g);

/*
** BinSolve Command:  (tested)
*/
Gen_node G_BinSolve(Gen_node g);

/*
** Scale Command       (tested)
*/
Gen_node G_Scale(Gen_node g);

/* 
** UnScale Command         (tested)
*/
Gen_node G_UnScale(Gen_node g);

/*
** ScaleXPL Command         (tested)
*/
Gen_node G_ScaleXPL(Gen_node g);

/*
** Affine Command
*/ 
Gen_node G_Affine(Gen_node g);

/*
** Normalize Command
*/
Gen_node G_Normalize(Gen_node g);


/*
** Set_T Command                (tested)
*/
Gen_node G_Set_T(Gen_node g);

Gen_node G_Re(Gen_node g);
 

Gen_node G_Im(Gen_node g);


