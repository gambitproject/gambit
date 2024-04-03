#
# This file is part of Gambit
# Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/pygambit/nash.pxi
# Cython wrapper for Nash equilibrium computations
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
import cython
from libcpp.memory cimport shared_ptr, make_shared
from cython.operator cimport dereference as deref


import typing


@cython.cfunc
def _convert_mspd(
        inlist: c_List[c_MixedStrategyProfileDouble]
) -> typing.List[MixedStrategyProfileDouble]:
    ret = []
    for i in range(inlist.Length()):
        p = MixedStrategyProfileDouble()
        p.profile = copyitem_list_mspd(inlist, i+1)
        ret.append(p)
    return ret


@cython.cfunc
def _convert_mspr(
        inlist: c_List[c_MixedStrategyProfileRational]
) -> typing.List[MixedStrategyProfileRational]:
    ret = []
    for i in range(inlist.Length()):
        p = MixedStrategyProfileRational()
        p.profile = copyitem_list_mspr(inlist, i+1)
        ret.append(p)
    return ret


@cython.cfunc
def _convert_mbpd(
        inlist: c_List[c_MixedBehaviorProfileDouble]
) -> typing.List[MixedBehaviorProfileDouble]:
    ret = []
    for i in range(inlist.Length()):
        p = MixedBehaviorProfileDouble()
        p.profile = copyitem_list_mbpd(inlist, i+1)
        ret.append(p)
    return ret


@cython.cfunc
def _convert_mbpr(
        inlist: c_List[c_MixedBehaviorProfileRational]
) -> typing.List[MixedBehaviorProfileRational]:
    ret = []
    for i in range(inlist.Length()):
        p = MixedBehaviorProfileRational()
        p.profile = copyitem_list_mbpr(inlist, i+1)
        ret.append(p)
    return ret


def _enumpure_strategy_solve(game: Game) -> typing.List[MixedStrategyProfileRational]:
    return _convert_mspr(EnumPureStrategySolve(game.game))


def _enumpure_agent_solve(game: Game) -> typing.List[MixedBehaviorProfileRational]:
    return _convert_mbpr(EnumPureAgentSolve(game.game))


def _enummixed_strategy_solve_double(game: Game) -> typing.List[MixedStrategyProfileDouble]:
    return _convert_mspd(EnumMixedStrategySolveDouble(game.game))


def _enummixed_strategy_solve_rational(game: Game) -> typing.List[MixedStrategyProfileRational]:
    return _convert_mspr(EnumMixedStrategySolveRational(game.game))


def _enummixed_strategy_solve_lrs(game: Game) -> typing.List[MixedStrategyProfileRational]:
    return _convert_mspr(EnumMixedStrategySolveLrs(game.game))


def _lcp_behavior_solve_double(
        game: Game, stop_after: int, max_depth: int
) -> typing.List[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LcpBehaviorSolveDouble(game.game, stop_after, max_depth))


def _lcp_behavior_solve_rational(
        game: Game, stop_after: int, max_depth: int
) -> typing.List[MixedBehaviorProfileRational]:
    return _convert_mbpr(LcpBehaviorSolveRational(game.game, stop_after, max_depth))


def _lcp_strategy_solve_double(
        game: Game, stop_after: int, max_depth: int
) -> typing.List[MixedStrategyProfileDouble]:
    return _convert_mspd(LcpStrategySolveDouble(game.game, stop_after, max_depth))


def _lcp_strategy_solve_rational(
        game: Game, stop_after: int, max_depth: int
) -> typing.List[MixedStrategyProfileRational]:
    return _convert_mspr(LcpStrategySolveRational(game.game, stop_after, max_depth))


def _lp_behavior_solve_double(game: Game) -> typing.List[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LpBehaviorSolveDouble(game.game))


def _lp_behavior_solve_rational(game: Game) -> typing.List[MixedBehaviorProfileRational]:
    return _convert_mbpr(LpBehaviorSolveRational(game.game))


def _lp_strategy_solve_double(game: Game) -> typing.List[MixedStrategyProfileDouble]:
    return _convert_mspd(LpStrategySolveDouble(game.game))


def _lp_strategy_solve_rational(game: Game) -> typing.List[MixedStrategyProfileRational]:
    return _convert_mspr(LpStrategySolveRational(game.game))


def _liap_strategy_solve(start: MixedStrategyProfileDouble,
                         maxregret: float,
                         maxiter: int) -> typing.List[MixedStrategyProfileDouble]:
    return _convert_mspd(LiapStrategySolve(deref(start.profile), maxregret, maxiter))


def _liap_behavior_solve(start: MixedBehaviorProfileDouble,
                         maxregret: float,
                         maxiter: int) -> typing.List[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LiapBehaviorSolve(deref(start.profile), maxregret, maxiter))


def _simpdiv_strategy_solve(
        start: MixedStrategyProfileRational, maxregret: Rational, gridstep: int, leash: int
) -> typing.List[MixedStrategyProfileRational]:
    return _convert_mspr(SimpdivStrategySolve(deref(start.profile),
                                              to_rational(str(maxregret).encode("ascii")),
                                              gridstep, leash))


def _ipa_strategy_solve(
        pert: MixedStrategyProfileDouble
) -> typing.List[MixedStrategyProfileDouble]:
    try:
        return _convert_mspd(IPAStrategySolve(deref(pert.profile)))
    except RuntimeError as e:
        if "does not have unique maximizer" in str(e):
            raise ValueError(str(e)) from None
        raise


def _gnm_strategy_solve(
        pert: MixedStrategyProfileDouble,
        end_lambda: float,
        steps: int,
        local_newton_interval: int,
        local_newton_maxits: int,
) -> typing.List[MixedStrategyProfileDouble]:
    try:
        return _convert_mspd(GNMStrategySolve(deref(pert.profile), end_lambda,
                                              steps, local_newton_interval, local_newton_maxits))
    except RuntimeError as e:
        if "does not have unique maximizer" in str(e):
            raise ValueError(str(e)) from None
        raise


def _logit_strategy_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
) -> typing.List[MixedStrategyProfileDouble]:
    return _convert_mspd(LogitStrategySolve(game.game, maxregret, first_step, max_accel))


def _logit_behavior_solve(
        game: Game, maxregret: float, first_step: float, max_accel: float,
) -> typing.List[MixedBehaviorProfileDouble]:
    return _convert_mbpd(LogitBehaviorSolve(game.game, maxregret, first_step, max_accel))


@cython.cclass
class LogitQREMixedStrategyProfile:
    thisptr = cython.declare(shared_ptr[c_LogitQREMixedStrategyProfile])

    def __init__(self, game=None):
        if game is not None:
            self.thisptr = make_shared[c_LogitQREMixedStrategyProfile](
                cython.cast(Game, game).game
            )

    def __repr__(self):
        return "LogitQREMixedStrategyProfile(lam=%f,profile=%s)" % (self.lam, self.profile)

    def __len__(self):
        return deref(self.thisptr).MixedProfileLength()

    def __getitem__(self, int i):
        return deref(self.thisptr).getitem(i+1)

    @property
    def game(self) -> Game:
        """The game on which this mixed strategy profile is defined."""
        g = Game()
        g.game = deref(self.thisptr).GetGame()
        return g

    @property
    def lam(self) -> double:
        """The value of the precision parameter."""
        return deref(self.thisptr).GetLambda()

    @property
    def log_like(self) -> double:
        """The log-likelihood of the data."""
        return deref(self.thisptr).GetLogLike()

    @property
    def profile(self) -> MixedStrategyProfileDouble:
        """The mixed strategy profile."""
        profile = MixedStrategyProfileDouble()
        profile.profile = (
            make_shared[c_MixedStrategyProfileDouble](deref(self.thisptr).GetProfile())
        )
        return profile


def logit_estimate(profile: MixedStrategyProfileDouble,
                   first_step: float = .03,
                   max_accel: float = 1.1) -> LogitQREMixedStrategyProfile:
    """Estimate QRE corresponding to mixed strategy profile using
    maximum likelihood along the principal branch.
    """
    ret = LogitQREMixedStrategyProfile()
    ret.thisptr = _logit_estimate(profile.profile, first_step, max_accel)
    return ret


def logit_atlambda(game: Game,
                   lam: float,
                   first_step: float = .03,
                   max_accel: float = 1.1) -> LogitQREMixedStrategyProfile:
    """Compute the first QRE along the principal branch with the given
    lambda parameter.
    """
    ret = LogitQREMixedStrategyProfile()
    ret.thisptr = _logit_atlambda(game.game, lam, first_step, max_accel)
    return ret


def logit_principal_branch(game: Game, first_step: float = .03, max_accel: float = 1.1):
    solns = _logit_principal_branch(game.game, 1.0e-8, first_step, max_accel)
    ret = []
    for i in range(solns.Length()):
        p = LogitQREMixedStrategyProfile()
        p.thisptr = copyitem_list_qrem(solns, i+1)
        ret.append(p)
    return ret
