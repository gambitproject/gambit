//
//  FILE: GSM.h  definition of GSM (stack machine) type
//
//


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
  static int FunctionsInitialized;
  static FunctionHashTable *FuncTable;

  gStack<Portion *> *stack;
  RefHashTable *RefTable;

  Portion *resolve_ref( Reference_Portion *p );

  void unary_operation( OperationMode mode );
  void binary_operation( OperationMode mode );


  // This function is located in gsmfunc.cc
  static void InitFunctions( void );


 public:
  GSM( int size );
  ~GSM();

  int Depth( void ) const;
  int MaxDepth( void ) const;

  void Push( const bool&      data );
  void Push( const double&    data );
  void Push( const gInteger&  data );
  void Push( const gRational& data );
  void Push( const gString&   data );

  void PushRef( const gString& data );

  void Assign( void );
  void UnAssign( const gString& ref );



  void Add      ( void );
  void Subtract ( void );
  void Multiply ( void );
  void Divide   ( void );
  void Negate   ( void );

  void EqualTo              ( void );
  void NotEqualTo           ( void );
  void GreaterThan          ( void );
  void LessThan             ( void );
  void GreaterThanOrEqualTo ( void );
  void LessThanOrEqualTo    ( void );
  
  void AND ( void );
  void OR  ( void );
  void NOT ( void );

  void Concatenate( void );

  static void AddFunction(const gString& funcname, FuncDescObj *func );
  static int FunctionParamCheck(const PortionType stack_param_type, 
				const PortionType func_param_type );
  void CallFunction( const gString& funcname );


  void Output( void );
  void Dump( void );

  void Flush( void );

};



#endif  // GSM_H

