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



#endif  // GSMFUNC_H
