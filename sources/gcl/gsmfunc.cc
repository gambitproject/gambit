//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementing predefined GCL functions
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <ctype.h>

#include "base/base.h"
#include "gsmfunc.h"

#include "gsm.h"
#include "portion.h"
#include "gsmhash.h"
#include "gsminstr.h"

#include "game/nfg.h"
#include "game/efg.h"
#include "game/nfstrat.h"
#include "game/efstrat.h"

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





//---------------------------------------------------------------
//                      gclParameter
//---------------------------------------------------------------

gclParameter::gclParameter(void)
  : Name(""), DefaultValue(REQUIRED), PassByReference(BYVAL)
{ }

gclParameter::gclParameter(const gclParameter& paraminfo)
  : Name(paraminfo.Name), Spec(paraminfo.Spec), 
    DefaultValue(paraminfo.DefaultValue), 
    PassByReference(paraminfo.PassByReference)
{ }

gclParameter::gclParameter(const gText& name, const PortionSpec& spec,
			     Portion* default_value, const bool pass_by_ref)
  : Name(name), Spec(spec), DefaultValue(default_value), 
    PassByReference(pass_by_ref)
{ }

gclParameter::~gclParameter()
{ }


//---------------------------------------------------------------------
//                     gclSignature
//---------------------------------------------------------------------

gclSignature::gclSignature(void)
  : UserDefined(false), FuncPtr(0), ReturnSpec(PortionSpec(porUNDEFINED)),
    Flag(funcLISTABLE), NumParams(0), ParamInfo(0)
{ }

gclSignature::gclSignature(const gclSignature& funcinfo)
  : UserDefined(funcinfo.UserDefined), ReturnSpec(funcinfo.ReturnSpec),
    Flag(funcinfo.Flag), NumParams(funcinfo.NumParams), Desc(funcinfo.Desc)
{
  if(!UserDefined)
    FuncPtr = funcinfo.FuncPtr;
  else
    FuncInstr = funcinfo.FuncInstr;
  ParamInfo = new gclParameter[NumParams];
  for (int i = 0; i < NumParams; i++)
    ParamInfo[i] = funcinfo.ParamInfo[i];
}

gclSignature::gclSignature(Portion *(*funcptr)(GSM &, Portion **),
			   PortionSpec returnspec, int numparams,
			   gclParameter* paraminfo, FuncFlagType flag)
  : UserDefined(false), FuncPtr(funcptr), ReturnSpec(returnspec),
    Flag(flag), NumParams(numparams)
{
  ParamInfo = new gclParameter[NumParams];
  if (paraminfo)
    for (int i = 0; i < NumParams; i++)
      ParamInfo[i] = paraminfo[i];
}

gclSignature::gclSignature(gclExpression* funcinstr,
			   PortionSpec returnspec,
			   int numparams,
			   gclParameter* paraminfo,
			   FuncFlagType flag)
  : UserDefined(true), FuncInstr(funcinstr), ReturnSpec(returnspec),
    Flag(flag), NumParams(numparams)
{
  ParamInfo = new gclParameter[NumParams];
  if (paraminfo)
    for (int i = 0; i < NumParams; i++)
      ParamInfo[i] = paraminfo[i];
}

gclSignature::~gclSignature()
{ }

//---------------------------------------------------------------------
//                   Function descriptor objects
//---------------------------------------------------------------------

RefCountHashTable< gclExpression* > gclFunction::_RefCountTable;


gclFunction::gclFunction(gclFunction &func)
  : m_environment(func.m_environment)
{
  int index;
  int f_index;

  _FuncName  = func._FuncName;
  _NumFuncs  = func._NumFuncs;
  _FuncInfo  = new gclSignature[_NumFuncs];

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
      new gclParameter[_FuncInfo[f_index].NumParams];

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


gclFunction::gclFunction(GSM &p_environment,
			 const gText& func_name, int numfuncs)
  : _FuncName(func_name), _NumFuncs(numfuncs), m_environment(p_environment)
{
  _FuncInfo = new gclSignature[_NumFuncs];
}

// Assumes one argument, which has a prototype func_proto
gclFunction::gclFunction(GSM &p_environment, const gText& func_proto, 
                         Portion *(*funcptr)(GSM &, Portion**),
                         FuncFlagType FFT /* = funcLISTABLE */)
  : m_environment(p_environment)
{
  _NumFuncs = 1;
  _FuncInfo = new gclSignature[1];
  
  unsigned int index = 0;
  gText func_name;
  char ch = func_proto[index++];

  while (isalpha(ch)) {
    func_name += ch;
    ch=func_proto[index++];
  }

  _FuncName = func_name;

  SetFuncInfo(0, func_proto, funcptr, FFT);
}


gclFunction::~gclFunction()
{
  int index;
  int f_index;
  
  for(f_index = 0; f_index < _NumFuncs; f_index++)
  {
    for(index = 0; index < _FuncInfo[f_index].NumParams; index++) {
      delete _FuncInfo[f_index].ParamInfo[index].DefaultValue;
    }
    if(_FuncInfo[f_index].UserDefined) {
      if (!_RefCountTable.IsDefined(_FuncInfo[f_index].FuncInstr) ||
	  _RefCountTable(_FuncInfo[f_index].FuncInstr) <= 0)
	throw gclRuntimeError("Internal GCL error");
      _RefCountTable(_FuncInfo[f_index].FuncInstr)--;
      if (_RefCountTable(_FuncInfo[f_index].FuncInstr) == 0) {
	_RefCountTable.Remove(_FuncInfo[f_index].FuncInstr);
	delete _FuncInfo[f_index].FuncInstr;
      }
    }
    delete[] _FuncInfo[f_index].ParamInfo;
  }
  delete[] _FuncInfo;
}


void gclFunction::SetFuncInfo(int funcindex, gclSignature funcinfo)
{
  if ((funcindex < 0) || (funcindex >= _NumFuncs))
    throw gclRuntimeError("Internal GCL error");
  _FuncInfo[funcindex] = funcinfo;
  if(funcinfo.UserDefined)
    if(!_RefCountTable.IsDefined(funcinfo.FuncInstr))
      _RefCountTable.Define(funcinfo.FuncInstr, 1);
    else
      _RefCountTable(funcinfo.FuncInstr)++;
}

void gclFunction::SetFuncInfo(int funcindex, const gText& s)
{
  SetFuncInfo(funcindex, s, 0);
}

  // In this function, the input string "s" is parsed, and the info contained 
  // in it is used to determine the function info, and then SetFuncInfo 
  // is called with all data passed as arguments rather than a string.

void gclFunction::SetFuncInfo(int funcindex, const gText& s,
                              Portion* (*funcptr)(GSM &, Portion **), 
                              FuncFlagType FFT /* = funcLISTABLE */)
{
  char ch = ' ';
  unsigned int index=0, length=s.Length();
  int numArgs=0;
  bool done = false;
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
    bool required = true;
    if (ch == '{')  {  // If the argument is optional
      ch=s[index++];
      required = false;
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

    if (ch == ']')  {  // If we hit the end (this should NEVER happen)
      throw gclRuntimeError("Bad function signature");
    }
    else {
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
        
        /*_gsm->OutputStream() << "Default: " << word << "\n";*/

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
        else if (word == "StdOut") {
          OutputPortion* tmp = new OutputPortion(m_environment.OutputStream());
          reqList.Append((Portion*) tmp);
          word = "OUTPUT";
        }
        else if (word == "NullOut") {
          OutputPortion* tmp = new OutputPortion(*new gNullOutput);
          reqList.Append((Portion*)tmp);
          word = "OUTPUT";
        }
        else if (word == "StdIn") {
          InputPortion* tmp = new InputPortion(*new gStandardInput);
          reqList.Append((Portion*)tmp);
          word = "INPUT";
        }
          // If it is a text string (for now just assume it is if it begins '"')
        else if (word[0u] == '"')
        {
          gText* tmp = new gText(word);
          reqList.Append((Portion*)tmp);
          word = "TEXT";
        }
          // If it is a number (int or double)
        else if ((word[0u] >= '0' && word[0u] <= '9') || word[0u] == '-') {
          int sign = 1;
          unsigned int index2 = 0;
          bool isDouble = false;
          /*gInteger num = 0, denom = 1;*/
          gNumber* num = new gNumber(0);
        
          char cha = word[index2++];
        
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

        else {
	  m_environment.OutputStream() << "Unknown optional type!!\n\n";
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
        index++;
        ch=s[index++];
      }
      else
        done = true;

    } // if not done (didn't hit ], and we found a ',')
  }  // while not done

    // Move ch to point to first char of return type
  while (ch != ':' && index<=length)
    ch=s[index++];

  index++;
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
  /*_gsm->OutputStream() << "\nReturn Type: " << word << "\n";*/
  /*_gsm->OutputStream() << "Return listNum: " << listNum << "\n";*/
  /*_gsm->OutputStream() << "SpecList: \n";*/
  /*specList.Dump(_gsm->OutputStream());*/
  /*_gsm->OutputStream() << "NameList: \n";*/
  /*nameList.Dump(_gsm->OutputStream());*/
  /*_gsm->OutputStream() << "ListList: \n";*/
  /*listList.Dump(_gsm->OutputStream());*/
  /*_gsm->OutputStream() << "RefList: \n";*/
  /*refList.Dump(_gsm->OutputStream());*/
  /*int rl = reqList.Length();*/
  /*_gsm->OutputStream() << "ReqL Length: " << rl << "\n";*/
  /*_gsm->OutputStream() << "NumArgs: " << numArgs << "\n";*/
  /*_gsm->OutputStream() << "\n\n";*/

  
  gclParameter *PIT = new gclParameter[numArgs];

  for (int i=1;i<=numArgs;i++)
  {
    PIT[i-1] = gclParameter(nameList[i], ToSpec(specList[i], listList[i]),
                                  reqList[i], refList[i]);
  }

  SetFuncInfo(funcindex, 
              gclSignature(funcptr, ToSpec(word, listNum), numArgs, PIT, FFT));
}

  // Replaces strings with their enumerated types.
PortionSpec ToSpec(gText &str, int num /* =0 */)
{
  /*_gsm->OutputStream() << "ToSpec called with " << str << " and " << num << ".\n";*/
  if (str == "BOOLEAN") 
    return PortionSpec(porBOOLEAN, num); 
  else if (str == "INTEGER")
    return PortionSpec(porINTEGER, num);
  else if (str == "NUMBER")
    return PortionSpec(porNUMBER, num);
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


  else if (str == "BOOLEAN*")
    return PortionSpec(porBOOLEAN, num, porNULLSPEC);
  else if (str == "INTEGER*")
    return PortionSpec(porINTEGER, num, porNULLSPEC);
  else if (str == "NUMBER*")
    return PortionSpec(porNUMBER, num, porNULLSPEC);
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

void gclFunction::SetParamInfo(int funcindex, int index, 
			       const gclParameter param)
{
  if ((funcindex < 0) || (funcindex >= _NumFuncs))
    throw gclRuntimeError("Internal GCL error");
  if ((index < 0) || (index >= _FuncInfo[funcindex].NumParams))
    throw gclRuntimeError("Internal GCL error");
  _FuncInfo[funcindex].ParamInfo[index] = param;
}

void gclFunction::SetParamInfo(int funcindex, const gclParameter params[])
{
  if ((funcindex < 0) || (funcindex >= _NumFuncs))
    throw gclRuntimeError("Internal GCL error");
  for (int i = 0; i < _FuncInfo[funcindex].NumParams; i++)
    _FuncInfo[funcindex].ParamInfo[i] = params[i];
}



bool gclFunction::Combine(gclFunction* newfunc)
{
  bool finalresult = true;
  bool same_params;
  int i;
  int j;
  int f_index;
  int index;
  gStack<int> delete_stack;

  for (i = 0; i < newfunc->_NumFuncs; i++) {
    bool result = true;
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

      if (same_params) {
	if (!_FuncInfo[f_index].UserDefined) {
	  m_environment.ErrorStream() << "Function ambiguous with " << FuncList()[f_index+1] <<'\n';
	  result = false;
	}
	else {
	  delete_stack.Push(f_index);
	}
      }
    }

    if (result) {
      while(delete_stack.Depth() > 0) {
	int delete_index = delete_stack.Pop();
	m_environment.ErrorStream() << "Replacing " << FuncList()[delete_index+1] << '\n';
	Delete(delete_index);
      }


      gclSignature* NewFuncInfo = new gclSignature[_NumFuncs+1];
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


bool gclFunction::Delete(gclFunction* newfunc)
{
  bool finalresult = true;
  bool same_params;
  int i;
  int f_index;
  int delete_index = 0;
  int index;

  for (i = 0; i < newfunc->_NumFuncs; i++) {
    bool result = false;
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


void gclFunction::Delete(int delete_index)
{
  gclSignature* NewFuncInfo = new gclSignature[_NumFuncs-1];  
  int j;
  for(j=0; j<delete_index; j++)
    NewFuncInfo[j] = _FuncInfo[j];
  for(j=delete_index+1; j<_NumFuncs; j++)
    NewFuncInfo[j-1] = _FuncInfo[j];
  delete[] _FuncInfo;
  _FuncInfo = NewFuncInfo;
  _NumFuncs--;
}


gText gclFunction::FuncName(void) const
{ return _FuncName; }



bool gclFunction::UDF(void) const
{
  for (int i = 0; i < _NumFuncs; i++) {
    if (_FuncInfo[i].UserDefined)
      return true;
  }
  return false;
}



bool gclFunction::BIF(void) const
{
  for (int i = 0; i < _NumFuncs; i++)  {
    if( !_FuncInfo[i].UserDefined )
      return true;
  }
  return false;
}



gList<gText> gclFunction::FuncList( bool udf, bool bif, bool getdesc ) const
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

void gclFunction::Dump(gOutput& f, int i) const
{ f << FuncList()[i+1] << '\n'; }

void gclFunction::Dump(gOutput& f) const
{
  for (int i = 0; i < _NumFuncs; Dump(f, i++));
}





//---------------------------------------------------------------------
//                      CallFuncObj
//---------------------------------------------------------------------


CallFuncObj::CallFuncObj(gclFunction *p_function,
			 int p_line, const gText &p_file)
  : gclFunction(*p_function), m_line(p_line), m_file(p_file),
    m_funcIndex(-1), m_numParams(0), m_numParamsDefined(0)
{
  for (int f_index = 0; f_index < _NumFuncs; f_index++)  {
    if (m_numParams < _FuncInfo[f_index].NumParams)
      m_numParams = _FuncInfo[f_index].NumParams;
  }

  m_params = new Portion*[m_numParams];
  m_runTimeParamInfo = new RunTimeParamInfoType[m_numParams];
  m_funcMatch = new bool[_NumFuncs];
  m_currParamIndex = 0;

  for (int f_index = 0; f_index < _NumFuncs; f_index++)
    m_funcMatch[f_index] = true;
  for (int index = 0; index < m_numParams; index++)  {
    m_params[index] = REQUIRED;
    m_runTimeParamInfo[index].Ref = 0;
    m_runTimeParamInfo[index].Defined = false;
    m_runTimeParamInfo[index].AutoValOrRef = false;
  }
}


CallFuncObj::~CallFuncObj()
{
  for (int index = 0; index < m_numParams; index++)  { 
    delete m_runTimeParamInfo[index].Ref;
    delete m_params[index];
  }
  delete[] m_funcMatch;
  delete[] m_runTimeParamInfo;
  delete[] m_params;
}

//--------------------------
// List-related functions
//--------------------------

static bool ListDimMatch(ListPortion* p1, ListPortion* p2)
{
  if (p1->Length() != p2->Length())
    return false;
  else {
    for (int i = 1; i <= p1->Length(); i++) {
      Portion *s1 = (*p1)[i];
      Portion *s2 = (*p2)[i];
      if (s1->Spec().ListDepth > 0 && s2->Spec().ListDepth > 0) {
	if (!ListDimMatch((ListPortion*) s1, (ListPortion*) s2))
	  return false;
      }
    }
  }
  return true;
}


static bool ListNestedCheck(Portion* p, const gclParameter& info)
{
  if (p == 0 && info.Spec.Type == porUNDEFINED)
    return false;
  else if (p->Spec().ListDepth > info.Spec.ListDepth)
    return true;
  else
    return false;
}


Portion *CallFuncObj::CallNormalFunction(GSM *gsm, Portion **param)
{
  try {
    if (!_FuncInfo[m_funcIndex].UserDefined)
      return _FuncInfo[m_funcIndex].FuncPtr(*gsm, param);
    else 
      return gsm->ExecuteUserFunc(*(_FuncInfo[m_funcIndex].FuncInstr), 
				  _FuncInfo[m_funcIndex], param,
				  FuncName() + gText((char) (m_funcIndex+1)));
  }
  catch (gclQuitOccurred &) {
    throw;
  }
  catch (...) {
    gsm->OutputStream() << "In function " << _FuncName 
			<< "[], in file \"" << m_file;
    gsm->OutputStream() << "\" at line " << ToText(m_line) << ":\n";
    throw;
  }
}

Portion *CallFuncObj::CallListFunction(GSM* gsm, Portion **ParamIn)
{
  Portion* result;
  int NumParams = _FuncInfo[m_funcIndex].NumParams;
  bool *Listed = new bool[NumParams];
  ListPortion* Source = 0;  // source to look at to create dimensionality
  bool recurse;

  // mark down the listed parameters
  for (int i = 0; i < NumParams; i++) {
    Listed[i] =
      ListNestedCheck(ParamIn[i], _FuncInfo[m_funcIndex].ParamInfo[i]);
    if (Listed[i])
      Source = (ListPortion*) ParamIn[i];
  }

  // check that all dimensionalities match
  for (int i = 0; i < NumParams; i++)  {
    for (int j = i + 1; j < NumParams; j++)  {
      if (Listed[i] && Listed[j]) {
	if (ParamIn[i]->Spec().ListDepth > 0 && 
	    ParamIn[j]->Spec().ListDepth > 0)
	  if (!ListDimMatch((ListPortion *) ParamIn[i],
			    (ListPortion *) ParamIn[j])) {
	    delete [] Listed;
	    throw gclRuntimeError("Mismatched dimensionalities");
	  }
      }
    }
  }


  Portion **CurrParam = new Portion *[NumParams];
  ListPortion *p = new ListPortion;

  // 'i' is the index for the lists
  for (int i = 1; i <= Source->Length(); i++)   {
    recurse = false;
    // while j is the param index
    // pickout the current element to process
    
    for (int j = 0; j < NumParams; j++)  {
      if (!Listed[j])
	CurrParam[j] = ParamIn[j]; 
      else {
	CurrParam[j] = (*(ListPortion*) ParamIn[j])[i];
	if (ListNestedCheck(CurrParam[j], 
			     _FuncInfo[m_funcIndex].ParamInfo[j]))
	  recurse = true;
      }
    }

    if (recurse) {
      result = CallListFunction(gsm, CurrParam);
    }
    else  {
      for (int j = 0; j < NumParams; j++)  {
	if (CurrParam[j]->IsReference()) {
	  CurrParam[j] = CurrParam[j]->RefCopy();
        }
	else  {
	  CurrParam[j] = CurrParam[j]->ValCopy();
        }
      }

      bool null_call = false;
      for (int j = 0; j < NumParams; j++)
	if (CurrParam[j]->Spec().Null && 
	    !_FuncInfo[m_funcIndex].ParamInfo[j].Spec.Null) {
	  null_call = true;
	  break;
	}

      if (!null_call) {
	try { 
	  result = CallNormalFunction(gsm, CurrParam);
	}
	catch (gclRuntimeError &E) {
	  throw gclRuntimeError(_FuncName + "[]: " + E.Description() + 
				"\nError occurred at element #" +
				ToText(p->Length() + 1) +
				" during listed function call to " +
				_FuncName);
	}
      }
      else if (null_call)  {
	throw gclRuntimeError("Null argument encountered");
      }
      else
	throw gclRuntimeError("Error in arguments");

      for (int j = 0; j < NumParams; j++)
	delete CurrParam[j];
    }

    if (result == 0)
      throw gclRuntimeError("");

    if (result->Spec().Type == porUNDEFINED &&
	((recurse && result->Spec().ListDepth > 
	  _FuncInfo[m_funcIndex].ReturnSpec.ListDepth) ||
	 (!recurse && result->Spec().ListDepth == 
	  _FuncInfo[m_funcIndex].ReturnSpec.ListDepth))) {
      ((ListPortion*) result)->
	SetDataType(_FuncInfo[m_funcIndex].ReturnSpec.Type);      
    }
    p->Append(result);


  }

  delete [] CurrParam;
  delete [] Listed;
  return p;
}


typedef enum { matchNONE = 0, matchSUPERTYPE = 1, matchEXACT = 2,
               matchSUPERLIST = 3, matchEXACTLIST = 4 } gclMatchLevel;

gclMatchLevel TypeMatch(Portion* p, PortionSpec ExpectedSpec, 
			bool Listable, bool return_type_check = false)
{
  if (p == 0 && ExpectedSpec.Type == porUNDEFINED)
    return matchEXACT;

  if (ExpectedSpec.Type == porANYTYPE && return_type_check)
    return matchEXACT;  

  bool nullPortion = false;

  PortionSpec CalledSpec = p->Spec();
  if (p->Spec().Type == porNULL) {
    CalledSpec = ((NullPortion*) p)->DataType();
    nullPortion = true;
  }

  gclMatchLevel matchtype = matchEXACT;

  if (CalledSpec.Type == porNUMBER && ExpectedSpec.Type == porINTEGER) {
    if (nullPortion) {
      if (!return_type_check) {
	matchtype = matchNONE;
      }
    }
    else if ((CalledSpec.ListDepth > 0 && ((ListPortion *) p)->IsInteger()) ||
	     (CalledSpec.ListDepth == 0 && ((NumberPortion *) p)->Value().IsInteger()))
      CalledSpec.Type = porINTEGER;
  }
  else if (CalledSpec.Type == porNUMBER && ExpectedSpec.Type == porNUMBER) {
    if (nullPortion) {
      if (!return_type_check) {
	matchtype = matchNONE;
      }
    }
    else if ((CalledSpec.ListDepth > 0 && ((ListPortion *) p)->IsInteger()) ||
	     (CalledSpec.ListDepth == 0 && ((NumberPortion *) p)->Value().IsInteger()))
      matchtype = matchSUPERTYPE;
  }

  if (CalledSpec.Type & ExpectedSpec.Type)  {
    if (CalledSpec.ListDepth == ExpectedSpec.ListDepth)
      return matchtype;
    else if (CalledSpec.ListDepth > ExpectedSpec.ListDepth && Listable) {
      if (matchtype == matchSUPERTYPE)
	return matchSUPERLIST;
      else if (matchtype == matchEXACT)
	return matchEXACTLIST;
      else
	return matchNONE;
    }
    else if (CalledSpec.ListDepth > 0 && ExpectedSpec.ListDepth == 1 && 
	    !Listable)
      return matchtype;
    else if (CalledSpec.ListDepth > 0 && ExpectedSpec.ListDepth == NLIST)
      return matchtype;
  }
  else if (CalledSpec.Type == porUNDEFINED && CalledSpec.ListDepth > 0)  {
    if (CalledSpec.ListDepth == 1 && ExpectedSpec.ListDepth > 0) 
      return matchtype;
  }

  return matchNONE;
}


bool CallFuncObj::SetCurrParamIndex(const gText& param_name)
{
  int TempFuncIndex = -1;
  int result = PARAM_NOT_FOUND;
  int times_found = 0;
  
  for (int f_index = 0; 
       f_index < _NumFuncs && result != PARAM_AMBIGUOUS;
       f_index++) {
    if (m_funcMatch[f_index])   {
      bool name_match_found = false;
      for (int index = 0; index < _FuncInfo[f_index].NumParams; index++) {
	if (_FuncInfo[f_index].ParamInfo[index].Name == param_name) {
	  name_match_found = true;
	  if (result == PARAM_NOT_FOUND) {
	    result = index;
	    TempFuncIndex = f_index;
	    times_found++;
	  }
	  else if(result == index) {
	    times_found++;
	  }
	  else { // (result != index)
	    result = PARAM_AMBIGUOUS;
	    break;
	  }
	}
      }

      if (!name_match_found)
	m_funcMatch[f_index] = false;
    }
  }
  
  
  if (times_found == 1) {
    if (m_funcIndex == -1) {
      m_funcIndex = TempFuncIndex;
    }
    else if (m_funcIndex != TempFuncIndex) {
      m_funcIndex = -1;
      throw gclRuntimeError(_FuncName +
			    "[] called with conflicting parameters");
    }
  }

  if (result == PARAM_NOT_FOUND)  {
    throw gclRuntimeError("Parameter \"" + param_name +
			  "\" is not defined for " + _FuncName + "[]"); 
  }
  else if (result == PARAM_AMBIGUOUS) {
    throw gclRuntimeError("Parameter \"" + param_name + 
			  "\" is ambiguous in " + _FuncName + "[]");
  }
  else {
    m_currParamIndex = result;
    return true;
  }
}

bool CallFuncObj::SetCurrParam(Portion *param, bool auto_val_or_ref)
{
  ReferencePortion* ref_param = 0;
  bool AllowUndefinedRef;
  int f_index;
  int funcs_matched;
  int last_match;

  // Parameter index out of bounds
  if (m_currParamIndex >= m_numParams)  {
    delete param;
    throw gclRuntimeError(_FuncName + "[] called with too many parameters");
  }

  // Repeated parameter definition
  if (m_runTimeParamInfo[m_currParamIndex].Defined)   {
    throw gclRuntimeError(_FuncName + "[] called with parameter #" +
			  ToText(m_currParamIndex) + " multiply defined");
  }

  // Passed an undefined variable
  if (param != 0 && param->Spec().Type == porREFERENCE)  {

    // check whether undefined variables are allowed
    AllowUndefinedRef = false;
    for (f_index = 0; f_index < _NumFuncs; f_index++) {
      if (m_currParamIndex < _FuncInfo[f_index].NumParams)  {
	if((_FuncInfo[f_index].ParamInfo[m_currParamIndex].PassByReference &&
	    _FuncInfo[f_index].ParamInfo[m_currParamIndex].DefaultValue != 0)|| 
	   _FuncInfo[f_index].UserDefined ||
	   _FuncInfo[f_index].ParamInfo[m_currParamIndex].Spec.Type == porUNDEFINED)
	  AllowUndefinedRef = true;
	else
	  m_funcMatch[f_index] = false;
      }
    }

    if (AllowUndefinedRef) {
      ref_param = (ReferencePortion*) param;
      param = 0;
    }
    else  {
      throw gclRuntimeError(_FuncName + "[]: Undefined reference \"" +
			    ((ReferencePortion *) param)->Value() +
			    "\" passed for parameter #" +
			    ToText(m_currParamIndex + 1)); 
    }
  }

  funcs_matched = 0;
  last_match = 0;
  for (f_index = 0; f_index < _NumFuncs; f_index++) {
    if (m_funcMatch[f_index]) {
      last_match = f_index;
      funcs_matched++;
    }
  }
  if (funcs_matched == 1)
    m_funcIndex = last_match;

  if (param != 0) {
    if (m_funcIndex == -1) {
      // Attempt on-the-fly function matching
      for (f_index = 0; f_index < _NumFuncs; f_index++) {
	if (m_currParamIndex < _FuncInfo[f_index].NumParams) {
	  if (TypeMatch(param, 
			_FuncInfo[f_index].ParamInfo[m_currParamIndex].Spec,
			(_FuncInfo[f_index].Flag & funcLISTABLE)) == matchNONE) {
	    m_funcMatch[f_index] = false;
	  }
	}
	else {
	  m_funcMatch[f_index] = false;
	}
      }
    }
    else { // (m_funcIndex != -1)
      if (m_currParamIndex < _FuncInfo[m_funcIndex].NumParams) {
	if (TypeMatch(param, 
		      _FuncInfo[m_funcIndex].ParamInfo[m_currParamIndex].Spec,
		      (_FuncInfo[m_funcIndex].Flag & funcLISTABLE)) == matchNONE) {
	  gText message = (_FuncName + "[]: Type mismatch on parameter #"+
			   ToText(m_currParamIndex + 1) + ", \"" +
			   _FuncInfo[m_funcIndex].ParamInfo[m_currParamIndex].Name +
			   "\"; expected " +
			   PortionSpecToText(_FuncInfo[m_funcIndex].
					     ParamInfo[m_currParamIndex].Spec) +
			   ", got ");
	  if (param->Spec().Type != porNULL)
	    message += PortionSpecToText(param->Spec());
	  else
	    message += "NULL(" + PortionSpecToText(((NullPortion *) param)->DataType()) + ")";
	  throw gclRuntimeError(message);
	}
      }
      else {
	throw gclRuntimeError(_FuncName + "[] called with too many parameters");
      }
    }
  }

  m_params[m_currParamIndex] = param;
  m_runTimeParamInfo[m_currParamIndex].Defined = true;
  m_runTimeParamInfo[m_currParamIndex].AutoValOrRef = auto_val_or_ref;
  m_runTimeParamInfo[m_currParamIndex].Ref = ref_param;
  m_currParamIndex++;
  m_numParamsDefined++;

  return true;
}


ReferencePortion* CallFuncObj::GetParamRef(int index) const
{
  return m_runTimeParamInfo[index].Ref;
}


// Attempt to identify the function being called out of all the
// overloaded versions.
void CallFuncObj::ComputeFuncIndex(void)
{
  int exact_index = 0, supertype_index = 0;
  bool exact_listmatch = false, supertype_listmatch = false;

  if (m_funcIndex == -1 && _NumFuncs == 1)
    m_funcIndex = 0;
  
  if (m_funcIndex == -1) {
    int param_upper_bound = -1;
    for (int index = 0; index < m_numParams; index++) {
      if (m_params[index] != 0 || m_runTimeParamInfo[index].Ref != 0)
	param_upper_bound = index;
    }
    
    int exact_matches = 0, supertype_matches = 0;
    for (int f_index = 0; f_index < _NumFuncs; f_index++)  {
      gclMatchLevel matchlevel = matchEXACT;
      if (param_upper_bound >= _FuncInfo[f_index].NumParams)
	matchlevel = matchNONE;

      if (!m_funcMatch[f_index])
      	matchlevel = matchNONE;

      for (int index = 0; (index < _FuncInfo[f_index].NumParams
                           && matchlevel != matchNONE); index++)   {
	if (m_params[index] != 0) {
	  // parameter is defined
	  gclMatchLevel parammatch = 
	    TypeMatch(m_params[index],
		      _FuncInfo[f_index].ParamInfo[index].Spec,
		      (_FuncInfo[f_index].Flag & funcLISTABLE));
	  if (parammatch != matchEXACT)
	    matchlevel = parammatch;
	}
	else {
	  // parameter is undefined
	  if (m_runTimeParamInfo[index].Ref != 0) {
	    // specified undefined variable
	    if(!_FuncInfo[f_index].ParamInfo[index].PassByReference)
	      matchlevel = matchNONE;
	  }
	  
	  if (_FuncInfo[f_index].ParamInfo[index].DefaultValue == 0 &&
	      _FuncInfo[f_index].ParamInfo[index].Spec.Type != porUNDEFINED)
	    matchlevel = matchNONE;
	}
      }
      
      if (matchlevel == matchEXACT) {
	if ((exact_matches == 0) ||
	    (exact_matches > 0 && exact_listmatch)) {
	  exact_matches = 1;
	  exact_index = f_index;
	  exact_listmatch = false;
	}
	else {
	  exact_index = f_index;
	  exact_matches++;
	  exact_listmatch = false;
	}
      }
      else if (matchlevel == matchEXACTLIST) {
	// listable matches are superseded by nonlistable ones
	if (exact_listmatch || exact_matches == 0) {
	  exact_index = f_index;
	  exact_matches++;
	  exact_listmatch = true;
	}
      }
      else if (matchlevel == matchSUPERTYPE) {
	if ((supertype_matches == 0) ||
	    (supertype_matches > 0 && supertype_listmatch)) {
	  supertype_matches = 1;
	  supertype_index = f_index;
	  supertype_listmatch = false;
	}
	else {
	  supertype_index = f_index;
	  supertype_matches++;
	  supertype_listmatch = false;
	}
      }
      else if (matchlevel == matchSUPERLIST) {
	// listable matches are superseded by nonlistable ones
	if (supertype_listmatch || supertype_matches == 0) {
	  supertype_index = f_index;
	  supertype_matches++;
	  supertype_listmatch = true;
	}
      }
    }

    if (exact_matches == 1)
      m_funcIndex = exact_index;
    else if (exact_matches > 1 ||
	     (exact_matches == 0 && supertype_matches > 1))
      throw gclRuntimeError(_FuncName + "[] called with ambiguous parameter(s)");
    else if (exact_matches == 0 && supertype_matches == 1)
      m_funcIndex = supertype_index;
    else
      throw gclRuntimeError("No matching parameter specifications found for " + _FuncName + "[]");
  }
  
  if (m_funcIndex != -1)  {
    for (int index = 0; index < _FuncInfo[m_funcIndex].NumParams; index++) {
      if (m_params[index] != 0) {
	if (TypeMatch(m_params[index],
		      _FuncInfo[m_funcIndex].ParamInfo[index].Spec,
		      (_FuncInfo[m_funcIndex].Flag & funcLISTABLE)) == matchNONE)
	  throw gclRuntimeError(_FuncName + "[] parameter #" +
				ToText(index + 1) + ", \"" +
				_FuncInfo[m_funcIndex].ParamInfo[index].Name +
				"\", type mismatch");
      }
    }
  }
}


Portion *CallFuncObj::CallFunction(GSM *gsm, Portion **param)
{
  int index;
  Portion* result;

  ComputeFuncIndex();

  // at this point m_funcIndex should be defined; i.e. the function
  // matching stage is done.  Now to weed out some particular errors:

  for (index = 0; index < _FuncInfo[m_funcIndex].NumParams; index++) {
    if (!m_runTimeParamInfo[index].Defined)  {
      if(_FuncInfo[m_funcIndex].ParamInfo[index].DefaultValue != 0) {
	m_params[index] = 
	  _FuncInfo[m_funcIndex].ParamInfo[index].DefaultValue->ValCopy();
      }
      else {
	throw gclRuntimeError(_FuncName + "[] required parameter #" +
			      ToText(index + 1) + ", \"" + 
			      _FuncInfo[m_funcIndex].ParamInfo[index].Name +
			      "\", missing");

      }
    }
    else { // (m_runTimeParamInfo[index].Defined)
      if (m_params[index] == 0) {
	if(_FuncInfo[m_funcIndex].ParamInfo[index].DefaultValue == 0) {
	  if(_FuncInfo[m_funcIndex].UserDefined) {
	    // default values for undefined variables
	    switch(_FuncInfo[m_funcIndex].ParamInfo[index].Spec.Type) {
	    case porBOOLEAN:
	      m_params[index] = new BoolPortion(triFALSE);
	      m_runTimeParamInfo[index].Defined = true;
	      break;
	    case porNUMBER:
	      m_params[index] = new NumberPortion(0);
	      m_runTimeParamInfo[index].Defined = true;
	      break;
	    case porTEXT:
	      m_params[index] = new TextPortion("");
	      m_runTimeParamInfo[index].Defined = true;
	      break;
	    default:
	      throw gclRuntimeError(_FuncName + "[] required parameter #" +
				    ToText(index + 1) + ", \"" + 
				    _FuncInfo[m_funcIndex].ParamInfo[index].Name +
				    "\", missing");
	    }
	  }
	  else if (_FuncInfo[m_funcIndex].ParamInfo[index].Spec.Type != 
		   porUNDEFINED) {
	    throw gclRuntimeError(_FuncName + "[] required parameter #" +
				  ToText(index + 1) + ", \"" + 
				  _FuncInfo[m_funcIndex].ParamInfo[index].Name +
				  "\", missing");
	  }
	}
	else if (!_FuncInfo[m_funcIndex].ParamInfo[index].PassByReference) {
	  throw gclRuntimeError(_FuncName + "[] value parameter #" +
				ToText(index + 1) + ", \"" +
				_FuncInfo[m_funcIndex].ParamInfo[index].Name +
				"\", undefined");
	}
	else if (m_runTimeParamInfo[index].Ref == 0) {
	  throw gclRuntimeError(_FuncName + "[] undefined parameter #" +
				ToText(index + 1) + ", \"" +
				_FuncInfo[m_funcIndex].ParamInfo[index].Name +
				"\", passed by value");
	}
	else if (m_runTimeParamInfo[index].Ref != 0) {
	  m_params[index] = 
	    _FuncInfo[m_funcIndex].ParamInfo[index].DefaultValue->ValCopy();
	}
      }
    }
  }


  // aborts when a value parameter is passed by reference or
  //        when a reference parameter is passed by value

  for (index = 0; index < _FuncInfo[m_funcIndex].NumParams; index++) {
    if (m_params[index] != 0 &&
	m_runTimeParamInfo[index].Defined) {
      if (!_FuncInfo[m_funcIndex].ParamInfo[index].PassByReference &&
	  m_params[index]->IsReference())	{
	if (m_runTimeParamInfo[index].AutoValOrRef)  {
	  Portion *old = m_params[index];
	  m_params[index] = old->ValCopy();
	  delete old;
	}
	else {
	  throw gclRuntimeError(_FuncName + "[] value parameter #" +
				ToText(index + 1) + ", \"" +
				_FuncInfo[m_funcIndex].ParamInfo[index].Name +
				"\", passed by reference");
	}
      }
    }
  }



  // This section makes the actual function call

  // first check for null objects; if any present in arguments, then
  // the function is not called.

  bool null_call = false;

  for (int i = 0; i < _FuncInfo[m_funcIndex].NumParams; i++) {
    if (m_params[i])
      if(m_params[i]->Spec().Null &&
	 !_FuncInfo[m_funcIndex].ParamInfo[i].Spec.Null)	{
	null_call = true;
	break;
      }
  }
  
  bool list_op = false;

  for (int i = 0; i < _FuncInfo[m_funcIndex].NumParams; i++)
    if (ListNestedCheck(m_params[i], _FuncInfo[m_funcIndex].ParamInfo[i])) {
      list_op = true;
      break;
    }

  if (null_call) { // if null objects are present in the argument list
    throw gclRuntimeError("Null argument encountered");
  }

  if (!list_op || !(_FuncInfo[m_funcIndex].Flag & funcLISTABLE)) {
      // normal func call
    result = CallNormalFunction( gsm, m_params );
  }
  else { // listed function call
    result = CallListFunction(gsm, m_params);
  }

  if (result == 0) {
    for (index = 0; index < m_numParams; index++)    {
      delete m_params[index];
      m_params[index] = 0;
      delete m_runTimeParamInfo[index].Ref;
      m_runTimeParamInfo[index].Ref = 0;
    }
    throw gclRuntimeError("A general error occurred while executing " +
			  _FuncName + "[]");
  }
  else if (result->Spec().Type == porUNDEFINED &&
	   ((list_op && result->Spec().ListDepth > 
	     _FuncInfo[m_funcIndex].ReturnSpec.ListDepth) ||
	    (!list_op && result->Spec().ListDepth == 
	     _FuncInfo[m_funcIndex].ReturnSpec.ListDepth))) {
    ((ListPortion*) result)->
      SetDataType(_FuncInfo[m_funcIndex].ReturnSpec.Type);      
  }
  else if (TypeMatch(result, _FuncInfo[m_funcIndex].ReturnSpec, 
		     list_op && (_FuncInfo[m_funcIndex].Flag & funcLISTABLE),
		     true) == matchNONE) {
    PortionSpec actualSpec = result->Spec();
    delete result;
    for (index = 0; index < m_numParams; index++)    {
      delete m_params[index];
      m_params[index] = 0;
      delete m_runTimeParamInfo[index].Ref;
      m_runTimeParamInfo[index].Ref = 0;
    }
    throw gclRuntimeError("Function " + _FuncName +
			  "[] return type does not match declaration;\n" +
			  "Expected " +  
			  PortionSpecToText(_FuncInfo[m_funcIndex].ReturnSpec) +
			  ", got " + PortionSpecToText(actualSpec));
  }

  for (index = 0; index < m_numParams; index++) {
    if (!_FuncInfo[m_funcIndex].ParamInfo[index].PassByReference) {
      delete m_runTimeParamInfo[index].Ref;
      m_runTimeParamInfo[index].Ref = 0;
    }
    if (m_runTimeParamInfo[index].Ref == 0) {
      delete m_params[index];
      m_params[index] = 0;
    }
  }

  for (index = 0; index < m_numParams; index++)  {
    param[index] = m_params[index];
  }    

  return result;
}





gText CallFuncObj::ParamName(const int index) const
{
  gText param_name;
  int f_index;

  if (m_funcIndex != -1 && m_currParamIndex < _FuncInfo[m_funcIndex].NumParams)
    return _FuncInfo[m_funcIndex].ParamInfo[m_currParamIndex].Name;
  
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
  if (m_funcIndex < 0)
    f << FuncName() << "[]\n";
  else
    gclFunction::Dump(f, m_funcIndex);
}



