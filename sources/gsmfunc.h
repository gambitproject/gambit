//#
//# FILE: gsmfunc.h -- definition of FuncDescObj, Function Descriptor Object
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef GSMFUNC_H
#define GSMFUNC_H


#include "gsmincl.h"

#include "gstring.h"


class gOutput;

#define NO_DEFAULT_VALUE  ( (Portion*)  0 )
#define REQUIRED          NO_DEFAULT_VALUE
#define PARAM_NOT_FOUND   ( (int)      -1 )
#define PARAM_AMBIGUOUS   ( (int)      -2 )


#define BYREF                true
#define BYVAL                false

#define AUTO_VAL_OR_REF      true

#define NON_LISTABLE         false
#define LISTABLE             true

#define NO_PREDEFINED_PARAMS ( (ParamInfoType*) 0 )



class FuncDescObj;
class CallFuncObj;

class gInteger;
class GSM;
class NewInstr;
class Portion;
class ListPortion;
class ReferencePortion;
class gString;

template <class T> class gList;
template <class T> class RefCountHashTable;


class ParamInfoType
{
public:
  gString      Name;
  PortionSpec  Spec;
  Portion*     DefaultValue;
  bool         PassByReference;

  ParamInfoType(void);
  ParamInfoType(const ParamInfoType& paraminfo);
  ParamInfoType
    ( 
     const gString& name, 
     const PortionSpec& spec,
     Portion* default_value = NO_DEFAULT_VALUE, 
     const bool pass_by_ref = false
     );
  ~ParamInfoType();
};



class FuncInfoType
{
public:
  bool                 UserDefined;
  union
  {
    Portion*             (*FuncPtr)(Portion **);
    gList<NewInstr*>*    FuncInstr;
  };
  PortionSpec          ReturnSpec;
  bool                 Listable;
  bool                 GameMatch;
  int                  NumParams;
  ParamInfoType*       ParamInfo;
  gString              Desc;

  FuncInfoType(void);
  FuncInfoType(const FuncInfoType& funcinfo);
  FuncInfoType
    (
     Portion* (*funcptr)(Portion**),
     PortionSpec returnspec,
     int numparams,
     ParamInfoType* paraminfo = 0,
     bool listable = LISTABLE,
     bool gamematch = false
     );
  FuncInfoType
    (
     gList<NewInstr*>* funcinstr,
     PortionSpec returnspec,
     int numparams,
     ParamInfoType* paraminfo = 0,
     bool listable = LISTABLE,
     bool gamematch = false
     );
  ~FuncInfoType();
};



class FuncDescObj
{
private:

  static RefCountHashTable< gList< NewInstr* >* > _RefCountTable;

protected:
  FuncDescObj( FuncDescObj& func );

  gString        _FuncName;
  int            _NumFuncs;
  FuncInfoType*  _FuncInfo;

  
public:
  FuncDescObj(const gString& func_name, int numfuncs);
  virtual ~FuncDescObj();

  void SetFuncInfo(int funcindex, FuncInfoType funcinfo);
  void SetParamInfo(int funcindex, int index, const ParamInfoType paraminfo);  
  void SetParamInfo(int funcindex, const ParamInfoType paraminfo[]);
  
  bool Combine(FuncDescObj* newfunc);
  bool Delete(FuncDescObj* newfunc);
  void Delete(int delete_index);

  gString FuncName(void) const;
  
  bool UDF( void ) const;
  bool BIF( void ) const;

  gList<gString> FuncList(bool udf = true, bool bif = true, 
			  bool getdesc = false) const;
  void Dump(gOutput& f, int i) const;
  void Dump(gOutput& f) const;
};




class CallFuncObj : public FuncDescObj
{
 private:
  struct RunTimeParamInfoType
  {
    ReferencePortion* Ref;
    bool              Defined;
    bool              AutoValOrRef;
  };

  gOutput&              _StdOut;
  gOutput&              _StdErr;

  int                   _FuncIndex;
  int                   _NumParams;
  int                   _NumParamsDefined;
  Portion**             _Param;
  RunTimeParamInfoType* _RunTimeParamInfo;
  bool*                 _FuncMatch;
  int                   _CurrParamIndex;
  bool                  _ErrorOccurred;

  gString _ParamName( const int index ) const;

  static bool _TypeMatch(Portion* p, PortionSpec ExpectedSpec, 
			 bool Listable, bool return_type_check = false);

  static bool _ListDimMatch( ListPortion* p1, ListPortion* p2 );
  static bool _ListNestedCheck( Portion* p, const ParamInfoType& info );

  static void _ErrorMessage
    (
     gOutput& s,
     const int error_num, 
     const long& num1 = 0,
     const gString& str1 = "",
     const gString& str2 = "",
     const gString& str3 = "",
     const gString& str4 = ""
     );

 public:
  CallFuncObj( FuncDescObj* func, gOutput& s_out, gOutput& s_err );
  ~CallFuncObj();

  int         NumParams ( void ) const;
  bool        SetCurrParamIndex ( const gString& param_name );

  bool  SetCurrParam ( Portion* param, bool auto_val_or_ref = false );

  void        SetErrorOccurred ( void );

  ReferencePortion* GetParamRef ( int index ) const;

  Portion* CallFunction       ( GSM* gsm, Portion** param );
  Portion* CallNormalFunction ( GSM* gsm, Portion** param );
  Portion* CallListFunction   ( GSM* gsm, Portion** ParamIn );

  void Dump(gOutput& f) const;
};



#endif  // GSMFUNC_H


