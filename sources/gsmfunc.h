//#
//# FILE: gsmfunc.h -- definition of FuncDescObj, Function Descriptor Object
//#                    companion to GSM
//#
//# $Id$
//#




#ifndef GSMFUNC_H
#define GSMFUNC_H


#include "portion.h"
#include "glist.h"
#include "gsmhash.h"



typedef enum { rcFAIL, rcSUCCESS, rcQUIT } GSM_ReturnCode;




#define NO_DEFAULT_VALUE  (Portion*)  0
#define PARAM_NOT_FOUND   (int)      -1
#define PARAM_AMBIGUOUS   (int)      -2

#define PASS_BY_REFERENCE true


class GSM;
class Instruction;


class FuncDescObj
{
private:

  static RefCountHashTable< gList< Instruction* >* > _RefCountTable;

  void _SetFuncInfo
    ( 
     const int f_index, 
     const int num_params
     );

  void _SetParamInfo
    ( 
     const int         f_index, 
     const int         index, 
     const gString&    name,
     const PortionType type,
     Portion*          default_value,
     const bool        pass_by_reference
     );

protected:
  struct ParamInfoType
  {
    gString      Name;
    PortionType  Type;
    Portion*     DefaultValue;
    bool         PassByReference;
  };

  struct FuncInfoType
  {
    bool                 UserDefined;
    Portion*             (*FuncPtr)(Portion **);
    gList<Instruction*>* FuncInstr;
    int                  NumParams;
    ParamInfoType*       ParamInfo;
  };

  gString        _FuncName;
  int            _NumFuncs;
  FuncInfoType*  _FuncInfo;


  public:
  FuncDescObj( FuncDescObj& func );
  FuncDescObj( const gString& func_name );
  virtual ~FuncDescObj();

  void SetFuncInfo
    (
     Portion*        (*func_ptr)(Portion**),
     const int       num_params = 0 
     );

  void SetFuncInfo
    (
     gList< Instruction* >* func_instr,
     const int       num_params = 0 
     );

  void SetParamInfo
    ( 
     Portion*          (*func_ptr)(Portion**),
     const int         index, 
     const gString&    name,
     const PortionType type,
     Portion*          default_value,
     const bool        pass_by_reference = false
     );

  void SetParamInfo
    ( 
     gList< Instruction* >* func_instr,
     const int         index, 
     const gString&    name,
     const PortionType type,
     Portion*          default_value,
     const bool        pass_by_reference = false
     );

  gString FuncName ( void ) const;
};




class CallFuncObj : public FuncDescObj
{
 private:
  struct RunTimeParamInfoType
  {
    bool               Defined;
    Reference_Portion* Ref;
    Portion*           ShadowOf;
  };

  gOutput&              _StdOut;
  gOutput&              _StdErr;

  int                   _FuncIndex;
  int                   _NumParams;
  int                   _NumParamsDefined;
  Portion**             _Param;
  RunTimeParamInfoType* _RunTimeParamInfo;
  int                   _CurrParamIndex;
  bool                  _ErrorOccurred;

  bool _TypeMatch( Portion* p, PortionType ExpectedType ) const;

  static void _ErrorMessage
    (
     gOutput& s,
     const int error_num, 
     const gInteger& num1 = 0,
     const gString& str1 = "",
     const gString& str2 = ""
     );

 public:
  CallFuncObj( FuncDescObj* func, gOutput& s_out, gOutput& s_err );
  ~CallFuncObj();

  int         NumParams ( void ) const;
  bool        ParamPassByReference( const int index ) const;
  int         FindParamName        ( const gString& param_name );

  void  SetCurrParamIndex   ( const int index );
  bool  SetCurrParam ( Portion* param, Reference_Portion* ref_param = 0 );

  void        SetErrorOccurred ( void );

  Reference_Portion* GetCurrParamRef ( void ) const;
  Portion*    GetCurrParamShadowOf ( void ) const;

  Portion*    CallFunction      ( GSM*, Portion** param );
};



#endif  // GSMFUNC_H


