//
// FILE: gsm.h -- definition of GSM (stack machine) type
//
// $Id$
//


#ifndef GSM_H
#define GSM_H


#include "gambitio.h"
#include "gstring.h"
//#include "gsmincl.h"
//#include "gsmfunc.h"

//class NewInstr;
class NewInstr;
class FuncDescObj;
class CallFuncObj;
class Portion;
class Reference_Portion;
class RefHashTable;
class FunctionHashTable;
class FuncInfoType;

class gString;
class gRational;

template <class T> class gList;
template <class T> class gStack;

#define GCL_VERSION   0.94

class GSM
{
private:
  static int _NumObj;

  gInput&  _StdIn;
  gOutput& _StdOut;
  gOutput& _StdErr;

  Portion* _OUTPUT;
  Portion* _INPUT;
  Portion* _NULL;

  gStack< gStack< Portion* >* >* _StackStack;
  gStack< CallFuncObj* >*                _CallFuncStack;
  gStack< RefHashTable* >*               _RefTableStack;
  FunctionHashTable*                             _FuncTable;

  Portion* _ResolveRef             ( Portion* p );

  bool _UnaryOperation  ( const gString& funcname );
  bool _BinaryOperation ( const gString& funcname );

  void _BindCheck ( void ) const;
  bool _Bind ( const gString& param_name ) const;

  bool     _VarIsDefined ( const gString& var_name ) const;
  // note that p may be changed after a call to _VarDefine()
  bool     _VarDefine    ( const gString& var_name, Portion*& p );
  Portion* _VarValue     ( const gString& var_name ) const;
  Portion* _VarRemove    ( const gString& var_name );

  int      _Depth( void ) const;
  void     _Push( Portion* p );
  Portion* _Pop( void );

  static void _ErrorMessage
    (
     gOutput&        s,
     const int       error_num = 0,
     const long& num1      = 0, 
     const long& num2      = 0,
     const gString&  str1      = "",
     const gString&  str2      = "",
     const gString&  str3      = ""
     );

  // This function is located in gsmfunc.cc
  void InitFunctions( void );


public:
  GSM( int size = 10, 
      gInput&  s_in  = gin, 
      gOutput& s_out = gout, 
      gOutput& s_err = gerr );
  ~GSM();

  int Depth    ( void ) const;
  int MaxDepth ( void ) const;


  Portion*& DefaultNfg( void );
  Portion*& DefaultEfg( void );



  bool Push(Portion* data);
  bool Push ( const bool&      data );
  bool Push ( const long&      data );
  bool Push ( const double&    data );
  bool Push ( const gRational& data );
  bool Push ( const gString&   data );
  bool Push ( gInput&    data );
  bool Push ( gOutput&   data );

  bool PushList ( const int num_of_elements );

  bool PushRef  ( const gString& ref );
  bool Assign   ( void );
  bool UnAssign ( void );


  bool Add      ( void );
  bool Subtract ( void );
  bool Multiply ( void );
  bool Dot      ( void );
  bool Divide   ( void );
  bool Negate   ( void );
  bool Power    ( void );

  bool Concat   ( void );

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
  bool Child ( void );
  bool Read ( void );
  bool Write ( void );
  
  bool AddFunction( FuncDescObj* func );

  bool InitCallFunction ( const gString& funcname );
  bool Bind           ( const gString& param_name = "" );
  bool BindVal        ( const gString& param_name = "" );
  bool BindRef        ( const gString& param_name = "", 
		       bool auto_val_or_ref = false );
  bool CallFunction   ( void );

  int Execute( gList<NewInstr*>& program, 
	      bool user_func = false );
  Portion* ExecuteUserFunc( gList<NewInstr*>& program, 
			   const FuncInfoType& func_info,
			   Portion** param );
  
  void Output ( void );
  void Dump   ( void );

  bool Pop    ( void );
  void Flush  ( void );
  void Clear  ( void );

  Portion* Help(gString text);
  Portion* HelpVars(gString text);
  Portion* UnAssignExt( void );
};



#endif  // GSM_H

