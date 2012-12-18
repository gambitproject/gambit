from cython.operator cimport dereference as deref
from gambit.lib.error import UndefinedOperationError

cdef class MixedStrategyProfile(object):
    def __repr__(self):    return str(list(self))
    def __richcmp__(MixedStrategyProfile self, other, whichop):
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

    def _resolve_index(self, index, players=True):
        # Given a string index, resolve into a player or strategy object.
        if players:
            try:
                # first check to see if string is referring to a player
                return self.game.players[index]
            except IndexError:
                pass
        else:
            # if no player matches, check strategy labels
            strategies = reduce(lambda x,y: x+y,
                                [ list(p.strategies) 
                                  for p in self.game.players ])
            matches = filter(lambda x: x.label==index, strategies)
            if len(matches) == 1:
                return matches[0]
            elif len(matches) == 0:
                if players:
                    raise IndexError("no player or strategy matching label '%s'" % index)
                else:
                    raise IndexError("no strategy matching label '%s'" % index)
            else:
                raise IndexError("multiple strategies matching label '%s'" % index)

    def __getitem__(self, index):
        if isinstance(index, int):
            return self._getprob(index+1)
        elif isinstance(index, Strategy):
            return self._getprob((<Strategy>index).strategy.deref().GetId())
        elif isinstance(index, Player):
            class MixedStrategy(object):
                def __init__(self, profile, player):
                    self.profile = profile
                    self.player = player
                def __eq__(self, other):
                    return list(self) == list(other)
                def __len__(self):
                    return len(self.player.strategies)
                def __repr__(self):
                    return str(list(self.profile[self.player]))
                def __getitem__(self, index):
                    return self.profile[self.player.strategies[index]]
                def __setitem__(self, index, value):
                    self.profile[self.player.strategies[index]] = value
            return MixedStrategy(self, index)
        elif isinstance(index, str):
            return self[self._resolve_index(index, players=True)]
        else:
            raise TypeError("profile indexes must be int, str, Player, or Strategy, not %s" %
                            index.__class__.__name__)

    def __setitem__(self, index, value):
        if isinstance(index, int):
            self._setprob(index+1, value)
        elif isinstance(index, Strategy):
            self._setprob((<Strategy>index).strategy.deref().GetId(), value)
        elif isinstance(index, str):
            self[self._resolve_index(index)] = value

    def payoff(self, player):
        if isinstance(player, Player):
            return self._payoff(player)
        elif isinstance(player, (int, str)):
            return self.payoff(self.game.players[player])
        raise TypeError("profile payoffs index must be int, str, or Player, not %s" %
                        player.__class__.__name__)

    def strategy_value(self, strategy):
        if isinstance(strategy, str):
            strategy = self._resolve_index(strategy, players=False)
        elif not isinstance(strategy, Strategy):
            raise TypeError("profile strategy value index must be str or Strategy, not %s" %
                            strategy.__class__.__name__)
        return self._strategy_value(strategy)
            
    def strategy_values(self, player):
        if isinstance(player, str):
            player = self.game.players[player]
        elif not isinstance(player, Player):
            raise TypeError("strategy values index must be str or Player, not %s" %
                            player.__class__.__name__)
        return [self.strategy_value(item) for item in player.strategies]

    def strategy_value_deriv(self, player, strategy1, strategy2):
        if isinstance(player, (int, str)):
            player = self.game.players[player]
        elif not isinstance(player, Player):
            raise TypeError("player index must be int, str, or Player, not %s" %
                            player.__class__.__name__)
        if isinstance(strategy1, str):
            strategy1 = self._resolve_index(strategy1, players=False)
        elif not isinstance(strategy1, Strategy):
            raise TypeError("profile strategy index must be str or Strategy, not %s" %
                            strategy1.__class__.__name__)
        if isinstance(strategy2, str):
            strategy2 = self._resolve_index(strategy2, players=False)
        elif not isinstance(strategy2, Strategy):
            raise TypeError("profile strategy index must be str or Strategy, not %s" %
                            strategy2.__class__.__name__)
        return self._strategy_value_deriv((<Player>player).player.deref().GetNumber(), strategy1, strategy2)


cdef class MixedStrategyProfileDouble(MixedStrategyProfile):
    cdef c_MixedStrategyProfileDouble *profile

    def __dealloc__(self):
        del_MixedStrategyProfileDouble(self.profile)
    def __len__(self):
        return self.profile.MixedProfileLength()

    def _getprob(self, int index):
        return self.profile.getitem(index)
    def _setprob(self, int index, value):
        setitem_MixedStrategyProfileDouble(self.profile, index, value)
    def _payoff(self, Player player):
        return self.profile.GetPayoff(player.player)
    def _strategy_value(self, Strategy strategy):
        return self.profile.GetStrategyValue(strategy.strategy)
    def _strategy_value_deriv(self, int pl,
                              Strategy s1, Strategy s2):
        return self.profile.GetPayoffDeriv(pl, s1.strategy, s2.strategy)

    def liap_value(self):
        return self.profile.GetLiapValue()
    def as_behav(self):
        cdef MixedBehavProfileDouble behav
        if not self.game.is_tree:
            raise UndefinedOperationError("Mixed behavior profiles are not "\
                                          "defined for strategic games")
        behav = MixedBehavProfileDouble()
        behav.profile = new_BehavFromMixedDouble(deref(self.profile))
        return behav
    def restriction(self):
        cdef StrategicRestriction s
        s = StrategicRestriction()
        s.support = copy_StrategySupport(self.profile.GetSupport())
        return s

    property game:
        def __get__(self):
            cdef Game g
            g = Game()
            g.game = self.profile.GetGame()
            return g


cdef class MixedStrategyProfileRational(MixedStrategyProfile):
    cdef c_MixedStrategyProfileRational *profile

    def __dealloc__(self):
        del_MixedStrategyProfileRational(self.profile)
    def __len__(self):
        return self.profile.MixedProfileLength()

    def _getprob(self, int index):
        return fractions.Fraction(rat_str(self.profile.getitem(index)).c_str()) 
    def _setprob(self, int index, value):
        cdef char *s
        if not isinstance(value, (int, fractions.Fraction)):
            raise TypeError("rational precision profile requires int or Fraction probability, not %s" %
                            value.__class__.__name__)
        t = str(value)
        s = t
        setitem_MixedStrategyProfileRational(self.profile, index, s)
    def _payoff(self, Player player):
        return fractions.Fraction(rat_str(self.profile.GetPayoff(player.player)).c_str())
    def _strategy_value(self, Strategy strategy):
        return fractions.Fraction(rat_str(self.profile.GetStrategyValue(strategy.strategy)).c_str())
    def _strategy_value_deriv(self, int pl,
                              Strategy s1, Strategy s2):
        return fractions.Fraction(rat_str(self.profile.GetPayoffDeriv(pl, s1.strategy, s2.strategy)).c_str())

    def liap_value(self):
        return fractions.Fraction(rat_str(self.profile.GetLiapValue()).c_str())
    def as_behav(self):
        cdef MixedBehavProfileRational behav
        if not self.game.is_tree:
            raise UndefinedOperationError("Mixed behavior profiles are not "\
                                          "defined for strategic games")
        behav = MixedBehavProfileRational()
        behav.profile = new_BehavFromMixedRational(deref(self.profile))
        return behav
    def restriction(self):
        cdef StrategicRestriction s
        s = StrategicRestriction()
        s.support = copy_StrategySupport(self.profile.GetSupport())
        return s
    
    property game:
        def __get__(self):
            cdef Game g
            g = Game()
            g.game = self.profile.GetGame()
            return g
