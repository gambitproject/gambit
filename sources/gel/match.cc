//
// FILE: match.cc -- Function matching code
//
// $Id$
//

#include <ctype.h>

#include "match.h"
#include "funcmisc.h"

#include "gnumber.h"
#include "glist.h"
#include "garray.h"

gelBadFunctionSig::gelBadFunctionSig(const gText &s) : sig(s)   { }

gText gelBadFunctionSig::Description(void) const
{ return "Bad function signature: " + sig; }


static int NameToDepth( gText name )
{
  // this assumes that there are no spaces in name
  assert( !name.LastOccur( ' ' ) );
  int depth = 0;
  while( name.Length() >= 5 && name.Left( 5 ) == "LIST(" )
  {
    ++depth;
    name = name.Right( name.Length() - 5 );
    assert( name.Right( 1 ) == ")" );
    name = name.Left( name.Length() - 1 );
  }
  return depth;
}


static gelType NameToType( gText name )
{
  // this assumes that there are no spaces in name
  assert( !name.LastOccur( ' ' ) );
  while( name.Length() >= 5 && name.Left( 5 ) == "LIST(" )
  {
    name = name.Right( name.Length() - 5 );
    assert( name.Right( 1 ) == ")" );
    name = name.Left( name.Length() - 1 );
  }

  if (name == "NUMBER")
    return gelNUMBER;
  else if (name == "BOOLEAN")
    return gelBOOLEAN;
  else if (name == "TEXT")
    return gelTEXT;
  else if (name == "EFG")
    return gelEFG;
  else if (name == "NODE")
    return gelNODE;
  else if (name == "ACTION")
    return gelACTION;
  else if (name == "INFOSET")
    return gelINFOSET;
  else if (name == "EFPLAYER")
    return gelEFPLAYER;
  else if (name == "EFOUTCOME")
    return gelEFOUTCOME;
  else if (name == "EFSUPPORT")
    return gelEFSUPPORT;
  else if (name == "BEHAV")
    return gelBEHAV;
  else if (name == "NFG")
    return gelNFG;
  else if (name == "STRATEGY")
    return gelSTRATEGY;
  else if (name == "NFPLAYER")
    return gelNFPLAYER;
  else if (name == "NFOUTCOME")
    return gelNFOUTCOME;
  else if (name == "NFSUPPORT")
    return gelNFSUPPORT;
  else if (name == "MIXED")
    return gelMIXED;
  else if (name == "INPUT")
    return gelINPUT;
  else if (name == "OUTPUT")
    return gelOUTPUT;
  else if (name == "ANYTYPE")
    return gelANYTYPE;
  else
    return gelUNDEFINED;
}

gelParamInfo::gelParamInfo(const gText &s)
  : m_Optional(false), m_ByReference(false)
{
  int index = 0, length = s.Length();
  gText word = "";
  bool quote = false;
  
  if (length == 0)
    throw gelBadFunctionSig();
  
  if (s[index] == '{')   {
    m_Optional = true;
    ++index;
  }
  
  while (isalnum(s[index]))   {
    if (index >= length)
      throw gelBadFunctionSig();
    word += s[index++];
  }
  m_Name = word;

  if (index >= length)
    throw gelBadFunctionSig();
  
  if (s[index] == '<')   {
    m_ByReference = true;
    ++index;
  }
  
  if (index >= length || s[index++] != '-')
    throw gelBadFunctionSig();
  if (index >= length || s[index++] != '>')
    throw gelBadFunctionSig();
  
  word = "";
  quote = false;
  while (isalnum(s[index]) || 
	 (ispunct(s[index]) && s[index] != '}') ||
	 quote)   {
    if (index >= length)
      throw gelBadFunctionSig();
    if (s[index] == '\"') 
      quote = !quote;
    word += s[index++];
  }
  
  if (m_Optional)  {
    if (index >= length || s[index] != '}')
      throw gelBadFunctionSig();
    m_Default = word;
    if (m_Default == "True")  {
      m_DefaultVal.bval = new gTriState(triTRUE);
      m_Type = gelBOOLEAN;
    }
    else if (m_Default == "False")  {
      m_DefaultVal.bval = new gTriState(triFALSE);
      m_Type = gelBOOLEAN;
    }
    else if (m_Default == "Maybe")  {
      m_DefaultVal.bval = new gTriState(triMAYBE);
      m_Type = gelBOOLEAN;
    }
    else if (isdigit(m_Default[0]) || m_Default[0] == '-' || m_Default[0] == '.') {
      m_DefaultVal.nval = new gNumber;
      FromText(m_Default, *m_DefaultVal.nval);
    }
    else if (m_Default[0] == '"')  {
      m_DefaultVal.tval = new gText;
      for (int i = 1; i < m_Default.Length() - 1; i++)
        *m_DefaultVal.tval += m_Default[i];
    }
    else
      throw gelBadFunctionSig();
  }
  else
  {
    m_Type = NameToType( word );
    m_Depth = NameToDepth( word );
  }
}

gelParamInfo::~gelParamInfo()
{
  if (m_Optional)   {
    if (m_Type == gelBOOLEAN)
      delete m_DefaultVal.bval;
    else if (m_Type == gelNUMBER)
      delete m_DefaultVal.nval;
    else if (m_Type == gelTEXT)
      delete m_DefaultVal.tval;
  }
}

gelExpr *gelParamInfo::DefaultVal(void) const
{
  if (!m_Optional)
    throw gelInternalError("Asked for default value of required parameter");
  if (m_Type == gelBOOLEAN)
    return new gelConstant<gTriState *>(new gTriState(*m_DefaultVal.bval));
  else if (m_Type == gelNUMBER)
    return new gelConstant<gNumber *>(new gNumber(*m_DefaultVal.nval));
  else   /* if (m_Type == gelTEXT) */
    return new gelConstant<gText *>(new gText(*m_DefaultVal.tval));

}

gOutput &operator<<(gOutput &f, const gelSignature &)  { return f; }

gelSignature::gelSignature(const gText &s, gelAdapter *bif)
  : m_IsUdf(false), m_Bif(bif)
{
  if (!m_Bif)
    throw gelBadFunctionSig();
  ParseSignature(s);
}

gelSignature::gelSignature(const gText &s)
  : m_IsUdf(true), m_Udf(0)
{
  ParseSignature(s);
}

void gelSignature::SetUdf(gelExpr *udf)
{
  if (!m_IsUdf)
    throw gelInternalError("Expression given for non-UDF");
  if (m_Udf != 0)
    throw gelInternalError("Tried to redefine UDF");
  if (udf == 0)
    throw gelInternalError("Tried to give null expression for UDF");

  m_Udf = udf;
  if (m_Type == gelUNDEFINED)
    m_Type = m_Udf->Type();
  if (m_Type != m_Udf->Type() || m_Type == gelUNDEFINED)
    throw gelInternalError("UDF return type mismatch");
}


void gelSignature::ParseSignature(const gText &s)
{
  gText word = "";
  int index = 0, param = 0;
  bool quote = false, done = false;
  int length = s.Length();

  if (index >= length)
    throw gelBadFunctionSig();
  while (s[index] != '[')  {
    if (index >= length)   
      throw gelBadFunctionSig();
    if (isalnum(s[index]))
      word += s[index];
    index++;
  }

  if (index >= length || s[index] != '[')   
    throw gelBadFunctionSig();
  m_Name = word;
  index++;

  while (!done)   { 
    if (index >= length)
      throw gelBadFunctionSig();

    word = "";
    quote = false;
    while ((s[index] != ',' && s[index] != ']') || quote)  {
      if (index >= length)
	throw gelBadFunctionSig();
      
      if (isalnum(s[index]) || ispunct(s[index]) || quote)  {
	if (s[index] == '\"')
	  quote = !quote;
	word += s[index];
      }
      index++;
    }

    if (word != "")   {
      m_Parameters.Append(new gelParamInfo(word));
      param++;
    }

    if (index >= length)
      throw gelBadFunctionSig();
    if (s[index++] == ']')
      done = true;
  }


  while (index < length && isspace(s[index]))
    index++;
  
  if (index >= length)  {
    // NOTICE
    // this is for those "incomplete functions" without return types
    // should be elimited in the future
    word = "ANYTYPE";
  }
  else  {
    if (index >= length || s[index++] != '=')
      throw gelBadFunctionSig();
    if (index >= length || s[index++] != ':')
      throw gelBadFunctionSig();

    word = "";
    while (index < length)  {
      if (isalnum(s[index]) || ispunct(s[index]))
	word += s[index];
      index++;
    }
  }

  m_Type = NameToType( word );
  m_Depth = NameToDepth( word );
}

gelSignature::~gelSignature()
{
  if (m_IsUdf)  delete m_Udf;
  for (int i = 1; i <= m_Parameters.Length(); delete m_Parameters[i++]);
}

bool gelSignature::Matches(const gText &n,
			   gBlock<gelExpr *> &actuals) const
{
  if (n != m_Name ||
      actuals.Length() > m_Parameters.Length())
    return false;

  gArray<gelExpr *> matched(m_Parameters.Length());
  for (int i = 1; i <= matched.Length(); matched[i++] = 0);

  for (int i = 1; i <= actuals.Length(); i++)   {
    if (m_Parameters[i]->Type() != gelANYTYPE &&
        ( ( actuals[i]->Type() != m_Parameters[i]->Type() ) ||
	  ( actuals[i]->Depth() < m_Parameters[i]->Depth() ) ) )
      return false;
  }

  for (int i = actuals.Length() + 1; i <= matched.Length(); i++)  {
    if (!m_Parameters[i]->IsOptional())   return false;
    matched[i] = m_Parameters[i]->DefaultVal();
  }

  for (int i = actuals.Length() + 1; i <= matched.Length(); i++)
    actuals.Append(matched[i]);
    
  return true;
}

gelExpr *gelSignature::Evaluate(const gArray<gelExpr *> &actuals) const
{
  if (m_IsUdf)  {
    switch (m_Type)  {
    case gelNUMBER:
      return new gelUDF<gNumber *>(*this, actuals,
                                 (gelExpression<gNumber *> *) m_Udf);
    case gelBOOLEAN:
      return new gelUDF<gTriState *>(*this, actuals,
				   (gelExpression<gTriState *> *) m_Udf);
    case gelTEXT:
      return new gelUDF<gText *>(*this, actuals,
			       (gelExpression<gText *> *) m_Udf);
    case gelEFG:
      return new gelUDF<Efg *>(*this, actuals,
                               (gelExpression<Efg *> *) m_Udf);
    case gelNODE:
      return new gelUDF<Node *>(*this, actuals,
                                (gelExpression<Node *> *) m_Udf);
    case gelACTION:
      return new gelUDF<Action *>(*this, actuals,
                                  (gelExpression<Action *> *) m_Udf);
    case gelINFOSET:
      return new gelUDF<Infoset *>(*this, actuals,
                                   (gelExpression<Infoset *> *) m_Udf);
    case gelEFPLAYER:
      return new gelUDF<EFPlayer *>(*this, actuals,
                                    (gelExpression<EFPlayer *> *) m_Udf);
    case gelEFOUTCOME:
      return new gelUDF<EFOutcome *>(*this, actuals,
                                     (gelExpression<EFOutcome *> *) m_Udf);
    case gelEFSUPPORT:
      return new gelUDF<EFSupport *>(*this, actuals,
				     (gelExpression<EFSupport *> *) m_Udf);
    case gelBEHAV:
      return new gelUDF<BehavSolution *>(*this, actuals,
					 (gelExpression<BehavSolution *> *) m_Udf);
    case gelNFG:
      return new gelUDF<Nfg *>(*this, actuals,
                               (gelExpression<Nfg *> *) m_Udf);
    case gelSTRATEGY:
      return new gelUDF<Strategy *>(*this, actuals,
                                    (gelExpression<Strategy *> *) m_Udf);
    case gelNFPLAYER:
      return new gelUDF<NFPlayer *>(*this, actuals,
                                    (gelExpression<NFPlayer *> *) m_Udf);
    case gelNFOUTCOME:
      return new gelUDF<NFOutcome *>(*this, actuals,
                                     (gelExpression<NFOutcome *> *) m_Udf);
    case gelNFSUPPORT:
      return new gelUDF<NFSupport *>(*this, actuals,
				     (gelExpression<NFSupport *> *) m_Udf);
    case gelMIXED:
      return new gelUDF<MixedSolution *>(*this, actuals,
					 (gelExpression<MixedSolution *> *) m_Udf);
    case gelUNDEFINED:
      throw gelInternalError("Undefined function return type");
      break;

    default:
      return 0;
    }
  }
  else
    return m_Bif(actuals);
}



//-----------------------------------------------------------
// DefineParams
//   Only defines the parameters; don't assign them
//   subvt - the inner, function scope
//-----------------------------------------------------------
void gelSignature::DefineParams(gelVariableTable *subvt) const
{
  if (!subvt)
    throw gelInternalError("Null pointer passed to DefineParams");
  
  for (int i = 1; i <= m_Parameters.Length(); i++)  {
    if (!m_Parameters[i])
      throw gelInternalError("Null parameter pointer");

    gText name = m_Parameters[i]->Name();
    switch (m_Parameters[i]->Type())  {
    case gelBOOLEAN:
    case gelNUMBER:
    case gelTEXT:
    case gelEFG:
    case gelNODE:
    case gelACTION:
    case gelINFOSET:
    case gelEFPLAYER:
    case gelEFOUTCOME:
    case gelEFSUPPORT:
    case gelBEHAV:
    case gelNFG:
    case gelSTRATEGY:
    case gelNFOUTCOME:
    case gelNFPLAYER:
    case gelNFSUPPORT:
    case gelMIXED:
      subvt->Define(name, m_Parameters[i]->Type());
      break;
    case gelINPUT:
    case gelOUTPUT:
    case gelANYTYPE:
    case gelUNDEFINED:
      throw gelInternalError("Undefined parameter type");
      break;
    }
  }
}

//-----------------------------------------------------------
// AssignParams
//   Both defines and assigns the parameters
//   subvt - the inner, function scope
//   vt    - the outer, global scope
//-----------------------------------------------------------
void gelSignature::AssignParams(gelVariableTable *subvt, gelVariableTable *vt,
				const gArray<gelExpr *> &params) const
{
  if (!subvt || !vt)
    throw gelInternalError("Null pointer passed to AssignParams");
  if (m_Parameters.Length() != params.Length())
    throw gelInternalError("Mismatched parameter lists");

  DefineParams(subvt);
  
  for (int i = 1; i <= m_Parameters.Length(); i++)   {
    if (!m_Parameters[i] || !params[i])
      throw gelInternalError("Null parameter in AssignParams");
    if (m_Parameters[i]->Type() != params[i]->Type())
      throw gelInternalError("Type mismatch in AssignParams");

    gText name = m_Parameters[i]->Name();
    switch (m_Parameters[i]->Type())  {
    case gelBOOLEAN:
      subvt->SetValue(name,
		      ((gelExpression<gTriState *> *) params[i])->Evaluate(vt));
      break;
    case gelNUMBER:
      subvt->SetValue(name,
		      ((gelExpression<gNumber *> *) params[i])->Evaluate(vt));
      break;
    case gelTEXT:
      subvt->SetValue(name,
		      ((gelExpression<gText *> *) params[i])->Evaluate(vt));
      break;
    case gelEFG:
      subvt->SetValue(name,
		      ((gelExpression<Efg *> *) params[i])->Evaluate(vt));
      break;
    case gelNODE:
      subvt->SetValue(name,
		      ((gelExpression<Node *> *) params[i])->Evaluate(vt));
      break;
    case gelACTION:
      subvt->SetValue(name,
		      ((gelExpression<Action *> *) params[i])->Evaluate(vt));
      break;
    case gelINFOSET:
      subvt->SetValue(name,
		      ((gelExpression<Infoset *> *) params[i])->Evaluate(vt));
      break;
    case gelEFPLAYER:
      subvt->SetValue(name,
		      ((gelExpression<EFPlayer *> *) params[i])->Evaluate(vt));
      break;
    case gelEFOUTCOME:
      subvt->SetValue(name,
		      ((gelExpression<EFOutcome *> *) params[i])->Evaluate(vt));
      break;
    case gelEFSUPPORT:
      subvt->SetValue(name,
		      ((gelExpression<EFSupport *> *) params[i])->Evaluate(vt));
      break;
    case gelBEHAV:
      subvt->SetValue(name,
		      ((gelExpression<BehavSolution *> *) params[i])->Evaluate(vt));
      break;
    case gelNFG:
      subvt->SetValue(name,
		      ((gelExpression<Nfg *> *) params[i])->Evaluate(vt));
      break;
    case gelSTRATEGY:
      subvt->SetValue(name,
		      ((gelExpression<Strategy *> *) params[i])->Evaluate(vt));
      break;
    case gelNFPLAYER:
      subvt->SetValue(name,
		      ((gelExpression<NFPlayer *> *) params[i])->Evaluate(vt));
      break;
    case gelNFOUTCOME:
      subvt->SetValue(name,
		      ((gelExpression<NFOutcome *> *) params[i])->Evaluate(vt));
      break;
    case gelNFSUPPORT:
      subvt->SetValue(name,
		      ((gelExpression<NFSupport *> *) params[i])->Evaluate(vt));
      break;
    case gelMIXED:
      subvt->SetValue(name,
		      ((gelExpression<MixedSolution *> *) params[i])->Evaluate(vt));
      break;
    case gelINPUT:
    case gelOUTPUT:
    case gelANYTYPE:
    case gelUNDEFINED:
      throw gelInternalError("Undefined parameter type"); 
      break;
    }
  }
}


extern void gelMathInit(gelEnvironment *);
extern void gelListInit(gelEnvironment *);
extern void gelEfgInit(gelEnvironment *);
extern void gelNfgInit(gelEnvironment *);
extern void gelMiscInit(gelEnvironment *);
extern void gelAlgInit(gelEnvironment *);
extern void gelSolInit(gelEnvironment *);

gelEnvironment::gelEnvironment(void)
{
  gelMathInit(this);
  // gelListInit(this);
  gelEfgInit(this);
  gelNfgInit(this);
  gelAlgInit(this);
  gelSolInit(this);
  gelMiscInit(this);
}

gelEnvironment::~gelEnvironment()
{ }

void gelEnvironment::Register(gelAdapter *func, const gText &sig)
{
  signatures.Append(new gelSignature(sig, func));
}

void gelEnvironment::Register(gelSignature *sig)
{
  signatures.Append(sig);
}

gelExpr *gelEnvironment::Match(const gText &name,
			       gBlock<gelExpr *> &actuals)
{
  for (int i = 1; i <= actuals.Length(); i++)
    if (!actuals[i])
      return 0;

  for (int i = 1; i <= signatures.Length(); i++)
    if (signatures[i]->Matches(name, actuals))
      return signatures[i]->Evaluate(actuals);

  return 0;
}

gelExpr *gelEnvironment::Match(const gText &name, gelExpr *op1, gelExpr *op2)
{
  gBlock<gelExpr *> actuals(2);
  actuals[1] = op1;
  actuals[2] = op2;
  return Match(name, actuals);
}

gelExpr *gelEnvironment::Match(const gText &name, gelExpr *op)
{
  gBlock<gelExpr *> actuals(1);
  actuals[1] = op;
  return Match(name, actuals);
}


#include "garray.imp"

template class gArray<gelExpr*>;

#include "glist.imp"

template class gList<gelSignature *>;
template class gList<gelAdapter *>;
template class gList<gelParamInfo *>;


