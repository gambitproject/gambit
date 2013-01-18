from libcpp cimport bool
from cython.operator cimport dereference as deref

cdef class MixedBehavProfile(object):
    def __repr__(self):    return str(list(self))
    def __richcmp__(MixedBehavProfile self, other, whichop):
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
        # Given a string index, resolve into a player or action object.
        if players:
            try:
                # first check to see if string is referring to a player
                return self.game.players[index]
            except IndexError:
                pass

        # if no player matches, check infoset labels
        infosets = reduce(lambda x,y: x+y,
                            [ list(p.infosets) 
                              for p in self.game.players ]) 
        matches = filter(lambda x: x.label==index, infosets)
        if len(matches) == 1:
            return matches[0]
        elif len(matches) > 1:
            raise IndexError("multiple infosets matching label '%s'" % index)
        
        # if no infoset matches, check action labels
        actions = reduce(lambda x,y: x+y,
                            [ list(i.actions) 
                              for p in self.game.players
                              for i in p.infosets ])
        matches = filter(lambda x: x.label==index, actions)
        if len(matches) == 1:
            return matches[0]
        elif len(matches) == 0:
            if players:
                raise IndexError("no player, infoset or action matching label '%s'" % index)
            else:
                raise IndexError("no infoset or action matching label '%s'" % index)
        else:
            raise IndexError("multiple actions matching label '%s'" % index)

    def _setinfoset(self, index, value):
        if len(index.actions) == len(value):
            for action, val in zip(index.actions, value):
                self._setaction(action, val)
        else:
            raise ValueError("value list length must be %s, not %s" %
                             (len(index.actions), len(value)))

    def _setplayer(self, index, value):
        if len(index.infosets) == len(value):
            for infoset, val in zip(index.infosets, value):
                self._setinfoset(infoset, val)
        else:
            raise ValueError("value list length must be %s, not %s" %
                             (len(index.actions), len(value)))

    def __getitem__(self, index):
        if isinstance(index, int):
            return self._getprob(index+1)
        elif isinstance(index, Action):
            return self._getaction(index)
        elif isinstance(index, Infoset):
            class MixedBehavInfoset(object):
                def __init__(self, profile, infoset):
                    self.profile = profile
                    self.infoset = infoset
                def __eq__(self, other):
                    return list(self) == list(other)
                def __len__(self):
                    return len(self.infoset.actions)
                def __repr__(self):
                    return str(list(self.profile[self.infoset]))
                def __getitem__(self, index):
                    return self.profile[self.infoset.actions[index]]
                def __setitem__(self, index, value):
                    self.profile[self.infoset.actions[index]] = value
            return MixedBehavInfoset(self, index)
        elif isinstance(index, Player):
            class MixedBehav(object):
                def __init__(self, profile, player):
                    self.profile = profile
                    self.player = player
                def __eq__(self, other):
                    return list(self) == list(other)
                def __len__(self):
                    return len(self.player.infosets)
                def __repr__(self):
                    return str(list(self.profile[self.player]))
                def __getitem__(self, index):
                    return self.profile[self.player.infosets[index]]
                def __setitem__(self, index, value):
                    self.profile[self.player.infosets[index]] = value
            return MixedBehav(self, index)
        elif isinstance(index, str):
            return self[self._resolve_index(index, players=True)]
        else:
            raise TypeError("profile indexes must be int, str, Player, Infoset or Action, not %s" %
                            index.__class__.__name__)

    def __setitem__(self, index, value):
        if isinstance(index, int):
            self._setprob(index+1, value)
        elif isinstance(index, Action):
            self._setaction(index, value)
        elif isinstance(index, Infoset):
            self._setinfoset(index,value)
        elif isinstance(index, Player):
            self._setplayer(index, value)
        elif isinstance(index, str):
            self[self._resolve_index(index)] = value
        else:
            raise TypeError("profile indexes must be int, str, Player, Infoset or Action, not %s" %
                            index.__class__.__name__)

    def is_defined_at(self, infoset):
        if isinstance(infoset, str):
            infoset = self._resolve_index(infoset, players=False)
            if not isinstance(infoset, Infoset):
                raise IndexError("no infoset matching label '%s'" % infoset.label)
        elif not isinstance(infoset, Infoset):
            raise TypeError("profile infoset index must be str or Infoset, not %s" %
                            infoset.__class__.__name__)
        return self._is_defined_at(infoset)

    def payoff(self, player):
        if isinstance(player, Player):
            return self._payoff(player)
        elif isinstance(player, (int, str)):
            return self.payoff(self.game.players[player])
        raise TypeError("profile payoffs index must be int, str, or Player, not %s" %
                        player.__class__.__name__)

    def infoset_prob(self, infoset):
        if isinstance(infoset, str):
            infoset = self._resolve_index(infoset, players=False)
            if not isinstance(infoset, Infoset):
                raise IndexError("no infoset matching label '%s'" % infoset.label)
        elif not isinstance(infoset, Infoset):
            raise TypeError("profile infoset probability index must be str or Infoset, not %s" %
                            infoset.__class__.__name__)
        return self._infoset_prob(infoset)

    def infoset_value(self, infoset):
        if isinstance(infoset, str):
            infoset = self._resolve_index(infoset, players=False)
            if not isinstance(infoset, Infoset):
                raise IndexError("no infoset matching label '%s'" % infoset.label)
        elif not isinstance(infoset, Infoset):
            raise TypeError("profile infoset value index must be str or Infoset, not %s" %
                            infoset.__class__.__name__)
        return self._infoset_value(infoset)

    def action_prob(self, action):
        if isinstance(action, str):
            action = self._resolve_index(action, players=False)
            if not isinstance(action, Action):
                raise IndexError("no action matching label '%s'" % action.label)
        elif not isinstance(action, Action):
            raise TypeError("profile action probability index must be str or Action, not %s" %
                            action.__class__.__name__)
        return self._action_prob(action)

    def action_value(self, action):
        if isinstance(action, str):
            action = self._resolve_index(action, players=False)
            if not isinstance(action, Action):
                raise IndexError("no action matching label '%s'" % action.label)
        elif not isinstance(action, Action):
            raise TypeError("profile action value index must be str or Action, not %s" %
                            action.__class__.__name__)
        return self._action_value(action)

    def regret(self, action):
        if isinstance(action, str):
            action = self._resolve_index(action, players=False)
            if not isinstance(action, Action):
                raise IndexError("no action matching label '%s'" % action.label)
        elif not isinstance(action, Action):
            raise TypeError("profile regret index must be str or Action, not %s" %
                            action.__class__.__name__)
        return self._regret(action)    

cdef class MixedBehavProfileDouble(MixedBehavProfile):
    cdef c_MixedBehavProfileDouble *profile

    def __dealloc__(self):
        del self.profile
    def __len__(self):
        return self.profile.Length()

    def _is_defined_at(self, Infoset infoset):
        return self.profile.IsDefinedAt(infoset.infoset)
    def _getprob(self, int index):
        return self.profile.getitem(index)
    def _getaction(self, Action index):
        return self.profile.getaction(index.action)
    def _setprob(self, int index, value):
        setitem_MixedBehavProfileDouble(self.profile, index, value)
    def _setaction(self, Action index, value):
        setaction_MixedBehavProfileDouble(self.profile, index.action, value)
    def _payoff(self, Player player):
        return self.profile.GetPayoff(player.player.deref().GetNumber())
    def _infoset_prob(self, Infoset infoset):
        return self.profile.GetInfosetProb(infoset.infoset)
    def _infoset_value(self, Infoset infoset):
        return self.profile.GetInfosetValue(infoset.infoset)
    def _action_prob(self, Action action):
        return self.profile.GetActionProb(action.action)
    def _action_value(self, Action action):
        return self.profile.GetActionValue(action.action)
    def _regret(self, Action action):
        return self.profile.GetRegret(action.action)

    def copy(self):
        cdef MixedBehavProfileDouble behav
        behav = MixedBehavProfileDouble()
        behav.profile = new c_MixedBehavProfileDouble(deref(self.profile))
        return behav
    def as_mixed(self):
        cdef MixedStrategyProfileDouble mixed
        mixed = MixedStrategyProfileDouble()
        mixed.profile = new c_MixedStrategyProfileDouble(deref(self.profile).ToMixedProfile())
        return mixed
    def liap_value(self):
        return self.profile.GetLiapValue()

    property game:
        def __get__(self):
            cdef Game g
            g = Game()
            g.game = self.profile.GetGame()
            return g


cdef class MixedBehavProfileRational(MixedBehavProfile):
    cdef c_MixedBehavProfileRational *profile

    def __dealloc__(self):
        del self.profile
    def __len__(self):
        return self.profile.Length()

    def _is_defined_at(self, Infoset infoset):
        return self.profile.IsDefinedAt(infoset.infoset)
    def _getprob(self, int index):
        return fractions.Fraction(rat_str(self.profile.getitem(index)).c_str()) 
    def _getaction(self, Action index):
        return fractions.Fraction(rat_str(self.profile.getaction(index.action)).c_str()) 
    def _setprob(self, int index, value):
        cdef char *s
        if not isinstance(value, (int, fractions.Fraction)):
            raise TypeError("rational precision profile requires int or Fraction probability, not %s" %
                            value.__class__.__name__)
        t = str(value)
        s = t
        setitem_MixedBehavProfileRational(self.profile, index, s)
    def _setaction(self, Action index, value):
        cdef char *s
        if not isinstance(value, (int, fractions.Fraction)):
            raise TypeError("rational precision profile requires int or Fraction probability, not %s" %
                            value.__class__.__name__)
        t = str(value)
        s = t
        setaction_MixedBehavProfileRational(self.profile, index.action, s)
    def _payoff(self, Player player):
        return fractions.Fraction(rat_str(self.profile.GetPayoff(player.player.deref().GetNumber())).c_str())
    def _infoset_prob(self, Infoset infoset):
        return fractions.Fraction(rat_str(self.profile.GetInfosetProb(infoset.infoset)).c_str())
    def _infoset_value(self, Infoset infoset):
        return fractions.Fraction(rat_str(self.profile.GetInfosetValue(infoset.infoset)).c_str())
    def _action_prob(self, Action action):
        return fractions.Fraction(rat_str(self.profile.GetActionProb(action.action)).c_str())
    def _action_value(self, Action action):
        return fractions.Fraction(rat_str(self.profile.GetActionValue(action.action)).c_str())
    def _regret(self, Action action):
        return fractions.Fraction(rat_str(self.profile.GetRegret(action.action)).c_str())
    
    def copy(self):
        cdef MixedBehavProfileRational behav
        behav = MixedBehavProfileRational()
        behav.profile = new c_MixedBehavProfileRational(deref(self.profile))
        return behav
    def as_mixed(self):
        cdef MixedStrategyProfileRational mixed
        mixed = MixedStrategyProfileRational()
        mixed.profile = new c_MixedStrategyProfileRational(deref(self.profile).ToMixedProfile())
        return mixed
    def liap_value(self):
        return fractions.Fraction(rat_str(self.profile.GetLiapValue()).c_str())

    property game:
        def __get__(self):
            cdef Game g
            g = Game()
            g.game = self.profile.GetGame()
            return g
