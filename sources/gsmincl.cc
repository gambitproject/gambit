//
// FILE: gsmincl.cc   miscellaneous GSM functions
//
// $Id$
//

#include "gsmincl.h"
#include "string.h"


bool PortionSpecMatch( const PortionSpec& t1, const PortionSpec& t2 )
{
  if( t1 == t2 )
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


#define NUM_PortionSpecs 30
#define NUM_CompositePortionSpecs 2

PortionSpecTextType _PortionSpecText[] =
{
  { porERROR,              "ERROR" },
  
  { porBOOL,               "BOOLEAN" },
  { porFLOAT,              "FLOAT" },
  { porINTEGER,            "INTEGER" },
  { porRATIONAL,           "RATIONAL" },
  { porTEXT,               "TEXT" },

  { porMIXED_FLOAT,        "MIXED(FLOAT)" },
  { porMIXED_RATIONAL,     "MIXED(RATIONAL)" },
  { porBEHAV_FLOAT,        "BEHAV(FLOAT)" },
  { porBEHAV_RATIONAL,     "BEHAV(RATIONAL)" },

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

  { porUNDEFINED,          "UNDEFINED" },

  { porNFG,                "NFG" },
  { porEFG,                "EFG" },
  { porEFOUTCOME,          "EFOUTCOME" },
  { porINFOSET,            "INFOSET" },
  { porNFSUPPORT,          "NFSUPPORT" },
  { porEFSUPPORT,          "EFSUPPORT" },

  { porMIXED,              "MIXED" },
  { porBEHAV,              "BEHAV" },

  { porANYTYPE,            "ANYTYPE" }
};


gString PortionSpecToText(const PortionSpec& spec)
{
  int i;
  gString result;
  for(i=0; i<NUM_PortionSpecs; i++)
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
    for(i=0; i<NUM_PortionSpecs-NUM_CompositePortionSpecs; i++)
      if(spec.Type & _PortionSpecText[i].Type)
	if(result == "")
	  result = _PortionSpecText[i].Text;
	else
	  result = result + " " + _PortionSpecText[i].Text;
  
  if(spec.ListDepth != NLIST)
    for(i=0; i<spec.ListDepth; i++)
      result = (gString) "LIST(" + result + ")";
  else
    result = (gString) "NLIST(" + result + ")";
  return result;
}


#include "gambitio.h"

PortionSpec TextToPortionSpec(const gString& text)
{
  int i;
  gString t = text;
  PortionSpec result = porUNDEFINED;

  while(t.left(5) == "LIST(")
  {
    result.ListDepth++;
    t = t.mid(t.length()-6, 6);
  }
  if(t.left(6) == "NLIST(")
  {
    result.ListDepth = NLIST;
    t = t.mid(t.length()-7, 7);
  }
  for(i=0; i<NUM_PortionSpecs; i++)
    if(t.left(strlen(_PortionSpecText[i].Text)) == _PortionSpecText[i].Text)
    {
      result.Type = result.Type | _PortionSpecText[i].Type;
      t = t.right(t.length() - strlen(_PortionSpecText[i].Text));
		if(t.left(1) == "*")
      {
	result.Null = true;
	t.remove(0);
      }
      if(t.left(1) == " ")
	t.remove(0);
    }
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
