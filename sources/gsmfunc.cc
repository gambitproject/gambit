//
// FILE: gsmfunc.cc -- handles initialization of defined functions for GSM
//                     companion to GSM
//
// $Id$
//

#include <assert.h>
#include <ctype.h>

#include "gsmfunc.h"

#include "glist.h"
#include "gstack.h"

#include "gsm.h"
#include "portion.h"
#include "gsmhash.h"
#include "gsminstr.h"
#include "rational.h"

#include "nfg.h"
#include "efg.h"
#include "nfstrat.h"
#include "efstrat.h"

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

void GSM::InitFunctions(void)
{
  Init_gsmoper(this);

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




Portion* CallFuncObj::CallNormalFunction( GSM* gsm, Portion** param )
{
  Portion* result = 0;

  // now check to see that all parameters belong to the same game,
  //   if so specified

  int index = 0;
  if( !_ErrorOccurred && (_FuncInfo[_FuncIndex].Flag & funcGAMEMATCH) )
  {
    void* game = NULL; 
    for(index = 0; index < _FuncInfo[_FuncIndex].NumParams; index++)
    {
      if(param[index] != 0 &&
	 _RunTimeParamInfo[index].Defined)
      {
	if( param[index]->Game() != NULL )
	{
	  if( game == NULL )
	    game = param[index]->Game();
	  else
	    if( game != param[index]->Game() )
	    {
	      _ErrorMessage( _StdErr, 29, index + 1, _FuncName );
	      _ErrorOccurred = true;
	    }
	}
      }
    }   
  }



  if( !_ErrorOccurred )
  {
    if(!_FuncInfo[_FuncIndex].UserDefined)
      result = _FuncInfo[_FuncIndex].FuncPtr( param );
    else 
      result = gsm->ExecuteUserFunc(*(_FuncInfo[_FuncIndex].FuncInstr), 
				    _FuncInfo[_FuncIndex], param,
				    FuncName() + gText((char) (_FuncIndex+1)) );
  }
  return result;
}


Portion* CallFuncObj::CallListFunction(GSM* gsm, Portion** ParamIn)
{
  int i;
  int j;
  ListPortion* p = 0;
  Portion* result;
  int NumParams = _FuncInfo[_FuncIndex].NumParams;
  Portion** CurrParam;
  bool *Listed=new bool[NumParams];
  ListPortion* Source = 0;  // source to look at to create dimensionality
  bool recurse;

  // mark down the listed parameters
  for(i = 0; i < NumParams; i++)
  {
    Listed[i] =
      _ListNestedCheck(ParamIn[i], _FuncInfo[_FuncIndex].ParamInfo[i]);
    if(Listed[i])
      Source = (ListPortion*) ParamIn[i];
  }

  // check that all dimensionalities match
  for(i = 0; i < NumParams; i++)
  {
    for(j = i + 1; j < NumParams; j++)
    {
      if(Listed[i] && Listed[j])
	if(ParamIn[i]->Spec().ListDepth > 0 && 
	   ParamIn[j]->Spec().ListDepth > 0)
	  if(!_ListDimMatch((ListPortion*)ParamIn[i],(ListPortion*)ParamIn[j]))
	  {
	    delete[] Listed;
	    throw gclRuntimeError("Mismatched dimensionalities");
	  }
    }
  }


  CurrParam = new Portion*[NumParams];
  p = new ListPortion();

  assert(Source != 0);

  // i is now the index in the lists
  for(i = 1; i <= Source->Length(); i++)
  {
    recurse = false;
    // while j is the param index
    // pickout the current element to process
    
    for(j = 0; j < NumParams; j++)
    {
      if(!Listed[j])
	CurrParam[j] = ParamIn[j];
      else
      {
	CurrParam[j] = (*(ListPortion*) ParamIn[j])[i];
	if(_ListNestedCheck(CurrParam[j], 
			     _FuncInfo[_FuncIndex].ParamInfo[j]))
	  recurse = true;
      }
    }

    if(recurse)
    {
      result = CallListFunction(gsm, CurrParam);
    }
    else  {
      for (j = 0; j < NumParams; j++)  {
	      if (CurrParam[j]->IsReference()) {
	        CurrParam[j] = CurrParam[j]->RefCopy();
        }
	      else  {
	        CurrParam[j] = CurrParam[j]->ValCopy();
        }
      }

      bool null_call = false;
      for (j = 0; j < NumParams; j++)
	if(CurrParam[j]->Spec().Null && 
	   !_FuncInfo[_FuncIndex].ParamInfo[j].Spec.Null) {
	  null_call = true;
	  break;
	}

      if (!null_call) {
	try { 
	  result = CallNormalFunction(gsm, CurrParam);
	}
	catch (gclRuntimeError &E) {
	  _StdErr << _FuncName << "[]: ";
	  _StdErr << E.Description() << "\n";
	  _ErrorOccurred = true;
	  _ErrorMessage(_StdErr, 27, p->Length()+1, _FuncName);
	  throw gclRuntimeError("");
	}
      }
      else if (null_call)  {
	throw gclRuntimeError("Null argument encountered");
      }
      else
	throw gclRuntimeError("Error in arguments");

      for (j = 0; j < NumParams; j++)
	delete CurrParam[j];
    }

    if (result == 0)
      throw gclRuntimeError("");

    if(result->Spec().Type == porUNDEFINED &&
       ((recurse && result->Spec().ListDepth > 
	 _FuncInfo[_FuncIndex].ReturnSpec.ListDepth) ||
	(!recurse && result->Spec().ListDepth == 
	 _FuncInfo[_FuncIndex].ReturnSpec.ListDepth)))
      {
	assert(result->Spec().ListDepth > 0);
	((ListPortion*) result)->
	  SetDataType(_FuncInfo[_FuncIndex].ReturnSpec.Type);      
      }
      p->Append(result);


  }

  delete[] CurrParam;
  delete[] Listed;
  return p;
}




//---------------------------------------------------------------
//                      ParamInfoType
//---------------------------------------------------------------

ParamInfoType::ParamInfoType(void)
:
 Name(""), 
 DefaultValue(REQUIRED), 
 PassByReference(BYVAL)
{}

ParamInfoType::ParamInfoType(const ParamInfoType& paraminfo)
:
 Name(paraminfo.Name), 
 Spec(paraminfo.Spec), 
 DefaultValue(paraminfo.DefaultValue), 
 PassByReference(paraminfo.PassByReference)
{}

ParamInfoType::ParamInfoType
(
 const gText& name,
 const PortionSpec& spec,
 Portion* default_value, 
 const bool pass_by_ref
)
:
 Name(name), 
 Spec(spec), 
 DefaultValue(default_value), 
 PassByReference(pass_by_ref)
{ }

ParamInfoType::~ParamInfoType()
{ }

//---------------------------------------------------------------------
//                     FuncInfoType
//---------------------------------------------------------------------

FuncInfoType::FuncInfoType(void)
:
 UserDefined(false),
 FuncPtr(0),
 ReturnSpec(PortionSpec(porUNDEFINED)),
 Flag( funcLISTABLE ),
 NumParams(0),
 ParamInfo(0)
{}

FuncInfoType::FuncInfoType(const FuncInfoType& funcinfo)
:
 UserDefined(funcinfo.UserDefined),
 ReturnSpec(funcinfo.ReturnSpec),
 Flag(funcinfo.Flag),
 NumParams(funcinfo.NumParams),
 Desc( funcinfo.Desc )
{
  int i;
  if(!UserDefined)
    FuncPtr = funcinfo.FuncPtr;
  else
    FuncInstr = funcinfo.FuncInstr;
  assert(funcinfo.ParamInfo);
  ParamInfo = new ParamInfoType[NumParams];
  for(i=0; i<NumParams; i++)
    ParamInfo[i] = funcinfo.ParamInfo[i];
}

FuncInfoType::FuncInfoType
(
 Portion* (*funcptr)(Portion**),
 PortionSpec returnspec,
 int numparams,
 ParamInfoType* paraminfo,
 FuncFlagType flag
)
:
 UserDefined(false),
 FuncPtr(funcptr),
 ReturnSpec(returnspec),
 Flag(flag),
 NumParams(numparams)
{
  int i;
  ParamInfo = new ParamInfoType[NumParams];
  if(paraminfo)
    for(i=0; i<NumParams; i++)
      ParamInfo[i] = paraminfo[i];
}

FuncInfoType::FuncInfoType(gclExpression* funcinstr,
			   PortionSpec returnspec,
			   int numparams,
			   ParamInfoType* paraminfo,
			   FuncFlagType flag)
  : UserDefined(true), FuncInstr(funcinstr), ReturnSpec(returnspec),
    Flag(flag), NumParams(numparams)
{
  int i;
  ParamInfo = new ParamInfoType[NumParams];
  if(paraminfo)
    for(i=0; i<NumParams; i++)
      ParamInfo[i] = paraminfo[i];
}

FuncInfoType::~FuncInfoType()
{}

//---------------------------------------------------------------------
//                   Function descriptor objects
//---------------------------------------------------------------------

RefCountHashTable< gclExpression* > FuncDescObj::_RefCountTable;


FuncDescObj::FuncDescObj(FuncDescObj& func)
{
  int index;
  int f_index;

  _FuncName  = func._FuncName;
  _NumFuncs  = func._NumFuncs;
  _FuncInfo  = new FuncInfoType[_NumFuncs];

  for(f_index = 0; f_index < _NumFuncs; f_index++)
  {
    _FuncInfo[f_index].UserDefined = func._FuncInfo[f_index].UserDefined;
    
    _FuncInfo[f_index].ReturnSpec  = func._FuncInfo[f_index].ReturnSpec;
    _FuncInfo[f_index].Flag    = func._FuncInfo[f_index].Flag;

    if(!_FuncInfo[f_index].UserDefined)
      _FuncInfo[f_index].FuncPtr     = func._FuncInfo[f_index].FuncPtr;
    else
      _FuncInfo[f_index].FuncInstr   = func._FuncInfo[f_index].FuncInstr;
    
    if(_FuncInfo[f_index].UserDefined)
      if(!_RefCountTable.IsDefined(_FuncInfo[f_index].FuncInstr))
	_RefCountTable.Define(_FuncInfo[f_index].FuncInstr, 1);
      else
	_RefCountTable(_FuncInfo[f_index].FuncInstr)++;
    
    _FuncInfo[f_index].NumParams = func._FuncInfo[f_index].NumParams;
    _FuncInfo[f_index].ParamInfo =
      new ParamInfoType[_FuncInfo[f_index].NumParams];

    for(index = 0; index < _FuncInfo[f_index].NumParams; index ++)
    {
      _FuncInfo[f_index].ParamInfo[index] = 
	func._FuncInfo[f_index].ParamInfo[index];

      if(_FuncInfo[f_index].ParamInfo[index].DefaultValue != 0)
      {
	_FuncInfo[f_index].ParamInfo[index].DefaultValue = 
	  _FuncInfo[f_index].ParamInfo[index].DefaultValue->ValCopy();
      }	
    }
  }
}


FuncDescObj::FuncDescObj(const gText& func_name, int numfuncs)
: _FuncName(func_name), _NumFuncs(numfuncs)
{
  _FuncInfo = new FuncInfoType[_NumFuncs];
}


  // Assumes one argument, which has a prototype func_proto
FuncDescObj::FuncDescObj( const gText& func_proto, 
                          Portion* (*funcptr)(Portion**),
                          FuncFlagType FFT /* = funcLISTABLE */  )
{
  _NumFuncs = 1;
  _FuncInfo = new FuncInfoType[1];
  
  char ch = ' ';
  int index = 0;
  gText func_name;

  ch = func_proto[index++];
  while (isalpha(ch))
  {
    func_name += ch;
    ch=func_proto[index++];
  }

  _FuncName = func_name;

  SetFuncInfo(0,func_proto,funcptr, FFT);
}


FuncDescObj::~FuncDescObj()
{
  int index;
  int f_index;
//  NewInstr* NewInstr;
  
  for(f_index = 0; f_index < _NumFuncs; f_index++)
  {
    for(index = 0; index < _FuncInfo[f_index].NumParams; index ++)
    {
      delete _FuncInfo[f_index].ParamInfo[index].DefaultValue;
    }
    if(_FuncInfo[f_index].UserDefined)
    {
      assert(_RefCountTable.IsDefined(_FuncInfo[f_index].FuncInstr));
      assert(_RefCountTable(_FuncInfo[f_index].FuncInstr) > 0);
      _RefCountTable(_FuncInfo[f_index].FuncInstr)--;
      if(_RefCountTable(_FuncInfo[f_index].FuncInstr) == 0)
      {
	_RefCountTable.Remove(_FuncInfo[f_index].FuncInstr);
	assert(_FuncInfo[f_index].FuncInstr != 0);
/*
	while(_FuncInfo[f_index].FuncInstr->Length() > 0)
	{
	  NewInstr = _FuncInfo[f_index].FuncInstr->Remove(1);
	  delete NewInstr;
	}
	delete _FuncInfo[f_index].FuncInstr;
	*/
	delete _FuncInfo[f_index].FuncInstr;
      }
    }
    delete[] _FuncInfo[f_index].ParamInfo;
  }
  delete[] _FuncInfo;
}


void FuncDescObj::SetFuncInfo(int funcindex, FuncInfoType funcinfo)
{
  assert((funcindex >= 0) && (funcindex < _NumFuncs));
  _FuncInfo[funcindex] = funcinfo;
  if(funcinfo.UserDefined)
    if(!_RefCountTable.IsDefined(funcinfo.FuncInstr))
      _RefCountTable.Define(funcinfo.FuncInstr, 1);
    else
      _RefCountTable(funcinfo.FuncInstr)++;
}

void FuncDescObj::SetFuncInfo(int funcindex, const gText& s)
{
  SetFuncInfo(funcindex, s, 0);
}

  // In this function, the input string "s" is parsed, and the info contained 
  // in it is used to determine the function info, and then SetFuncInfo 
  // is called with all data passed as arguments rather than a string.

void FuncDescObj::SetFuncInfo(int funcindex, const gText& s,
                              Portion* (*funcptr)(Portion**), 
                              FuncFlagType FFT /* = funcLISTABLE */  )
{

  char ch = ' ';
  int index=0, length=s.Length();
  int numArgs=0;
  bool done = false;
  bool required = false;
  gList<gText> specList;
  gList<gText> nameList;
  gList<int>     listList;
  gList<bool>    refList;
  gList<Portion*>     reqList;
  
    // Move ch to the first variable name
  while (ch != '[' && index<=length)  
    ch=s[index++];
  ch=s[index++];  // ch should now hold first letter of the first name

  if (ch == ']') // If there are no parameters
    done = true;

    // Loop through the string, parsing a word-argument pair at a time.
  while (!done)  // ch should always be at beginning of next name here
  {
  
    if (ch == '{')  // If the argument is optional
    {
      ch=s[index++];
      required = false;
    }
    else            // The argument is required
    {
      required = true;
    }
  
      // name gets the name of the variable
    gText name = ch;
    ch=s[index++];
    while (isalpha(ch)) { name += ch; ch = s[index++]; }
    nameList.Append(name);
  
      // See if it passed by reference
    if (ch == '<')
    {
      refList.Append(BYREF);
    }
    else
    {
      refList.Append(BYVAL);
    }
    
      // Move ch so that it holds the first character of the type;
    while (ch != '>' && ch != ']' && index<=length)  
      ch=s[index++];

    if (ch == ']')  // If we hit the end (this should NEVER happen)
    {
      gout << "hit end when unexpected\n\n";
      assert(0);
      done = true;
    }
    else
    {
      ch=s[index++];
  
        // Word gets the word, which is the type of variable.
      gText word = ch;
      int     listNum = 0;
      ch=s[index++];
      if (required)  // If required, get word in normal fashion.
      {
        reqList.Append( REQUIRED );
        while (isalpha(ch)) { word += ch; ch = s[index++]; }
      }
      else  // If optional, get default argument and parse.
      {
        while (ch != '}') { word += ch; ch = s[index++]; }
        ch=s[index++];
        
        /*gout << "Default: " << word << "\n";*/

        if (word == "true" || word == "True" || word == "TRUE")
        {
          bool* tmp = new bool(true);
          reqList.Append((Portion*)tmp);
          word = "BOOLEAN";
        }
        else if (word == "false" || word == "False" || word == "FALSE")
        {
          bool* tmp = new bool(false);
          reqList.Append((Portion*)tmp);
          word = "BOOLEAN";
        }
        else if (word == "StdOut")
        {
          OutputPortion* tmp = new OutputPortion(gout);
          reqList.Append((Portion*)tmp);
          word = "OUTPUT";
        }
        else if (word == "NullOut")
        {
          OutputPortion* tmp = new OutputPortion(gnull);
          reqList.Append((Portion*)tmp);
          word = "OUTPUT";
        }
        else if (word == "StdIn")
        {
          InputPortion* tmp = new InputPortion(gin);
          reqList.Append((Portion*)tmp);
          word = "INPUT";
        }
          // If it is a text string (for now just assume it is if it begins '"')
        else if (word[0] == '"')
        {
          gText* tmp = new gText(word);
          reqList.Append((Portion*)tmp);
          word = "TEXT";
        }
          // If it is a number (int or double)
        else if ((word[0] >= '0' && word[0] <= '9') || word[0] == '-')
        {
          char cha = ' ';
          int sign = 1;
          int index2 = 0;
          bool isDouble = false;
          /*gInteger num = 0, denom = 1;*/
          gNumber* num = new gNumber(0);
        
          cha = word[index2++];
        
          if (cha == '-')  {  // If the number is negative
            sign = -1;
            cha = word[index2++];
          }
        
          while (cha >= '0' && cha <= '9')   {  // Read number
            *num *= 10;
            *num += (int) (cha - '0');
            cha = word[index2++];
          }
        
          if (cha == '.')    // If it is a float
          {
            isDouble = true;
            int factor = 1;
            cha = word[index2++];
        
            while (cha >= '0' && cha <= '9')   {
              *num += ( ((float) (cha - '0')) / (pow((int)10, (long)factor)) );
              factor++;
              cha = word[index2++];
            }
  
            *num *= sign;
          }  // If it is a float
        
          if (isDouble)
          {
            word = "NUMBER";
            reqList.Append((Portion*) num);
          }
          else
          {
            word = "INTEGER";
            gInteger* tmp = new gInteger((int)*num);
            reqList.Append((Portion*) tmp);
            delete num;
          }

        } // If it is int or float

        else
        {
          gout << "Unknown optional type!!\n\n";
          int* silly = new int;
          reqList.Append( (Portion *)silly);
          word = "MIXED";    // TEMPORARY ONLY SO THAT IT WORKS -- REMOVE!!
        }

      }

      numArgs++;
  
        // Check to see if it is a list  (should prolly only do if req.)
      if (word == "LIST")
      {
        while (word == "LIST")
        {
          // increment counter, and while it is a list, keep incrementing.
          listNum++;
          ch=s[index++];  // move ch past left parenthesis
    
            // Word gets the word, which is the type of variable.
          word = ch;
          ch=s[index++];
          while (isalpha(ch)) { word += ch; ch = s[index++]; }
    
        }
      }
 
  
        // Check to see if it is a list  (should prolly only do if req.)
      if (word == "NLIST")
      {
          // set listnum to be NLIST
        listNum = NLIST;
        ch=s[index++];  // move ch past left parenthesis
   
          // Word gets the word, which is the type of variable.
        word = ch;
        ch=s[index++];
        while (isalpha(ch)) { word += ch; ch = s[index++]; }
    
      }
 

        // See if it is modified by a star
      if (ch == '*')
      {  
        word += ch;
        ch=s[index++];
      }

      specList.Append(word);
      listList.Append(listNum);
  
        // Move ch past the right parentheses (and braces), if applicable
      while (ch == ')') {  ch=s[index++];  }
      /*while (ch == '}') {  ch=s[index++];  }*/

      if (ch == ',')  // If there will be another variable
      {
          // Move ch to the first letter of the next variable name 
        ch=s[index++];
        ch=s[index++];
      }
      else
        done = true;

    } // if not done (didn't hit ], and we found a ',')
  }  // while not done

    // Move ch to point to first char of return type
  while (ch != ':' && index<=length)
    ch=s[index++];
  ch=s[index++];
  ch=s[index++];
  
    // Word gets the word, which is the type of variable.
  gText word = ch;
  ch=s[index++];
  while (isalpha(ch)) { word += ch; ch = s[index++]; }
  int     listNum = 0;
  
  
    // Check to see if it is a list
  if (word == "LIST")
  {
    while (word == "LIST")
    {
        // increment counter, and while it is a list, keep incrementing.
      listNum++;
      ch=s[index++];  // move ch past left parenthesis

        // Word gets the word, which is the type of variable.
      word = ch;
      ch=s[index++];
      while (isalpha(ch)) { word += ch; ch = s[index++]; }
    }
  }
  
    // Check to see if it is a list  (should prolly only do if req.)
  if (word == "NLIST")
  {
      // set listnum to be NLIST
    listNum = NLIST;
    ch=s[index++];  // move ch past left parenthesis
 
      // Word gets the word, which is the type of variable.
    word = ch;
    ch=s[index++];
    while (isalpha(ch)) { word += ch; ch = s[index++]; }
  }
 

    // Bunch of prints for debugging purposes.
  /*gout << "\nReturn Type: " << word << "\n";*/
  /*gout << "Return listNum: " << listNum << "\n";*/
  /*gout << "SpecList: \n";*/
  /*specList.Dump(gout);*/
  /*gout << "NameList: \n";*/
  /*nameList.Dump(gout);*/
  /*gout << "ListList: \n";*/
  /*listList.Dump(gout);*/
  /*gout << "RefList: \n";*/
  /*refList.Dump(gout);*/
  /*int rl = reqList.Length();*/
  /*gout << "ReqL Length: " << rl << "\n";*/
  /*gout << "NumArgs: " << numArgs << "\n";*/
  /*gout << "\n\n";*/

  
  ParamInfoType *PIT = new ParamInfoType[numArgs];

  for (int i=1;i<=numArgs;i++)
  {
    PIT[i-1] = ParamInfoType(nameList[i], ToSpec(specList[i], listList[i]),
                                  reqList[i], refList[i]);
  }

  SetFuncInfo(funcindex, 
              FuncInfoType(funcptr, ToSpec(word, listNum), numArgs, PIT, FFT));
}

  // Replaces strings with their enumerated types.
PortionSpec ToSpec(gText &str, int num /* =0 */)
{
  /*gout << "ToSpec called with " << str << " and " << num << ".\n";*/
  if (str == "NUMBER")
    return PortionSpec(porNUMBER, num);
  else if (str == "BOOLEAN") 
    return PortionSpec(porBOOL, num); 
  else if (str == "TEXT")
    return PortionSpec(porTEXT, num);
  else if (str == "EFG")
    return PortionSpec(porEFG, num);
  else if (str == "EFPLAYER")
    return PortionSpec(porEFPLAYER, num);
  else if (str == "EFOUTCOME")
    return PortionSpec(porEFOUTCOME, num);
  else if (str == "NODE")
    return PortionSpec(porNODE, num);
  else if (str == "INFOSET")
    return PortionSpec(porINFOSET, num);
  else if (str == "BEHAV")
    return PortionSpec(porBEHAV, num);
  else if (str == "NFG")
    return PortionSpec(porNFG, num);
  else if (str == "NFPLAYER")
    return PortionSpec(porNFPLAYER, num);
  else if (str == "NFOUTCOME")
    return PortionSpec(porNFOUTCOME, num);
  else if (str == "MIXED")
    return PortionSpec(porMIXED, num);
  else if (str == "STRATEGY")
    return PortionSpec(porSTRATEGY, num);
  else if (str == "ACTION")
    return PortionSpec(porACTION, num);
  else if (str == "ANYTYPE")
    return PortionSpec(porANYTYPE, num);
  else if (str == "EFSUPPORT")
    return PortionSpec(porEFSUPPORT, num);
  else if (str == "EFBASIS")
    return PortionSpec(porEFBASIS, num);
  else if (str == "NFSUPPORT")
    return PortionSpec(porNFSUPPORT, num);
  else if (str == "INPUT")
    return PortionSpec(porINPUT, num);
  else if (str == "OUTPUT")
    return PortionSpec(porOUTPUT, num);


  else if (str == "NUMBER*")
    return PortionSpec(porNUMBER, num, porNULLSPEC);
  else if (str == "BOOLEAN*")
    return PortionSpec(porBOOL, num, porNULLSPEC);
  else if (str == "TEXT*")
    return PortionSpec(porTEXT, num, porNULLSPEC);
  else if (str == "EFG*")
    return PortionSpec(porEFG, num, porNULLSPEC);
  else if (str == "EFPLAYER*")
    return PortionSpec(porEFPLAYER, num, porNULLSPEC);
  else if (str == "EFOUTCOME*")
    return PortionSpec(porEFOUTCOME, num, porNULLSPEC);
  else if (str == "NODE*")
    return PortionSpec(porNODE, num, porNULLSPEC);
  else if (str == "INFOSET*")
    return PortionSpec(porINFOSET, num, porNULLSPEC);
  else if (str == "BEHAV*")
    return PortionSpec(porBEHAV, num, porNULLSPEC);
  else if (str == "NFG*")
    return PortionSpec(porNFG, num, porNULLSPEC);
  else if (str == "NFPLAYER*")
    return PortionSpec(porNFPLAYER, num, porNULLSPEC);
  else if (str == "NFOUTCOME*")
    return PortionSpec(porNFOUTCOME, num, porNULLSPEC);
  else if (str == "MIXED*")
    return PortionSpec(porMIXED, num, porNULLSPEC);
  else if (str == "STRATEGY*")
    return PortionSpec(porSTRATEGY, num, porNULLSPEC);
  else if (str == "ACTION*")
    return PortionSpec(porACTION, num, porNULLSPEC);
  else if (str == "ANYTYPE*")
    return PortionSpec(porANYTYPE, num, porNULLSPEC);
  else if (str == "EFSUPPORT*")
    return PortionSpec(porEFSUPPORT, num, porNULLSPEC);
  else if (str == "EFBASIS*")
    return PortionSpec(porEFBASIS, num, porNULLSPEC);
  else if (str == "NFSUPPORT*")
    return PortionSpec(porNFSUPPORT, num, porNULLSPEC);
  else if (str == "INPUT*")
    return PortionSpec(porINPUT, num, porNULLSPEC);
  else if (str == "OUTPUT*")
    return PortionSpec(porOUTPUT, num, porNULLSPEC);
  else
    throw gclRuntimeError("ERROR: incorrect type, " + str + ", in function definition");
}

void FuncDescObj::SetParamInfo(int funcindex, int index, 
			       const ParamInfoType param)
{
  assert((funcindex >= 0) && (funcindex < _NumFuncs));
  assert((index >= 0) && (index < _FuncInfo[funcindex].NumParams));
  _FuncInfo[funcindex].ParamInfo[index] = param;
}

void FuncDescObj::SetParamInfo(int funcindex, const ParamInfoType params[])
{
  assert((funcindex >= 0) && (funcindex < _NumFuncs));
  int i;
  for(i = 0; i < _FuncInfo[funcindex].NumParams; i++)
    _FuncInfo[funcindex].ParamInfo[i] = params[i];
}



bool FuncDescObj::Combine(FuncDescObj* newfunc)
{
  bool result = true;
  bool finalresult = true;
  bool same_params;
  int i;
  int j;
  int f_index;
  int index;
  gStack<int> delete_stack;

  for(i = 0; i < newfunc->_NumFuncs; i++)
  {
    result = true;
    for(f_index = 0; f_index < _NumFuncs; f_index++)
    {
      same_params = true;
      for(index = 0; 
	  (index < _FuncInfo[f_index].NumParams) &&
	  (index < newfunc->_FuncInfo[i].NumParams); 
	  index++)
      {
	if((_FuncInfo[f_index].ParamInfo[index].Name ==
	    newfunc->_FuncInfo[i].ParamInfo[index].Name) &&
	   (_FuncInfo[f_index].ParamInfo[index].Spec.Type ==
	    newfunc->_FuncInfo[i].ParamInfo[index].Spec.Type) &&
	   (_FuncInfo[f_index].ParamInfo[index].Spec.ListDepth ==
	    newfunc->_FuncInfo[i].ParamInfo[index].Spec.ListDepth))
	{
	  same_params = same_params & true;
	}
	else
	{
	  same_params = false;
	  break;
	}
      }

      if(same_params)
      {
	if(_FuncInfo[f_index].NumParams < newfunc->_FuncInfo[i].NumParams)
	{
	  for(index = _FuncInfo[f_index].NumParams;
	      index < newfunc->_FuncInfo[i].NumParams; 
	      index++)
	  {
	    if(newfunc->_FuncInfo[i].ParamInfo[index].DefaultValue==0)
	    {
	      same_params = false;
	      break;
	    }
	  }
	}
	else if(_FuncInfo[f_index].NumParams > newfunc->_FuncInfo[i].NumParams)
	{
	  for(index = newfunc->_FuncInfo[i].NumParams; 
	      index < _FuncInfo[f_index].NumParams;
	      index++)
	  {
	    if(_FuncInfo[f_index].ParamInfo[index].DefaultValue==0)
	    {
	      same_params = false;
	      break;
	    }
	  }
	}
      }

      if(same_params)
      {
	if(!_FuncInfo[f_index].UserDefined)
	{
	  gerr << "Function ambiguous with " << FuncList()[f_index+1] <<'\n';
	  result = false;
	}
	else
	{
	  delete_stack.Push(f_index);
	}
      }
    }

    if(result)
    {
      int delete_index = 0;
      while(delete_stack.Depth() > 0)
      {
	delete_index = delete_stack.Pop();
	gerr << "Replacing " << FuncList()[delete_index+1] << '\n';
	Delete(delete_index);
      }


      FuncInfoType* NewFuncInfo = new FuncInfoType[_NumFuncs+1];
      for(j=0; j<_NumFuncs; j++)
	NewFuncInfo[j] = _FuncInfo[j];
      delete[] _FuncInfo;
      _FuncInfo = NewFuncInfo;
      _NumFuncs++;
      
      if(newfunc->_FuncInfo[i].UserDefined)
	SetFuncInfo(_NumFuncs-1, newfunc->_FuncInfo[i]);
      else
	SetFuncInfo(_NumFuncs-1, newfunc->_FuncInfo[i]);
      for(j = 0; j < newfunc->_FuncInfo[i].NumParams; j++)
      {
	SetParamInfo(_NumFuncs-1, j, newfunc->_FuncInfo[i].ParamInfo[j]);
	newfunc->_FuncInfo[i].ParamInfo[j].DefaultValue = 0;
      }
    }
    finalresult = finalresult & result;
  }
  delete newfunc;
  return finalresult;
}


bool FuncDescObj::Delete(FuncDescObj* newfunc)
{
  bool result = true;
  bool finalresult = true;
  bool same_params;
  int i;
  int f_index;
  int delete_index = 0;
  int index;

  for(i = 0; i < newfunc->_NumFuncs; i++)
  {
    result = false;
    for(f_index = 0; f_index < _NumFuncs; f_index++)
    {
      if(_FuncInfo[f_index].NumParams == newfunc->_FuncInfo[i].NumParams) 
      {
	same_params = true;
	for(index = 0; 
	    index < _FuncInfo[f_index].NumParams;
	    index++)
	{
	  if((_FuncInfo[f_index].ParamInfo[index].DefaultValue == 0 && 
	      newfunc->_FuncInfo[i].ParamInfo[index].DefaultValue == 0) &&
	     (_FuncInfo[f_index].ParamInfo[index].Name ==
	      newfunc->_FuncInfo[i].ParamInfo[index].Name) &&
	     (_FuncInfo[f_index].ParamInfo[index].Spec ==
	      newfunc->_FuncInfo[i].ParamInfo[index].Spec))
	  {
	    same_params = same_params & true;
	  }
	  else
	  {
	    same_params = false;
	    break;
	  }
	}
	
	if(same_params)
	{
	  delete_index = f_index;
	  result = true;
	  break;
	}
      }
    }

    if(result)
      Delete(delete_index);
    finalresult = finalresult & result;
  }
  delete newfunc;
  return finalresult;
}


void FuncDescObj::Delete(int delete_index)
{
  FuncInfoType* NewFuncInfo = new FuncInfoType[_NumFuncs-1];  
  int j;
  for(j=0; j<delete_index; j++)
    NewFuncInfo[j] = _FuncInfo[j];
  for(j=delete_index+1; j<_NumFuncs; j++)
    NewFuncInfo[j-1] = _FuncInfo[j];
  delete[] _FuncInfo;
  _FuncInfo = NewFuncInfo;
  _NumFuncs--;
}


gText FuncDescObj::FuncName(void) const
{ return _FuncName; }



bool FuncDescObj::UDF( void ) const
{
  int i = 0;
  for( i = 0; i < _NumFuncs; i++ )
  {
    if( _FuncInfo[i].UserDefined )
      return true;
  }
  return false;
}



bool FuncDescObj::BIF( void ) const
{
  for (int i = 0; i < _NumFuncs; i++)  {
    if( !_FuncInfo[i].UserDefined )
      return true;
  }
  return false;
}



gList<gText> FuncDescObj::FuncList( bool udf, bool bif, bool getdesc ) const
{
  gList<gText> list;
  gText f;
  int i;
  int j;

  for(i = 0; i < _NumFuncs; i++)
  {  
    if( (udf && _FuncInfo[i].UserDefined) ||
       (bif && !_FuncInfo[i].UserDefined) )
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
	  f += _FuncInfo[i].ParamInfo[j].DefaultValue->OutputString();
	else
	  f += PortionSpecToText(_FuncInfo[i].ParamInfo[j].Spec);
	
	if(_FuncInfo[i].ParamInfo[j].DefaultValue) 
	  f += '}';
      }
      f += "] =: ";
      f += PortionSpecToText(_FuncInfo[i].ReturnSpec);

      /*
      if( getdesc && _FuncInfo[i].UserDefined )
	f += '\n';
	*/

      if( getdesc && _FuncInfo[i].Desc.Length() > 0 )
      {
	f += '\n';
	f += _FuncInfo[i].Desc;
	f += '\n';
      }

      list.Append(f);
    }
  }
  return list;
}

void FuncDescObj::Dump(gOutput& f, int i) const
{ f << FuncList()[i+1] << '\n'; }

void FuncDescObj::Dump(gOutput& f) const
{
  int i;
  for(i = 0; i < _NumFuncs; i++)
    Dump(f, i);
}





//---------------------------------------------------------------------
//                      CallFuncObj
//---------------------------------------------------------------------


CallFuncObj::CallFuncObj(FuncDescObj* func, gOutput& s_out, gOutput& s_err)
:FuncDescObj(*func), _StdOut(s_out), _StdErr(s_err)
{
  int index;
  int f_index;

  _FuncIndex = -1;
  _NumParams = 0;
  _NumParamsDefined = 0;
  for(f_index = 0; f_index < _NumFuncs; f_index++)
  {
    if(_NumParams < _FuncInfo[f_index].NumParams)
      _NumParams = _FuncInfo[f_index].NumParams;
  }

  _Param = new Portion*[_NumParams];
  _RunTimeParamInfo = new RunTimeParamInfoType[_NumParams];
  _FuncMatch = new bool[_NumFuncs];
  _CurrParamIndex = 0;
  _ErrorOccurred = false;

  for(f_index = 0; f_index < _NumFuncs; f_index++)
    _FuncMatch[f_index] = true;
  for(index = 0; index < _NumParams; index++)
  {
    _Param[index] = REQUIRED;
    _RunTimeParamInfo[index].Ref = 0;
    _RunTimeParamInfo[index].Defined = false;
    _RunTimeParamInfo[index].AutoValOrRef = false;
  }
}


CallFuncObj::~CallFuncObj()
{
  int index;
  if(_ErrorOccurred)
    for(index = 0; index < _NumParams; index++)
    {
      delete _RunTimeParamInfo[index].Ref;
      delete _Param[index];
    }
  delete[] _FuncMatch;
  delete[] _RunTimeParamInfo;
  delete[] _Param;
}


int CallFuncObj::NumParams(void) const
{ return _NumParams; }


bool CallFuncObj::_TypeMatch(Portion* p, PortionSpec ExpectedSpec, 
			     bool Listable, bool return_type_check)
{
  bool        result = false;
  PortionSpec CalledSpec;


  if(p == 0 && ExpectedSpec.Type == porUNDEFINED)
    return true;

  if(ExpectedSpec.Type == porANYTYPE && return_type_check)
    return true;  

  assert(p != 0);
  CalledSpec = p->Spec();

  if(p->Spec().Type == porNULL)
    CalledSpec = ((NullPortion*) p)->DataType();
  

  if(CalledSpec.Type & ExpectedSpec.Type)
  {
    if(CalledSpec.ListDepth == ExpectedSpec.ListDepth)
      result = true;
    else if(CalledSpec.ListDepth > ExpectedSpec.ListDepth && Listable)
      result = true;
    else if(CalledSpec.ListDepth > 0 && ExpectedSpec.ListDepth == 1 && 
	    !Listable)
      result = true;
    else if(CalledSpec.ListDepth > 0 && ExpectedSpec.ListDepth == NLIST)
      result = true;
  }
  else if(CalledSpec.Type == porUNDEFINED && CalledSpec.ListDepth > 0)
  {
    if(CalledSpec.ListDepth == 1 && ExpectedSpec.ListDepth > 0)
    {
      // ((ListPortion*) p)->SetDataType(ExpectedSpec.Type);
      result = true;
    }
  }

  return result;
}



bool CallFuncObj::SetCurrParamIndex(const gText& param_name)
{
  int f_index;
  int TempFuncIndex = -1;
  int index;
  int result = PARAM_NOT_FOUND;
  int times_found = 0;
  bool name_match_found;

  
  if(_ErrorOccurred)
    return true;


  for(f_index = 0; 
      f_index < _NumFuncs && result != PARAM_AMBIGUOUS;
      f_index++)
  {
    if (_FuncMatch[f_index])
    {
      name_match_found = false;
      for(index = 0; index < _FuncInfo[f_index].NumParams; index++)
      {
	if (_FuncInfo[f_index].ParamInfo[index].Name == param_name)
	{
	  name_match_found = true;
	  if(result == PARAM_NOT_FOUND)
	  {
	    result = index;
	    TempFuncIndex = f_index;
	    times_found++;
	  }
	  else if(result == index)
	  {
	    times_found++;
	  }
	  else // (result != index)
	  {
	    result = PARAM_AMBIGUOUS;
	    break;
	  }
	}
      }
      if(!name_match_found)
	_FuncMatch[f_index] = false;
    }
  }
  
  
  if(times_found == 1)
  {
    if(_FuncIndex == -1)
    {
      _FuncIndex = TempFuncIndex;
    }
    else if(_FuncIndex != TempFuncIndex)
    {
      _ErrorMessage(_StdErr, 2, 0, _FuncName);
      _FuncIndex = -1;
      _ErrorOccurred = true;
      return false;
    }
  }

  if(result == PARAM_NOT_FOUND)
  {
    _ErrorOccurred = true;
    _ErrorMessage(_StdErr, 23, 0, _FuncName, param_name);
    return false;
  }
  else if(result == PARAM_AMBIGUOUS)
  {
    _ErrorOccurred = true;
    _ErrorMessage(_StdErr, 24, 0, _FuncName, param_name);
    return false;
  }
  else
  {
    _CurrParamIndex = result;
    return true;
  }
}


void CallFuncObj::SetErrorOccurred(void)
{
  _ErrorOccurred = true;
}



bool CallFuncObj::SetCurrParam(Portion *param, bool auto_val_or_ref)
{
  ReferencePortion* ref_param = 0;
  bool AllowUndefinedRef;
  int f_index;
  int funcs_matched;
  int last_match;


  // An error had already occurred with the current function call
  if(_ErrorOccurred)
  {
    delete param;
    return true;
  }

  // Parameter index out of bounds
  if(_CurrParamIndex >= _NumParams)
  {
    _ErrorMessage(_StdErr, 4, 0, _FuncName);
    delete param;
    _ErrorOccurred = true;
    return false;
  }

  // Repeated parameter defition
  if(_RunTimeParamInfo[_CurrParamIndex].Defined)
  {
    _ErrorMessage(_StdErr, 3, _CurrParamIndex + 1, _FuncName, 
		  _ParamName(_CurrParamIndex));
    delete param;
    _ErrorOccurred = true;
    return false;
  }

  // Passed an undefined variable
  if(param != 0 && param->Spec().Type == porREFERENCE)
  {

    // check whether undefined variables are allowed
    AllowUndefinedRef = false;
    for(f_index = 0; f_index < _NumFuncs; f_index++)
    {
      if(_CurrParamIndex < _FuncInfo[f_index].NumParams)
      {
	if((_FuncInfo[f_index].ParamInfo[_CurrParamIndex].PassByReference &&
	    _FuncInfo[f_index].ParamInfo[_CurrParamIndex].DefaultValue != 0)|| 
	   _FuncInfo[f_index].UserDefined ||
	   _FuncInfo[f_index].ParamInfo[_CurrParamIndex].Spec.Type == porUNDEFINED)
	  AllowUndefinedRef = true;
	else
	  _FuncMatch[f_index] = false;
      }
    }

    if(AllowUndefinedRef)
    {
      ref_param = (ReferencePortion*) param;
      param = 0;
    }
    else
    {
      _ErrorMessage(_StdErr, 25, _CurrParamIndex + 1, _FuncName, 
		    _ParamName(_CurrParamIndex),
		    ((ReferencePortion*) param)->Value());
      delete param;
      _ErrorOccurred = true;
      return false;
    }
  }

  

  funcs_matched = 0;
  last_match = 0;
  for(f_index = 0; f_index < _NumFuncs; f_index++)
  {
    if(_FuncMatch[f_index])
    {
      last_match = f_index;
      funcs_matched++;
    }
  }
  if(funcs_matched == 1)
    _FuncIndex = last_match;


  if(param != 0)
  {
    if(_FuncIndex == -1)
    {
      // Attempt on-the-fly function matching
      for(f_index = 0; f_index < _NumFuncs; f_index++)
      {
	if(_CurrParamIndex < _FuncInfo[f_index].NumParams)
	{
	  if(!_TypeMatch
	     (param, 
	      _FuncInfo[f_index].ParamInfo[_CurrParamIndex].Spec,
	      (_FuncInfo[f_index].Flag & funcLISTABLE) ))
	  {
	    _FuncMatch[f_index] = false;
	  }
	}
	else
	{
	  _FuncMatch[f_index] = false;
	}
      }
    }
    else // (_FuncIndex != -1)
    {
      if(_CurrParamIndex < _FuncInfo[_FuncIndex].NumParams)
      {
	if(!_TypeMatch
	   (param, 
	    _FuncInfo[_FuncIndex].ParamInfo[_CurrParamIndex].Spec,
	    (_FuncInfo[_FuncIndex].Flag & funcLISTABLE) ))
	{
	  _ErrorMessage(_StdErr, 26, _CurrParamIndex + 1, _FuncName,
			_FuncInfo[_FuncIndex].ParamInfo[_CurrParamIndex].Name,
			PortionSpecToText(_FuncInfo[_FuncIndex].
					  ParamInfo[_CurrParamIndex].Spec),
			PortionSpecToText(param->Spec()));
	  _ErrorOccurred = true;
	  delete param;
	  return false;
	}
      }
      else
      {
	_ErrorMessage(_StdErr, 4, 0, _FuncName);
	_ErrorOccurred = true;
	delete param;
	return false;
      }
    }
  }
  assert(_Param[_CurrParamIndex] == 0);
  _Param[_CurrParamIndex] = param;
  _RunTimeParamInfo[_CurrParamIndex].Defined = true;
  _RunTimeParamInfo[_CurrParamIndex].AutoValOrRef = auto_val_or_ref;
  _RunTimeParamInfo[_CurrParamIndex].Ref = ref_param;
  _CurrParamIndex++;
  _NumParamsDefined++;


  return true;
}


ReferencePortion* CallFuncObj::GetParamRef(int index) const
{
  return _RunTimeParamInfo[index].Ref;
}



Portion* CallFuncObj::CallFunction(GSM* gsm, Portion **param)
{
  int index;
  int f_index;
  int curr_f_index = 0;
  int param_sets_matched;
  Portion* result = 0;

  int param_upper_bound;
  bool match_ok;


  // Attempt to identify the function being called out of all the
  // overloaded versions.
  
  if (_FuncIndex == -1 && _NumFuncs == 1)
    _FuncIndex = 0;
  
  if (_FuncIndex == -1) {
    param_upper_bound = 0;
    for (index = 0; index < _NumParams; index++) {
      if(_Param[index] != 0 || _RunTimeParamInfo[index].Ref != 0)
	param_upper_bound = index;
    }
    
    param_sets_matched = 0;
    for (f_index = 0; f_index < _NumFuncs; f_index++)  {
      match_ok = true;
      if(param_upper_bound >= _FuncInfo[f_index].NumParams)
	match_ok = false;

      if(!_FuncMatch[f_index])
	match_ok = false;

      for (index = 0; index < _FuncInfo[f_index].NumParams; index++)   {
	if(_Param[index] != 0) {
	  // parameter is defined
	  if(!_TypeMatch(_Param[index],
			 _FuncInfo[f_index].ParamInfo[index].Spec,
			 (_FuncInfo[f_index].Flag & funcLISTABLE)))
	    match_ok = false;
	}
	else {
	  // parameter is undefined
	  if(_RunTimeParamInfo[index].Ref != 0) {
	    // specified undefined variable
	    if(!_FuncInfo[f_index].ParamInfo[index].PassByReference)
	      match_ok = false;
	  }
	  
	  if(_FuncInfo[f_index].ParamInfo[index].DefaultValue == 0)
	    match_ok = false;
	}
      }
      
      if(match_ok) {
	curr_f_index = f_index;
	param_sets_matched++;
      }
    }

    if(param_sets_matched == 1) {
      _FuncIndex = curr_f_index;
    }
    else {
      if(param_sets_matched > 1)
	_ErrorMessage(_StdErr, 5, 0, _FuncName);
      else
	_ErrorMessage(_StdErr, 8, 0, _FuncName);
      _ErrorOccurred = true;
    }
  }
  
  if (_FuncIndex != -1) {
    for(index = 0; index < _FuncInfo[_FuncIndex].NumParams; index++) {
      if(_Param[index] != 0) {
	if(!_TypeMatch(_Param[index], 
		       _FuncInfo[_FuncIndex].ParamInfo[index].Spec,
		       (_FuncInfo[_FuncIndex].Flag & funcLISTABLE))) {
	  _ErrorMessage(_StdErr, 7, index + 1, _FuncName, 
			_FuncInfo[_FuncIndex].ParamInfo[index].Name);
	  _ErrorOccurred = true;
	}
      }
    }
  }


  // at this point _FuncIndex should be defined; i.e. the function
  // matching stage is done.  Now to weed out some particular errors:

  if(!_ErrorOccurred)
  {
    for(index = 0; index < _FuncInfo[_FuncIndex].NumParams; index++)
    {
      if(!_RunTimeParamInfo[index].Defined)
      {
	assert(_Param[index] == 0);
	if(_FuncInfo[_FuncIndex].ParamInfo[index].DefaultValue != 0)
	{
	  _Param[index] = 
	    _FuncInfo[_FuncIndex].ParamInfo[index].DefaultValue->ValCopy();
	}
	else
	{
	  _ErrorMessage(_StdErr, 9, index + 1, _FuncName, 
			_FuncInfo[_FuncIndex].ParamInfo[index].Name);
	  _ErrorOccurred = true;
	}
      }
      else // (_RunTimeParamInfo[index].Defined)
      {
	if(_Param[index] == 0)
	{
	  if(_FuncInfo[_FuncIndex].ParamInfo[index].DefaultValue == 0)
	  {	    
	    if(_FuncInfo[_FuncIndex].UserDefined)
	    {
	      // default values for undefined variables
	      switch(_FuncInfo[_FuncIndex].ParamInfo[index].Spec.Type)
	      {
	      case porBOOL:
		_Param[index] = new BoolPortion(false);
		_RunTimeParamInfo[index].Defined = true;
		break;
	      case porNUMBER:
		_Param[index] = new NumberPortion(0);
		_RunTimeParamInfo[index].Defined = true;
		break;
	      case porTEXT:
		_Param[index] = new TextPortion("");
		_RunTimeParamInfo[index].Defined = true;
		break;
	      default:
		_ErrorMessage(_StdErr, 9, index + 1, _FuncName, 
			      _FuncInfo[_FuncIndex].ParamInfo[index].Name);
		_ErrorOccurred = true;
	      }
	    }
	    else if(_FuncInfo[_FuncIndex].ParamInfo[index].Spec.Type != 
		    porUNDEFINED)
	    {
	      _ErrorMessage(_StdErr, 9, index + 1, _FuncName, 
			    _FuncInfo[_FuncIndex].ParamInfo[index].Name);
	      _ErrorOccurred = true;
	    }
	  }
	  else if(!_FuncInfo[_FuncIndex].ParamInfo[index].PassByReference)
	  {
	    _ErrorMessage(_StdErr, 10, index + 1, _FuncName, 
			  _FuncInfo[_FuncIndex].ParamInfo[index].Name);
	    _ErrorOccurred = true;
	  }
	  else if(_RunTimeParamInfo[index].Ref == 0)
	  {
	    _ErrorMessage(_StdErr, 11, index + 1, _FuncName, 
			  _FuncInfo[_FuncIndex].ParamInfo[index].Name);
	    _ErrorOccurred = true;
	  }
	  else if(_RunTimeParamInfo[index].Ref != 0)
	  {
	    _Param[index] = 
	      _FuncInfo[_FuncIndex].ParamInfo[index].DefaultValue->
		ValCopy();
	  }
	}
      }
    }
  }




  // aborts when a value paremeter is passed by reference or
  //        when a reference parameter is passed by value

  if(!_ErrorOccurred)
  {
    for(index = 0; index < _FuncInfo[_FuncIndex].NumParams; index++)
    {
      if(_Param[index] != 0 &&
	 _RunTimeParamInfo[index].Defined)
      {
	if(!_FuncInfo[_FuncIndex].ParamInfo[index].PassByReference &&
	   _Param[index]->IsReference())
	{
	  if(_RunTimeParamInfo[index].AutoValOrRef)
	  {
	    Portion* old = _Param[index];
	    _Param[index] = old->ValCopy();
	    delete old;
	  }
	  else
	  {
	    _ErrorMessage(_StdErr, 12, index + 1, _FuncName, 
			  _FuncInfo[_FuncIndex].ParamInfo[index].Name);
	    _ErrorOccurred = true;
	  }
	}
      }
    }
  }



  // This section makes the actual function call

  // first check for null objects; if any present in arguments, then
  // the function is not called.

  int i;

  bool null_call = false;
  if(!_ErrorOccurred)
  {
    for(i = 0; i < _FuncInfo[_FuncIndex].NumParams; i++)
      if(_Param[i])
	if(_Param[i]->Spec().Null && 
	   !_FuncInfo[_FuncIndex].ParamInfo[i].Spec.Null)
	{
	  null_call = true;
	  break;
	}
  }
  
  bool list_op;
  list_op = false;  
  if(!_ErrorOccurred)
  {
    for(i = 0; i < _FuncInfo[_FuncIndex].NumParams; i++)
      if(_ListNestedCheck(_Param[i], _FuncInfo[_FuncIndex].ParamInfo[i]))
      {
	list_op = true;
	break;
      }

    if(null_call) // if null objects are present in the argument list
    {
      throw gclRuntimeError("Null argument encountered");
    }
    else if(!list_op || !(_FuncInfo[_FuncIndex].Flag & funcLISTABLE))
    {
      // normal func call
      result = CallNormalFunction( gsm, _Param );
    }
    else { // listed function call
      try {
	result = CallListFunction(gsm, _Param);
      }
      catch (gclRuntimeError &E) {
	_StdErr << _FuncName << "[]: ";
	_StdErr << E.Description() << '\n';
	_ErrorOccurred = true;
	throw gclRuntimeError("");
      }
    }

    if(result == 0)
    {
      _ErrorMessage(_StdErr, 20, 0, _FuncName);
      _ErrorOccurred = true;
      throw gclRuntimeError("");
    }
    else if(result->Spec().Type == porUNDEFINED &&
	    ((list_op && result->Spec().ListDepth > 
	      _FuncInfo[_FuncIndex].ReturnSpec.ListDepth) ||
	     (!list_op && result->Spec().ListDepth == 
	      _FuncInfo[_FuncIndex].ReturnSpec.ListDepth)))
    {
      assert(result->Spec().ListDepth > 0);
      ((ListPortion*) result)->
	SetDataType(_FuncInfo[_FuncIndex].ReturnSpec.Type);      
    }
    else if(!_TypeMatch(result, _FuncInfo[_FuncIndex].ReturnSpec, 
			list_op && (_FuncInfo[_FuncIndex].Flag & funcLISTABLE),
			true))
    {
      _ErrorMessage(_StdErr, 28, 0, _FuncName, 
		    PortionSpecToText(_FuncInfo[_FuncIndex].ReturnSpec),
		    PortionSpecToText(result->Spec()));		    
      delete result;
      _ErrorOccurred = true;
      throw gclRuntimeError("");
    }
  }



  if(!_ErrorOccurred)
  {
    for(index = 0; index < _NumParams; index++)
    {
      if(!_FuncInfo[_FuncIndex].ParamInfo[index].PassByReference)
      {
	delete _RunTimeParamInfo[index].Ref;
	_RunTimeParamInfo[index].Ref = 0;
      }
      if(_RunTimeParamInfo[index].Ref == 0)
      {
	delete _Param[index];
	_Param[index] = 0;
      }
    }
  }
  else
  {
    for(index = 0; index < _NumParams; index++)
    {
      delete _Param[index];
      _Param[index] = 0;
      delete _RunTimeParamInfo[index].Ref;
      _RunTimeParamInfo[index].Ref = 0;
    }
  }


  for(index = 0; index < _NumParams; index++)
  {
    param[index] = _Param[index];
  }    

  if(result == 0)
    throw gclRuntimeError("");


  return result;
}





gText CallFuncObj::_ParamName(const int index) const
{
  gText param_name;
  int f_index;

  if(_FuncIndex != -1 && _CurrParamIndex < _FuncInfo[_FuncIndex].NumParams)
    return _FuncInfo[_FuncIndex].ParamInfo[_CurrParamIndex].Name;
  
  // check whether all index'th parameters have the same name
  param_name = "";
  for(f_index = 0; f_index < _NumFuncs; f_index++)
  {
    if(index < _FuncInfo[f_index].NumParams)
    {
      if(_FuncInfo[f_index].ParamInfo[index].Name != param_name)
      {
	if(param_name == "")
	{
	  param_name = _FuncInfo[f_index].ParamInfo[index].Name;
	}
	else
	{
	  param_name = "";
	  break;
	}
      }
    }
  }

  if(param_name != "")
    return param_name;
  else
    return "";
}


void CallFuncObj::Dump(gOutput& f) const
{
  if(_FuncIndex < 0)
    f << FuncName() << "[]\n";
  else
    FuncDescObj::Dump(f, _FuncIndex);
}



void CallFuncObj::_ErrorMessage
(
 gOutput& s,
 const int error_num, 
 const long& num1,
 const gText& str1,
 const gText& str2,
 const gText& str3,
 const gText& str4
)
{
#if 0
  s << "CallFuncObj Error " << error_num << " : \n";
#endif // 0

  s << "GCL: ";
  
  switch(error_num)
  {
  case 2:
    s << str1 << "[] called with conflicting parameters\n";
    break;
  case 3:
    s << str1 << "[] called with parameter #" << num1;
    if(str2 != "") s << ", \"" << str2 << "\",";
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
    if(str2 != "") s << ", \"" + str2 + "\"";
    s << "\n";
    break;
  case 26:
    s << str1 << "[]: Type mismatch on parameter #" << num1 << ", \"";
    s << str2 << "\"; expected " << str3 << ", got " << str4 << "\n";
    break;
  case 27:
    s << "Error occurred at element #" << num1;
    s << " during listed function call to " << str1 << "[]\n";
    break;
  case 28:
    s << "Function " << str1 << "[] return type does not match declaration;\n";
    s << "Expected " << str2 << ", got " << str3 << "\n";
    break;
  case 29:
    s << "Function " << str1 << "[] parameters do not belong to same game\n";
    s << "Error at parameter #" << num1 << '\n';
    break;
  case 30:
    s << "Function " << str1 << "[] parameters do not have the same subtype\n";
    s << "Error at parameter #" << num1 << '\n';
    break;
  case 31:
    s << "Function " << str1 << "[] called with a list of mixed types\n";
    s << "Error at parameter #" << num1 << '\n';
    break;
  default:
    s << "General Error #" << error_num << '\n';
    break;
  }
}

