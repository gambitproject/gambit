//#
//# FILE: gsmincl.h   header file for GSM components
//#
//# $Id$
//#



#ifndef GSMINCL_H
#define GSMINCL_H

#include "gstring.h"
#include "gmisc.h"

class gString;
class gOutput;

typedef enum 
{ 
  opERROR, 

  opNEG,
  opADD, opSUB, opMUL, opDIV, 
  opINTDIV, opMOD, 

  opEXP, opLOG,
  opPOW_INT, opPOW_FLOAT,

  opEQU, opNEQ, opGTN, opLTN, opGEQ, opLEQ,

  opAND, opOR, opNOT,

  opSUBSCRIPT,

  opINTEGER, opFLOAT, opRATIONAL
} OperationMode;



#define rcFAIL     0
#define rcSUCCESS -1
#define rcQUIT    -2






#define  porERROR      ( 0x0000 )
#define  porBOOL       ( 0x0001 )
#define  porFLOAT      ( 0x0002 )
#define  porINTEGER    ( 0x0004 )
#define  porRATIONAL   ( 0x0008 )
#define  porTEXT       ( 0x0010 )

#define  porNFG_FLOAT      ( 0x0040 )
#define  porNFG_RATIONAL   ( 0x0080 )
#define  porNFG            ( porNFG_FLOAT | porNFG_RATIONAL )
#define  porEFG_FLOAT      ( 0x0100 )
#define  porEFG_RATIONAL   ( 0x0200 )
#define  porEFG            ( porEFG_FLOAT | porEFG_RATIONAL )
#define  porMIXED_FLOAT    ( 0x0400 )
#define  porMIXED_RATIONAL ( 0x0800 )
#define  porMIXED          ( porMIXED_FLOAT | porMIXED_RATIONAL )
#define  porBEHAV_FLOAT    ( 0x1000 )
#define  porBEHAV_RATIONAL ( 0x2000 )
#define  porBEHAV          ( porBEHAV_FLOAT | porBEHAV_RATIONAL )

#define  porOUTCOME_FLOAT    ( 0x4000 )
#define  porOUTCOME_RATIONAL ( 0x800000 )
#define  porOUTCOME          ( porOUTCOME_FLOAT | porOUTCOME_RATIONAL )

#define  porPLAYER_EFG  ( 0x8000 )
#define  porPLAYER_NFG  ( 0x01000000 )
#define  porPLAYER      ( porPLAYER_EFG | porPLAYER_NFG ) 
#define  porINFOSET    ( 0x010000 )
#define  porNODE       ( 0x020000 )
#define  porACTION     ( 0x040000 )

#define  porREFERENCE  ( 0x080000 )

#define  porOUTPUT     ( 0x100000 )
#define  porINPUT      ( 0x200000 )

#define  porUNDEFINED  ( 0x400000 )

#define  porSTRATEGY   ( 0x02000000 )
#define  porNF_SUPPORT ( 0x04000000 )
#define  porEF_SUPPORT ( 0x08000000 )

#define  porANYTYPE    ( 0xFFFFFFFF )



class PortionSpec
{
public:
  unsigned long Type;
  unsigned short ListDepth;

  PortionSpec(const PortionSpec& spec)
    : Type(spec.Type), ListDepth(spec.ListDepth)
    {}
  PortionSpec(unsigned long type = porERROR, signed short listdepth = 0)
    : Type(type), ListDepth(listdepth)
    {}
  ~PortionSpec()
  {}

  bool operator==(const PortionSpec& spec) const
  { return Type==spec.Type && ListDepth==spec.ListDepth; }
};


bool PortionSpecMatch( const PortionSpec& t1, const PortionSpec& t2 );


gString PortionSpecToText( const PortionSpec& type );
PortionSpec TextToPortionSpec( const gString& text );
void PrintPortionSpec( gOutput& s, PortionSpec type );
gOutput& operator << ( gOutput& s, PortionSpec spec );

#endif // GSMINCL_H

