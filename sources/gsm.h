//#
//# FILE: gsm.h -- definition of GSM (stack machine) type
//#
//# $Id$
//#


#ifndef GSM_H
#define GSM_H

#include "glist.h"
#include "gstack.h"

#include "portion.h"
#include "gsmfunc.h"
#include "gsminstr.h"
#include "gsmhash.h"


int FuncParamCheck( const PortionType stack_param_type, 
		    const PortionType func_param_type );


class FunctionHashTable;
class Instruction;

class GSM
{
 private:
  gStack< Portion* >*      _Stack;
  gStack< CallFuncObj* >*  _CallFuncStack;
  RefHashTable*            _RefTable;
  FunctionHashTable*       _FuncTable;

  Portion* resolve_ref    ( Reference_Portion* p );
  Portion* resolve_subref ( Reference_Portion* p );

  bool unary_operation  ( OperationMode mode );
  bool binary_operation ( OperationMode mode );


  // This function is located in gsmfunc.cc
  void InitFunctions( void );


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

  bool GenerateNfg ( const double& data );

  bool PushList ( const int num_of_elements );

  bool PushRef  ( const gString& ref );
  bool PushRef  ( const gString& ref, const gString& subref );
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

  
  void AddFunction( FuncDescObj* func );

  bool InitCallFunction ( const gString& funcname );
  bool Bind             ( void );
  bool Bind             ( const gString& param_name );
  bool CallFunction     ( void );

  bool Execute( gList< Instruction* >& program );
     
  void Output ( void );
  void Dump   ( void );
  void Flush  ( void );
};



#endif  // GSM_H

