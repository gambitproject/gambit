//
// FILE: gsminstr.h -- definition of Instruction classes for GSM's
//                     instruction queue subsystem
//                     companion to GSM
//
// $Id$
//

#ifndef GSMINSTR_H
#define GSMINSTR_H

#include "base/base.h"
#include "portion.h"

class GSM;

class gclExpression  {
protected:
  int m_line;
  gText m_file;

public:
  gclExpression(void);
  gclExpression(int, const gText &);
  virtual ~gclExpression()  { }
  
  virtual PortionSpec Type(void) const   { return porANYTYPE; }
  virtual Portion *Evaluate(GSM &) = 0;
};


class gclQuitOccurred : public gException {
private:
  int m_exitValue;

public:
  gclQuitOccurred(int p_exitValue) : m_exitValue(p_exitValue) { }
  ~gclQuitOccurred() { }

  gText Description(void) const { return "Quit expression executed"; }
  int Value(void) const { return m_exitValue; }
};


class gclQuitExpression : public gclExpression  {
  public:
    gclQuitExpression(void)  { }
    virtual ~gclQuitExpression()  { } 

    PortionSpec Type(void) const;
    Portion *Evaluate(GSM &);
};


class gclSemiExpr : public gclExpression  {
  private:
    gclExpression *lhs, *rhs;

  public:
    gclSemiExpr(gclExpression *l, gclExpression *r);
    virtual ~gclSemiExpr();

    PortionSpec Type(void) const;
    Portion *Evaluate(GSM &);
};

class gclReqParameterList  {
  private:
    gList<gclExpression *> exprs;

  public: 
    gclReqParameterList(void);
    gclReqParameterList(gclExpression *);
    ~gclReqParameterList();
  
    void Append(gclExpression *);

    int NumParams(void) const;
    gclExpression *operator[](int index) const;
};


class gclOptParameterList  {
  private:
    gList<gText> names;
    gList<gclExpression *> exprs;

  public: 
    gclOptParameterList(void);
    gclOptParameterList(const gText &, gclExpression *);
    virtual ~gclOptParameterList();
  
    void Append(const gText &, gclExpression *);

    int NumParams(void) const;
    gclExpression *operator[](int index) const;
    gText FormalName(int index) const;
};

class gclParameterList   {
  friend class gclFunctionCall;
  private:
    gclReqParameterList *req;
    gclOptParameterList *opt;

  public:
    gclParameterList(void);
    gclParameterList(gclReqParameterList *r);
    gclParameterList(gclOptParameterList *o);
    gclParameterList(gclReqParameterList *r, gclOptParameterList *o);
    ~gclParameterList();
};


class gclFunctionCall : public gclExpression   {
  private:
    gText name;
    gclParameterList *params;
    
    Portion *(*funcptr)(GSM &, Portion **);
    PortionSpec type;    

  public:
    gclFunctionCall(const gText &name, int, const gText &);
    gclFunctionCall(const gText &name, gclExpression *op, int, const gText &);
    gclFunctionCall(const gText &name,
                    gclExpression *op1, gclExpression *op2,
		    int, const gText &);
    gclFunctionCall(const gText &name, gclParameterList *params, 
		    int, const gText &);
    virtual ~gclFunctionCall();

    PortionSpec Type(void) const;
    Portion *Evaluate(GSM &);
};


class gclAssignment : public gclExpression  {
  private:
    gclExpression *variable, *value;

  public:
    gclAssignment(gclExpression *value, gclExpression *var);
    virtual ~gclAssignment();

    Portion *Evaluate(GSM &);
};

class gclUnAssignment : public gclExpression  {
  private:
    gclExpression *variable;

  public:
    gclUnAssignment(gclExpression *var);
    virtual ~gclUnAssignment();

    Portion *Evaluate(GSM &);
};

class gclFunction;

class gclFunctionDef : public gclExpression  {
  private:
    gclFunction *func;
    gclExpression *body;

  public:
    gclFunctionDef(gclFunction *f, gclExpression *b);
    virtual ~gclFunctionDef();

    Portion *Evaluate(GSM &);
};

class gclDeleteFunction : public gclExpression  {
private:
  gclFunction *func;

public:
  gclDeleteFunction(gclFunction *f);
  virtual ~gclDeleteFunction();

  Portion *Evaluate(GSM &);
};

class gclConstExpr : public gclExpression    {
  private:
    Portion *value;

  public:
    gclConstExpr(Portion *value);
    virtual ~gclConstExpr();

    PortionSpec Type(void) const;
    Portion *Evaluate(GSM &);
};

class gclListConstant : public gclExpression  {
  private:
    gList<gclExpression *> values;

  public:
    gclListConstant(void);
    gclListConstant(gclExpression *);
    virtual ~gclListConstant();

    void Append(gclExpression *);

    Portion *Evaluate(GSM &);
};


class gclVarName : public gclExpression   {
  private:
    Portion *value;

  public:
    gclVarName(const gText &);
    virtual ~gclVarName();

    Portion *Evaluate(GSM &);
};


class gclConditional : public gclExpression  {
  private:
    gclExpression *guard, *truebr, *falsebr;

  public:
    gclConditional(gclExpression *guard, gclExpression *iftrue);
    gclConditional(gclExpression *guard,
                   gclExpression *iftrue, gclExpression *iffalse);
    virtual ~gclConditional();

    Portion *Evaluate(GSM &);
};

class gclWhileExpr : public gclExpression  {
  private:
    gclExpression *guard, *body;

  public:
    gclWhileExpr(gclExpression *guard, gclExpression *body);
    virtual ~gclWhileExpr();

    Portion *Evaluate(GSM &);
};

class gclForExpr : public gclExpression  {
  private:
    gclExpression *init, *guard, *step, *body;

  public:
    gclForExpr(gclExpression *init, gclExpression *guard,
               gclExpression *step, gclExpression *body);
    virtual ~gclForExpr();

    Portion *Evaluate(GSM &);
};


#endif // GSMINSTR_H


