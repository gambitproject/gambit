//
// FILE: gsm.h -- definition of GSM (stack machine) type
//
// $Id$
//


#ifndef GSM_H
#define GSM_H


#include "gambitio.h"
#include "gstring.h"
#include "gsmincl.h"
#include "gsminstr.h"
#include "gsmhash.h"

class FuncDescObj;
class CallFuncObj;
class Portion;
class RefHashTable;
class FunctionHashTable;
class FuncInfoType;

class gString;
class gRational;

class Node;
class Infoset;
class Efg;

template <class T> class gList;
template <class T> class gStack;

template <class T> class RefCountHashTable;


#define GCL_VERSION   0.95

class GSM
{
friend class gclFunctionCall;
friend class gclVarName;
private:
  static int _NumObj;

  static RefCountHashTable< void* > _GameRefCount;

  bool _Verbose;

  gInput&  _StdIn;
  gOutput& _StdOut;
  gOutput& _StdErr;

  Portion* _OUTPUT;
  Portion* _INPUT;
  Portion* _NULL;

  gStack< gStack< Portion* >* >* _StackStack;
  gStack< RefHashTable* >*       _RefTableStack;
  RefHashTable                   _GlobalRefTable;
  FunctionHashTable*             _FuncTable;

  void _BindCheck ( void ) const;
  bool _Bind ( const gString& param_name ) const;

  Portion* _VarRemove    ( const gString& var_name );

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

  static int& GameRefCount(void*);

  bool PushRef  ( const gString& ref );
  Portion* _ResolveRef             ( Portion* p );

  bool Assign   ( Portion *, Portion * );
  bool VarDefine  ( const gString& var_name, Portion* p );
  bool VarIsDefined ( const gString& var_name ) const;
  Portion* VarValue     ( const gString& var_name ) const;
  bool UnAssign ( Portion * );
  Portion* UnAssignExt( Portion * );

  bool AddFunction( FuncDescObj* func );
  bool DeleteFunction( FuncDescObj* func );

  Portion *Execute(gclExpression *, bool user_func = false );
  Portion* ExecuteUserFunc( gclExpression& program, 
			   const FuncInfoType& func_info,
			   Portion** param );
  void Clear  ( void );

  Portion* PopValue( void );

  Portion* Help(gString text, bool udf, bool bif, bool getdesc = false );
  Portion* HelpVars(gString text);

  void InvalidateGameProfile( void* game, bool IsEfg );
  void UnAssignGameElement( void* game, bool IsEfg, PortionSpec spec );  

  // Unassigns a SINGLE given element of an Efg
  void UnAssignEfgElement( Efg* game, PortionSpec spec, void* data );

  // This function will unassign an infoset and all associated actions
  void UnAssignEfgInfoset( Efg* game, Infoset* infoset );

  // This function will unassign the subtree rooted by the given node
  void UnAssignEfgSubTree( Efg* game, Node* node ); 






  void GlobalVarDefine     ( const gString& var_name, Portion* p );
  bool GlobalVarIsDefined  ( const gString& var_name ) const;
  Portion* GlobalVarValue  ( const gString& var_name ) const;
  void GlobalVarRemove     ( const gString& var_name );

};



#endif  // GSM_H

