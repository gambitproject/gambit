//#
//# FILE: gsmfunc.cc -- handles initialization of defined functions for GSM
//#                     companion to GSM
//#
//# $Id$
//#


#include "gsmfunc.h"

#include <assert.h>
#include "glist.h"

#include "gsm.h"
#include "portion.h"
#include "gsmhash.h"
#include "gsminstr.h"



// This function is called once at the first instance of GSM.
// The Init function of each module should be placed in this function:
// Each Init() function should take the argument "this" so each instance
// of GSM can add those functions to their memory.

extern void Init_gclmath( GSM* );
extern void Init_gsmoper( GSM* );
extern void Init_nfgfunc( GSM* );
extern void Init_efgfunc( GSM* );
extern void Init_algfunc(GSM *);
extern void Init_listfunc(GSM *);
void Init_userfunc( GSM* );

void GSM::InitFunctions( void )
{
  Init_gclmath( this );
  Init_gsmoper( this );

  Init_listfunc(this);

  Init_nfgfunc(this);
  Init_efgfunc(this);
  Init_algfunc(this);

  Init_userfunc( this );
}




void Init_userfunc( GSM* gsm )
{
  gList< Instruction* >* prog;
  FuncDescObj* func;

  prog = new gList< Instruction* >;
  prog->Append( new PushRef( "x" ) );
  prog->Append( new PushRef( "y" ) );
  prog->Append( new Assign );
  
  func = new FuncDescObj( "Assign" );
  func->SetFuncInfo( prog, 2 );
  func->SetParamInfo( prog, 0, "x", 
		     porVALUE, NO_DEFAULT_VALUE,
		     PASS_BY_REFERENCE );
  func->SetParamInfo( prog, 1, "y", 
		     porVALUE );
  gsm->AddFunction( func );

}






//---------------------------------------------------------------
//                      ParamInfoType
//---------------------------------------------------------------


ParamInfoType::ParamInfoType( void )
{
  Name = "";
  Type = porERROR;
  DefaultValue = NO_DEFAULT_VALUE;
  PassByReference = 0;
}

ParamInfoType::ParamInfoType( const ParamInfoType& param_info )
:
 Name( param_info.Name ),
 Type( param_info.Type ),
 DefaultValue( param_info.DefaultValue ),
 PassByReference( param_info.PassByReference )
{ }

ParamInfoType::ParamInfoType
( 
 const gString& name, 
 const PortionType& type,
 Portion* default_value, 
 const bool pass_by_ref
 )
:
 Name( name ), 
 Type( type ), 
 DefaultValue( default_value ), 
 PassByReference( pass_by_ref )
{ }

ParamInfoType::~ParamInfoType()
{ }

ParamInfoType& ParamInfoType::operator = ( const ParamInfoType& param_info )
{
  Name = param_info.Name;
  Type = param_info.Type;
  DefaultValue = param_info.DefaultValue;
  PassByReference = param_info.PassByReference;
  return *this;
}



//---------------------------------------------------------------------
//                   Function descriptor objects
//---------------------------------------------------------------------

RefCountHashTable< gList< Instruction* >* > FuncDescObj::_RefCountTable;

   
FuncDescObj::FuncDescObj( FuncDescObj& func )
{
  int index;
  int f_index;

  _FuncName  = func._FuncName;
  _NumFuncs  = func._NumFuncs;
  _FuncInfo  = new FuncInfoType[ _NumFuncs ];

  for( f_index = 0; f_index < _NumFuncs; f_index++ )
  {
    _FuncInfo[ f_index ].UserDefined = func._FuncInfo[ f_index ].UserDefined;
    _FuncInfo[ f_index ].FuncPtr     = func._FuncInfo[ f_index ].FuncPtr;

    _FuncInfo[ f_index ].FuncInstr   = func._FuncInfo[ f_index ].FuncInstr;
    if( !_RefCountTable.IsDefined( _FuncInfo[ f_index ].FuncInstr ) )
    {
      _RefCountTable.Define( _FuncInfo[ f_index ].FuncInstr, 1 );
    }
    else
    {
      _RefCountTable( _FuncInfo[ f_index ].FuncInstr )++;
    }

    _FuncInfo[ f_index ].NumParams = func._FuncInfo[ f_index ].NumParams;
    _FuncInfo[ f_index ].ParamInfo =
      new ParamInfoType[ _FuncInfo[ f_index ].NumParams ];

    for( index = 0; index < _FuncInfo[ f_index ].NumParams; index ++ )
    {
      _FuncInfo[ f_index ].ParamInfo[ index ] = 
	func._FuncInfo[ f_index ].ParamInfo[ index ];

      if( _FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue != 0 )
      {
	_FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue = 
	  _FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue->ValCopy();
      }	
    }
  }
}


FuncDescObj::FuncDescObj( const gString& func_name )
     : _FuncName  ( func_name )
{ 
  _NumFuncs = 0;
  _FuncInfo = 0;
}


FuncDescObj::~FuncDescObj()
{
  int index;
  int f_index;
  Instruction* instruction;
  
  for( f_index = 0; f_index < _NumFuncs; f_index++ )
  {
    for( index = 0; index < _FuncInfo[ f_index ].NumParams; index ++ )
    {
      delete _FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue;
    }
    if( _FuncInfo[ f_index ].UserDefined )
    {
      assert( _RefCountTable.IsDefined( _FuncInfo[ f_index ].FuncInstr ) );
      assert( _RefCountTable( _FuncInfo[ f_index ].FuncInstr ) > 0 );
      _RefCountTable( _FuncInfo[ f_index ].FuncInstr )--;
      if( _RefCountTable( _FuncInfo[ f_index ].FuncInstr ) == 0 )
      {
	_RefCountTable.Remove( _FuncInfo[ f_index ].FuncInstr );
	assert( _FuncInfo[ f_index ].FuncInstr != 0 );
	while( _FuncInfo[ f_index ].FuncInstr->Length() > 0 )
	{
	  instruction = _FuncInfo[ f_index ].FuncInstr->Remove( 1 );
	  delete instruction;
	}
	delete _FuncInfo[ f_index ].FuncInstr;
      }
    }
    delete [] _FuncInfo[ f_index ].ParamInfo;
  }
  delete [] _FuncInfo;
}



void FuncDescObj::SetFuncInfo
(
 Portion*  (*func_ptr)(Portion**),
 const int  num_params,
 const ParamInfoType param_info[]
 )
{
  int i;
  int f_index = -1;

  for( i = 0; i < _NumFuncs; i++ )
  {
    if( !_FuncInfo[ i ].UserDefined && ( _FuncInfo[ i ].FuncPtr == func_ptr ) )
    {
      f_index = i;
      break;
    }
  }

  _SetFuncInfo( f_index, num_params );
  
  _FuncInfo[ _NumFuncs - 1 ].UserDefined = false;
  _FuncInfo[ _NumFuncs - 1 ].FuncPtr = func_ptr;

  if( param_info != 0 )
    SetParamInfo( func_ptr, param_info );
}


void FuncDescObj::SetFuncInfo
(
 gList< Instruction* >* func_instr,
 const int num_params,
 const ParamInfoType param_info[]
 )
{
  int i;
  int f_index = -1;

  for( i = 0; i < _NumFuncs; i++ )
  {
    if( _FuncInfo[ i ].UserDefined && (_FuncInfo[ i ].FuncInstr == func_instr))
    {
      f_index = i;
      break;
    }
  }

  _SetFuncInfo( f_index, num_params );
  
  _FuncInfo[ _NumFuncs - 1 ].UserDefined = true;
  _FuncInfo[ _NumFuncs - 1 ].FuncInstr = func_instr;

  if( !_RefCountTable.IsDefined( _FuncInfo[ _NumFuncs - 1 ].FuncInstr ) )
  {
    _RefCountTable.Define( _FuncInfo[ _NumFuncs - 1 ].FuncInstr, 1 );
  }
  else
  {
    _RefCountTable( _FuncInfo[ _NumFuncs - 1 ].FuncInstr )++;
  }

  if( param_info != 0 )
    SetParamInfo( func_instr, param_info );
}


void FuncDescObj::_SetFuncInfo( const int f_index, const int num_params )
{
  int i;
  FuncInfoType* NewFuncInfo;

  if( f_index == -1 )  // new function
  {
    _NumFuncs++;
    NewFuncInfo = new FuncInfoType[ _NumFuncs ];
    for( i = 0; i < _NumFuncs - 1; i++ )
    {
      NewFuncInfo[ i ] = _FuncInfo[ i ];
    }
    delete [] _FuncInfo;
    _FuncInfo = NewFuncInfo;

    _FuncInfo[ _NumFuncs - 1 ].UserDefined = false;
    _FuncInfo[ _NumFuncs - 1 ].FuncPtr   = 0;
    _FuncInfo[ _NumFuncs - 1 ].FuncInstr = 0;
    _FuncInfo[ _NumFuncs - 1 ].NumParams = num_params;
    _FuncInfo[ _NumFuncs - 1 ].ParamInfo = new ParamInfoType[ num_params ];
  }
#ifndef NDEBUG
  else // function already defined
  {
    gerr << "FuncDescObj Error:\n";
    gerr << "  Attempted to initialize the same function\n";
    gerr << "  multiple times as different overloaded\n";
    gerr << "  versions while initializing function " << _FuncName << "[]\n";
    assert( 0 );
  }  
#endif // NDEBUG
}


void FuncDescObj::SetParamInfo
( 
 Portion*           (*func_ptr)(Portion**),
 const int          param_index,
 const gString&     param_name, 
 const PortionType  param_type, 
 Portion*           param_default_value,
 const bool         param_pass_by_reference
 )
{
  int i;
  int f_index = -1;
  
  for( i = 0; i < _NumFuncs; i++ )
  {
    if( !_FuncInfo[ i ].UserDefined && ( _FuncInfo[ i ].FuncPtr == func_ptr ) )
    {
      f_index = i;
      break;
    }
  }

  _SetParamInfo
    ( 
     f_index,
     param_index,
     param_name, 
     param_type, 
     param_default_value,
     param_pass_by_reference
     );
}


void FuncDescObj::SetParamInfo
( 
 gList< Instruction* >* func_instr,
 const int              param_index,
 const gString&         param_name, 
 const PortionType      param_type, 
 Portion*               param_default_value,
 const bool             param_pass_by_reference
 )
{
  int i;
  int f_index = -1;
  
  for( i = 0; i < _NumFuncs; i++ )
  {
    if( _FuncInfo[ i ].UserDefined && (_FuncInfo[ i ].FuncInstr == func_instr))
    {
      f_index = i;
      break;
    }
  }

  _SetParamInfo
    ( 
     f_index,
     param_index,
     param_name, 
     param_type, 
     param_default_value,
     param_pass_by_reference
     );
}


void FuncDescObj::_SetParamInfo
(
 const int          f_index,
 const int          param_index,
 const gString&     param_name, 
 const PortionType  param_type, 
 Portion*           param_default_value,
 const bool         param_pass_by_reference
 )
{
  int index;
  int repeated_variable_declaration = false;
  
#ifndef NDEBUG
  if( !( param_index >= 0 && param_index < _FuncInfo[ f_index ].NumParams ) )
  {
    gerr << "FuncDescObj Error:\n";
    gerr << "  An invalid parameter index specified\n";
    gerr << "  for SetParamInfo( ... ) while initializing\n";
    gerr << "  the function " << _FuncName << "[] for\n";
    gerr << "  the parameter \"" << param_name << "\"\n";
    gerr << "  Index specified: " << param_index << "\n";
  }
  assert( param_index >= 0 && param_index < _FuncInfo[ f_index ].NumParams );

  if( f_index == -1 )
  {
    gerr << "FuncDescObj Error:\n";
    gerr << "  An undefined function pointer specified\n";
    gerr << "  for SetParamInfo( ... ) while initializing\n";
    gerr << "  the function " << _FuncName << "[]\n";
  }
  assert( f_index >= 0 && f_index < _NumFuncs );
#endif // NDEBUG

  for( index = 0; index < _FuncInfo[ f_index ].NumParams; index++ )
  {
    if( _FuncInfo[ f_index ].ParamInfo[ index ].Name == param_name )
    {
      repeated_variable_declaration = true;
      break;
    }
  }

#ifndef NDEBUG
  if( repeated_variable_declaration )
  {
    gerr << "FuncDescObj Error:\n";
    gerr << "  Multiple parameters of a functions were\n";
    gerr << "  declared with the same formal name \"" << param_name << "\"\n";
    gerr << "  during initialization\n";
  }
  assert( !repeated_variable_declaration );
  
  if( _FuncInfo[ f_index ].ParamInfo[ param_index ].Name != "" )
  {
    gerr << "FuncDescObj Error:\n";
    gerr << "  Parameter #" << param_index << " of a\n";
    gerr << "  function in " << _FuncName << "[] was\n";
    gerr << "  declared multiple times\n";
  }
  assert( _FuncInfo[ f_index ].ParamInfo[ param_index ].Name == "" );
#endif // NDEBUG
  
  _FuncInfo[ f_index ].ParamInfo[ param_index ].Name = 
    param_name;
  _FuncInfo[ f_index ].ParamInfo[ param_index ].Type = 
    param_type;
  _FuncInfo[ f_index ].ParamInfo[ param_index ].DefaultValue = 
    param_default_value;
  _FuncInfo[ f_index ].ParamInfo[ param_index ].PassByReference = 
    param_pass_by_reference;
}


void FuncDescObj::SetParamInfo
( 
 Portion*          (*func_ptr)(Portion**),
 const ParamInfoType     param_info[]
 )
{
  int i;
  int f_index = -1;

  for( i = 0; i < _NumFuncs; i++ )
  {
    if( !_FuncInfo[ i ].UserDefined && ( _FuncInfo[ i ].FuncPtr == func_ptr ) )
    {
      f_index = i;
      break;
    }
  }

 for( i = 0; i < _FuncInfo[ f_index ].NumParams; i++ )
 {
   _FuncInfo[ f_index ].ParamInfo[ i ] = ParamInfoType( param_info[ i ] );
 }
}


void FuncDescObj::SetParamInfo
( 
 gList< Instruction* >* func_instr,
 const ParamInfoType     param_info[]
 )
{
  int i;
  int f_index = -1;

  for( i = 0; i < _NumFuncs; i++ )
  {
    if( _FuncInfo[ i ].UserDefined && ( _FuncInfo[ i ].FuncInstr == func_instr ) )
    {
      f_index = i;
      break;
    }
  }

 for( i = 0; i < _FuncInfo[ f_index ].NumParams; i++ )
 {
   _FuncInfo[ f_index ].ParamInfo[ i ] = ParamInfoType( param_info[ i ] );
 }
}



bool FuncDescObj::Combine( FuncDescObj* newfunc )
{
  bool result = true;
  bool same_params;
  int i;
  int j;
  int f_index;
  int index;

  for( i = 0; i < newfunc->_NumFuncs; i++ )
  {
    for( f_index = 0; f_index < _NumFuncs; f_index++ )
    {
      same_params = true;
      for( index = 0; 
	  index < _FuncInfo[ f_index ].NumParams &&
	  index < newfunc->_FuncInfo[ i ].NumParams; 
	  index++ )
      {
	if(
	   ( _FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue == 0 && 
	    newfunc->_FuncInfo[ i ].ParamInfo[ index ].DefaultValue == 0 ) &&
	   ( _FuncInfo[ f_index ].ParamInfo[ index ].Name ==
	    newfunc->_FuncInfo[ i ].ParamInfo[ index ].Name ) &&
	   PortionTypeMatch( _FuncInfo[ f_index ].ParamInfo[ index ].Type,
			    newfunc->_FuncInfo[ i ].ParamInfo[ index ].Type ) 
	   )
	{
	  same_params = same_params & true;
	}
	else
	{
	  same_params = false;
	  break;
	}
      }
      
      if( same_params )
      {
	result = false;
	break;
      }
    }

    if( result )
    {
      if( newfunc->_FuncInfo[ i ].UserDefined )
      {
	SetFuncInfo( newfunc->_FuncInfo[ i ].FuncInstr, 
		    newfunc->_FuncInfo[ i ].NumParams,
		    newfunc->_FuncInfo[ i ].ParamInfo );
      }
      else
      {
	SetFuncInfo( newfunc->_FuncInfo[ i ].FuncPtr, 
		    newfunc->_FuncInfo[ i ].NumParams,
		    newfunc->_FuncInfo[ i ].ParamInfo );
      } 
    }
    
    for( j = 0; j < newfunc->_FuncInfo[ i ].NumParams; j++ )
    {
      newfunc->_FuncInfo[ i ].ParamInfo[ j ].DefaultValue = 0;
    }
    
    if( !result )
      break;
  }
  delete newfunc;
  return result;
}


gString FuncDescObj::FuncName( void ) const
{
  return _FuncName;
}



//---------------------------------------------------------------------
//                      CallFuncObj
//---------------------------------------------------------------------

  
CallFuncObj::CallFuncObj( FuncDescObj* func, gOutput& s_out, gOutput& s_err )
     :FuncDescObj( *func ), _StdOut( s_out ), _StdErr( s_err )
{
  int index;
  int f_index;

  _FuncIndex = -1;
  _NumParams = 0;
  _NumParamsDefined = 0;
  for( f_index = 0; f_index < _NumFuncs; f_index++ )
  {
    if( _NumParams < _FuncInfo[ f_index ].NumParams )
      _NumParams = _FuncInfo[ f_index ].NumParams;
  }

  _Param = new Portion* [ _NumParams ];
  _RunTimeParamInfo = new RunTimeParamInfoType [ _NumParams ];
  _CurrParamIndex = 0;
  _ErrorOccurred = false;
  
  for( index = 0; index < _NumParams; index++ )
  {
    _Param[ index ] = NO_DEFAULT_VALUE;
    _RunTimeParamInfo[ index ].Defined = false;
    _RunTimeParamInfo[ index ].AutoValOrRef = false;
    _RunTimeParamInfo[ index ].Ref = 0;
  }
}


CallFuncObj::~CallFuncObj()
{
  int index;
  if( _ErrorOccurred )
  {
    for( index = 0; index < _NumParams; index++ )
    {
      delete _RunTimeParamInfo[ index ].Ref;
      delete _Param[ index ];
    }
  }
  delete[] _RunTimeParamInfo;
  delete[] _Param;
}


int CallFuncObj::NumParams( void ) const
{
  return _NumParams;
}


bool CallFuncObj::_TypeMatch( Portion* p, PortionType ExpectedType ) const
{
  bool        result = false;
  PortionType CalledType;
  PortionType ExpectedListType;

  assert( p != 0 );
  CalledType = p->Type();

  if( (ExpectedType != porVALUE) && (ExpectedType & porLIST) )
  {
    if( CalledType == porLIST )
    {
      if( ExpectedType == porLIST )
      {
	result = true;
      }
      else
      {
	CalledType = ( (ListPortion*) p )->DataType();
	ExpectedListType = ExpectedType & ~porLIST;
	result = CalledType & ExpectedListType;
      }
    }
  }
  else // normal type checking
  {
    result = CalledType & ExpectedType;
  }
  return result;
}



int CallFuncObj::FindParamName( const gString& param_name )
{
  int f_index;
  int TempFuncIndex = -1;
  int index;
  int result = PARAM_NOT_FOUND;
  int times_found = 0;

  for( f_index = 0; 
      f_index < _NumFuncs && result != PARAM_AMBIGUOUS;
      f_index++ )
  {
    for( index = 0; index < _FuncInfo[ f_index ].NumParams; index++ )
    {
      if( _FuncInfo[ f_index ].ParamInfo[ index ].Name == param_name )
      {
	if( result == PARAM_NOT_FOUND )
	{
	  result = index;
	  TempFuncIndex = f_index;
	  times_found++;
	}
	else if( result == index )
	{
	  times_found++;
	}
	else // ( result != index )
	{
	  result = PARAM_AMBIGUOUS;
	  break;
	}
      }
    }
  }

  if( !_ErrorOccurred && times_found == 1 )
  {
    if( _FuncIndex == -1 )
    {
      _FuncIndex = TempFuncIndex;
    }
    else if( _FuncIndex != TempFuncIndex )
    {
      _ErrorMessage( _StdErr, 2, 0, _FuncName );
      _FuncIndex = -1;
      _ErrorOccurred = true;
    }
  }

  if( result == PARAM_NOT_FOUND )
    _ErrorMessage( _StdErr, 23, 0, _FuncName, param_name );
  else if( result == PARAM_AMBIGUOUS )
    _ErrorMessage( _StdErr, 24, 0, _FuncName, param_name );
  
  return result;
}


void CallFuncObj::SetErrorOccurred( void )
{
  _ErrorOccurred = true;
}


void CallFuncObj::SetCurrParamIndex( const int index )
{
  _CurrParamIndex = index;
}


bool CallFuncObj::SetCurrParam( Portion *param, bool auto_val_or_ref )
{
  ReferencePortion* ref_param = 0;
  

  // Parameter index out of bounds
  if( _CurrParamIndex >= _NumParams )
  {
    _ErrorMessage( _StdErr, 4, 0, _FuncName );
    delete param;
    _ErrorOccurred = true;
    return false;
  }

  // Repeated parameter defition
  if( _RunTimeParamInfo[ _CurrParamIndex ].Defined )
  {
    _ErrorMessage( _StdErr, 3, _CurrParamIndex, _FuncName );
    delete param;
    _ErrorOccurred = true;
    return false;
  }
  
  // An error had already occurred with the current function call
  if( _ErrorOccurred )
  {
    delete param;
    return true;
  }


  if( param != 0 && param->Type() == porREFERENCE )
  {
    ref_param = (ReferencePortion*) param;
    param = 0;
  }

  assert( _Param[ _CurrParamIndex ] == 0 );
  _Param[ _CurrParamIndex ] = param;
  _RunTimeParamInfo[ _CurrParamIndex ].Defined = true;
  _RunTimeParamInfo[ _CurrParamIndex ].AutoValOrRef = auto_val_or_ref;
  _RunTimeParamInfo[ _CurrParamIndex ].Ref = ref_param;
  _CurrParamIndex++;
  _NumParamsDefined++;
  
  return true;
}


ReferencePortion* CallFuncObj::GetCurrParamRef( void ) const
{
  return _RunTimeParamInfo[ _CurrParamIndex ].Ref;
}



Portion* CallFuncObj::CallFunction( GSM* gsm, Portion **param )
{
  int index;
  int f_index;
  int curr_f_index;
  int param_sets_matched;
  Portion* result = 0;

  int param_upper_bound;
  bool match_ok;



  // Attempt to identify the function being called out of all the
  // overloaded versions.

  if( _FuncIndex == -1 && _NumFuncs == 1 )
    _FuncIndex = 0;

  if( _FuncIndex == -1 )
  {
    param_upper_bound = 0;
    for( index = 0; index < _NumParams; index++ )
    {
      if( _Param[ index ] != 0 || _RunTimeParamInfo[ index ].Ref != 0 )
	param_upper_bound = index;
    }

    param_sets_matched = 0;
    for( f_index = 0; f_index < _NumFuncs; f_index++ )
    {
      match_ok = true;
      if( param_upper_bound >= _FuncInfo[ f_index ].NumParams )
	match_ok = false;

      for( index = 0; 
	  index < _FuncInfo[ f_index ].NumParams; 
	  index++ )
      {
	if( _Param[ index ] != 0 )
	{
	  // parameter is defined
	  if( !_TypeMatch( _Param[ index ], 
			  _FuncInfo[ f_index ].ParamInfo[ index ].Type ) )
	    match_ok = false;
	}
	else
	{
	  // parameter is undefined
	  if( _RunTimeParamInfo[ index ].Ref != 0 )
	  {
	    // specified undefined variable
	    if( !_FuncInfo[ f_index ].ParamInfo[ index ].PassByReference )
	      match_ok = false;
	  }

	  if( _FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue == 0 )
	    match_ok = false;
	}
      }

      if( match_ok )
      {
	curr_f_index = f_index;
	param_sets_matched++;
      }
    }

    if( param_sets_matched == 1 )
    {
      _FuncIndex = curr_f_index;
    }
    else 
    {
      if( param_sets_matched > 1 )
	_ErrorMessage( _StdErr, 5, 0, _FuncName );
      else
	_ErrorMessage( _StdErr, 8, 0, _FuncName );
      _ErrorOccurred = true;
    }
  }

  
  if( _FuncIndex != -1 )
  {
    for( index = 0; 
	index < _FuncInfo[ _FuncIndex ].NumParams; 
	index++ )
    {
      if( _Param[ index ] != 0 )
      {
	if( _Param[ index ]->Type() == porERROR )
	{
	  _ErrorMessage( _StdErr, 6, index, _FuncName,
			_FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	  _ErrorOccurred = true;
	}
	else if( !_TypeMatch( _Param[ index ], 
			     _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Type ))
	{
	  _ErrorMessage( _StdErr, 7, 0, _FuncName, 
			_FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	  _ErrorOccurred = true;
	}
      }
    }
  }



  // at this point _FuncIndex should be defined; i.e. the function
  // matching stage is done.  Now to weed out some particular errors:

  if( !_ErrorOccurred )
  {
    for( index = 0; index < _FuncInfo[ _FuncIndex ].NumParams; index++ )
    {
      if( !_RunTimeParamInfo[ index ].Defined )
      {
	assert( _Param[ index ] == 0 );
	if( _FuncInfo[ _FuncIndex ].ParamInfo[ index ].DefaultValue != 0 )
	{
	  _Param[ index ] = 
	    _FuncInfo[ _FuncIndex ].ParamInfo[ index ].DefaultValue->ValCopy();
	}
	else
	{
	  _ErrorMessage( _StdErr, 9, 0, _FuncName, 
			_FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	  _ErrorOccurred = true;
	}
      }
      else // ( _RunTimeParamInfo[ index ].Defined )
      {
	if( _Param[ index ] == 0 )
	{
	  if( _FuncInfo[ _FuncIndex ].ParamInfo[ index ].DefaultValue == 0 )
	  {
	    _ErrorMessage( _StdErr, 9, 0, _FuncName, 
			  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	    _ErrorOccurred = true;
	  }
	  else if( !_FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference)
	  {
	    _ErrorMessage( _StdErr, 10, 0, _FuncName, 
			  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	    _ErrorOccurred = true;
	  }
	  else if( _RunTimeParamInfo[ index ].Ref == 0 )
	  {
	    _ErrorMessage( _StdErr, 11, 0, _FuncName, 
			  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	    _ErrorOccurred = true;
	  }
	  else if( _RunTimeParamInfo[ index ].Ref != 0 )
	  {
	    _Param[ index ] = 
	      _FuncInfo[ _FuncIndex ].ParamInfo[ index ].DefaultValue->
		ValCopy();
	  }
	}
      }
    }
  }



  // aborts when a value paremeter is passed by reference or
  //        when a reference parameter is passed by value

  if( !_ErrorOccurred )
  {
    for( index = 0; index < _FuncInfo[ _FuncIndex ].NumParams; index++ )
    {
      if( _Param[ index ] != 0 &&
	 _RunTimeParamInfo[ index ].Defined )
      {
	if( !_FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference &&
	   _Param[ index ]->IsReference() )
	{
	  if( _RunTimeParamInfo[ index ].AutoValOrRef )
	  {
	    Portion* old = _Param[ index ];
	    _Param[ index ] = old->ValCopy();
	    delete old;
	  }
	  else
	  {
	    _ErrorMessage( _StdErr, 12, 0, _FuncName, 
			  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	    _ErrorOccurred = true;
	  }
	}
	else if( _FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference &&
		!_Param[ index ]->IsReference() && 
		_RunTimeParamInfo[ index ].Ref == 0 )
	{
	  _ErrorMessage( _StdErr, 13, 0, _FuncName, 
			_FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	  _ErrorOccurred = true;
	}
      }
    }
  }




  // This section makes the actual function call

  if( !_ErrorOccurred )
  {
    if( !_FuncInfo[ _FuncIndex ].UserDefined )
      result = _FuncInfo[ _FuncIndex ].FuncPtr( _Param );
    else
      result = gsm->ExecuteUserFunc( *(_FuncInfo[ _FuncIndex ].FuncInstr), 
				    _FuncInfo[ _FuncIndex ], _Param );

    if( result == 0 )
    {
      _ErrorMessage( _StdErr, 20, 0, _FuncName );
      _ErrorOccurred = true;
    }
    else if( result->Type() == porERROR )
    {
      result->Output( _StdErr );
      delete result;
      result = 0;
      _ErrorOccurred = true;
    }
  }



  
  if( !_ErrorOccurred )
  {
    for( index = 0; index < _NumParams; index++ )
    {
      if( !_FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference )
      {
	delete _RunTimeParamInfo[ index ].Ref;
	_RunTimeParamInfo[ index ].Ref = 0;
      }
      if( _RunTimeParamInfo[ index ].Ref == 0 )
      {
	delete _Param[ index ];
	_Param[ index ] = 0;
      }
    }
  }
  else
  {
    for( index = 0; index < _NumParams; index++ )
    {
      delete _Param[ index ];
      _Param[ index ] = 0;
      delete _RunTimeParamInfo[ index ].Ref;
      _RunTimeParamInfo[ index ].Ref = 0;
    }
  }


  for( index = 0; index < _NumParams; index++ )
  {
    param[ index ] = _Param[ index ];
  }    

  return result;
}






void CallFuncObj::_ErrorMessage
( 
 gOutput& s,
 const int error_num, 
 const long& num1,
 const gString& str1,
 const gString& str2
 )
{
#if 0
  s << "CallFuncObj Error " << error_num << " : \n";
#endif // 0
  
  switch( error_num )
  {
  case 2:
    s << str1 << "[] called with conflicting parameters\n";
    break;
  case 3:
    s << str1 << "[] called with parameter #" << num1 << "multiply defined\n";
    break;
  case 4:
    s << str1 << "[] called with too many parameters\n";
    break;
  case 5:
    s << str1 << "[] called with ambiguous parameter(s)\n";
    break;
  case 6:
    s << str1 << "[] parameter \"" << str2 << "\" undefined\n";
    break;
  case 7:
    s << str1 << "[] parameter \"" << str2 << "\" type mismatch\n";
    break;
  case 8:
    s << "No matching parameter specifications found for " + str1 + "[]\n";
    break;
  case 9:
    s << str1 << "[] required parameter \"" << str2 << "\" missing\n";
    break;
  case 10:
    s << str1 << "[] value parameter \"" << str2 << "\" undefined\n";
    break;
  case 11:
    s << str1 << "[] undefined parameter \"" << str2 << "\" passed by value\n";
    break;
  case 12:
    s << str1 << "[] value parameter \"" << str2 << "\" passed by reference\n";
    break;
  case 13:
    s << str1 << "[] reference parameter \"" << str2 << "\" passed by value\n";
    break;
  case 20:
    s << "A general error occurred while executing " << str1 << "[]\n";
    break;
  case 23:
    s << "Parameter \"" << str1 << "\" is not defined for " << str2 << "[]\n";
    break;
  case 24:
    s << "Parameter \"" << str1 << "\" is ambiguous in "<< str2 << "[]\n";
    break;
  default:
    s << "General error\n";
  }
}

 

