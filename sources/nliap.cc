//#
//# FILE: liap.cc -- Liapunov module
//#
//# $Id$
//#

#include "mixed.h"
#include "normal.h"

/*
double NormalForm::Liap(MixedProfile s) const
{
  double result;
  assert(s.GetNormalForm() == this);
  for(i = 1; i <= s.GetPlayers(); i++)
    { MixedStrategy tmp(s.GetMixedStrategy(player));
      // first set all the strategies for that particular player to 0;
      for(int i = 1; i <= strategies[player]->Length(); i++)
	tmp[i] = 0.0;
      result += _Liap(i, s, tmp);
    }
  return result;
}

double NormalForm::_Liap(int player, MixedProfile s, MixedStrategy ms) const
{
  double result = 0, tmp;
  MixedProfile tmp(s);
  // then for each strategy for that player set it to 1 and evaluate
  for(int i = 1; i <= strategies[player]->Length(); i++)
    { ms[i] = 1;
      tmp.SetMixedStrategy(ms);
      tmp = (*this)(player,tmp);
      tmp -= (*this)(player,s);
      tmp = (tmp > 0 ? tmp : 0);
      result += tmp * tmp;
      ms[i] = 0; // unset so that it is still valid for next time
    }
  return result;
}
*/
