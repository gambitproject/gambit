
//#
//# FILE: listfunc.h -- List and text oriented function implementations
//#
//# $Id$
//#


#include "portion.h"


Portion *GSM_NthElement(Portion **param);
Portion *GSM_Remove(Portion **param);
Portion *GSM_Contains(Portion **param);
Portion *GSM_LengthList(Portion **param);
Portion *GSM_LengthText(Portion **param);
Portion *GSM_NthChar(Portion **param);
Portion *GSM_TextInt(Portion **param);
Portion *GSM_TextFloat(Portion **param);
Portion *GSM_TextRat(Portion **param);
Portion *GSM_TextText(Portion **param);
Portion *GSM_IntegerRational(Portion **param);
Portion *GSM_IntegerInteger(Portion **param);
Portion *GSM_IntegerFloat(Portion **param);
Portion *GSM_FloatRational(Portion **param);
Portion *GSM_FloatInteger(Portion **param);
Portion *GSM_FloatFloat(Portion **param);
Portion *GSM_RationalFloat(Portion **param);
Portion *GSM_RationalInteger(Portion **param);
Portion *GSM_RationalRational(Portion **param);
Portion *GSM_StartWatch(Portion **param);
Portion *GSM_StopWatch(Portion **param);
Portion *GSM_ElapsedTime(Portion **param);
Portion *GSM_IsWatchRunning(Portion **param);
