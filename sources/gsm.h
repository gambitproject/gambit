//#
//# FILE: gsm.h -- definition of GSM (stack machine) type
//#
//# $Id$
//#


#ifndef GSM_H
#define GSM_H

#include "glist.h"
#include "ggrstack.h"

#include "portion.h"
#include "gsmfunc.h"
#include "gsminstr.h"
#include "gsmhash.h"



typedef enum { rcFAIL, rcSUCCESS, rcQUIT } GSM_ReturnCode;



class FunctionHashTable;
class Instruction;



class GSM
{
 private:
  gOutput&                         _StdOut;
  gOutput&                         _StdErr;

  gGrowableStack< Portion* >*      _Stack;
  gGrowableStack< CallFuncObj* >*  _CallFuncStack;
  RefHashTable*                    _RefTable;
  FunctionHashTable*               _FuncTable;

  Portion* _ResolveRef            ( Reference_Portion* p );
  Portion* _ResolveRefWithoutError ( Reference_Portion* p );
  Portion* _ResolvePrimaryRefOnly ( Reference_Portion* p );

  bool _UnaryOperation  ( OperationMode mode );
  bool _BinaryOperation ( OperationMode mode );

  void _BindCheck ( void ) const;
  bool _BindCheck ( const gString& param_name ) const;


  // This function is located in gsmfunc.cc
  void InitFunctions( void );


 public:
  GSM( int size, gOutput& s_out = gout, gOutput& s_err = gerr );
  ~GSM();

  gOutput& StdErr( void ) const;

  int Depth    ( void ) const;
  int MaxDepth ( void ) const;


  bool Push ( const bool&      data );
  bool Push ( const double&    data );
  bool Push ( const gInteger&  data );
  bool Push ( const gRational& data );
  bool Push ( const gString&   data );

  bool Push ( Outcome* data );
  bool Push ( Player*  data );
  bool Push ( Infoset* data );
  bool Push ( Action*  data );
  bool Push ( Node*    data );

  bool PushStream( const gString& data );

  bool PushList ( const int num_of_elements );

  bool PushRef  ( const gString& ref );
  bool PushRef  ( const gString& ref, const gString& subref );
  bool Assign   ( void );
  bool UnAssign ( void );


  bool Add      ( void );
  bool Subtract ( void );
  bool Multiply ( void );
  bool Divide   ( void );
  bool Negate   ( void );

  bool IntegerDivide ( void );
  bool Modulus       ( void );

  bool EqualTo              ( void );
  bool NotEqualTo           ( void );
  bool GreaterThan          ( void );
  bool LessThan             ( void );
  bool GreaterThanOrEqualTo ( void );
  bool LessThanOrEqualTo    ( void );
  
  bool AND ( void );
  bool OR  ( void );
  bool NOT ( void );
  
  bool Subscript ( void );

  
  void AddFunction( FuncDescObj* func );

  bool InitCallFunction ( const gString& funcname );
  bool Bind             ( void );
  bool BindVal          ( void );
  bool BindRef          ( void );
  bool Bind             ( const gString& param_name );
  bool BindVal          ( const gString& param_name );
  bool BindRef          ( const gString& param_name );
  bool CallFunction     ( void );

  GSM_ReturnCode Execute( gList< Instruction* >& program );
     
  bool Pop ( void );

  void Output ( void );
  void Dump   ( void );
  void Flush  ( void );
};




#endif  // GSM_H

