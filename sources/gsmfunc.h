//
//  FILE: gsmfunc.h - declaration of FuncDescObj, Function Descriptor Object type
//                    companion to GSM
//



#ifndef GSMFUNC_H
#define GSMFUNC_H


#include "portion.h"


class FuncDescObj
{
 private:
  int num_of_params;
  PortionType *param_type;
  Portion *(*function)(Portion **);

 public:
  FuncDescObj(Portion *(*funcname)(Portion **), 
	      const int& size = 0 );
  ~FuncDescObj();
  Portion *CallFunction(Portion **param);
  int NumParams( void ) const;
  PortionType ParamType( const int& index ) const;
  PortionType& ParamType( const int& index );
};



#endif  // GSMFUNC_H
