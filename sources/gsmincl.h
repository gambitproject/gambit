//#
//# FILE: gsmincl.h   header file for GSM components
//#
//# $Id$
//#



#ifndef GSMINCL_H
#define GSMINCL_H





typedef enum 
{ 
  opERROR, 
  opADD, opSUB, opMUL, opDIV, opNEG,
  opINTDIV, opMOD,
  opEQU, opNEQ, opGTN, opLTN, opGEQ, opLEQ,
  opAND, opOR, opNOT,
  opSUBSCRIPT
} OperationMode;



typedef enum { rcFAIL, rcSUCCESS, rcQUIT } GSM_ReturnCode;








typedef unsigned int PortionType;

#define  porERROR      ( 0x0000 )
#define  porBOOL       ( 0x0001 )
#define  porFLOAT      ( 0x0002 )
#define  porINTEGER    ( 0x0004 )
#define  porRATIONAL   ( 0x0008 )
#define  porTEXT       ( 0x0010 )
#define  porLIST       ( 0x0020 )

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

#define  porUNKNOWN    ( 0x400000 )

#define  porSTRATEGY   ( 0x02000000 )
#define  porNF_SUPPORT ( 0x04000000 )

#define  porNUMERICAL  ( porFLOAT | porINTEGER | porRATIONAL )
#define  porALL        ( 0xFFFFFFFF )
#define  porVALUE      ( porALL & ~porREFERENCE )


#endif // GSMINCL_H
