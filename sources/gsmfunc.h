//#
//# FILE: gsmfunc.h -- definition of FuncDescObj, Function Descriptor Object
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef GSMFUNC_H
#define GSMFUNC_H


#include "portion.h"

#define NO_DEFAULT_VALUE  (Portion*)  0
#define PARAM_NOT_FOUND   (int)      -1
#define PARAM_AMBIGUOUS   (int)      -2

#define PASS_BY_REFERENCE true


class FuncDescObj
{
 protected:
  struct ParamInfoType
  {
    gString      Name;
    PortionType  Type;
    Portion*     DefaultValue;
    bool         PassByReference;
  };

  struct FuncInfoType
  {
    Portion*       (*FuncPtr)(Portion **);
    int            NumParams;
    ParamInfoType* ParamInfo;
  };

  gString        _FuncName;
  int            _NumFuncs;
  FuncInfoType*  _FuncInfo;

 public:
  FuncDescObj( FuncDescObj& func );
  FuncDescObj( const gString& func_name );
  virtual ~FuncDescObj();

  void SetFuncInfo
    (
     Portion*        (*func_ptr)(Portion**),
     const int       num_params = 0 
     );

  void SetParamInfo
    ( 
     Portion*          (*func_ptr)(Portion**),
     const int         index, 
     const gString&    name,
     const PortionType type,
     Portion*          default_value,
     const bool        pass_by_reference = false
     );

  gString     FuncName             ( void ) const;
};




class CallFuncObj : public FuncDescObj
{
 private:
  struct RunTimeParamInfoType
  {
    bool               Defined;
    Reference_Portion* Ref;
  };
  int                   _FuncIndex;
  int                   _NumParams;
  int                   _NumParamsDefined;
  Portion**             _Param;
  RunTimeParamInfoType* _RunTimeParamInfo;
  int                   _CurrParamIndex;
  bool                  _ErrorOccurred;

 public:
  CallFuncObj( FuncDescObj* func );
  ~CallFuncObj();

  int         NumParams ( void ) const;
  bool        ParamPassByReference( const int index ) const;
  int         FindParamName        ( const gString& param_name );

  void        SetCurrParamIndex   ( const int index );
  bool        SetCurrParam        ( Portion *new_param );
  void        SetCurrParamRef ( Reference_Portion* ref );

  void        SetErrorOccurred ( void );

  Reference_Portion* GetParamRef ( const int index ) const;

  int         GetCurrParamIndex           ( void ) const;
  PortionType GetCurrParamType            ( void ) const;
  bool        GetCurrParamPassByRef ( void ) const;
  Reference_Portion* GetCurrParamRef ( void ) const;

  Portion*    CallFunction      ( Portion** param );
};



#endif  // GSMFUNC_H

