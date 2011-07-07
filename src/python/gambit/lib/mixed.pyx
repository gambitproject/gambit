cdef class MixedStrategyProfileDouble:
    cdef c_MixedStrategyProfileDouble *profile

    def __dealloc__(self):
        del_MixedStrategyProfileDouble(self.profile)

    def __len__(self):
        return self.profile.MixedProfileLength()

    def __repr__(self):
        return str(list(self))

    def __getitem__(self, index):
        if isinstance(index, int):
            return self.profile.getitem_int(index+1)
        elif isinstance(index, Strategy):
            return self.profile.getitem_Strategy((<Strategy>index).strategy)
        elif isinstance(index, Player):
            return [ self[s] for s in index.strategies ]
        else:
            raise TypeError, "unexpected type passed to __getitem__ on strategy profile"

    def __setitem__(self, index, value):
        if isinstance(index, int):
            setitem_MixedStrategyProfileDouble_int(self.profile, index+1, value)
        elif isinstance(index, Strategy):
            setitem_MixedStrategyProfileDouble_Strategy(self.profile, 
                                                        (<Strategy>index).strategy, value)
        else:
            raise TypeError, "unexpected type passed to __getitem__ on strategy profile"

    def __richcmp__(MixedStrategyProfileDouble self, other, whichop):
        if whichop == 0:
            return list(self) < list(other)
        elif whichop == 1:
            return list(self) <= list(other)
        elif whichop == 2:
            return list(self) == list(other)
        elif whichop == 3:
            return list(self) != list(other)
        elif whichop == 4:
            return list(self) > list(other)
        else:
            return list(self) >= list(other)

    def payoff(self, Player player):
        return self.profile.GetPayoff(player.player)

    def strategy_value(self, Strategy strategy):
        return self.profile.GetStrategyValue(strategy.strategy)

    def liap_value(self):
        return self.profile.GetLiapValue()
