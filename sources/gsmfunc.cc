//#
//# FILE: gsmfunc.cc -- handles initialization of defined functions for GSM
//#                     companion to GSM
//#
//# $Id$
//#



// Usage:
//
// For each module of functions to be added to GSM, create an 
// 'void Init_<name>( GSM* gsm )' function that calls GSM::AddFunction()
// to add the desired functions into GSM, where <name> is the name of the 
// particular module.  Then create a header file for that module, and include
// it in this file.  Put the Init_<name>() function call in the 
// GSM::InitFunctions() in this file.
//
// The definition for GSM::AddFunction is:
//   GSM::AddFunction( FuncDescObj *func_desc_obj );
//
// FuncDescObj is a class which stores the characteristics of each
// particular function, such as the number of parameters and the type of
// each parameter.  GSM will pass the function parameters in the form of
// an array of pointers to Portion class, so all parameter types should
// be decendents of Portion class.  Each function is expected to have
// a return type of 'Portion *'.
//
// The FuncDescObj is created by passing its constructor with the function
// name, a pointer to the actual implementation of the function, and the 
// number of parameters.  Then the type of each parameter can be specified 
// with FuncDescObj::SetParamInfo() function, as shown in the example.  It 
// is important to define the types for all parameters.  The valid parameter 
// types are listed in portion.h, typedef PortionType.
//
// Example:
//   void Init_myfunc( GSM* gsm )
//   {
//     FuncDescObj *FuncObj;
//
//     FuncObj = new FuncDescObj( (gString) "Sqr", GCL_Sqr, 1 );
//     FuncObj->SetParamInfo( 0, "n", porNUMERICAL, NO_DEFAULT_VALUE );
//     gsm->AddFunction( FuncObj );
//
//     FuncObj = new FuncDescObj( (gString) "Angle", GCL_Angle, 2 );
//     FuncObj->SetParamInfo( 0, "x", porDOUBLE, new numerical_Portion<double>( 1 ) );
//     FuncObj->SetParamInfo( 1, "y", porDOUBLE, new numerical_Portion<double>( 1 ) );
//     gsm->AddFunction( FuncObj );
//   }
//
// Specifying a parameter type as porNUMERICAL means that the parameter
// can be any of porDOUBLE, porINTEGER, or porRATIONAL types.  When a
// parameter can accept multiple parameter types, those type declarations
// can be ORed together with the bit-wise operater |. It is then up
// to the function implementation to explicitly type cast those parameters.
//
// The current implementation mandates that if a parameter has a default
// value, then that parameter can only accept a value with the same type
// as the default value; otherwise the results are unpredictable.
//
// If a function returns 0 (null pointer), GSM will halt and give a general
// error message.  So it may be a good idea to initialize the return value
// to 0 until all necessary processing has been completed.
//
// All the header files should #include "gsm.h".  The definitions for 
// Portion types are already included in "gsm.h".
//
// None of the parameters passed to a function should be deleted, and the
// return value of each function should be newly allocated in the function.
//
// Take a look at gclmath.h and gclmath.cc to see how it works.


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

void GSM::InitFunctions( void )
{
  Init_gsmoper( this );

  Init_gclmath( this );

  Init_nfgfunc( this );
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
	gerr << "\"\n while executing CallFunction() on\n";
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
  bool param_match;
  bool any_defined;
  Portion* result = 0;

  if( _FuncIndex == -1 )
  {
    any_defined = false;
    for( index = 0; index < _NumFuncs; index++ )
    {
      if( _Param[ index ] != 0 )
      {
	any_defined = true;
	break;
      }
    }
    if( any_defined )
    {
      for( f_index = 0; f_index < _NumFuncs; f_index++ )
      {
	param_match = true;
	for( index = 0; 
	    index < _FuncInfo[ f_index ].NumParams; 
	    index++ )
	{
	  if( _Param[ index ] != 0 )
	    if( !( _Param[ index ]->Type() & 
	       _FuncInfo[ f_index ].ParamInfo[ index ].Type ) )
	      param_match = false;
	}
	if( param_match )
	{
	  _FuncIndex = f_index;
	  break;
	}
      }
    }
    else // ( !any_defined )
    {
      if( _NumFuncs == 1 )
      {
	_FuncIndex = 0;
      }
      else
      {
	gerr << "CallFuncObj Error: no defined parameters; function call\n";
	gerr << "                   ambiguous\n";
	_ErrorOccurred = true;
      }
    }
  }
  
  if( _FuncIndex < 0 || _FuncIndex >= _NumFuncs )
  {
    gerr << "CallFuncObj Error: no matching parameter list found for\n";
    gerr << "                   function \"" + _FuncName + "\"\n";
    _ErrorOccurred = true;
  }
  
  if( !_ErrorOccurred )
  {
    for( index = 0; index < _NumParams; index++ )
    {
      if( !_RunTimeParamInfo[ index ].Defined )
      {
	assert( _Param[ index ] == 0 );
	_Param[ index ] = 
	  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].DefaultValue->Copy();
      }
      if( ( !_RunTimeParamInfo[ index ].Defined ) && ( _Param[ index ] == 0 ) )
      {
	gerr << "GSM Error: required parameter \"" << 
	  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name;
	gerr << "\"" << " not found while executing CallFunction()\n";
	gerr << "           on function \"" << _FuncName << "\"\n";
	_ErrorOccurred = true;
      }
    }
  }

  if( !_ErrorOccurred )
  {
    result = _FuncInfo[ _FuncIndex ].FuncPtr( _Param );
    if( result == 0 )
      _ErrorOccurred = true;
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
