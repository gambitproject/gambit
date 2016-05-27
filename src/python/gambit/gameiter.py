#
# This file is part of Gambit
# Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/gameiter.py
# Iteration tools over games in pure Python
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
"""
Iterator tools over games in pure Python.
"""

class Contingencies(object):
    """
    An object representing the contingencies of strategies in a strategic game.
    Contingencies may be restricted by specifying the strategies of any number
    of players via calls to __getitem__.
    """
    def __init__(self, game, cont={}):
        self.game = game
        self.cont = cont

    def __getitem__(self, strategy):
        cont = dict(self.cont)
        player = [ p for p in self.game.players if strategy in p.strategies ][0]
        cont[player] = strategy
        return Contingencies(self.game, cont)

    def __len__(self):
        ncont = 1
        for player in self.game.players:
            if player not in self.cont:
                ncont *= len(player.strategies)
        return ncont

    def __iter__(self):
        if len(self.cont) == len(self.game.players):
            yield [ list(player.strategies).index(self.cont[player])
                    for player in self.game.players ]
            raise StopIteration
        nextpl = min([ pl for (pl, player) in enumerate(self.game.players)
                       if player not in self.cont ])
        for (st, strategy) in enumerate(self.game.players[nextpl].strategies):
            for cont in self[strategy]:
                yield cont
    
