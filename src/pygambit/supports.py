#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/pygambit/supports.py
# Access to algorithms which compute on supports.
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

import pygambit as gbt
import pygambit.gambit as libgbt


def undominated_strategies_solve(
        profile: gbt.StrategySupportProfile,
        strict: bool = False,
        external: bool = False
) -> gbt.StrategySupportProfile:
    """Return a support profile including only the strategies in `profile` which are not dominated
    by another pure strategy.

    This function performs only one round of elimination.

    Parameters
    ----------
    profile: StrategySupportProfile
        The initial profile of strategies

    strict : bool, default False
        If specified `True`, eliminate only strategies which are strictly dominated.
        If `False`, strategies which are weakly dominated are also eliminated.

    external : bool, default False
        The default is to consider dominance only by strategies which are in
        the support profile for that player.  If `True`, strategies which are dominated
        by another strategy not in the support profile are also eliminated.

    Returns
    -------
    StrategySupportProfile
        A new support profile containing only the strategies which are not dominated.
    """
    return libgbt._undominated_strategies_solve(profile, strict, external)
