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

void GSM::InitFunctions( void )
{
  Init_gclmath( this );
  Init_gsmoper( this );

  Init_nfgfunc( this );
  Init_efgfunc( this );
}





/*******************************************************************/
//                   Function descriptor objects
/*******************************************************************/

RefCountHashTable< gList< Instruction* >* > FuncDescObj::_RefCountTable;

   
FuncDescObj::FuncDescObj( FuncDescObj& func )
{
  int index;
  int f_index;
  int i;

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

    _FuncInfo[ f_index ].NumParams   = func._FuncInfo[ f_index ].NumParams;
    _FuncInfo[ f_index ].ParamInfo =
      new ParamInfoType[ _FuncInfo[ f_index ].NumParams ];

    for( index = 0; index < _FuncInfo[ f_index ].NumParams; index ++ )
    {
      /*
      _FuncInfo[ f_index ].ParamInfo[ index ] =
	ParamInfoType( func._FuncInfo[ f_index ].ParamInfo[ index ] );
	*/
      _FuncInfo[ f_index ].ParamInfo[ index ].Name =
	func._FuncInfo[ f_index ].ParamInfo[ index ].Name;
      _FuncInfo[ f_index ].ParamInfo[ index ].Type =
	func._FuncInfo[ f_index ].ParamInfo[ index ].Type;
      if( func._FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue == 
	 NO_DEFAULT_VALUE )
      {
	_FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue = 
	  NO_DEFAULT_VALUE;
      }
      else
      {
	_FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue = 
	  func._FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue->Copy();
      }
      _FuncInfo[ f_index ].ParamInfo[ index ].PassByReference =
	func._FuncInfo[ f_index ].ParamInfo[ index ].PassByReference;
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
 const int  num_params
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
}


void FuncDescObj::SetFuncInfo
(
 gList< Instruction* >* func_instr,
 const int num_params
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
    gerr << "FuncDescObj Error: attempted to initialize the same function\n";
    gerr << "                   multiple times as different overloaded\n";
    gerr << "                   version while initializing function\n";
    gerr << "                   \"" << _FuncName << "\"\n";
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
  int i;
  
#ifndef NDEBUG
  if( !( param_index >= 0 && param_index < _FuncInfo[ f_index ].NumParams ) )
  {
    gerr << "FuncDescObj Error: an invalid parameter index specified\n";
    gerr << "                   for SetParamInfo( ... ) while initializing\n";
    gerr << "                   the function \"" << _FuncName << "\" for\n";
    gerr << "                   the parameter \"" << param_name << "\"\n";
    gerr << "                   Index specified: " << param_index << "\n";
  }
  assert( param_index >= 0 && param_index < _FuncInfo[ f_index ].NumParams );

  if( f_index == -1 )
  {
    gerr << "FuncDescObj Error: an undefined function pointer specified\n";
    gerr << "                   for SetParamInfo( ... ) while initializing\n";
    gerr << "                   the function \"" << _FuncName << "\" for\n";
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
    gerr << "FuncDescObj Error: multiple parameters of a functions were\n";
    gerr << "                   declared with the same formal name ";
    gerr << "\"" << param_name << "\"\n";
    gerr << "                   during initialization\n";
  }
  assert( !repeated_variable_declaration );
#endif // NDEBUG
  
  _FuncInfo[ f_index ].ParamInfo[ param_index ].Name = 
    param_name;
  _FuncInfo[ f_index ].ParamInfo[ param_index ].Type = 
    param_type;
  _FuncInfo[ f_index ].ParamInfo[ param_index ].DefaultValue = 
    param_default_value;
/*
  if( _FuncInfo[ f_index ].ParamInfo[ param_index ].DefaultValue != 0 )
    _FuncInfo[ f_index ].ParamInfo[ param_index ].DefaultValue = 
      _FuncInfo[ f_index ].ParamInfo[ param_index ].DefaultValue->Copy();
*/
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


gString FuncDescObj::FuncName( void ) const
{
  return _FuncName;
}



/*******************************************************************/
//                      CallFuncObj
/*******************************************************************/

  
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
    _RunTimeParamInfo[ index ].Ref = 0;
    _RunTimeParamInfo[ index ].ShadowOf = 0;
  }
}


CallFuncObj::~CallFuncObj()
{
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
      CalledType = ( (List_Portion*) p )->DataType();
      ExpectedListType = ExpectedType & ~porLIST;
      result = CalledType & ExpectedListType;
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
	  result == PARAM_AMBIGUOUS;
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
      _ErrorMessage( _StdErr, 1 );
      _FuncIndex = -1;
      _ErrorOccurred = true;
    }
  }
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


bool CallFuncObj::SetCurrParam( Portion *param, Reference_Portion* ref_param )
{
  bool result = true;
  bool type_match;

  if( !_ErrorOccurred )
  {
    if( _CurrParamIndex < _NumParams )
    {
      if( !_RunTimeParamInfo[ _CurrParamIndex ].Defined )
      {
	type_match = false;
	if( param == 0 || _FuncIndex == -1 )
	  type_match = true;
	else 
	  if( _TypeMatch( param, _FuncInfo[ _FuncIndex ].ParamInfo[ _CurrParamIndex ].Type ) )
	    type_match = true;

	if( type_match )
	{
	  if( _Param[ _CurrParamIndex ] != NO_DEFAULT_VALUE )
	  {
	    delete _Param[ _CurrParamIndex ];
	  }
	  _Param[ _CurrParamIndex ] = param;
	  _RunTimeParamInfo[ _CurrParamIndex ].Defined = true;
	  if( param )
	    _RunTimeParamInfo[ _CurrParamIndex ].ShadowOf = param->ShadowOf();
	  _RunTimeParamInfo[ _CurrParamIndex ].Ref = ref_param;
	  _CurrParamIndex++;
	  _NumParamsDefined++;
	}
	else
	{
	  _ErrorMessage( _StdErr, 2, _CurrParamIndex, _FuncName );
	  result = false;
	}
      }
      else
      {
	_ErrorMessage( _StdErr, 3, 0, 
		      _FuncInfo[_FuncIndex].ParamInfo[_CurrParamIndex].Name,
		      _FuncName );
	result = false;
      }
    }
    else // ( _CurrParamIndex >= _NumParams )
    {
      _ErrorMessage( _StdErr, 4, 0, _FuncName );
      result = false;
    }
  }
  else
  {
    delete param;
    delete ref_param;
  }

  if( result == false )
  {
    _ErrorOccurred = true;
    delete param;
    delete ref_param;
  }
  return result;
}


Reference_Portion* CallFuncObj::GetCurrParamRef( void ) const
{
  return _RunTimeParamInfo[ _CurrParamIndex ].Ref;
}


Portion* CallFuncObj::GetCurrParamShadowOf( void ) const
{
  return _RunTimeParamInfo[ _CurrParamIndex ].ShadowOf;
}


Portion* CallFuncObj::CallFunction( GSM* gsm, Portion **param )
{
  int index;
  int f_index;
  int curr_f_index;
  int params_defined;
  int params_matched;
  int param_sets_matched;
  Portion* result = 0;
  GSM_ReturnCode return_code;


  if( _FuncIndex == -1 && _NumFuncs == 1 )
    _FuncIndex = 0;

  if( _FuncIndex == -1 )
  {
    params_defined = 0;
    for( index = 0; index < _NumParams; index++ )
    {
      if( _Param[ index ] != 0 )
      {
	params_defined++;
      }
    }
    if( params_defined )
    {
      param_sets_matched = 0;
      for( f_index = 0; f_index < _NumFuncs; f_index++ )
      {
	params_matched = 0;
	for( index = 0; 
	    index < _FuncInfo[ f_index ].NumParams; 
	    index++ )
	{
	  if( _Param[ index ] != 0 )
	  {
	    if( _TypeMatch( _Param[ index ], 
			   _FuncInfo[ f_index ].ParamInfo[ index ].Type ) )
	      params_matched++;
	  }
	}
	if( params_matched == params_defined )
	{
	  curr_f_index = f_index;
	  param_sets_matched++;
	}
      }

      if( param_sets_matched == 1 )
      {
	_FuncIndex = curr_f_index;
      }
      if ( param_sets_matched > 1 )
      {
	_ErrorMessage( _StdErr, 5, 0, _FuncName );
	_ErrorOccurred = true;
      }
    }
    else // ( !params_defined )
    {
      _ErrorMessage( _StdErr, 6, 0, _FuncName );
      _ErrorOccurred = true;
    }
  }
  else // ( _FuncIndex != -1 )
  {
    for( index = 0; 
	index < _FuncInfo[ _FuncIndex ].NumParams; 
	index++ )
    {
      if( _Param[ index ] != 0 )
      {
	if( !_TypeMatch( _Param[ index ], 
			_FuncInfo[ _FuncIndex ].ParamInfo[ index ].Type ) )
	{
	  _ErrorMessage( _StdErr, 7, index, _FuncName );
	  _ErrorOccurred = true;
	}	  
      }
    }
  }

 
  if( !_ErrorOccurred )
  { 
    if( _FuncIndex < 0 || _FuncIndex >= _NumFuncs )
    {
      _ErrorMessage( _StdErr, 8, 0, _FuncName );
      _ErrorOccurred = true;
    }
  }
  
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
	    _FuncInfo[ _FuncIndex ].ParamInfo[ index ].DefaultValue->Copy();
	}
	else
	{
	  _ErrorMessage( _StdErr, 9, 0, 
			_FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name,
			_FuncName );
	  _ErrorOccurred = true;
	}
      }
      else // ( _RunTimeParamInfo[ index ].Defined )
      {
	if( _Param[ index ] == 0 )
	{
	  if( !_FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference )
	  {
	    _ErrorMessage( _StdErr, 10, 0,
			  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name,
			  _FuncName );
	    _ErrorOccurred = true;
	  }
	  else if( _RunTimeParamInfo[ index ].Ref != 0 )
	  {
	    if( _FuncInfo[ _FuncIndex ].ParamInfo[ index ].DefaultValue != 0 )
	    {
	      _Param[ index ] = 
		_FuncInfo[ _FuncIndex ].ParamInfo[ index ].DefaultValue->
		  Copy();
	    }	  
	  }
	}
      }
    }
  }

  /* This section makes the actual fuunction call */
  if( !_ErrorOccurred )
  {
    if( !_FuncInfo[ _FuncIndex ].UserDefined )
      result = _FuncInfo[ _FuncIndex ].FuncPtr( _Param );
    else
    {
      return_code = gsm->Execute( *(_FuncInfo[_FuncIndex].FuncInstr), true );
      if( return_code == rcSUCCESS )
	result = new bool_Portion( true );
      else
      {
	result = 0;
	_ErrorOccurred = true;
      }
    }

    if( result == 0 )
      _ErrorOccurred = true;
    else if( result->Type() == porERROR )
    {
      result->Output( _StdErr );
      _StdErr << "\n";
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
	delete _Param[ index ];
	_Param[ index ] = 0;
	delete _RunTimeParamInfo[ index ].Ref;
	_RunTimeParamInfo[ index ].Ref = 0;
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


bool CallFuncObj::ParamPassByReference( const int index ) const
{
  if( _FuncIndex >= 0 && _FuncIndex < _NumFuncs )
  {
    if( index >= 0 && index < _FuncInfo[ _FuncIndex ].NumParams )
      return _FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference;
  }

  return false;
}





void CallFuncObj::_ErrorMessage
( 
 gOutput& s,
 const int error_num, 
 const gInteger& num1,
 const gString& str1,
 const gString& str2
 )
{
  s << "CallFuncObj Error " << error_num << " : \n";
  switch( error_num )
  {
  case 1:
    s << "  Conflicting variable name specification\n";
    break;
  case 2:
    s << "  Parameter #" << num1 << " type mismatch while calling\n";
    s << "  function \"" << str1 << "\"\n";
    break;
  case 3:
    s << "  Multiple definitions found for parameter \"" <<  str1 << "\"\n";
    s << "  while executing CallFunction() on\n";
    s << "  function \"" << str2 << "\"\n";
    break;
  case 4:
    s << "  Too many parameters specified for\n";
    s << "  function \"" << str1 << "\"\n";
    break;
  case 5:
    s << "  Function \"" << str1 << "\" called with ambiguous parameters\n";
    break;
  case 6:
    s << "  No defined parameters;\n";
    s << "  function call ambiguous for function \"" << str1 << "\"\n";
    break;
  case 7:
    s << "  Parameter #" << num1 << " type mismatch while calling\n";
    s << "  function \"" << str1 << "\"\n";
    break;
  case 8:
    s << "  No matching parameter list found for\n";
    s << "  function \"" + str1 + "\"\n";
    break;
  case 9:
    s << "  Required parameter \"" << str1 << "\" not found while executing\n";
    s << "  CallFunction() on function \"" << str2 << "\"\n";
    break;
  case 10:
    s << "  Required parameter \"" << str1 << "\" undefined while executing\n";
    s << "  CallFunction() on function \"" << str2 << "\"\n";
    break;
  default:
    s << "  General error\n";
  }
}

 
