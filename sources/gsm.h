//#
//# FILE: gsm.h -- definition of GSM (stack machine) type
//#
//# $Id$
//#


#ifndef GSM_H
#define GSM_H

#include <assert.h>
#include "rational.h"
#include "gstack.h"
#include "gambitio.h"


#include "portion.h"
#include "gsmfunc.h"



// These two classes implemented in gsm.cc
class FunctionHashTable;
class RefHashTable;


class GSM
{
 private:
  gStack<Portion*>*   stack;
  RefHashTable*       RefTable;
  FunctionHashTable*  FuncTable;

  Portion* resolve_ref( Reference_Portion* p );

  bool unary_operation  ( OperationMode mode );
  bool binary_operation ( OperationMode mode );


  // This function is located in gsmfunc.cc
  void InitFunctions( void );

  int FuncParamCheck( const PortionType stack_param_type, 
		     const PortionType func_param_type );


 public:
  GSM( int size );
  ~GSM();

  int Depth    ( void ) const;
  int MaxDepth ( void ) const;


  bool Push ( const bool&      data );
  bool Push ( const double&    data );
  bool Push ( const gInteger&  data );
  bool Push ( const gRational& data );
  bool Push ( const gString&   data );

  bool PushList ( const int num_of_elements );

  bool PushRef  ( const gString& data );
  bool Assign   ( void );
  bool UnAssign ( const gString& ref );


  bool Add      ( void );
  bool Subtract ( void );
  bool Multiply ( void );
  bool Divide   ( void );
  bool Negate   ( void );

  bool EqualTo              ( void );
  bool NotEqualTo           ( void );
  bool GreaterThan          ( void );
  bool LessThan             ( void );
  bool GreaterThanOrEqualTo ( void );
  bool LessThanOrEqualTo    ( void );
  
  bool AND ( void );
  bool OR  ( void );
  bool NOT ( void );

  bool Concatenate ( void );

  
  void AddFunction(const gString& funcname, FuncDescObj* func );

  bool CallFunction
    ( 
     const gString&  funcname,
     const int       num_of_params_passed,
     const gString*  name
     );

     
  void Output ( void );
  void Dump   ( void );
  void Flush  ( void );

};



#endif  // GSM_H

