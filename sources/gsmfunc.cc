//#
//# FILE: gsmfunc.cc -- handles initialization of defined functions for GSM
//#                     companion to GSM
//#
//# $Id$
//#


#include <assert.h>
#include "gsmfunc.h"



// The header files for each module should be placed here:

#include "gsmoper.h"

#include "gclmath.h"



// This function is called once at the first instance of GSM.
// The Init function of each module should be placed in this function:
// Each Init() function should take the argument "this" so each instance
// of GSM can add those functions to their memory.

extern void Init_nfgfunc(GSM *);
extern void Init_efgfunc(GSM *);

void GSM::InitFunctions( void )
{
  Init_gsmoper( this );

  Init_gclmath( this );

  Init_nfgfunc( this );
  Init_efgfunc( this );
}





/*******************************************************************/
//                   Function descriptor objects
/*******************************************************************/

   
FuncDescObj::FuncDescObj( FuncDescObj& func )
{
  int index;
  int f_index;

  _FuncName  = func._FuncName;
  _NumFuncs  = func._NumFuncs;
  _FuncInfo  = new FuncInfoType[ _NumFuncs ];

  for( f_index = 0; f_index < _NumFuncs; f_index++ )
  {
    _FuncInfo[ f_index ].FuncPtr   = func._FuncInfo[ f_index ].FuncPtr;
    _FuncInfo[ f_index ].NumParams = func._FuncInfo[ f_index ].NumParams;
    _FuncInfo[ f_index ].ParamInfo =
      new ParamInfoType[ _FuncInfo[ f_index ].NumParams ];

    for( index = 0; index < _FuncInfo[ f_index ].NumParams; index ++ )
    {
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

  for( f_index = 0; f_index < _NumFuncs; f_index++ )
  {
    for( index = 0; index < _FuncInfo[ f_index ].NumParams; index ++ )
    {
      delete _FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue;
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
  FuncInfoType* NewFuncInfo;

  for( i = 0; i < _NumFuncs; i++ )
  {
    if( _FuncInfo[ i ].FuncPtr == func_ptr )
    {
      f_index = i;
      break;
    }
  }

  if( f_index = -1 )  // new function
  {
    _NumFuncs++;
    NewFuncInfo = new FuncInfoType[ _NumFuncs ];
    for( i = 0; i < _NumFuncs - 1; i++ )
    {
      NewFuncInfo[ i ] = _FuncInfo[ i ];
    }
    delete [] _FuncInfo;
    _FuncInfo = NewFuncInfo;

    _FuncInfo[ _NumFuncs - 1 ].FuncPtr = func_ptr;
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
  int index;
  int repeated_variable_declaration = false;
  int i;
  int f_index = -1;
  
  for( i = 0; i < _NumFuncs; i++ )
  {
    if( _FuncInfo[ i ].FuncPtr == func_ptr )
    {
      f_index = i;
      break;
    }
  }

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
  _FuncInfo[ f_index ].ParamInfo[ param_index ].PassByReference = 
    param_pass_by_reference;
}


gString FuncDescObj::FuncName( void ) const
{
  return _FuncName;
}



/*******************************************************************/
//                      CallFuncObj
/*******************************************************************/

  
CallFuncObj::CallFuncObj( FuncDescObj* func )
     :FuncDescObj( *func )
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
      gerr << "CallFuncObj Error: conflicting variable name specification\n";
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


void CallFuncObj::SetCurrParamRef( Reference_Portion* ref )
{
  assert( _CurrParamIndex < _NumParams );
  _RunTimeParamInfo[ _CurrParamIndex ].Ref = ref;
}


bool CallFuncObj::SetCurrParam( Portion *param )
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
	else if( param->Type() & 
		_FuncInfo[ _FuncIndex ].ParamInfo[ _CurrParamIndex ].Type )
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
	  _CurrParamIndex++;
	  _NumParamsDefined++;
	}
	else
	{
	  gerr << "CallFuncObj Error: parameter type mismatch\n";
	  result = false;
	}
      }
      else
      {
	gerr << "CalFuncObj Error: multiple definitions found for parameter ";
	gerr << "\"" << _FuncInfo[_FuncIndex].ParamInfo[_CurrParamIndex].Name;
	gerr << "\"\n";
	gerr << "                  while executing CallFunction() on\n";
	gerr << "                  function \"" << _FuncName << "\" )\n";
	result = false;
      }
    }
    else // ( _CurrParamIndex >= _NumParams )
    {
      gerr << "CallFuncObj Error: too many parameters specified for\n";
      gerr << "                   function \"" << _FuncName << "\"\n";
      result = false;
    }
  }
  else
  {
    if( param != 0 )
    {
      delete param;
    }
  }

  if( result == false )
  {
    _ErrorOccurred = true;
    delete param;
    delete _RunTimeParamInfo[ _CurrParamIndex ].Ref;
  }
  return result;
}


Reference_Portion* CallFuncObj::GetParamRef( const int index ) const
{
  return _RunTimeParamInfo[ index ].Ref;
}


int CallFuncObj::GetCurrParamIndex( void ) const
{
  return _CurrParamIndex;
}


PortionType CallFuncObj::GetCurrParamType( void ) const
{
  if( _CurrParamIndex < _NumParams )
  {
    return _FuncInfo[ _FuncIndex ].ParamInfo[ _CurrParamIndex ].Type;
  }
  else // ( _CurrParamIndex >= _NumParams )
  {
    return porERROR;
  }
}


bool CallFuncObj::GetCurrParamPassByRef( void ) const
{
  assert( _FuncIndex >= 0 && _FuncIndex < _NumFuncs );
  assert( _CurrParamIndex >= 0 && 
	 _CurrParamIndex < _FuncInfo[ _FuncIndex ].NumParams );
  return _FuncInfo[ _FuncIndex ].ParamInfo[ _CurrParamIndex ].PassByReference;
}


Reference_Portion* CallFuncObj::GetCurrParamRef( void ) const
{
  return _RunTimeParamInfo[ _CurrParamIndex ].Ref;
}


Portion* CallFuncObj::GetCurrParamShadowOf( void ) const
{
  return _RunTimeParamInfo[ _CurrParamIndex ].ShadowOf;
}


Portion* CallFuncObj::CallFunction( Portion **param )
{
  int index;
  int f_index;
  int curr_f_index;
  int params_defined;
  int params_matched;
  int param_sets_matched;
  Portion* result = 0;



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
	    if( _Param[ index ]->Type() & 
	       _FuncInfo[ f_index ].ParamInfo[ index ].Type )
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
	gerr << "CallFuncObj Error: function \"" << _FuncName << "\" called";
	gerr << " with ambiguous parameters\n";
	_ErrorOccurred = true;
      }
    }
    else // ( !params_defined )
    {
      gerr << "CallFuncObj Error: no defined parameters; function call\n";
      gerr << "                   ambiguous for function \"" << _FuncName;
      gerr << "\"\n";
      _ErrorOccurred = true;
    }
  }
 
  if( !_ErrorOccurred )
  { 
    if( _FuncIndex < 0 || _FuncIndex >= _NumFuncs )
    {
      gerr << "CallFuncObj Error: no matching parameter list found for\n";
      gerr << "                   function \"" + _FuncName + "\"\n";
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
	  gerr << "GSM Error: required parameter \"" << 
	    _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name;
	  gerr << "\"" << " not found while executing CallFunction()\n";
	  gerr << "           on function \"" << _FuncName << "\"\n";
	  _ErrorOccurred = true;
	}
      }
      else // ( _RunTimeParamInfo[ index ].Defined )
      {
	if( _Param[ index ] == 0 )
	{
	  if( !_FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference )
	  {
	    gerr << "GSM Error: required parameter \"" << 
	      _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name;
	    gerr << "\"" << " undefined while executing CallFunction()\n";
	    gerr << "           on function \"" << _FuncName << "\"\n";
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
    result = _FuncInfo[ _FuncIndex ].FuncPtr( _Param );

    if( result == 0 )
      _ErrorOccurred = true;
    else if( result->Type() == porERROR )
    {
      result->Output( gerr );
      gerr << "\n";
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
