#
# This file is part of Gambit
# Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
#
# FILE: src/pygambit/gameiter.py
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


class Contingencies:
    """
    An object representing the contingencies of strategies in a strategic game.
    Contingencies may be restricted to a single strategy for one or more players via
    repeated calls to __getitem__, each specifying a (player, strategy label) pair.
    """
    def __init__(self, game, cont=None):
        self.game = game
        self.cont = cont if cont is not None else {}

    def __getitem__(self, key):
        player, strategy = key
        cont = dict(self.cont)
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
            yield {player.label: self.cont[player] for player in self.game.players}
        else:
            players = list(self.game.players)
            nextpl = min(pl for (pl, player) in enumerate(players)
                         if player not in self.cont)
            for strategy in players[nextpl].strategies:
                yield from self[players[nextpl], strategy]
