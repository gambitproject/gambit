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


#define  porNULLSPEC  true



#define  porERROR              ( 0x00000000 )
#define  porBOOL               ( 0x00000001 )
#define  porFLOAT              ( 0x00000002 )
#define  porINTEGER            ( 0x00000004 )
#define  porRATIONAL           ( 0x00000008 )
#define  porTEXT               ( 0x00000010 )

#define  porNFOUTCOME          ( 0x00000020 )

#define  porNFG_FLOAT          ( 0x00000040 )
#define  porNFG_RATIONAL       ( 0x00000080 )
#define  porNFG                ( porNFG_FLOAT | porNFG_RATIONAL )
#define  porEFG_FLOAT          ( 0x00000100 )
#define  porEFG_RATIONAL       ( 0x00000200 )
#define  porEFG                ( porEFG_FLOAT | porEFG_RATIONAL )
#define  porMIXED_FLOAT        ( 0x00000400 )
#define  porMIXED_RATIONAL     ( 0x00000800 )
#define  porMIXED              ( porMIXED_FLOAT | porMIXED_RATIONAL )
#define  porBEHAV_FLOAT        ( 0x00001000 )
#define  porBEHAV_RATIONAL     ( 0x00002000 )
#define  porBEHAV              ( porBEHAV_FLOAT | porBEHAV_RATIONAL )

#define  porOUTCOME_FLOAT      ( 0x00004000 )
#define  porOUTCOME_RATIONAL   ( 0x00008000 )
#define  porOUTCOME            ( porOUTCOME_FLOAT | porOUTCOME_RATIONAL )

#define  porINFOSET_FLOAT      ( 0x00010000 )
#define  porINFOSET_RATIONAL   ( 0x00020000 )
#define  porINFOSET            ( porINFOSET_FLOAT | porINFOSET_RATIONAL )

#define  porNFSUPPORT_FLOAT    ( 0x00040000 )
#define  porNFSUPPORT_RATIONAL ( 0x00080000 )
#define  porNFSUPPORT          ( porNFSUPPORT_FLOAT | porNFSUPPORT_RATIONAL )

#define  porEFSUPPORT_FLOAT    ( 0x00100000 )
#define  porEFSUPPORT_RATIONAL ( 0x00200000 )
#define  porEFSUPPORT          ( porEFSUPPORT_FLOAT | porEFSUPPORT_RATIONAL )

#define  porNFPLAYER           ( 0x00400000 )
#define  porEFPLAYER           ( 0x00800000 )
#define  porNODE               ( 0x01000000 )
#define  porACTION             ( 0x02000000 )
#define  porREFERENCE          ( 0x04000000 )
#define  porOUTPUT             ( 0x08000000 )
#define  porINPUT              ( 0x10000000 )
#define  porSTRATEGY           ( 0x20000000 )

#define  porNULL               ( 0x40000000 )
#define  porUNDEFINED          ( 0x80000000 )

#define  porANYTYPE            ( 0xFFFFFFFF )

#define  NLIST 32767


class PortionSpec
{
public:
  unsigned long Type;
  unsigned short ListDepth;
  bool Null;

  PortionSpec(const PortionSpec& spec)
    : Type(spec.Type), ListDepth(spec.ListDepth), Null(spec.Null)
    {}
  PortionSpec(unsigned long type = porERROR, 
	      signed short listdepth = 0,
	      bool null = false )
    : Type(type), ListDepth(listdepth), Null(null)
    {}
  ~PortionSpec()
  {}

  bool operator==(const PortionSpec& spec) const
  {
    return (Type==spec.Type && 
	    ListDepth==spec.ListDepth &&
	    Null==spec.Null);
  }
};


bool PortionSpecMatch( const PortionSpec& t1, const PortionSpec& t2 );


gString PortionSpecToText( const PortionSpec& type );
PortionSpec TextToPortionSpec( const gString& text );
void PrintPortionSpec( gOutput& s, PortionSpec type );
gOutput& operator << ( gOutput& s, PortionSpec spec );

#endif // GSMINCL_H

