//
// FILE: exprtree.h -- Expression tree classes for GEL
//
// $Id$
//

#ifndef EXPRTREE_H
#define EXPRTREE_H


typedef enum  
{
  gelUNDEFINED, 
  gelBOOLEAN, 
  gelNUMBER, 
  gelTEXT,
  gelEFG,
  gelNODE,
  gelINFOSET,
  gelACTION,
  gelEFPLAYER,
  gelEFOUTCOME,
  gelEFSUPPORT,
  gelBEHAV,
  gelNFG,
  gelSTRATEGY,
  gelNFPLAYER,
  gelNFOUTCOME,
  gelNFSUPPORT,
  gelMIXED,
  gelINPUT,
  gelOUTPUT,
  gelANYTYPE
} gelType;

class Efg;
class Node;
class Infoset;
class Action;
class EFPlayer;
class EFOutcome;
class EFSupport;
class BehavSolution;
class Nfg;
class NFPlayer;
class NFOutcome;
class Strategy;
class NFSupport;
class MixedSolution;


#include "gnlist.h"

#include "tristate.h"
#include "gtext.h"
#include "funcmisc.h"

class gelVariableTable;
class gelSignature;


const int MAX_DEPTH = 32767;

class gelExpr   
{
public:
  virtual ~gelExpr();
  virtual gelType Type(void) const = 0;
  virtual int Depth( void ) const = 0;
  virtual void Execute(gelVariableTable *) const = 0;
}; 

template <class T> class gelExpression : public gelExpr   
{
public:
  virtual ~gelExpression();
  gelType Type(void) const;
  virtual int Depth( void ) const { return MAX_DEPTH; }
  void Execute(gelVariableTable *) const;
  virtual gNestedList<T> Evaluate(gelVariableTable *) const = 0;
};
 
template <class T> class gelAssignment : public gelExpression<T>   
{
private:
  gText name;
  gelExpression<T> *rhs;
  
public:
  gelAssignment(const gText &, gelExpression<T> *);
  virtual ~gelAssignment();
  virtual gNestedList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelConstant : public gelExpression<T>  
{
private:
  gNestedList<T> m_Value;
  
public:
  gelConstant(T);
  gelConstant(const gNestedList<T> &);
  virtual ~gelConstant();
  virtual int Depth( void ) const { return m_Value.Depth(); }
  gNestedList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelVariable : public gelExpression<T>  
{
private:
  gText m_Name;
  
public:
  gelVariable(const gText &name);
  virtual ~gelVariable();
  
  const gText &Name(void) const { return m_Name; }
  gNestedList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelConditional : public gelExpression<T>  
{
private:
  gelExpression< gTriState * > *guard;
  gelExpression< T > *truebr, *falsebr;
  
public:
  gelConditional(gelExpression<gTriState *> *guard,
		 gelExpression<T> *iftrue);
  gelConditional(gelExpression<gTriState *> *guard,
		 gelExpression<T> *iftrue, gelExpression<T> *iffalse);
  virtual ~gelConditional();
  
  gNestedList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelWhileLoop : public gelExpression<T>  
{
private:
  gelExpression<gTriState *> *guard;
  gelExpression<T> *body;
  
public:
  gelWhileLoop(gelExpression<gTriState *> *, gelExpression<T> *);
  virtual ~gelWhileLoop();
  
  gNestedList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelForLoop : public gelExpression<T>  
{
private:
  gelExpr *init;
  gelExpression<gTriState *> *guard;
  gelExpr *incr;
  gelExpression<T> *body;
  
public:
  gelForLoop(gelExpr *, gelExpression<gTriState *> *, gelExpr *,
	     gelExpression<T> *);
  virtual ~gelForLoop();
  
  gNestedList<T> Evaluate(gelVariableTable *) const;
};


#include "tristate.h"

class gelQuitExpr : public gelExpression<gTriState *>  
{
public:
  gelQuitExpr(void)  { }
  virtual ~gelQuitExpr()   { }
  
  gNestedList<gTriState *> Evaluate(gelVariableTable *) const;
};



#include "funcmisc.h"






#define DECLARE_NOPARAM( funcclass, T )               \
class funcclass : public gelExpression<T>             \
{                                                     \
public:                                               \
  funcclass(void) {}                                  \
  virtual ~funcclass() {}                             \
  gNestedList<T> Evaluate(gelVariableTable *) const;  \
};


// This one is for listed functions
#define DECLARE_UNARY( funcclass, type1, T )          \
class funcclass : public gelExpression<T>             \
{                                                     \
private:                                              \
  gelExpression<type1>* op1;                          \
                                                      \
  T EvalItem( type1 item1 ) const;                    \
                                                      \
public:                                               \
  funcclass( gelExpression<type1>* x ) : op1(x) {}    \
  virtual ~funcclass() { delete op1; }                \
  gNestedList<T> Evaluate( gelVariableTable *vt ) const;     \
};                                                    \
                                                      \
gNestedList<T> funcclass :: Evaluate(gelVariableTable *vt) const    \
{                                                     \
  gNestedList<type1> arg1 = op1->Evaluate( vt );      \
  gNestedList<T> ret( arg1.Dim() );                   \
  for (int i = 1; i <= arg1.Data().Length(); i++)     \
    ret[i] = EvalItem(arg1.Data()[i]);         \
  return ret;                                         \
}



// This one is for non-listed functions
#define DECLARE_UNARY_LIST( funcclass, type1, T )     \
class funcclass : public gelExpression<T>             \
{                                                     \
private:                                              \
  gelExpression<type1>* op1;                          \
                                                      \
public:                                               \
  funcclass( gelExpression<type1>* x ) : op1(x) {}    \
  virtual ~funcclass() { delete op1; }                \
  gNestedList<T> Evaluate( gelVariableTable *vt ) const;    \
};




// This one is for listed functions
#define DECLARE_BINARY( funcclass, type1, type2, T )  \
class funcclass : public gelExpression<T>             \
{                                                     \
private:                                              \
  gelExpression<type1>* op1;                          \
  gelExpression<type2>* op2;                          \
                                                      \
  T EvalItem( type1 item1, type2 item2 ) const;       \
                                                      \
public:                                               \
  funcclass( gelExpression<type1>* x1,                \
             gelExpression<type2>* x2 )               \
    : op1(x1), op2(x2) {}                             \
  virtual ~funcclass() { delete op1; delete op2; }    \
  gNestedList<T> Evaluate( gelVariableTable *vt ) const;           \
};                                                                 \
                                                                   \
gNestedList<T> funcclass::Evaluate(gelVariableTable *vt) const     \
{                                                                  \
  gNestedList<type1> arg1 = op1->Evaluate( vt );                   \
  gNestedList<type2> arg2 = op2->Evaluate( vt );                   \
								       \
  if( (arg1.Dim()[1] == 0) != (arg2.Dim()[1] == 0) )                   \
  {                                                                    \
    /* only one of the arguments is a scalar */                        \
                                                                       \
    if( arg1.Dim()[1] == 0 ) /* the first argument is a scalar */      \
    {                                                                  \
      gNestedList<T> ret( arg2.Dim() );                                \
      for (int i = 1; i <= arg2.Data().Length(); i++)                  \
	ret[i] = EvalItem(arg1.Data()[1], arg2.Data()[i]);   \
      return ret;                                                      \
    }                                                                  \
    else /* the second argument is a scalar */                         \
    {                                                                  \
      gNestedList<T> ret( arg1.Dim() );                                \
      for (int i = 1; i <= arg1.Data().Length(); i++)                  \
	ret[i] = EvalItem(arg1.Data()[i], arg2.Data()[1]);   \
      return ret;                                                      \
    }                                                                  \
                                                                       \
  }                                                                    \
  else                                                                 \
  {                                                                    \
    gNestedList<T> ret( arg1.Dim() );                                  \
    if( arg1.Data().Length() != arg2.Data().Length() )                 \
      throw gDimMismatchException();                                   \
    if( arg1.Dim() != arg2.Dim() )                                     \
      throw gDimMismatchException();                                   \
    for (int i = 1; i <= arg1.Data().Length(); i++)                    \
      ret[i] = EvalItem(arg1.Data()[i], arg2.Data()[i]);     \
    return ret;                                                        \
  }                                                                    \
}                                                                      \


// This one is for non-listed functions
#define DECLARE_BINARY_LIST( funcclass, type1, type2, T )  \
class funcclass : public gelExpression<T>             \
{                                                     \
private:                                              \
  gelExpression<type1>* op1;                          \
  gelExpression<type2>* op2;                          \
                                                      \
public:                                               \
  funcclass( gelExpression<type1>* x1,                \
             gelExpression<type2>* x2 )               \
    : op1(x1), op2(x2) {}                             \
  virtual ~funcclass() { delete op1; delete op2; }    \
  gNestedList<T> Evaluate( gelVariableTable *vt ) const;  \
};




template <class T> class gelUDF : public gelExpression<T>  
{
private:
  const gelSignature& m_Signature;
  gArray<gelExpr*> m_Params;
  const gelExpression<T>* m_Body;
  
public:
  gelUDF(const gelSignature& sig, 
	 const gArray<gelExpr *>& params, 
	 const gelExpression<T>* exp );
  virtual ~gelUDF();

  virtual int Depth( void ) const { return m_Signature.Depth(); }
  gNestedList<T> Evaluate(gelVariableTable *) const;
};







class gDimMismatchException : public gException   
{
public:
  virtual gText Description(void) const { return "Dimension mismatch"; }
}; 


class gelRuntimeError : public gException   {
private:
  gText message;
  
public:
  gelRuntimeError(const gText &m) : message(m)   { }
  virtual ~gelRuntimeError()   { }
  gText Description(void) const  { return message; }
};

class gelGameMismatchError : public gelRuntimeError   {
public:
  gelGameMismatchError(const gText &fn) :
     gelRuntimeError("Game mismatch error in call to " + fn)   { }
  virtual ~gelGameMismatchError()   { }
};



#endif   // EXPRTREE_H

