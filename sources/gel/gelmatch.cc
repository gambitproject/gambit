//
// FILE: gelmatch.cc -- GELCompiler members for matching control structs
//

#include "gstream.h"
#include "gtext.h"
#include "gblock.h"
#include "gstack.h"

#include "gnlist.h"

#include "gnumber.h"
#include "tristate.h"
#include "exprtree.h"
#include "match.h"

#include "gel.h"

gelExpr *GELCompiler::MatchAssignment(gelExpr *lhs, gelExpr *rhs)
{
  if (!lhs || !rhs)   {
    if (lhs)   delete lhs;
    if (rhs)   delete rhs;
    return 0;
  }

  gelVariableTable* vartable = m_GlobalVarTable;
  if( m_FuncVarTable )
    vartable = m_FuncVarTable;

  switch (rhs->Type())   {
    case gelBOOLEAN:
      vartable->Define(((gelVariable<gTriState> *) lhs)->Name(), gelBOOLEAN);  
      return new gelAssignment<gTriState>(((gelVariable<gTriState> *) lhs)->Name(),
		   		        ((gelExpression<gTriState> *) rhs));
    case gelNUMBER:
      vartable->Define(((gelVariable<gNumber> *) lhs)->Name(), gelNUMBER);  
      return new gelAssignment<gNumber>(((gelVariable<gNumber> *) lhs)->Name(),
		   		        ((gelExpression<gNumber> *) rhs));
    case gelTEXT:
      vartable->Define(((gelVariable<gText> *) lhs)->Name(), gelTEXT);  
      return new gelAssignment<gText>(((gelVariable<gText> *) lhs)->Name(),
		   		      ((gelExpression<gText> *) rhs));
    case gelEFG:
      vartable->Define(((gelVariable<Efg *> *) lhs)->Name(), gelEFG);
      return new gelAssignment<Efg *>(((gelVariable<Efg *> *) lhs)->Name(),
				      ((gelExpression<Efg *> *) rhs));
    case gelNODE:
      vartable->Define(((gelVariable<Node *> *) lhs)->Name(), gelNODE);
      return new gelAssignment<Node *>(((gelVariable<Node *> *) lhs)->Name(),
				      ((gelExpression<Node *> *) rhs));
    case gelACTION:
      vartable->Define(((gelVariable<Action *> *) lhs)->Name(), gelACTION);
      return new gelAssignment<Action *>(((gelVariable<Action *> *) lhs)->Name(),
				      ((gelExpression<Action *> *) rhs));
    case gelINFOSET:
      vartable->Define(((gelVariable<Infoset *> *) lhs)->Name(), gelINFOSET);
      return new gelAssignment<Infoset *>(((gelVariable<Infoset *> *) lhs)->Name(),
				      ((gelExpression<Infoset *> *) rhs));
    case gelEFPLAYER:
      vartable->Define(((gelVariable<EFPlayer *> *) lhs)->Name(), gelEFPLAYER);
      return new gelAssignment<EFPlayer *>(((gelVariable<EFPlayer *> *) lhs)->Name(),
				      ((gelExpression<EFPlayer *> *) rhs));
    case gelEFOUTCOME:
      vartable->Define(((gelVariable<EFOutcome *> *) lhs)->Name(), gelEFOUTCOME);
      return new gelAssignment<EFOutcome *>(((gelVariable<EFOutcome *> *) lhs)->Name(),
				      ((gelExpression<EFOutcome *> *) rhs));
    case gelEFSUPPORT:
      vartable->Define(((gelVariable<EFSupport *> *) lhs)->Name(), gelEFSUPPORT);
      return new gelAssignment<EFSupport *>(((gelVariable<EFSupport *> *) lhs)->Name(),
				      ((gelExpression<EFSupport *> *) rhs));
    case gelBEHAV:
      vartable->Define(((gelVariable<BehavSolution *> *) lhs)->Name(), gelBEHAV);
      return new gelAssignment<BehavSolution *>(((gelVariable<BehavSolution *> *) lhs)->Name(),
				      ((gelExpression<BehavSolution *> *) rhs));
    case gelNFG:
      vartable->Define(((gelVariable<Nfg *> *) lhs)->Name(), gelNFG);
      return new gelAssignment<Nfg *>(((gelVariable<Nfg *> *) lhs)->Name(),
				      ((gelExpression<Nfg *> *) rhs));
    case gelSTRATEGY:
      vartable->Define(((gelVariable<Strategy *> *) lhs)->Name(), gelSTRATEGY);
      return new gelAssignment<Strategy *>(((gelVariable<Strategy *> *) lhs)->Name(),
				      ((gelExpression<Strategy *> *) rhs));
    case gelNFPLAYER:
      vartable->Define(((gelVariable<NFPlayer *> *) lhs)->Name(), gelNFPLAYER);
      return new gelAssignment<NFPlayer *>(((gelVariable<NFPlayer *> *) lhs)->Name(),
				      ((gelExpression<NFPlayer *> *) rhs));
    case gelNFOUTCOME:
      vartable->Define(((gelVariable<NFOutcome *> *) lhs)->Name(), gelNFOUTCOME);
      return new gelAssignment<NFOutcome *>(((gelVariable<NFOutcome *> *) lhs)->Name(),
				      ((gelExpression<NFOutcome *> *) rhs));
    case gelNFSUPPORT:
      vartable->Define(((gelVariable<NFSupport *> *) lhs)->Name(), gelNFSUPPORT);
      return new gelAssignment<NFSupport *>(((gelVariable<NFSupport *> *) lhs)->Name(),
				      ((gelExpression<NFSupport *> *) rhs));
    case gelMIXED:
      vartable->Define(((gelVariable<MixedSolution *> *) lhs)->Name(), gelMIXED);
      return new gelAssignment<MixedSolution *>(((gelVariable<MixedSolution *> *) lhs)->Name(),
				      ((gelExpression<MixedSolution *> *) rhs));
    default:
      delete lhs;
      delete rhs;
      return 0;
  }
}

gelExpr *GELCompiler::MatchWhile(gelExpr *guard, gelExpr *body)
{
  if (!guard || !body)   {
    if (guard)   delete guard;
    if (body)    delete body;
  }

  if (guard->Type() != gelBOOLEAN)  {
    delete guard;
    delete body;
    return 0;
  }

  switch (body->Type())   {
    case gelBOOLEAN:
      return new gelWhileLoop<gTriState>(((gelExpression<gTriState> *) guard),
	  	   		         ((gelExpression<gTriState> *) body));
    case gelNUMBER:
      return new gelWhileLoop<gNumber>(((gelExpression<gTriState> *) guard),
	  	   		       ((gelExpression<gNumber> *) body));
    case gelTEXT:
      return new gelWhileLoop<gText>(((gelExpression<gTriState> *) guard),
	  	   		     ((gelExpression<gText> *) body));
    case gelEFG:
      return new gelWhileLoop<Efg *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<Efg *> *) body));
    case gelNODE:
      return new gelWhileLoop<Node *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<Node *> *) body));
    case gelACTION:
      return new gelWhileLoop<Action *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<Action *> *) body));
    case gelINFOSET:
      return new gelWhileLoop<Infoset *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<Infoset *> *) body));
    case gelEFPLAYER:
      return new gelWhileLoop<EFPlayer *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<EFPlayer *> *) body));
    case gelEFOUTCOME:
      return new gelWhileLoop<EFOutcome *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<EFOutcome *> *) body));
    case gelEFSUPPORT:
      return new gelWhileLoop<EFSupport *>(((gelExpression<gTriState> *) guard),
					   ((gelExpression<EFSupport *> *) body));
    case gelBEHAV:
      return new gelWhileLoop<BehavSolution *>(((gelExpression<gTriState> *) guard),
					       ((gelExpression<BehavSolution *> *) body));
    case gelNFG:
      return new gelWhileLoop<Nfg *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<Nfg *> *) body));
    case gelSTRATEGY:
      return new gelWhileLoop<Strategy *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<Strategy *> *) body));
    case gelNFPLAYER:
      return new gelWhileLoop<NFPlayer *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<NFPlayer *> *) body));
    case gelNFOUTCOME:
      return new gelWhileLoop<NFOutcome *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<NFOutcome *> *) body));
    case gelNFSUPPORT:
      return new gelWhileLoop<NFSupport *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<NFSupport *> *) body));
    case gelMIXED:
      return new gelWhileLoop<MixedSolution *>(((gelExpression<gTriState> *) guard),
				     ((gelExpression<MixedSolution *> *) body));

    default:
      delete guard;
      delete body;
      return 0;
  }
}

gelExpr *GELCompiler::MatchFor(gelExpr *init, gelExpr *guard,
			       gelExpr *incr, gelExpr *body)
{
  if (!init || !guard || !incr || !body)   {
    if (init)  delete init;
    if (guard) delete guard;
    if (incr)  delete incr;
    if (body)  delete body;
    return 0;
  }

  if (guard->Type() != gelBOOLEAN)   {
    delete init;
    delete guard;
    delete incr;
    delete body;
    return 0;
  }

  switch (body->Type())   {
    case gelBOOLEAN:
      return new gelForLoop<gTriState>(init,
				       (gelExpression<gTriState> *) guard,
                                       incr,
                                       (gelExpression<gTriState> *) body);
    case gelNUMBER:
      return new gelForLoop<gNumber>(init,
				     (gelExpression<gTriState> *) guard,
                                     incr,
                                     (gelExpression<gNumber> *) body);
    case gelTEXT:
      return new gelForLoop<gText>(init,
				   (gelExpression<gTriState> *) guard,
                                   incr,
                                   (gelExpression<gText> *) body);
    case gelEFG:
      return new gelForLoop<Efg *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<Efg *> *) body);
    case gelNODE:
      return new gelForLoop<Node *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<Node *> *) body);
    case gelACTION:
      return new gelForLoop<Action *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<Action *> *) body);
    case gelINFOSET:
      return new gelForLoop<Infoset *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<Infoset *> *) body);
    case gelEFPLAYER:
      return new gelForLoop<EFPlayer *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<EFPlayer *> *) body);
    case gelEFSUPPORT:
      return new gelForLoop<EFSupport *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<EFSupport *> *) body);
    case gelBEHAV:
      return new gelForLoop<BehavSolution *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<BehavSolution *> *) body);
    case gelNFG:
      return new gelForLoop<Nfg *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<Nfg *> *) body);
    case gelSTRATEGY:
      return new gelForLoop<Strategy *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<Strategy *> *) body);
    case gelNFPLAYER:
      return new gelForLoop<NFPlayer *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<NFPlayer *> *) body);
    case gelNFOUTCOME:
      return new gelForLoop<NFOutcome *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<NFOutcome *> *) body);
    case gelNFSUPPORT:
      return new gelForLoop<NFSupport *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<NFSupport *> *) body);
    case gelMIXED:
      return new gelForLoop<MixedSolution *>(init,
				   (gelExpression<gTriState> *) guard,
				   incr,
				   (gelExpression<MixedSolution *> *) body);
    default:
      delete init;
      delete guard;
      delete incr;
      delete body;
      return 0;
  }
}

gelExpr *GELCompiler::MatchConditional(gelExpr *guard,
			               gelExpr *iftrue, gelExpr *iffalse)
{
  if (!guard || !iftrue || !iffalse)   {
    if (guard)   delete guard;
    if (iftrue)  delete iftrue;
    if (iffalse) delete iffalse;
    return 0;
  }

  if (guard->Type() != gelBOOLEAN ||
      iftrue->Type() != iffalse->Type())   {
    delete guard;
    delete iftrue;
    delete iffalse;
  }

  switch (iftrue->Type())   {
    case gelBOOLEAN:
      return new gelConditional<gTriState>((gelExpression<gTriState> *) guard, 
	    	   		           (gelExpression<gTriState> *) iftrue,
					   (gelExpression<gTriState> *) iffalse);
    case gelNUMBER:
      return new gelConditional<gNumber>((gelExpression<gTriState> *) guard, 
	    	   		         (gelExpression<gNumber> *) iftrue,
					 (gelExpression<gNumber> *) iffalse);
    case gelTEXT:
      return new gelConditional<gText>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<gText> *) iftrue,
				       (gelExpression<gText> *) iffalse);
    case gelEFG:
      return new gelConditional<Efg *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<Efg *> *) iftrue,
				       (gelExpression<Efg *> *) iffalse);
    case gelNODE:
      return new gelConditional<Node *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<Node *> *) iftrue,
				       (gelExpression<Node *> *) iffalse);
    case gelACTION:
      return new gelConditional<Action *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<Action *> *) iftrue,
				       (gelExpression<Action *> *) iffalse);
    case gelINFOSET:
      return new gelConditional<Infoset *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<Infoset *> *) iftrue,
				       (gelExpression<Infoset *> *) iffalse);
    case gelEFPLAYER:
      return new gelConditional<EFPlayer *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<EFPlayer *> *) iftrue,
				       (gelExpression<EFPlayer *> *) iffalse);
    case gelEFOUTCOME:
      return new gelConditional<EFOutcome *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<EFOutcome *> *) iftrue,
				       (gelExpression<EFOutcome *> *) iffalse);
    case gelEFSUPPORT:
      return new gelConditional<EFSupport *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<EFSupport *> *) iftrue,
				       (gelExpression<EFSupport *> *) iffalse);
    case gelBEHAV:
      return new gelConditional<BehavSolution *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<BehavSolution *> *) iftrue,
				       (gelExpression<BehavSolution *> *) iffalse);
    case gelNFG:
      return new gelConditional<Nfg *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<Nfg *> *) iftrue,
				       (gelExpression<Nfg *> *) iffalse);
    case gelSTRATEGY:
      return new gelConditional<Strategy *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<Strategy *> *) iftrue,
				       (gelExpression<Strategy *> *) iffalse);
    case gelNFPLAYER:
      return new gelConditional<NFPlayer *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<NFPlayer *> *) iftrue,
				       (gelExpression<NFPlayer *> *) iffalse);
    case gelNFOUTCOME:
      return new gelConditional<NFOutcome *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<NFOutcome *> *) iftrue,
				       (gelExpression<NFOutcome *> *) iffalse);
    case gelNFSUPPORT:
      return new gelConditional<NFSupport *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<NFSupport *> *) iftrue,
				       (gelExpression<NFSupport *> *) iffalse);
    case gelMIXED:
      return new gelConditional<MixedSolution *>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<MixedSolution *> *) iftrue,
				       (gelExpression<MixedSolution *> *) iffalse);
      
    default:
      delete guard;
      delete iftrue;
      delete iffalse;
      return 0;
  }
}

gelExpr *GELCompiler::MatchConditional(gelExpr *guard, gelExpr *iftrue)
{
  if (!guard || !iftrue)   {
    if (guard)  delete guard;
    if (iftrue) delete iftrue;
    return 0;
  }

  if (guard->Type() != gelBOOLEAN)  {
    delete guard;
    delete iftrue;
    return 0;
  }

  switch (iftrue->Type())   {
    case gelBOOLEAN:
      return new gelConditional<gTriState>((gelExpression<gTriState> *) guard, 
	    	   		           (gelExpression<gTriState> *) iftrue,
					   new gelConstant<gTriState>(triFALSE));
    case gelNUMBER:
      return new gelConditional<gNumber>((gelExpression<gTriState> *) guard, 
	    	   		         (gelExpression<gNumber> *) iftrue,
					 new gelConstant<gNumber>(0));
    case gelTEXT:
      return new gelConditional<gText>((gelExpression<gTriState> *) guard, 
	    	   		       (gelExpression<gText> *) iftrue,
				       new gelConstant<gText>(""));

    default:
      delete guard;
      delete iftrue;
      return 0;
  }
}



gelExpr* GELCompiler::DefineFunction( gelSignature* sig )
{
  environment.Register( sig );
  return new gelConstant< gNumber >( 0 );
}



gelExpr *GELCompiler::LookupVar(const gText &name)
{ 

  gelVariableTable* vartable = m_GlobalVarTable;
  if( m_FuncVarTable )
    vartable = m_FuncVarTable;


  if (!vartable->IsDefined(name))  return 0;

  switch (vartable->Type(name))   {
    case gelNUMBER:
      return new gelVariable<gNumber>(name);
    case gelBOOLEAN:
      return new gelVariable<gTriState>(name);
    case gelTEXT:
      return new gelVariable<gText>(name);
    case gelEFG:
      return new gelVariable<Efg *>(name); 
    case gelNODE:
      return new gelVariable<Node *>(name); 
    case gelACTION:
      return new gelVariable<Action *>(name); 
    case gelINFOSET:
      return new gelVariable<Infoset *>(name); 
    case gelEFPLAYER:
      return new gelVariable<EFPlayer *>(name); 
    case gelEFOUTCOME:
      return new gelVariable<EFOutcome *>(name); 
    case gelEFSUPPORT:
      return new gelVariable<EFSupport *>(name);
    case gelBEHAV:
      return new gelVariable<BehavSolution *>(name); 
    case gelNFG:
      return new gelVariable<Nfg *>(name); 
    case gelSTRATEGY:
      return new gelVariable<Strategy *>(name); 
    case gelNFPLAYER:
      return new gelVariable<NFPlayer *>(name); 
    case gelNFOUTCOME:
      return new gelVariable<NFOutcome *>(name); 
    case gelNFSUPPORT:
      return new gelVariable<NFSupport *>(name);
    case gelMIXED:
      return new gelVariable<MixedSolution *>(name);
    default:
      return 0;
  }
}   
    
