//
// FILE: gsmfunc.h -- definition of gclFunction, Function Descriptor Object
//                    companion to GSM
//
// $Id$
//




#ifndef GSMFUNC_H
#define GSMFUNC_H


#include "gsmincl.h"
#include "gtext.h"


class gOutput;

typedef unsigned int    FuncFlagType;
#define funcNULL        ( (FuncFlagType) 0x0000 )
#define funcNONLISTABLE funcNULL
#define funcLISTABLE    ( (FuncFlagType) 0x0001 )
#define funcGAMEMATCH   ( (FuncFlagType) 0x0002 )


#define REQUIRED          ( (Portion*)  0 )

#define BYREF                true
#define BYVAL                false

#define NO_PREDEFINED_PARAMS ( (gclParameter*) 0 )


#define AUTO_VAL_OR_REF      true


#define PARAM_NOT_FOUND   ( (int)      -1 )
#define PARAM_AMBIGUOUS   ( (int)      -2 )




class gclFunction;
class CallFuncObj;

class gInteger;
class GSM;
class Portion;
class ListPortion;
class ReferencePortion;
class gText;
class gclExpression;

template <class T> class gList;
template <class T> class RefCountHashTable;


class gclParameter
{
public:
  gText      Name;
  PortionSpec  Spec;
  Portion*     DefaultValue;
  bool         PassByReference;

  gclParameter(void);
  gclParameter(const gclParameter& paraminfo);
  gclParameter(const gText &name, const PortionSpec &spec,
		Portion *default_value = REQUIRED, 
		const bool pass_by_ref = false);
  ~gclParameter();
};


class gclSignature  {
public:
  bool UserDefined;
  union  {
    Portion *(*FuncPtr)(Portion **);
    gclExpression *FuncInstr;
  };
  PortionSpec ReturnSpec;
  FuncFlagType Flag;
  int NumParams;
  gclParameter *ParamInfo;
  gText Desc;

  gclSignature(void);
  gclSignature(const gclSignature& funcinfo);
  gclSignature(Portion* (*funcptr)(Portion**),
	       PortionSpec returnspec, int numparams,
	       gclParameter* paraminfo = 0, FuncFlagType = funcLISTABLE);
  gclSignature(gclExpression* funcinstr, PortionSpec returnspec,
	       int numparams, gclParameter* paraminfo = 0,
	       FuncFlagType = funcLISTABLE);
  ~gclSignature();
};



class gclFunction  {
friend class gclFunctionCall;
private:
  static RefCountHashTable<gclExpression *> _RefCountTable;

protected:
  gclFunction( gclFunction& func );

  gText        _FuncName;
  int            _NumFuncs;
  gclSignature*  _FuncInfo;

  
public:
  gclFunction(const gText& func_name, int numfuncs);
  gclFunction(const gText& func_proto, Portion* (*funcptr)(Portion**),
                   FuncFlagType = funcLISTABLE  );
  virtual ~gclFunction();

  void SetFuncInfo(int funcindex, gclSignature funcinfo);
  void SetFuncInfo(int funcindex, const gText& s);
  void SetFuncInfo(int funcindex, const gText& s, 
                   Portion* (*funcptr)(Portion**), 
                   FuncFlagType = funcLISTABLE  );
  void SetParamInfo(int funcindex, int index, const gclParameter paraminfo);  
  void SetParamInfo(int funcindex, const gclParameter paraminfo[]);
  
  bool Combine(gclFunction* newfunc);
  bool Delete(gclFunction* newfunc);
  void Delete(int delete_index);

  gText FuncName(void) const;
  
  bool UDF( void ) const;
  bool BIF( void ) const;

  gList<gText> FuncList(bool udf = true, bool bif = true, 
			  bool getdesc = false) const;
  void Dump(gOutput& f, int i) const;
  void Dump(gOutput& f) const;
};




class CallFuncObj : public gclFunction {
private:
  struct RunTimeParamInfoType {
    ReferencePortion *Ref;
    bool Defined, AutoValOrRef;
  };

  int m_line;
  gText m_file;
  int m_funcIndex, m_numParams, m_numParamsDefined;
  Portion **m_params;
  RunTimeParamInfoType *m_runTimeParamInfo;
  bool *m_funcMatch;
  int m_currParamIndex;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  gText ParamName(int index) const;

  void ComputeFuncIndex(void);
  Portion *CallNormalFunction(GSM *, Portion **);
  Portion *CallListFunction(GSM *, Portion **);

 public:
  CallFuncObj(gclFunction *, int, const gText &);
  ~CallFuncObj();

  int NumParams(void) const { return m_numParams; }
  bool SetCurrParamIndex(const gText &);
  bool SetCurrParam(Portion *, bool = false);

  ReferencePortion *GetParamRef(int p_index) const;
  Portion *CallFunction(GSM *, Portion **);

  void Dump(gOutput &) const;
};

PortionSpec ToSpec(gText &str, int num = 0);


#endif  // GSMFUNC_H


