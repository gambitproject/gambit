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
// Every function is expected to delete all of its pointer parameters, except
// in cases when one of the pointers is used as the return value.
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

void GSM::InitFunctions( void )
{
  Init_gsmoper( this );

  Init_gclmath( this );
}





/*******************************************************************/
//                   Function descriptor objects
/*******************************************************************/

   
FuncDescObj::FuncDescObj( FuncDescObj& func )
{
  int index;

#ifndef NDEBUG
  if( func._NumParams < 0 )
  {
    gerr << "FuncDescObj Error: an invalid or corrupted FuncDescObj passed\n";
    gerr << "                   as initializer to the constructor\n";
  }
  assert( func._NumParams >= 0 );
#endif // NDEBUG

  _FuncName  = func._FuncName;
  _FuncPtr   = func._FuncPtr;
  _NumParams = func._NumParams;
  _ParamInfo = new ParamInfoType[ _NumParams ];
  for( index = 0; index < _NumParams; index++ )
  {
    _ParamInfo[ index ].Name = func._ParamInfo[ index ].Name;
    _ParamInfo[ index ].Type = func._ParamInfo[ index ].Type;
    if( func._ParamInfo[ index ].DefaultValue == NO_DEFAULT_VALUE )
    {
      _ParamInfo[ index ].DefaultValue = NO_DEFAULT_VALUE;
    }
    else
    {
      _ParamInfo[ index ].DefaultValue = 
	func._ParamInfo[ index ].DefaultValue->Copy();
    }
    _ParamInfo[ index ].PassByReference = func._ParamInfo[ index ].PassByReference;
  }
}


FuncDescObj::FuncDescObj
  (
   const gString&  func_name,
   Portion*        (*func_ptr)(Portion**), 
   const int       num_params
   )
   : _FuncName  ( func_name ), 
     _FuncPtr   ( func_ptr ), 
     _NumParams ( num_params )
{
#ifndef NDEBUG
  if( _NumParams < 0 )
  {
    gerr << "FuncDescObj Error: invalid number of parameters specified for\n";
    gerr << "                   the function \"" << _FuncName << "\"\n";
  }
  assert( _NumParams >= 0 );
#endif

  _ParamInfo = new ParamInfoType[ _NumParams ];
}


FuncDescObj::~FuncDescObj()
{
  int index;

  for( index = 0; index < _NumParams; index++ )
  {
    delete _ParamInfo[ index ].DefaultValue;
  }
  delete[] _ParamInfo;
}


void FuncDescObj::SetParamInfo
  ( 
   const int          param_index,
   const gString&     param_name, 
   const PortionType  param_type, 
   Portion*           param_default_value,
   bool               param_pass_by_reference
   )
{
  int index;
  int repeated_variable_declaration = false;

#ifndef NDEBUG
  if( !( param_index >= 0 && param_index < _NumParams ) )
  {
    gerr << "FuncDescObj Error: an invalid parameter index specified\n";
    gerr << "                   for SetParamInfo( ... ) while initializing\n";
    gerr << "                   the function \"" << _FuncName << "\" for\n";
    gerr << "                   the parameter \"" << param_name << "\"\n";
  }
  assert( param_index >= 0 && param_index < _NumParams );
#endif // NDEBUG

  for( index = 0; index < _NumParams; index++ )
  {
    if( _ParamInfo[ index ].Name == param_name )
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
  
  _ParamInfo[ param_index ].Name            = param_name;
  _ParamInfo[ param_index ].Type            = param_type;
  _ParamInfo[ param_index ].DefaultValue    = param_default_value;
  _ParamInfo[ param_index ].PassByReference = param_pass_by_reference;
}


gString FuncDescObj::FuncName( void ) const
{
  return _FuncName;
}


int FuncDescObj::NumParams( void ) const
{
  return _NumParams;
}


gString FuncDescObj::ParamName( const int index ) const
{
#ifndef NDEBUG
  if( !( index >= 0 && index < _NumParams ) )
  {
    gerr << "FuncDescObj Error: an invalid parameter index specified for\n";
    gerr << "                   the function \"" << _FuncName << "\" when\n";
    gerr << "                   calling FuncDescObj::ParamName()\n";
  }
  assert( index >= 0 && index < _NumParams );
#endif // NDEBUG

  return _ParamInfo[ index ].Name;
}


PortionType FuncDescObj::ParamType( const int index ) const
{
#ifndef NDEBUG
  if( !( index >= 0 && index < _NumParams ) )
  {
    gerr << "FuncDescObj Error: an invalid parameter index specified for\n";
    gerr << "                   the function \"" << _FuncName << "\" when\n";
    gerr << "                   calling FuncDescObj::ParamType()\n";
  }
  assert( index >= 0 && index < _NumParams );
#endif // NDEBUG

  return _ParamInfo[ index ].Type;
}


Portion* FuncDescObj::ParamDefaultValue( const int index ) const
{
#ifndef NDEBUG
  if( !( index >= 0 && index < _NumParams ) )
  {
    gerr << "FuncDescObj Error: an invalid parameter index specified for\n";
    gerr << "                   the function \"" << _FuncName << "\" when\n";
    gerr << "                   calling FuncDescObj::ParamDefaultValue()\n";
  }
  assert( index >= 0 && index < _NumParams );
#endif // NDEBUG

  if( _ParamInfo[ index ].DefaultValue == NO_DEFAULT_VALUE )
  {
    return NO_DEFAULT_VALUE;
  }
  else
  {
    return _ParamInfo[ index ].DefaultValue->Copy();
  }
}


bool FuncDescObj::ParamPassByReference( const int index ) const
{
  return _ParamInfo[ index ].PassByReference;
}


int FuncDescObj::FindParamName( const gString& param_name ) const
{
  int index;
  int result = PARAM_NOT_FOUND;

  for( index = 0; index < _NumParams; index++ )
  {
    if( _ParamInfo[ index ].Name == param_name )
    {
      result = index;
      break;
    }
  }
  return result;
}



/*******************************************************************/
//                      CallFuncObj
/*******************************************************************/

  
CallFuncObj::CallFuncObj( FuncDescObj* func )
     :FuncDescObj( *func )
{
  int index;

  _Param = new Portion* [ _NumParams ];
  _RunTimeParamInfo = new RunTimeParamInfoType [ _NumParams ];
  _CurrParamIndex = 0;
  
  for( index = 0; index < _NumParams; index++ )
  {
    if( _ParamInfo[ index ].DefaultValue == NO_DEFAULT_VALUE )
    {
      _Param[ index ] = NO_DEFAULT_VALUE;
    }
    else
    {
      _Param[ index ] = _ParamInfo[ index ].DefaultValue->Copy();
    }
    _RunTimeParamInfo[ index ].Defined = false;
  }
}


CallFuncObj::~CallFuncObj()
{
  delete[] _RunTimeParamInfo;
  delete[] _Param;
}


void CallFuncObj::SetCurrParamIndex( const int index )
{
  _CurrParamIndex = index;
}

void CallFuncObj::SetCurrParamRefName( const gString& ref_name )
{
  assert( _CurrParamIndex < _NumParams );
  _RunTimeParamInfo[ _CurrParamIndex ].RefName = ref_name;
}


bool CallFuncObj::SetCurrParam( Portion *param )
{
  bool result = true;

  if( _CurrParamIndex < _NumParams )
  {
    if( !_RunTimeParamInfo[ _CurrParamIndex ].Defined )
    {
      if( _Param[ _CurrParamIndex ] != NO_DEFAULT_VALUE )
      {
	delete _Param[ _CurrParamIndex ];
      }
      _Param[ _CurrParamIndex ] = param;
      _RunTimeParamInfo[ _CurrParamIndex ].Defined = true;
      _CurrParamIndex++;
    }
    else
    {
      gerr << "CalFuncObj Error: multiple definitions found for parameter ";
      gerr << "\"" << _ParamInfo[ _CurrParamIndex ].Name;
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
  
  return result;
}


gString& CallFuncObj::GetParamRefName( const int index ) const
{
  return _RunTimeParamInfo[ index ].RefName;
}


int CallFuncObj::GetCurrParamIndex( void ) const
{
  return _CurrParamIndex;
}


PortionType CallFuncObj::GetCurrParamType( void ) const
{
  if( _CurrParamIndex < _NumParams )
  {
    return _ParamInfo[ _CurrParamIndex ].Type;
  }
  else // ( _CurrParamIndex >= _NumParams )
  {
    return porERROR;
  }
}


bool CallFuncObj::GetCurrParamPassByReference( void ) const
{
  return _ParamInfo[ _CurrParamIndex ].PassByReference;
}


gString& CallFuncObj::GetCurrParamRefName( void ) const
{
  return _RunTimeParamInfo[ _CurrParamIndex ].RefName;
}


Portion* CallFuncObj::CallFunction( Portion **param )
{
  int index;
  Portion* result = 0;

  for( index = 0; index < _NumParams; index++ )
  {
    if( ( !_RunTimeParamInfo[ index ].Defined ) && ( _Param[ index ] == 0 ) )
    {
      gerr << "GSM Error: required parameter \"" << _ParamInfo[ index ].Name;
      gerr << "\"" << " not found while executing CallFunction()\n";
      gerr << "           on function \"" << _FuncName << "\"\n";
      return 0;
    }
  }

  result = _FuncPtr( _Param );

  for( index = 0; index < _NumParams; index++ )
  {
    param[ index ] = _Param[ index ];
  }

  return result;
}
