//
// FILE: gsmfunc.h -- definition of FuncDescObj, Function Descriptor Object
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

#define NO_PREDEFINED_PARAMS ( (ParamInfoType*) 0 )


#define AUTO_VAL_OR_REF      true


#define PARAM_NOT_FOUND   ( (int)      -1 )
#define PARAM_AMBIGUOUS   ( (int)      -2 )




class FuncDescObj;
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


class ParamInfoType
{
public:
  gText      Name;
  PortionSpec  Spec;
  Portion*     DefaultValue;
  bool         PassByReference;

  ParamInfoType(void);
  ParamInfoType(const ParamInfoType& paraminfo);
  ParamInfoType
    ( 
     const gText& name, 
     const PortionSpec& spec,
     Portion* default_value = REQUIRED, 
     const bool pass_by_ref = false
     );
  ~ParamInfoType();
};



class FuncInfoType
{
private:

public:
  bool                 UserDefined;
  union
  {
    Portion*             (*FuncPtr)(Portion **);
    gclExpression*      FuncInstr;
  };
  PortionSpec          ReturnSpec;
  FuncFlagType         Flag;
  int                  NumParams;
  ParamInfoType*       ParamInfo;
  gText              Desc;

  FuncInfoType(void);
  FuncInfoType(const FuncInfoType& funcinfo);
  FuncInfoType
    (
     Portion* (*funcptr)(Portion**),
     PortionSpec returnspec,
     int numparams,
     ParamInfoType* paraminfo = 0,
     FuncFlagType = funcLISTABLE
     );
  FuncInfoType
    (
     gclExpression* funcinstr,
     PortionSpec returnspec,
     int numparams,
     ParamInfoType* paraminfo = 0,
     FuncFlagType = funcLISTABLE
     );
  ~FuncInfoType();
};



class FuncDescObj
{
friend class gclFunctionCall;
private:

  static RefCountHashTable< gclExpression* > _RefCountTable;

protected:
  FuncDescObj( FuncDescObj& func );

  gText        _FuncName;
  int            _NumFuncs;
  FuncInfoType*  _FuncInfo;

  
public:
  FuncDescObj(const gText& func_name, int numfuncs);
  FuncDescObj(const gText& func_proto, Portion* (*funcptr)(Portion**),
                   FuncFlagType = funcLISTABLE  );
  virtual ~FuncDescObj();

  void SetFuncInfo(int funcindex, FuncInfoType funcinfo);
  void SetFuncInfo(int funcindex, const gText& s);
  void SetFuncInfo(int funcindex, const gText& s, 
                   Portion* (*funcptr)(Portion**), 
                   FuncFlagType = funcLISTABLE  );
  void SetParamInfo(int funcindex, int index, const ParamInfoType paraminfo);  
  void SetParamInfo(int funcindex, const ParamInfoType paraminfo[]);
  
  bool Combine(FuncDescObj* newfunc);
  bool Delete(FuncDescObj* newfunc);
  void Delete(int delete_index);

  gText FuncName(void) const;
  
  bool UDF( void ) const;
  bool BIF( void ) const;

  gList<gText> FuncList(bool udf = true, bool bif = true, 
			  bool getdesc = false) const;
  void Dump(gOutput& f, int i) const;
  void Dump(gOutput& f) const;
};




class CallFuncObj : public FuncDescObj
{
 private:
  struct RunTimeParamInfoType
  {
    ReferencePortion* Ref;
    bool              Defined;
    bool              AutoValOrRef;
  };

  gOutput&              _StdOut;
  gOutput&              _StdErr;

  int                   _FuncIndex;
  int                   _NumParams;
  int                   _NumParamsDefined;
  Portion**             _Param;
  RunTimeParamInfoType* _RunTimeParamInfo;
  bool*                 _FuncMatch;
  int                   _CurrParamIndex;
  bool                  _ErrorOccurred;

  gText _ParamName( const int index ) const;

  static bool _TypeMatch(Portion* p, PortionSpec ExpectedSpec, 
			 bool Listable, bool return_type_check = false);

  static bool _ListDimMatch( ListPortion* p1, ListPortion* p2 );
  static bool _ListNestedCheck( Portion* p, const ParamInfoType& info );

  static void _ErrorMessage
    (
     gOutput& s,
     const int error_num, 
     const long& num1 = 0,
     const gText& str1 = "",
     const gText& str2 = "",
     const gText& str3 = "",
     const gText& str4 = ""
     );

 public:
  CallFuncObj( FuncDescObj* func, gOutput& s_out, gOutput& s_err );
  ~CallFuncObj();

  int         NumParams ( void ) const;
  bool        SetCurrParamIndex ( const gText& param_name );

  bool  SetCurrParam ( Portion* param, bool auto_val_or_ref = false );

  void        SetErrorOccurred ( void );

  ReferencePortion* GetParamRef ( int index ) const;

  Portion* CallFunction       ( GSM* gsm, Portion** param );
  Portion* CallNormalFunction ( GSM* gsm, Portion** param );
  Portion* CallListFunction   ( GSM* gsm, Portion** ParamIn );

  void Dump(gOutput& f) const;
};

PortionSpec ToSpec(gText &str, int num=0);


#endif  // GSMFUNC_H


