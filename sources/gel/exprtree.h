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


#include "tristate.h"
#include "gtext.h"
#include "funcmisc.h"





class gelVariableTable;
class gelSignature;

class gelExpr   {
  public:
    virtual ~gelExpr();
    virtual gelType Type(void) const = 0;
    virtual void Execute(gelVariableTable *) const = 0;
}; 

template <class T> class gelExpression : public gelExpr   {
  public:
    virtual ~gelExpression();
    gelType Type(void) const;
    void Execute(gelVariableTable *) const;
    virtual T Evaluate(gelVariableTable *) const = 0;
};
 
template <class T> class gelAssignment : public gelExpression<T>   {
  private:
    gText name;
    gelExpression<T> *rhs;

  public:
    gelAssignment(const gText &, gelExpression<T> *);
    virtual ~gelAssignment();
    virtual T Evaluate(gelVariableTable *) const;
};

template <class T> class gelConstant : public gelExpression<T>  {
  private:
    T value;
 
  public:
    gelConstant(const T &);
    virtual ~gelConstant();
    T Evaluate(gelVariableTable *) const;
};

template <class T> class gelVariable : public gelExpression<T>  {
  private:
    gText m_Name;
  
  public:
    gelVariable(const gText &name);
    virtual ~gelVariable();

    const gText &Name(void) const { return m_Name; }
    T Evaluate(gelVariableTable *) const;
};

template <class T> class gelConditional : public gelExpression<T>  {
  private:
    gelExpression< gTriState > *guard;
    gelExpression< T > *truebr, *falsebr;

  public:
    gelConditional(gelExpression<gTriState> *guard,
		   gelExpression<T> *iftrue);
    gelConditional(gelExpression<gTriState> *guard,
                   gelExpression<T> *iftrue, gelExpression<T> *iffalse);
    virtual ~gelConditional();

    T Evaluate(gelVariableTable *) const;
};

template <class T> class gelWhileLoop : public gelExpression<T>  {
  private:
    gelExpression<gTriState> *guard;
    gelExpression<T> *body;

  public:
    gelWhileLoop(gelExpression<gTriState> *, gelExpression<T> *);
    virtual ~gelWhileLoop();

    T Evaluate(gelVariableTable *) const;
};

template <class T> class gelForLoop : public gelExpression<T>  {
  private:
    gelExpr *init;
    gelExpression<gTriState> *guard;
    gelExpr *incr;
    gelExpression<T> *body;

  public:
    gelForLoop(gelExpr *, gelExpression<gTriState> *, gelExpr *,
	       gelExpression<T> *);
    virtual ~gelForLoop();

    T Evaluate(gelVariableTable *) const;
};


#include "tristate.h"

class gelQuitExpr : public gelExpression<gTriState>  {
  public:
    gelQuitExpr(void)  { }
    virtual ~gelQuitExpr()   { }

    gTriState Evaluate(gelVariableTable *) const;
};



#include "funcmisc.h"

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

  T Evaluate(gelVariableTable *) const;
};




#endif   // EXPRTREE_H

