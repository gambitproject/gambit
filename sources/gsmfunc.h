//#
//# FILE: gsmfunc.h -- definition of FuncDescObj, Function Descriptor Object
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef GSMFUNC_H
#define GSMFUNC_H


#include "gsmincl.h"

#include "basic.h"
#include "gstring.h"

#include "portion.h"


#define NO_DEFAULT_VALUE  (Portion*)  0
#define PARAM_NOT_FOUND   (int)      -1
#define PARAM_AMBIGUOUS   (int)      -2

#define PASS_BY_REFERENCE true

class FuncDescObj;
class CallFuncObj;

class gInteger;
class GSM;
class Instruction;
class Portion;
class Reference_Portion;

template <class T> class gList;
template <class T> class RefCountHashTable;


class ParamInfoType
{
friend FuncDescObj;
friend CallFuncObj;
friend GSM;
  
private:
  gString      Name;
  PortionType  Type;
  Portion*     DefaultValue;
  bool         PassByReference;

public:
  ParamInfoType( void )
  {
    Name = "";
    Type = porERROR;
    DefaultValue = NO_DEFAULT_VALUE;
    PassByReference = 0;
  }

  ParamInfoType( const ParamInfoType& param_info )
    :
  Name( param_info.Name ),
  Type( param_info.Type ),
  DefaultValue( param_info.DefaultValue ),
  PassByReference( param_info.PassByReference )
  { }

  ParamInfoType
    ( 
     const gString& name, 
     const PortionType& type,
     Portion* default_value, 
     const bool pass_by_ref = false 
     )
      :
  Name( name ), 
  Type( type ), 
  DefaultValue( default_value ), 
  PassByReference( pass_by_ref )
  { }

  ~ParamInfoType()
  { }
};



class FuncInfoType
{
public:
  bool                 UserDefined;
  Portion*             (*FuncPtr)(Portion **);
  gList<Instruction*>* FuncInstr;
  int                  NumParams;
  ParamInfoType*       ParamInfo;
};



class FuncDescObj
{
private:

  static RefCountHashTable< gList< Instruction* >* > _RefCountTable;

  void _SetFuncInfo
    ( 
     const int f_index, 
     const int num_params
     );

  void _SetParamInfo
    ( 
     const int         f_index, 
     const int         index, 
     const gString&    name,
     const PortionType type,
     Portion*          default_value,
     const bool        pass_by_reference
     );

protected:
  FuncDescObj( FuncDescObj& func );

  gString        _FuncName;
  int            _NumFuncs;
  FuncInfoType*  _FuncInfo;

  
public:
  FuncDescObj( const gString& func_name );
  virtual ~FuncDescObj();

  void SetFuncInfo
    (
     Portion*        (*func_ptr)(Portion**),
     const int       num_params = 0 
     );

  void SetFuncInfo
    (
     gList< Instruction* >* func_instr,
     const int       num_params = 0 
     );

  void SetParamInfo
    ( 
     Portion*          (*func_ptr)(Portion**),
     const int         index, 
     const gString&    name,
     const PortionType type,
     Portion*          default_value = NO_DEFAULT_VALUE,
     const bool        pass_by_reference = false
     );

  void SetParamInfo
    ( 
     gList< Instruction* >* func_instr,
     const int         index, 
     const gString&    name,
     const PortionType type,
     Portion*          default_value = NO_DEFAULT_VALUE,
     const bool        pass_by_reference = false
     );
  
  void SetParamInfo
    (
     Portion* (*func_ptr)(Portion**),
     const ParamInfoType param_info[]
     );
  
  gString FuncName ( void ) const;
};




class CallFuncObj : public FuncDescObj
{
 private:
  struct RunTimeParamInfoType
  {
    bool               Defined;
    Reference_Portion* Ref;
    Portion*           ShadowOf;
  };

  gOutput&              _StdOut;
  gOutput&              _StdErr;

  int                   _FuncIndex;
  int                   _NumParams;
  int                   _NumParamsDefined;
  Portion**             _Param;
  RunTimeParamInfoType* _RunTimeParamInfo;
  int                   _CurrParamIndex;
  bool                  _ErrorOccurred;

  bool _TypeMatch( Portion* p, PortionType ExpectedType ) const;

  static void _ErrorMessage
    (
     gOutput& s,
     const int error_num, 
     const gInteger& num1 = 0,
     const gString& str1 = "",
     const gString& str2 = ""
     );

 public:
  CallFuncObj( FuncDescObj* func, gOutput& s_out, gOutput& s_err );
  ~CallFuncObj();

  int         NumParams ( void ) const;
  bool        ParamPassByReference( const int index ) const;
  int         FindParamName        ( const gString& param_name );

  void  SetCurrParamIndex   ( const int index );
  bool  SetCurrParam ( Portion* param, Reference_Portion* ref_param = 0 );

  void        SetErrorOccurred ( void );

  Reference_Portion* GetCurrParamRef ( void ) const;
  Portion*    GetCurrParamShadowOf ( void ) const;

  Portion*    CallFunction      ( GSM*, Portion** param );
};



#endif  // GSMFUNC_H


