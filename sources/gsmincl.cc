//
// FILE: gsmincl.cc   miscellaneous GSM functions
//
// $Id$
//

#include "gsmincl.h"


bool PortionSpecMatch( const PortionSpec& t1, const PortionSpec& t2 )
{
  if (t1 == t2)
    return true;
  else if((t1.Type & t2.Type) && ((t1.ListDepth>0) == (t2.ListDepth>0)))
    return true;
  else if(((t1.ListDepth>0) == (t2.ListDepth>0)) &&
	  (((t1.Type & porMIXED  ) && (t2.Type & porMIXED  )) ||
	   ((t1.Type & porBEHAV  ) && (t2.Type & porBEHAV  )) ||
	   ((t1.Type & porEFOUTCOME) && (t2.Type & porEFOUTCOME)) ||
	   ((t1.Type & porNFG    ) && (t2.Type & porNFG    )) ||
	   ((t1.Type & porEFG    ) && (t2.Type & porEFG    ))))
    return true;
  else
    return false;
}





struct PortionSpecTextType
{
  unsigned long Type;
  char* Text;
};  


#define NUM_PortionSpecs 26
#define NUM_CompositePortionSpecs 2

PortionSpecTextType _PortionSpecText[] =
{
  { porBOOLEAN,            "BOOLEAN" },
  { porINTEGER,            "INTEGER" }, 
  { porNUMBER,             "NUMBER" },
  { porTEXT,               "TEXT" },

  { porNFOUTCOME,          "NFOUTCOME" },

  { porNFPLAYER,           "NFPLAYER" },
  { porEFPLAYER,           "EFPLAYER" },
  { porNODE,               "NODE" },
  { porACTION,             "ACTION" },
  { porSTRATEGY,           "STRATEGY" },

  { porREFERENCE,          "REFERENCE" },

  { porOUTPUT,             "OUTPUT" },
  { porINPUT,              "INPUT" },

  { porNULL,               "NULL" },

  { porNFG,                "NFG" },
  { porEFG,                "EFG" },
  { porEFOUTCOME,          "EFOUTCOME" },
  { porINFOSET,            "INFOSET" },
  { porNFSUPPORT,          "NFSUPPORT" },
  { porEFSUPPORT,          "EFSUPPORT" },
  { porPRECISION,          "PRECISION" },
  { porEFBASIS,            "EFBASIS" },


  { porMIXED,              "MIXED" },
  { porBEHAV,              "BEHAV" },

  { porUNDEFINED,          "UNDEFINED" },

  { porANYTYPE,            "ANYTYPE" }
};


gText PortionSpecToText(const PortionSpec& spec)
{
  gText result;
  for (unsigned int i=0; i<NUM_PortionSpecs; i++)
    if(spec.Type == _PortionSpecText[i].Type)
    {
      if(result == "")
	result = _PortionSpecText[i].Text;
      else
	result = result + " " + _PortionSpecText[i].Text;
      if(spec.Null)
	result = result + "*";
    }

  if(result == "")
    for(unsigned int i=0; i<NUM_PortionSpecs-NUM_CompositePortionSpecs; i++)
      if(spec.Type & _PortionSpecText[i].Type)
	if(result == "")
	  result = _PortionSpecText[i].Text;
	else
	  result = result + " " + _PortionSpecText[i].Text;
  
  if (spec.ListDepth != NLIST)
    for (unsigned int i = 0; i < spec.ListDepth; i++)
      result = (gText) "LIST(" + result + ")";
  else
    result = (gText) "NLIST(" + result + ")";
  return result;
}


#include "base/gstream.h"
#include "gsm.h"

PortionSpec TextToPortionSpec(const gText& text)
{
  gText t = text;
  PortionSpec result = 0;

  while(t.Left(5) == "LIST(")
  {
    result.ListDepth++;
    t = t.Mid(t.Length()-6, 6);
  }
  if(t.Left(6) == "NLIST(")
  {
    result.ListDepth = NLIST;
    t = t.Mid(t.Length()-7, 7);
  }
  for (int i = 0; i < NUM_PortionSpecs; i++)  
    if (t.Left(strlen(_PortionSpecText[i].Text)) == gText(_PortionSpecText[i].Text)) {
      result.Type = result.Type | _PortionSpecText[i].Type;
      t = t.Right(t.Length() - strlen(_PortionSpecText[i].Text));
      if(t.Left(1) == "*") {
	result.Null = true;
	t.Remove(0);
      }
      if(t.Left(1) == " ")
	t.Remove(0);
    }

  if (result == 0)
    throw gclRuntimeError("");

  return result;
}


void PrintPortionSpec( gOutput& s, PortionSpec type )
{
  s << PortionSpecToText( type );
}

gOutput& operator << ( gOutput& s, PortionSpec spec )
{
  PrintPortionSpec( s, spec );
  return s;
}
