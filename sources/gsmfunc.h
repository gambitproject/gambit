//#
//# FILE: gsmfunc.h -- definition of FuncDescObj, Function Descriptor Object
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef GSMFUNC_H
#define GSMFUNC_H


#include "portion.h"

#define NO_DEFAULT_VALUE (Portion*) 0


class FuncDescObj
{
 private:
  struct ParamInfoType
  {
    gString      Name;
    PortionType  Type;
    Portion*     DefaultValue;
  };

  int            num_of_params;
  ParamInfoType* ParamInfo;
  Portion*       (*function)(Portion **);

 public:
  FuncDescObj( Portion* (*funcname)(Portion**), const int size = 0 );
  ~FuncDescObj();

  Portion*    CallFunction      ( Portion** param );
  int         NumParams         ( void ) const;
  gString     ParamName         ( const int index ) const;
  PortionType ParamType         ( const int index ) const;
  Portion*    ParamDefaultValue ( const int index ) const;
  int         FindParamName     ( const gString& name ) const;

  void SetParamInfo
    ( 
     const int         index, 
     const gString&    name,
     const PortionType type,
     Portion*          default_value
     );
};




class CallFunctionObject
{
 private:
  gString       func_name;
  FuncDescObj*  func_desc_obj;
  Portion**     param;
  int           current_param_index;

 public:
  CallFunctionObject( const gString& name, FuncDescObj* func );

  PortionType GetCurrParamType  ( void ) const;
  int         GetCurrParamIndex ( void ) const;
  void        SetCurrParamIndex ( const int index );
  void        SetCurrParam      ( Portion *new_param );
  int         FindParamName     ( const gString& name ) const;
  gString     FuncName          ( void ) const;
  Portion*    CallFunction      ( void );
};



#endif  // GSMFUNC_H
