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

  int unary_operation( OperationMode mode );
  int binary_operation( OperationMode mode );

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

  int  EqualTo              ( void );
  int  NotEqualTo           ( void );
  int  GreaterThan          ( void );
  int  LessThan             ( void );
  int  GreaterThanOrEqualTo ( void );
  int  LessThanOrEqualTo    ( void );
  
  void AND ( void );
  void OR  ( void );
  void NOT ( void );

  void Concatenate( void );


  static void AddFunction( const gString& funcname, Portion *(*function)( void ) );
  void CallFunction( const gString& funcname );


  void Output( void );
  void Dump( void );

  void Flush( void );

};




#endif  // GSM_H

