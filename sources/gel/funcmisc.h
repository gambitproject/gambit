//
// FILE: funcmisc.h -- Miscellaneous internal structures for function matching
//
// $Id$ %G
//

#ifndef FUNCMISC_H
#define FUNCMISC_H

#include "gnumber.h"
#include "gtext.h"
#include "glist.h"
#include "gblock.h"
#include "exprtree.h"

class gelExpr;
class gelVariableTable;

typedef gelExpr *gelAdapter(const gArray<gelExpr *> &); 


class gelBadFunctionSig : public gException   {
 private:
  gText sig;

 public:
  gelBadFunctionSig(void)   { }
  gelBadFunctionSig(const gText &);
  virtual ~gelBadFunctionSig()   { }
  gText Description(void) const;
};

class gelInternalError : public gException  {
 private:
  gText msg;

 public:
  gelInternalError(const gText &s) : msg(s)  { }
  virtual ~gelInternalError()   { }
  gText Description(void) const   { return msg; }
};


class gelParamInfo  {
private:
  gText m_Name;
  gelType m_Type;
  int m_Depth;
  gText m_Default;
  union  {
    gTriState *bval;
    gNumber *nval;
    gText *tval;
  } m_DefaultVal;
  bool m_Optional;
  bool m_ByReference;
  
public:
  gelParamInfo(const gText &fn);
  ~gelParamInfo();

  const gText& Name(void) const { return m_Name; }
  gelType Type(void) const { return m_Type; }
  int Depth(void) const { return m_Depth; }

  bool IsOptional(void) const   { return m_Optional; }
  gelExpr *DefaultVal(void) const;
};

class gelSignature   
{
private:
  gText m_Name;
  gelType m_Type;
  int m_Depth;
  gList<gelParamInfo *> m_Parameters;

  bool m_IsUdf;
  union
  {
    gelAdapter* m_Bif;
    gelExpr*    m_Udf;
  };

  void ParseSignature( const gText& s );  // All constructors call this!

public:
  gelSignature( const gText& s, gelAdapter* bif );
  gelSignature( const gText& s );  
  void SetUdf( gelExpr* exp );

  ~gelSignature();


  gelType Type( void ) const { return m_Type; }
  int Depth( void ) const { return m_Depth; }
  

  bool Matches(const gText &,
	       gBlock<gelExpr *> &) const;
  bool operator==(const gelSignature &) const  { return false; } 
  bool operator!=(const gelSignature &) const  { return false; }
  
  const gText &Name(void) const   { return m_Name; }

  gelExpr* Evaluate( const gArray<gelExpr *>& ) const;


  //-----------------------------------------------------------
  // DefineParams
  //   Only defines the parameters; don't assign them
  //   subvt - the inner, function scope
  //-----------------------------------------------------------
  void DefineParams( gelVariableTable* subvt ) const;

  //-----------------------------------------------------------
  // AssignParams
  //   Both defines and assigns the parameters
  //   subvt - the inner, function scope
  //   vt    - the outer, global scope
  //-----------------------------------------------------------
  void AssignParams( gelVariableTable* subvt, gelVariableTable* vt,
		    const gArray<gelExpr *>& params ) const;

};


#endif   // FUNCMISC_H
