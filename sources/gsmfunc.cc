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
#include "rational.h"


// This function is called once at the first instance of GSM.
// The Init function of each module should be placed in this function:
// Each Init() function should take the argument "this" so each instance
// of GSM can add those functions to their memory.

extern void Init_gsmoper(GSM *);
extern void Init_nfgfunc(GSM *);
extern void Init_efgfunc(GSM *);
extern void Init_algfunc(GSM *);
extern void Init_solfunc(GSM *);
extern void Init_listfunc(GSM *);
extern void Init_listmath(GSM *);

void GSM::InitFunctions( void )
{
  Init_gsmoper( this );

  Init_listfunc(this);
  //  Init_listmath(this);

  Init_nfgfunc(this);
  Init_efgfunc(this);
  Init_algfunc(this);
  Init_solfunc(this);
}





//----------------------------- List stuff --------------------------

bool CallFuncObj::_ListDimMatch(ListPortion* p1, ListPortion* p2)
{
  int i;
  bool result = true;
  Portion* s1;
  Portion* s2;

  if(p1->Length() != p2->Length())
    result = false;
  else
    for(i = 1; i <= p1->Length(); i++)
    {
      s1 = (*p1)[i];
      s2 = (*p2)[i];
      if(s1->Spec().ListDepth > 0 && s2->Spec().ListDepth > 0)
	if(!_ListDimMatch((ListPortion*) s1, (ListPortion*) s2))
	  result = false;
    }
  return result;
}


bool CallFuncObj::_ListNestedCheck(Portion* p, const ParamInfoType& info)
{
  bool result = false;
  if(p == 0 && info.Spec.Type == porUNDEFINED)
    result = false;
  else 
  {
    assert(p);
    if(p->Spec().ListDepth > info.Spec.ListDepth)
      result = true;
  }
  return result;
}


Portion* CallFuncObj::
CallListFunction( GSM* gsm, Portion** ParamIn )
{
  int i;
  int j;
  ListPortion* p = 0;
  Portion* result;
  int NumParams = _FuncInfo[_FuncIndex].NumParams;
  Portion** CurrParam;
  bool Listed[NumParams];
  ListPortion* Source = 0;  // source to look at to create dimensionality
  bool recurse;

  // mark down the listed parameters
  for( i = 0; i < NumParams; i++ )
  {
    Listed[i] =
      _ListNestedCheck( ParamIn[i], _FuncInfo[_FuncIndex].ParamInfo[i] );
    if(Listed[i])
      Source = (ListPortion*) ParamIn[i];
  }

  // check that all dimensionalities match
  for( i = 0; i < NumParams; i++ )
  {
    for( j = i + 1; j < NumParams; j++ )
    {
      if( Listed[i] && Listed[j] )
	if( ParamIn[i]->Spec().ListDepth > 0 && 
	   ParamIn[j]->Spec().ListDepth > 0)
	  if(!_ListDimMatch((ListPortion*)ParamIn[i],(ListPortion*)ParamIn[j]))
	  {
	    gout << "NumParams: " << NumParams << " i: " << i << " j: " << j << '\n';
	    gout << "i: " << ((ListPortion*)ParamIn[i])->Length() << '\n';
	    gout << "j: " << ((ListPortion*)ParamIn[j])->Length() << '\n';
	    gout << "i: " << PortionSpecToText(ParamIn[i]->Spec()) << '\n';
	    gout << "j: " << PortionSpecToText(ParamIn[j]->Spec()) << '\n';
	    gout << "i: " << ParamIn[i] << '\n';
	    gout << "j: " << ParamIn[j] << '\n';
	    return new ErrorPortion( "Mismatched dimensionalities" );
	  }
    }
  }


  CurrParam = new Portion*[NumParams];
  p = new ListValPortion();

  assert( Source != 0 );

  // i is now the index in the lists
  for( i = 1; i <= Source->Length(); i++ )
  {
    recurse = false;
    // while j is the param index
    // pickout the current element to process
    
    for( j = 0; j < NumParams; j++ )
    {
      if( !Listed[j] )
	CurrParam[j] = ParamIn[j];
      else
      {
	CurrParam[j] = (*(ListPortion*) ParamIn[j])[i];
	if( _ListNestedCheck( CurrParam[j], 
			     _FuncInfo[_FuncIndex].ParamInfo[j] ) )
	  recurse = true;
      }
    }


    if( recurse )
    {
      result = CallListFunction( gsm, CurrParam );
    }
    else
    {
      for( j = 0; j < NumParams; j++ )
        CurrParam[j] = CurrParam[j]->RefCopy();

      if( !_FuncInfo[ _FuncIndex ].UserDefined )
	result = _FuncInfo[ _FuncIndex ].FuncPtr( CurrParam );
      else
	result = gsm->ExecuteUserFunc( *(_FuncInfo[ _FuncIndex ].FuncInstr), 
				      _FuncInfo[ _FuncIndex ], CurrParam );

      for( j = 0; j < NumParams; j++ )
	delete CurrParam[j];
    }

    p->Append( result );
  }

  delete[] CurrParam;
  return p;
}




//---------------------------------------------------------------
//                      ParamInfoType
//---------------------------------------------------------------


ParamInfoType::ParamInfoType( void )
{
  Name = "";
  Spec = PortionSpec(porERROR);
  DefaultValue = NO_DEFAULT_VALUE;
  PassByReference = 0;
}

ParamInfoType::ParamInfoType( const ParamInfoType& param_info )
:
 Name( param_info.Name ),
 Spec( param_info.Spec ),
 DefaultValue( param_info.DefaultValue ),
 PassByReference( param_info.PassByReference )
{ }

ParamInfoType::ParamInfoType
( 
 const gString& name,
 const PortionSpec& spec,
 Portion* default_value, 
 const bool pass_by_ref
 )
:
 Name( name ), 
 Spec( spec ), 
 DefaultValue( default_value ), 
 PassByReference( pass_by_ref )
{ }

ParamInfoType::~ParamInfoType()
{ }

ParamInfoType& ParamInfoType::operator = ( const ParamInfoType& param_info )
{
  Name = param_info.Name;
  Spec = param_info.Spec;
  DefaultValue = param_info.DefaultValue;
  PassByReference = param_info.PassByReference;
  return *this;
}



//---------------------------------------------------------------------
//                   Function descriptor objects
//---------------------------------------------------------------------

RefCountHashTable< gList< NewInstr* >* > FuncDescObj::_RefCountTable;


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
    _FuncInfo[ f_index ].Listable    = func._FuncInfo[ f_index ].Listable;
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
  NewInstr* NewInstr;
  
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
	  NewInstr = _FuncInfo[ f_index ].FuncInstr->Remove( 1 );
	  delete NewInstr;
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
 const ParamInfoType param_info[],
 const bool listable
 )
{
  int i;
  int f_index = -1;

  for( i = 0; i < _NumFuncs; i++ )
    if( !_FuncInfo[ i ].UserDefined && ( _FuncInfo[ i ].FuncPtr == func_ptr ) )
    {
      f_index = i;
      break;
    }

  _SetFuncInfo( f_index, num_params, listable );
  
  _FuncInfo[ _NumFuncs - 1 ].UserDefined = false;
  _FuncInfo[ _NumFuncs - 1 ].FuncPtr = func_ptr;

  if( param_info != 0 )
    SetParamInfo( func_ptr, param_info );
}


void FuncDescObj::SetFuncInfo
(
 gList< NewInstr* >* func_instr,
 const int num_params,
 const ParamInfoType param_info[],
 const bool listable
 )
{
  int i;
  int f_index = -1;

  for( i = 0; i < _NumFuncs; i++ )
    if( _FuncInfo[ i ].UserDefined && (_FuncInfo[ i ].FuncInstr == func_instr))
    {
      f_index = i;
      break;
    }

  _SetFuncInfo( f_index, num_params, listable );
  
  _FuncInfo[ _NumFuncs - 1 ].UserDefined = true;
  _FuncInfo[ _NumFuncs - 1 ].FuncInstr = func_instr;

  if( !_RefCountTable.IsDefined( _FuncInfo[ _NumFuncs - 1 ].FuncInstr ) )
    _RefCountTable.Define( _FuncInfo[ _NumFuncs - 1 ].FuncInstr, 1 );
  else
    _RefCountTable( _FuncInfo[ _NumFuncs - 1 ].FuncInstr )++;

  if( param_info != 0 )
    SetParamInfo( func_instr, param_info );
}


void FuncDescObj::
_SetFuncInfo( const int f_index, const int num_params, const bool listable )
{
  int i;
  FuncInfoType* NewFuncInfo;

  if( f_index == -1 )  // new function
  {
    _NumFuncs++;
    NewFuncInfo = new FuncInfoType[ _NumFuncs ];
    for( i = 0; i < _NumFuncs - 1; i++ )
      NewFuncInfo[ i ] = _FuncInfo[ i ];
    delete [] _FuncInfo;
    _FuncInfo = NewFuncInfo;

    _FuncInfo[ _NumFuncs - 1 ].FuncPtr   = 0;
    _FuncInfo[ _NumFuncs - 1 ].FuncInstr = 0;
    _FuncInfo[ _NumFuncs - 1 ].UserDefined = false;
    _FuncInfo[ _NumFuncs - 1 ].Listable = listable;
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
 const PortionSpec  param_spec, 
 Portion*           param_default_value,
 const bool         param_pass_by_reference
 )
{
  int i;
  int f_index = -1;
  
  for( i = 0; i < _NumFuncs; i++ )
    if( !_FuncInfo[ i ].UserDefined && ( _FuncInfo[ i ].FuncPtr == func_ptr ) )
    {
      f_index = i;
      break;
    }

  _SetParamInfo
    ( 
     f_index,
     param_index,
     param_name, 
     param_spec, 
     param_default_value,
     param_pass_by_reference
     );
}


void FuncDescObj::SetParamInfo
( 
 gList< NewInstr* >* func_instr,
 const int              param_index,
 const gString&         param_name, 
 const PortionSpec      param_spec, 
 Portion*               param_default_value,
 const bool             param_pass_by_reference
 )
{
  int i;
  int f_index = -1;
  
  for( i = 0; i < _NumFuncs; i++ )
    if( _FuncInfo[ i ].UserDefined && (_FuncInfo[ i ].FuncInstr == func_instr))
    {
      f_index = i;
      break;
    }

  _SetParamInfo
    ( 
     f_index,
     param_index,
     param_name, 
     param_spec, 
     param_default_value,
     param_pass_by_reference
     );
}


void FuncDescObj::_SetParamInfo
(
 const int          f_index,
 const int          param_index,
 const gString&     param_name, 
 const PortionSpec  param_spec, 
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


  for( index = 0; index < _FuncInfo[ f_index ].NumParams; index++ )
    if( _FuncInfo[ f_index ].ParamInfo[ index ].Name == param_name )
    {
      repeated_variable_declaration = true;
      break;
    }

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
  _FuncInfo[ f_index ].ParamInfo[ param_index ].Spec = 
    param_spec;
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
    if( !_FuncInfo[ i ].UserDefined && ( _FuncInfo[ i ].FuncPtr == func_ptr ) )
    {
      f_index = i;
      break;
    }

#ifndef NDEBUG
  if( f_index == -1 )
  {
    gerr << "FuncDescObj Error:\n";
    gerr << "  An undefined function pointer specified\n";
    gerr << "  for SetParamInfo( ... ) while initializing\n";
    gerr << "  the function " << _FuncName << "[]\n";
  }
  assert( f_index >= 0 && f_index < _NumFuncs );
#endif // NDEBUG

  for( i = 0; i < _FuncInfo[ f_index ].NumParams; i++ )
    _FuncInfo[ f_index ].ParamInfo[ i ] = ParamInfoType( param_info[ i ] );
}


void FuncDescObj::SetParamInfo
( 
 gList< NewInstr* >* func_instr,
 const ParamInfoType     param_info[]
 )
{
  int i;
  int f_index = -1;

  for(i=0; i<_NumFuncs; i++)
    if(_FuncInfo[i].UserDefined && (_FuncInfo[i].FuncInstr == func_instr))
    {
      f_index = i;
      break;
    }

#ifndef NDEBUG
  if( f_index == -1 )
  {
    gerr << "FuncDescObj Error:\n";
    gerr << "  An undefined function pointer specified\n";
    gerr << "  for SetParamInfo( ... ) while initializing\n";
    gerr << "  the function " << _FuncName << "[]\n";
  }
  assert( f_index >= 0 && f_index < _NumFuncs );
#endif // NDEBUG

  for( i = 0; i < _FuncInfo[ f_index ].NumParams; i++ )
    _FuncInfo[ f_index ].ParamInfo[ i ] = param_info[ i ];
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
	  (index < _FuncInfo[ f_index ].NumParams) &&
	  (index < newfunc->_FuncInfo[ i ].NumParams); 
	  index++ )
      {
	if((_FuncInfo[ f_index ].ParamInfo[ index ].DefaultValue == 0 && 
	    newfunc->_FuncInfo[ i ].ParamInfo[ index ].DefaultValue == 0) &&
	   (_FuncInfo[ f_index ].ParamInfo[ index ].Name ==
	    newfunc->_FuncInfo[ i ].ParamInfo[ index ].Name) &&
	   (_FuncInfo[ f_index ].ParamInfo[ index ].Spec ==
	    newfunc->_FuncInfo[ i ].ParamInfo[ index ].Spec))
	  same_params = same_params & true;
	else
	{
	  same_params = false;
	  break;
	}
      }
      
      if(same_params)
      {
	result = false;
	break;
      }
    }

    if( result )
      if( newfunc->_FuncInfo[ i ].UserDefined )
	SetFuncInfo( newfunc->_FuncInfo[ i ].FuncInstr, 
		    newfunc->_FuncInfo[ i ].NumParams,
		    newfunc->_FuncInfo[ i ].ParamInfo );
      else
	SetFuncInfo( newfunc->_FuncInfo[ i ].FuncPtr, 
		    newfunc->_FuncInfo[ i ].NumParams,
		    newfunc->_FuncInfo[ i ].ParamInfo );
    
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
{ return _FuncName; }


gList<gString> FuncDescObj::FuncList(void) const
{
  gList<gString> list;
  gString f;
  int i;
  int j;

  for(i = 0; i < _NumFuncs; i++)
  {  
    f = _FuncName + '[';
    for(j = 0; j < _FuncInfo[i].NumParams; j++)
    {
      if(j != 0) f += ", ";
      if(_FuncInfo[i].ParamInfo[j].DefaultValue) f += '{';
      f += _FuncInfo[i].ParamInfo[j].Name;
      if(_FuncInfo[i].ParamInfo[j].PassByReference) f += '<';
      f += "->";
      if(_FuncInfo[i].ParamInfo[j].DefaultValue)
      {
	switch(_FuncInfo[i].ParamInfo[j].DefaultValue->Spec().Type)
	{
	case porBOOL:
	  if(((BoolPortion*) _FuncInfo[i].ParamInfo[j].DefaultValue)->Value())
	    f += "True";
	  else
	    f += "False";
	  break;
	case porINTEGER:
	  f += ToString(((IntPortion*) 
			 _FuncInfo[i].ParamInfo[j].DefaultValue)->Value());
	  break;
	case porFLOAT:
	  f += ToString(((FloatPortion*) 
			 _FuncInfo[i].ParamInfo[j].DefaultValue)->Value());
	  break;
	case porRATIONAL:
	  f += ToString(((RationalPortion*) 
			 _FuncInfo[i].ParamInfo[j].DefaultValue)->Value());
	  break;
	case porTEXT:
	  f += ((TextPortion*)_FuncInfo[i].ParamInfo[j].DefaultValue)->Value();
	  break;
	case porINPUT:
	  f += "INPUT";
	  break;
	case porOUTPUT:
	  f += "OUTPUT";
	  break;
	default:
	  f += "N/A";
	}
      }
      else
	f += PortionSpecToText(_FuncInfo[i].ParamInfo[j].Spec);

      if(_FuncInfo[i].ParamInfo[j].DefaultValue) 
	f += '}';
    }
    f += ']';
    list.Append(f);
  }
  return list;
}

void FuncDescObj::Dump(gOutput& f, int i) const
{ f << FuncList()[i] << '\n'; }

void FuncDescObj::Dump(gOutput& f) const
{
  int i;
  for(i = 0; i < _NumFuncs; i++)
    Dump(f, i);
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

  _Param = new Portion*[ _NumParams ];
  _RunTimeParamInfo = new RunTimeParamInfoType[ _NumParams ];
  _FuncMatch = new bool[ _NumFuncs ];
  _CurrParamIndex = 0;
  _ErrorOccurred = false;

  for( f_index = 0; f_index < _NumFuncs; f_index++ )
    _FuncMatch[ f_index ] = true;
  for( index = 0; index < _NumParams; index++ )
  {
    _Param[ index ] = NO_DEFAULT_VALUE;
    _RunTimeParamInfo[ index ].Ref = 0;
    _RunTimeParamInfo[ index ].Defined = false;
    _RunTimeParamInfo[ index ].AutoValOrRef = false;
  }
}


CallFuncObj::~CallFuncObj()
{
  int index;
  if( _ErrorOccurred )
    for( index = 0; index < _NumParams; index++ )
    {
      delete _RunTimeParamInfo[ index ].Ref;
      delete _Param[ index ];
    }
  delete[] _FuncMatch;
  delete[] _RunTimeParamInfo;
  delete[] _Param;
}


int CallFuncObj::NumParams( void ) const
{ return _NumParams; }


bool CallFuncObj::
_TypeMatch( Portion* p, PortionSpec ExpectedSpec, bool Listable )
{
  bool        result = false;
  PortionSpec CalledSpec;


  if( p == 0 && ExpectedSpec.Type == porUNDEFINED )
    return true;
  
  assert(p != 0);
  CalledSpec = p->Spec();
  
  if(CalledSpec == ExpectedSpec)
    result = true;
  else if((CalledSpec.Type & ExpectedSpec.Type) &&
	  (CalledSpec.ListDepth == ExpectedSpec.ListDepth))
    result = true;
  else if((CalledSpec.Type & ExpectedSpec.Type) &&
	  (!Listable) &&
	  (CalledSpec.ListDepth>0) &&
	  (ExpectedSpec.ListDepth>0))
    result = true;
  else if((CalledSpec.ListDepth > ExpectedSpec.ListDepth))
    if((CalledSpec.Type & ExpectedSpec.Type) && Listable)
      result = true;
    else if(ExpectedSpec.ListDepth == 0 && CalledSpec.Type == porUNDEFINED)
    {
      result = true;
      ( (ListPortion*) p )->SetDataType( ExpectedSpec.Type );
    }
  return result;
}



bool CallFuncObj::SetCurrParamIndex( const gString& param_name )
{
  int f_index;
  int TempFuncIndex = -1;
  int index;
  int result = PARAM_NOT_FOUND;
  int times_found = 0;
  bool name_match_found;

  
  if( _ErrorOccurred )
    return true;


  for( f_index = 0; 
      f_index < _NumFuncs && result != PARAM_AMBIGUOUS;
      f_index++ )
  {
    if(_FuncMatch[f_index])
    {
      name_match_found = false;
      for( index = 0; index < _FuncInfo[ f_index ].NumParams; index++ )
      {
	if( _FuncInfo[ f_index ].ParamInfo[ index ].Name == param_name )
	{
	  name_match_found = true;
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
      if( !name_match_found )
	_FuncMatch[ f_index ] = false;
    }
  }
  
  
  if( times_found == 1 )
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
      return false;
    }
  }

  if( result == PARAM_NOT_FOUND )
  {
    _ErrorOccurred = true;
    _ErrorMessage( _StdErr, 23, 0, _FuncName, param_name );
    return false;
  }
  else if( result == PARAM_AMBIGUOUS )
  {
    _ErrorOccurred = true;
    _ErrorMessage( _StdErr, 24, 0, _FuncName, param_name );
    return false;
  }
  else
  {
    _CurrParamIndex = result;
    return true;
  }
}


void CallFuncObj::SetErrorOccurred( void )
{
  _ErrorOccurred = true;
}



bool CallFuncObj::SetCurrParam( Portion *param, bool auto_val_or_ref )
{
  ReferencePortion* ref_param = 0;
  bool AllowUndefinedRef;
  int f_index;
  int funcs_matched;
  int last_match;


  // An error had already occurred with the current function call
  if( _ErrorOccurred )
  {
    delete param;
    return true;
  }

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
    _ErrorMessage( _StdErr, 3, _CurrParamIndex + 1, _FuncName, 
		  _ParamName( _CurrParamIndex ) );
    delete param;
    _ErrorOccurred = true;
    return false;
  }

  // Passed an undefined variable
  if( param != 0 && param->Spec().Type == porREFERENCE )
  {

    // check whether undefined variables are allowed
    AllowUndefinedRef = false;
    for( f_index = 0; f_index < _NumFuncs; f_index++ )
    {
      if( _CurrParamIndex < _FuncInfo[ f_index ].NumParams )
      {
	if((_FuncInfo[f_index].ParamInfo[_CurrParamIndex].PassByReference &&
	    _FuncInfo[f_index].ParamInfo[_CurrParamIndex].DefaultValue != 0)|| 
	   _FuncInfo[f_index].UserDefined ||
	   _FuncInfo[f_index].ParamInfo[_CurrParamIndex].Spec.Type == porUNDEFINED )
	  AllowUndefinedRef = true;
	else
	  _FuncMatch[ f_index ] = false;
      }
    }

    if( AllowUndefinedRef )
    {
      ref_param = (ReferencePortion*) param;
      param = 0;
    }
    else
    {
      _ErrorMessage( _StdErr, 25, _CurrParamIndex + 1, _FuncName, 
		    _ParamName( _CurrParamIndex ),
		    ( (ReferencePortion*) param )->Value() );
      delete param;
      _ErrorOccurred = true;
      return false;
    }
  }

  

  funcs_matched = 0;
  last_match = 0;
  for( f_index = 0; f_index < _NumFuncs; f_index++ )
  {
    if( _FuncMatch[ f_index ] )
    {
      last_match = f_index;
      funcs_matched++;
    }
  }
  if( funcs_matched == 1 )
    _FuncIndex = last_match;


  if( param != 0 )
  {
    if( _FuncIndex == -1 )
    {
      // Attempt on-the-fly function matching
      for( f_index = 0; f_index < _NumFuncs; f_index++ )
      {
	if( _CurrParamIndex < _FuncInfo[ f_index ].NumParams )
	{
	  if( !_TypeMatch
	     ( param, 
	      _FuncInfo[ f_index ].ParamInfo[_CurrParamIndex].Spec,
	      _FuncInfo[ f_index ].Listable ) )
	  {
	    _FuncMatch[ f_index ] = false;
	  }
	}
	else
	{
	  _FuncMatch[ f_index ] = false;
	}
      }
    }
    else // ( _FuncIndex != -1 )
    {
      if( _CurrParamIndex < _FuncInfo[ _FuncIndex ].NumParams )
      {
	if( !_TypeMatch
	   ( param, 
	    _FuncInfo[ _FuncIndex ].ParamInfo[_CurrParamIndex].Spec,
	    _FuncInfo[ _FuncIndex ].Listable ) )
	{
	  _ErrorMessage( _StdErr, 26, _CurrParamIndex + 1, _FuncName,
			_FuncInfo[_FuncIndex].ParamInfo[_CurrParamIndex].Name,
			PortionSpecToText( _FuncInfo[ _FuncIndex ].
					  ParamInfo[_CurrParamIndex].Spec ),
			PortionSpecToText( param->Spec() ) );
	  _ErrorOccurred = true;
	  delete param;
	  return false;
	}
      }
      else
      {
	_ErrorMessage( _StdErr, 4, 0, _FuncName );
	_ErrorOccurred = true;
	delete param;
	return false;
      }
    }
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


ReferencePortion* CallFuncObj::GetParamRef( int index ) const
{
  return _RunTimeParamInfo[ index ].Ref;
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

      if( !_FuncMatch[ f_index ] )
	match_ok = false;

      for( index = 0;
	  index < _FuncInfo[ f_index ].NumParams;
	  index++ )
      {
	if( _Param[ index ] != 0 )
	{
	  // parameter is defined
	  if( !_TypeMatch
	     ( _Param[ index ],
	      _FuncInfo[ f_index ].ParamInfo[ index ].Spec,
	      _FuncInfo[ f_index ].Listable ) )
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
	if( _Param[ index ]->Spec().Type == porERROR )
	{
	  _ErrorMessage( _StdErr, 6, index + 1, _FuncName,
			_FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	  _ErrorOccurred = true;
	}
	else if( !_TypeMatch
		( _Param[ index ], 
		 _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Spec,
		 _FuncInfo[ _FuncIndex ].Listable ) )
	{
	  _ErrorMessage( _StdErr, 7, index + 1, _FuncName, 
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
	  _ErrorMessage( _StdErr, 9, index + 1, _FuncName, 
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
	    if( _FuncInfo[ _FuncIndex ].UserDefined )
	    {
	      // default values for undefined variables
	      switch( _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Spec.Type )
	      {
	      case porBOOL:
		_Param[ index ] = new BoolValPortion( false );
		_RunTimeParamInfo[ index ].Defined = true;
		break;
	      case porINTEGER:
		_Param[ index ] = new IntValPortion( 0 );
		_RunTimeParamInfo[ index ].Defined = true;
		break;
	      case porFLOAT:
		_Param[ index ] = new FloatValPortion( 0 );
		_RunTimeParamInfo[ index ].Defined = true;
		break;
	      case porRATIONAL:
		_Param[ index ] = new RationalValPortion( 0 );
		_RunTimeParamInfo[ index ].Defined = true;
		break;
	      case porTEXT:
		_Param[ index ] = new TextValPortion( "" );
		_RunTimeParamInfo[ index ].Defined = true;
		break;
	      default:
		_ErrorMessage( _StdErr, 9, index + 1, _FuncName, 
			      _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name);
		_ErrorOccurred = true;
	      }
	    }
	    else if( _FuncInfo[_FuncIndex].ParamInfo[index].Spec.Type != 
		    porUNDEFINED )
	    {
	      _ErrorMessage( _StdErr, 9, index + 1, _FuncName, 
			    _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	      _ErrorOccurred = true;
	    }
	  }
	  else if( !_FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference)
	  {
	    _ErrorMessage( _StdErr, 10, index + 1, _FuncName, 
			  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	    _ErrorOccurred = true;
	  }
	  else if( _RunTimeParamInfo[ index ].Ref == 0 )
	  {
	    _ErrorMessage( _StdErr, 11, index + 1, _FuncName, 
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
	    _ErrorMessage( _StdErr, 12, index + 1, _FuncName, 
			  _FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	    _ErrorOccurred = true;
	  }
	}
	/*
	else if( _FuncInfo[ _FuncIndex ].ParamInfo[ index ].PassByReference &&
		!_Param[ index ]->IsReference() && 
		_RunTimeParamInfo[ index ].Ref == 0 )
	{
	  _ErrorMessage( _StdErr, 13, index + 1, _FuncName, 
			_FuncInfo[ _FuncIndex ].ParamInfo[ index ].Name );
	  _ErrorOccurred = true;
	}
	*/
      }
    }
  }





  // This section makes the actual function call

  bool list_op;
  int i;
  list_op = false;
  if( !_ErrorOccurred )
  {
    for( i = 0; i < _FuncInfo[_FuncIndex].NumParams; i++ )
      if( _ListNestedCheck( _Param[i], _FuncInfo[_FuncIndex].ParamInfo[i] ) )
      {
	list_op = true;
	break;
      }

    if( !list_op || !_FuncInfo[_FuncIndex].Listable )
    {
      if( !_FuncInfo[ _FuncIndex ].UserDefined )
	result = _FuncInfo[ _FuncIndex ].FuncPtr( _Param );
      else
	result = gsm->ExecuteUserFunc( *(_FuncInfo[ _FuncIndex ].FuncInstr), 
				      _FuncInfo[ _FuncIndex ], _Param );
    }
    else
    {
      result = CallListFunction( gsm, _Param );
    }

    if( result == 0 )
    {
      _ErrorMessage( _StdErr, 20, 0, _FuncName );
      _ErrorOccurred = true;
      result = new ErrorPortion;
    }
    else if( result->Spec().Type == porERROR )
    {
      _StdErr << _FuncName << "[]: ";
      result->Output( _StdErr );
      _StdErr << "\n";
      delete result;
      result = new ErrorPortion;
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

  if( result == 0 )
    result = new ErrorPortion;


  return result;
}





gString CallFuncObj::_ParamName( const int index ) const
{
  gString param_name;
  int f_index;

  if( _FuncIndex != -1 && _CurrParamIndex < _FuncInfo[ _FuncIndex ].NumParams )
    return _FuncInfo[ _FuncIndex ].ParamInfo[ _CurrParamIndex ].Name;
  
  // check whether all index'th parameters have the same name
  param_name = "";
  for( f_index = 0; f_index < _NumFuncs; f_index++ )
  {
    if( index < _FuncInfo[ f_index ].NumParams )
    {
      if( _FuncInfo[ f_index ].ParamInfo[ index ].Name != param_name )
      {
	if( param_name == "" )
	{
	  param_name = _FuncInfo[ f_index ].ParamInfo[ index ].Name;
	}
	else
	{
	  param_name = "";
	  break;
	}
      }
    }
  }

  if( param_name != "" )
    return param_name;
  else
    return "";
}


void CallFuncObj::Dump(gOutput& f) const
{
  int i;
  int j;
  bool first = true;

  if(_FuncIndex < 0)
    f << FuncName() << "[]\n";
  else
  {
    i = _FuncIndex;
    f << _FuncName << '[';
    for(j = 0; j < _FuncInfo[i].NumParams; j++)
    {
      if(_RunTimeParamInfo[j].Defined)
      {
	if(!first) f << ", ";
	first = false;      
	// if(_FuncInfo[i].ParamInfo[j].DefaultValue) f << '{';
	f << _FuncInfo[i].ParamInfo[j].Name;
	if(_FuncInfo[i].ParamInfo[j].PassByReference) f << '<';
	f << "->";
	// if(_FuncInfo[i].ParamInfo[j].DefaultValue)
	//   f << _FuncInfo[i].ParamInfo[j].DefaultValue;
	// else
	f << PortionSpecToText(_FuncInfo[i].ParamInfo[j].Spec);
	// if(_FuncInfo[i].ParamInfo[j].DefaultValue) f << '}';
      }
    }
    f << "]\n";  
  }
}



void CallFuncObj::_ErrorMessage
( 
 gOutput& s,
 const int error_num, 
 const long& num1,
 const gString& str1,
 const gString& str2,
 const gString& str3,
 const gString& str4
 )
{
#if 0
  s << "CallFuncObj Error " << error_num << " : \n";
#endif // 0

  s << "GCL: ";
  
  switch( error_num )
  {
  case 2:
    s << str1 << "[] called with conflicting parameters\n";
    break;
  case 3:
    s << str1 << "[] called with parameter #" << num1;
    if( str2 != "" ) s << ", \"" << str2 << "\",";
    s << " multiply defined\n";
    break;
  case 4:
    s << str1 << "[] called with too many parameters\n";
    break;
  case 5:
    s << str1 << "[] called with ambiguous parameter(s)\n";
    break;
  case 6:
    s << str1 << "[] parameter #" << num1 << ", \"" << str2;
    s << "\", undefined\n";
    break;
  case 7:
    s << str1 << "[] parameter #" << num1 << ", \"" << str2;
    s << "\", type mismatch\n";
    break;
  case 8:
    s << "No matching parameter specifications found for " + str1 + "[]\n";
    break;
  case 9:
    s << str1 << "[] required parameter #" << num1 << ", \"" << str2;
    s << "\", missing\n";
    break;
  case 10:
    s << str1 << "[] value parameter #" << num1 <<  ", \"" << str2;
    s << "\", undefined\n";
    break;
  case 11:
    s << str1 << "[] undefined parameter #" << num1 << ", \"" << str2;
    s << "\", passed by value\n";
    break;
  case 12:
    s << str1 << "[] value parameter #" << num1 << ", \"" << str2;
    s << "\", passed by reference\n";
    break;
  case 13:
    s << str1 << "[] reference parameter #" << num1 << ", \"" << str2;
    s << "\", passed by value\n";
    break;
  case 20:
    s << "A general error occurred while executing " << str1 << "[]\n";
    break;
  case 23:
    s << "Parameter \"" << str2 << "\" is not defined for " << str1 << "[]\n";
    break;
  case 24:
    s << "Parameter \"" << str2 << "\" is ambiguous in "<< str1 << "[]\n";
    break;
  case 25:
    s << str1 << "[]: Undefined reference \"" << str3;
    s << "\" passed for parameter #" << num1;
    if( str2 != "" ) s << ", \"" + str2 + "\"";
    s << "\n";
    break;
  case 26:
    s << str1 << "[]: Type mismatch on parameter #" << num1 << ", \"";
    s << str2 << "\"; expected " << str3 << ", got " << str4 << "\n";
    break;
  default:
    s << "General error\n";
  }
}

