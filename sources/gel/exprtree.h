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
  gelTEXT 
} gelType;


#include <assert.h>

#include "tristate.h"
#include "gtext.h"
#include "funcmisc.h"





class gelVariableTable;
class gelSignature;

class gelExpr   
{
public:
  virtual ~gelExpr();
  virtual gelType Type(void) const = 0;
  virtual void Execute(gelVariableTable *) const = 0;
}; 

template <class T> class gelExpression : public gelExpr   
{
public:
  virtual ~gelExpression();
  gelType Type(void) const;
  void Execute(gelVariableTable *) const;
  virtual gList<T> Evaluate(gelVariableTable *) const = 0;
};
 
template <class T> class gelAssignment : public gelExpression<T>   
{
private:
  gText name;
  gelExpression<T> *rhs;
  
public:
  gelAssignment(const gText &, gelExpression<T> *);
  virtual ~gelAssignment();
  virtual gList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelConstant : public gelExpression<T>  
{
private:
  gList<T> m_Value;
  
public:
  gelConstant(const T &);
  gelConstant(const gList<T> &);
  virtual ~gelConstant();
  gList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelVariable : public gelExpression<T>  
{
private:
  gText m_Name;
  
public:
  gelVariable(const gText &name);
  virtual ~gelVariable();
  
  const gText &Name(void) const { return m_Name; }
  gList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelConditional : public gelExpression<T>  
{
private:
  gelExpression< gTriState > *guard;
  gelExpression< T > *truebr, *falsebr;
  
public:
  gelConditional(gelExpression<gTriState> *guard,
		 gelExpression<T> *iftrue);
  gelConditional(gelExpression<gTriState> *guard,
		 gelExpression<T> *iftrue, gelExpression<T> *iffalse);
  virtual ~gelConditional();
  
  gList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelWhileLoop : public gelExpression<T>  
{
private:
  gelExpression<gTriState> *guard;
  gelExpression<T> *body;
  
public:
  gelWhileLoop(gelExpression<gTriState> *, gelExpression<T> *);
  virtual ~gelWhileLoop();
  
  gList<T> Evaluate(gelVariableTable *) const;
};

template <class T> class gelForLoop : public gelExpression<T>  
{
private:
  gelExpr *init;
  gelExpression<gTriState> *guard;
  gelExpr *incr;
  gelExpression<T> *body;
  
public:
  gelForLoop(gelExpr *, gelExpression<gTriState> *, gelExpr *,
	     gelExpression<T> *);
  virtual ~gelForLoop();
  
  gList<T> Evaluate(gelVariableTable *) const;
};


#include "tristate.h"

class gelQuitExpr : public gelExpression<gTriState>  
{
public:
  gelQuitExpr(void)  { }
  virtual ~gelQuitExpr()   { }
  
  gList<gTriState> Evaluate(gelVariableTable *) const;
};



#include "funcmisc.h"






#define DECLARE_NOPARAM( funcclass, T )               \
class funcclass : public gelExpression<T>             \
{                                                     \
public:                                               \
  funcclass(void) {}                                  \
  virtual ~funcclass() {}                             \
  gList<T> Evaluate(gelVariableTable *) const;        \
};


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
  gList<T> Evaluate( gelVariableTable *vt ) const     \
  {                                                   \
    gList<T> ret;                                     \
    gList<type1> arg1 = op1->Evaluate( vt );          \
    int i = 0;                                        \
    for( i = 1; i <= arg1.Length(); ++i )             \
      ret += EvalItem( arg1[i] );                     \
    return ret;                                       \
  }                                                   \
};


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
  gList<T> Evaluate( gelVariableTable *vt ) const     \
  {                                                   \
    gList<T> ret;                                     \
    gList<type1> arg1 = op1->Evaluate( vt );          \
    gList<type2> arg2 = op2->Evaluate( vt );          \
    assert( arg1.Length() == arg2.Length() );         \
    int i = 0;                                        \
    for( i = 1; i <= arg1.Length(); ++i )             \
      ret += EvalItem( arg1[i], arg2[i] );            \
    return ret;                                       \
  }                                                   \
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

  gList<T> Evaluate(gelVariableTable *) const;
};




#endif   // EXPRTREE_H

