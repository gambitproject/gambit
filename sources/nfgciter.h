//#
//# FILE: normiter.h -- Normal form contingency iterator class
//#
//# @(#)normiter.h	1.7 11/22/94
//#

#ifndef CONTITER_H
#define CONTITER_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__
#include "gstring.h"
#include "gvector.h"
//
// This class is useful for iterating around the normal form.  This iterator
// allows the user choose to hold some players' strategies constant while
// iterating over the other players' strategies.  This is useful, for example,
// in displaying a 2D window into the n-D space of strategy profiles as in
// the normal form display code.  The constructor takes the normal form to
// work on and a gBlock<int> containing the numbers of players the user wants
// to hold constant.  The iteration is based on an index that goes from 1 to
// the total number of possible contingencies in increments of 1.
//

template <class T> class ContIter    {
	class NormalForm<T>;
	private:
		const NormalForm<T> &rep;
		StrategyProfile p;
		gBlock<int> hold_const;
		gBlock<int> order;			// Yes, these CAN be recalculated every time to
		gBlock<int> strats;			// save memory, but why?!
		long	index,max_cont;
		void SetIndex(void);		// Sets the profile according to index
		void GetIndex(void);		// Sets the index according to the profile
	public:
		ContIter(const NormalForm<T> &r) : rep(r), p(rep.NumPlayers()) { }
		~ContIter()   { }
//
// Determines which players' strategies will be held constant for continency
// iteration.  This MUST be called before the iterator can be used.
//
		void SetHoldConst(const gBlock<int> &hold);
//
// Set the internal strategy profile to be the profile consisting of the
// first strategy for each player
//
		void GoFirst(void) {index=1;SetIndex();};
//
// Returns the total number of contingencies for the hold_const players
//
		long NumContingencies(void) {return max_cont;}
//
// Returns the current index number
//
		long Index(void) const {return index;}
//
// Increment the strategy number of the ith player.  Returns the updated index
// Note: this updates the index unless ith player is one of those held const.
//
		long Next(int i);
//
// Increment the contingency for the hold_const  players.  Returns the index.
//
		long NextContingency(void) {index=index%max_cont+1;SetIndex();return index;}
//
// Set the strategy number for the ith player to be his jth strategy
// Note that this also allows the user to change the strategy of one of
// the hold_const players.  Returns the updated index.
// Note: this updates the index unless ith player is one of those held const.
//
		long Set(int i, int j);
//
// Set the index.  This automatically updates all the non-const player's
// strategies
//
		void Set(long _index) {index=_index;SetIndex();}
//
// Evaluate the normal form at the given pure strategy profile for the
// given player
//
		T Evaluate(int player) const {return rep.Payoff(player, p);}
		gVector<T> Evaluate(void) const {return rep.Payoff(p);}
		gString EvaluateS(int player) const;
		gString EvaluateS(void) const;
//
// Return the current profile (strategy # for each player) in a gBlock<int>
//
		gBlock<int> CurProfileB(void) const;
//
// Return the current strategy profile in a StrategyProfile
//
		StrategyProfile CurProfile(void) const {return p;}
};

#ifdef __GNUG__
#define INLINE inline
#elif defined __BORLANDC__
#define INLINE
#endif   // __GNUG__, __BORLANDC__

template <class T> INLINE void ContIter<T>::GetIndex(void)
{
int pos=0;
index=1;
for (int i=1;i<=rep.NumPlayers();i++)
{
	while (hold_const.Contains(i)) i++;
	pos++;
	index+=(p.GetStrategy(i).GetStrategyNumber()-1)*order[pos];
}
}

template <class T> INLINE void ContIter<T>::SetIndex(void)
{
int st,i,pos=0;
for (i=1;i<=rep.NumPlayers();i++)
{
	while (hold_const.Contains(i)) i++;
	pos++;
	st=((index-1)/order[pos])%strats[pos]+1;
	Strategy foo = rep.GetStrategy(i, st);
	p.SetStrategy(foo);
}
}



template <class T>
INLINE void ContIter<T>::SetHoldConst(const gBlock<int> &hold)
{
int i,pos=0,num_players=rep.NumPlayers();
hold_const=hold;
order=gBlock<int>(num_players+1-hold_const.Length());
strats=gBlock<int>(num_players-hold_const.Length());
order[1]=1;
max_cont=1;
for (i=1;i<=num_players;i++)
{
	while (hold_const.Contains(i)) i++;
	pos++;
	strats[pos]=rep.NumStrats(i);
	order[pos+1]=order[pos]*strats[pos];
	max_cont*=strats[pos];
}
GoFirst();
}

template <class T> INLINE long ContIter<T>::Set(int i, int j)
{
	Strategy foo = rep.GetStrategy(i, j);
	p.SetStrategy(foo);
	GetIndex();
	return index;
}

template <class T> INLINE long ContIter<T>::Next(int i)
{
	int j = p.GetStrategy(i).GetStrategyNumber();

	if (j < rep.NumStrats(i))  j++;
	else j = 1;

	Strategy foo = rep.GetStrategy(i, j);
	p.SetStrategy(foo);
	GetIndex();
	return index;
}

template <class T> INLINE gString ContIter<T>::EvaluateS(int player) const
{
	T payoff=rep.Payoff(player, p);
	char tmp_str[10];
	sprintf(tmp_str,"%2.2lf",(double)payoff);
	return gString(tmp_str);
}

template <class T> INLINE gString ContIter<T>::EvaluateS(void) const
{
	gString pay_str;
	for (int i=1;i<=rep.NumPlayers();i++)
	{
		pay_str+=EvaluateS(i);
		if (i!=rep.NumPlayers()) pay_str+=',';
	}
	return pay_str;
}

template <class T> INLINE gBlock<int> ContIter<T>::CurProfileB(void) const
{
gBlock<int> result(rep.NumPlayers());
for (int i=1;i<=rep.NumPlayers();i++)
 result[i]=p.GetStrategy(i).GetStrategyNumber();
return result;
}


#endif   // CONTITER_H




