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
  opADD, opSUBTRACT, opMULTIPLY, opDIVIDE, opNEGATE,
  opINTEGER_DIVIDE, opMODULUS,
  opEQUAL_TO, opNOT_EQUAL_TO, opGREATER_THAN, opLESS_THAN,
  opGREATER_THAN_OR_EQUAL_TO, opLESS_THAN_OR_EQUAL_TO,
  opLOGICAL_AND, opLOGICAL_OR, opLOGICAL_NOT,
  opSUBSCRIPT
} OperationMode;



typedef enum { rcFAIL, rcSUCCESS, rcQUIT } GSM_ReturnCode;








typedef unsigned int PortionType;

#define  porERROR      ( 0x0000 )
#define  porBOOL       ( 0x0001 )
#define  porDOUBLE     ( 0x0002 )
#define  porINTEGER    ( 0x0004 )
#define  porRATIONAL   ( 0x0008 )
#define  porSTRING     ( 0x0010 )
#define  porLIST       ( 0x0020 )

#define  porNFG_DOUBLE     ( 0x0040 )
#define  porNFG_RATIONAL   ( 0x0080 )
#define  porNFG            ( porNFG_DOUBLE | porNFG_RATIONAL )
#define  porEFG_DOUBLE     ( 0x0100 )
#define  porEFG_RATIONAL   ( 0x0200 )
#define  porEFG            ( porEFG_DOUBLE | porEFG_RATIONAL )
#define  porMIXED_DOUBLE   ( 0x0400 )
#define  porMIXED_RATIONAL ( 0x0800 )
#define  porMIXED          ( porMIXED_DOUBLE | porMIXED_RATIONAL )
#define  porBEHAV_DOUBLE   ( 0x1000 )
#define  porBEHAV_RATIONAL ( 0x2000 )
#define  porBEHAV          ( porBEHAV_DOUBLE | porBEHAV_RATIONAL )

#define  porOUTCOME    ( 0x4000 )
#define  porPLAYER     ( 0x8000 )
#define  porINFOSET    ( 0x010000 )
#define  porNODE       ( 0x020000 )
#define  porACTION     ( 0x040000 )

#define  porSTREAM     ( 0x080000 )

#define  porREFERENCE  ( 0x100000 )

#define  porALLOWS_SUBVARIABLES ( porNFG | porEFG )
				  
#define  porNUMERICAL  ( porDOUBLE | porINTEGER | porRATIONAL )
#define  porALL        ( 0xFFFFFFFF )
#define  porVALUE      ( porALL & ~porREFERENCE )


#endif // GSMINCL_H
